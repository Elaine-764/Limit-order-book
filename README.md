# Limit-order-book

A limit order book (LOB) matching engine with a TCP feed handler and terminal UI, written in C++20.

## Build
```bash
mkdir build && cd build
cmake ..
make
```

## Run
Start the server, then the client in a separate terminal:
```bash
./lob_server   # terminal 1
./lob_client   # terminal 2
```
The client UI supports adding, cancelling, and modifying orders via keyboard commands (A / C / M / Q), as per instructions upon client terminal startup.

## Architecture
```
client (ncurses UI)
      │  binary OrderMessage over TCP
      ▼
server (feed handler)
      │  deserializes → Order
      ▼
Exchange
      │  routes by ticker
      ▼
OrderBook (one per asset)
      │  two std::maps (bids descending, asks ascending)
      ▼
PriceLevel → deque<Order>  (FIFO within price)
```

**Key design decisions**

- Prices stored as integer ticks (e.g. $100.23 → `10023`) — avoids floating point comparison issues and enables future flat array optimization
- Two-map architecture: `std::map<Price, PriceLevel, std::greater>` for bids, `std::less` for asks — `begin()` is always best bid/ask, O(log n) insert/lookup
- `order_index` (`unordered_map<id, {side, price}>`) in each OrderBook enables O(1) cancel without scanning price levels
- `order_id_to_ticker` in Exchange enables cancel-by-id without requiring the client to specify a ticker
- Fixed-size `OrderMessage` struct over TCP — same approach as NASDAQ ITCH, read exactly `sizeof(OrderMessage)` bytes per message
- Matching engine separated from network layer — OrderBook has no knowledge of sockets or serialization

**Order types supported:** limit, market, stop, stop-limit  
**Time-in-force:** GTC, day  
**Fill policies:** normal, FOK, AON

## Potential improvements

- Replace `std::map` with a sorted flat array or skip list for better cache performance
- Pool allocator for Order objects to avoid per-order heap allocation
- Atomic order IDs for thread safety
- Broadcast book snapshots after each match so the client UI reflects live book state

# Tests

## Basic Matching
- buy 10 of AAPL @ 100, sell 10 of AAPL @ 100 → exact fill, both orders gone
- buy 10 of AAPL @ 101, sell 10 of AAPL @ 100 → fill at ask price ($100), aggressor pays passive price
- buy 10 of AAPL @ 100, sell 10 of AAPL @ 101 → no fill, both rest in book
- reverse all of the above (sell first, then buy as aggressor)

## Partial Fills
- buy 10, sell 3 → bid partially filled (7 remaining), ask fully filled
- buy 3, sell 10 → bid fully filled, ask partially filled (7 remaining)
- buy 10, sell 3, sell 3, sell 4 → three separate fills drain the bid completely
- buy 3 @ 100, buy 5 @ 100, sell 10 @ 100 → two bids at same level filled in time order (FIFO)

## Price Priority
- buy 5 @ 101, buy 5 @ 100, sell 10 @ 99 → higher bid (101) fills first, then 100
- sell 5 @ 99, sell 5 @ 100, buy 10 @ 101 → lower ask (99) fills first, then 100

## Time Priority (FIFO within price level)
- buy 5 @ 100 (order A), buy 5 @ 100 (order B), sell 3 @ 100
  → order A fills 3, order B untouched (A was first)
- buy 5 @ 100 (order A), buy 5 @ 100 (order B), sell 8 @ 100
  → order A fills completely (5), order B fills 3

## Cancel
- add order, cancel it → order gone from book, spread/qty updates
- cancel order that doesn't exist → graceful error, no crash
- cancel already-filled order → graceful error
- add 3 orders at same price level, cancel middle one → remaining two still fill correctly

## Modify
- modify price down (bid): order keeps queue position, no fill
- modify price up (bid): order moves to new level, loses queue position
- modify qty decrease: order keeps queue position
- modify qty increase: order loses queue position (cancel + reinsert)
- modify to a price that crosses the spread → should trigger immediate fill
- modify order that doesn't exist → graceful error

## Multiple Price Levels
- bids @ 101, 100, 99 — sell enough to drain 101 completely, partial fill at 100
  → 101 level erased, 100 level partially filled, 99 untouched
- verify spread updates correctly after each fill

## Multiple Tickers
- buy AAPL @ 100, buy MSFT @ 200 — sell AAPL @ 99 → only AAPL matches, MSFT untouched
- orders for AAPL and MSFT at same price → no cross-ticker matching

## Edge Cases
- buy 0 quantity → reject or handle gracefully
- buy at price 0 → reject or handle gracefully  
- add 1000 orders at same price level → all queue correctly, fill in order
- buy exactly matches ask on multiple levels simultaneously
- empty book: cancel, modify, get spread → all handle gracefully without crash
- two clients sending orders simultaneously (if you support multiple connections)

## FOK / AON (if implemented)
- FOK buy 10, book only has 8 available → entire order cancelled, nothing fills
- FOK buy 10, book has exactly 10 → fills completely
- AON buy 10, book has 8 → order rests but does not partially fill
- AON buy 10, book accumulates to 10 → fills completely

## Stop Orders (if implemented)  
- place stop sell @ 95, market trades at 96, 95 → stop triggers at 95
- place stop sell @ 95, market never reaches 95 → stop never triggers
- stop limit: triggers but limit not reachable → rests in book at limit price