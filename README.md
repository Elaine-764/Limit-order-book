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