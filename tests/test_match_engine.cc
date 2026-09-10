// tests/test_match_engine.cc
#include <gtest/gtest.h>
#include <chrono>
#include <vector>

#include "../include/types.h"
#include "../include/data.h"
#include "../include/orderbook.h"
#include "../include/engine.h"
#include "test_helpers.h"

// 1. Test Aggressive Limit Order Matching
TEST(MatchingEngineTest, LimitOrderExactMatch) {
    MatchingEngine engine;

    // Resting Sell Order: 10 units at $100
    engine.submit(makeOrder(1, Side::SELL, Price{100}, Qty{10}));

    // Incoming Buy Order: 10 units at $100
    Order buy_order = makeOrder(2, Side::BUY, Price{100}, Qty{10});
    auto match_result = engine.submit(buy_order);

    // Assertions
    EXPECT_EQ(match_result.trades.size(), 1);
    EXPECT_EQ(match_result.trades[0].price, Price{100});
    EXPECT_EQ(match_result.trades[0].quantity, Qty{10});
    EXPECT_EQ(match_result.trades[0].maker_id, OrderId{1});
    EXPECT_EQ(match_result.trades[0].taker_id, OrderId{2});
}

// 2. Test Price-Time Priority (Queue Mechanics)
TEST(MatchingEngineTest, PriceTimePriority) {
    MatchingEngine engine;

    // First Resting Buy Order at $100
    engine.submit(makeOrder(1, Side::BUY, Price{100}, Qty{5}));
    
    // Second Resting Buy Order at $100
    engine.submit(makeOrder(2, Side::BUY, Price{100}, Qty{5}));

    // Incoming Sell Order at $100 matches with Order 1 first due to arrival time
    Order sell_order = makeOrder(3, Side::SELL, Price{100}, Qty{5});
    auto result = engine.submit(sell_order);

    ASSERT_EQ(result.trades.size(), 1);
    EXPECT_EQ(result.trades[0].maker_id, OrderId{1});
    EXPECT_EQ(result.trades[0].quantity, Qty{5});
}

// 3. Test Partial Fills
TEST(MatchingEngineTest, PartialOrderFill) {
    MatchingEngine engine;

    // Large resting Buy Order: 50 units at $100
    engine.submit(makeOrder(1, Side::BUY, Price{100}, Qty{50}));

    // Smaller incoming Sell Order: 20 units at $100
    Order sell_order = makeOrder(2, Side::SELL, Price{100}, Qty{20});
    auto result = engine.submit(sell_order);

    ASSERT_EQ(result.trades.size(), 1);
    EXPECT_EQ(result.trades[0].quantity, Qty{20});
    EXPECT_EQ(engine.get_remaining_qty(OrderId{1}), Qty{30});
}

// 4. Test Market Orders crossing the Spread
TEST(MatchingEngineTest, MarketOrderFillsAtBestPrice) {
    MatchingEngine engine;

    // Populate Sell Side Depth
    engine.submit(makeOrder(1, Side::SELL, Price{99}, Qty{10}));  // Best Ask
    engine.submit(makeOrder(2, Side::SELL, Price{100}, Qty{10})); // Deep Ask

    // Incoming Market Buy Order for 15 units
    Order market_buy = makeOrder(3, Side::BUY, Price{0}, Qty{15}, OrderType::MARKET);
    auto result = engine.submit(market_buy);

    // Should generate 2 trades: 10 units at $99, then 5 units at $100
    ASSERT_EQ(result.trades.size(), 2);
    
    EXPECT_EQ(result.trades[0].price, Price{99});
    EXPECT_EQ(result.trades[0].quantity, Qty{10});
    
    EXPECT_EQ(result.trades[1].price, Price{100});
    EXPECT_EQ(result.trades[1].quantity, Qty{5});
}

// 5. Test Order Cancellation
TEST(MatchingEngineTest, CancelledOrderDoesNotMatch) {
    MatchingEngine engine;

    engine.submit(makeOrder(1, Side::BUY, Price{100}, Qty{10}));
    engine.cancel(OrderId{1});

    // Immediate incoming match should fail because Order 1 was removed
    Order sell_order = makeOrder(2, Side::SELL, Price{100}, Qty{10});
    auto result = engine.submit(sell_order);

    EXPECT_TRUE(result.trades.empty());
}

// ============================================================================
// 1. MARKET ORDER EDGE CASES
// ============================================================================

TEST(MatchingEngineEdgeTest, MarketOrderNewTickerOrEmptyBook) {
    MatchingEngine engine;
    OrderBook book;
    StopOrderBook stop_book;

    // Submit a Market Buy Order into an empty book
    Order market_order = makeOrder(1, Side::BUY, Price{0}, Qty{100}, OrderType::MARKET);
    auto reports = engine.submit(std::move(market_order), book, stop_book);

    // Expected behavior: Zero fills, returned immediately with a single CANCELED execution report
    ASSERT_EQ(reports.size(), 1u);
    EXPECT_EQ(reports[0].order_id, OrderId{1});
    EXPECT_EQ(reports[0].type, ExecutionType::CANCELED);
    EXPECT_EQ(reports[0].quantity_affected.get(), 100);
    EXPECT_TRUE(reports[0].fills.empty());
    EXPECT_TRUE(book.getAsks().empty());
}

TEST(MatchingEngineEdgeTest, MarketOrderPartialFillSweepsBookCompletely) {
    MatchingEngine engine;
    OrderBook book;
    StopOrderBook stop_book;

    // Inject resting ask liquidity: 30 total units available
    book.addRestingOrder(makeOrder(10, Side::SELL, Price{100}, Qty{10}), Side::SELL);
    book.addRestingOrder(makeOrder(11, Side::SELL, Price{101}, Qty{20}), Side::SELL);

    // Submit an over-sized Market Buy order for 50 units (Insatiable demand)
    Order market_buy = makeOrder(1, Side::BUY, Price{0}, Qty{50}, OrderType::MARKET);
    auto reports = engine.submit(std::move(market_buy), book, stop_book);

    // Flat loop layout should output:
    // Report 0: PARTIALLY_FILLED report with the matching sweeps (30 units filled)
    // Report 1: CANCELED report because the remaining 20 units are forbidden from resting
    ASSERT_EQ(reports.size(), 2u);

    // 1. Evaluate Fill Report
    EXPECT_EQ(reports[0].order_id, OrderId{1});
    EXPECT_EQ(reports[0].type, ExecutionType::PARTIALLY_FILLED);
    EXPECT_EQ(reports[0].quantity_affected.get(), 20); // 50 - 30 filled = 20 remaining
    ASSERT_EQ(reports[0].fills.size(), 2u);
    EXPECT_EQ(reports[0].fills[0].quantity.get(), 10);
    EXPECT_EQ(reports[0].fills[1].quantity.get(), 20);

    // 2. Evaluate Cancellation Report
    EXPECT_EQ(reports[1].order_id, OrderId{1});
    EXPECT_EQ(reports[1].type, ExecutionType::CANCELED);
    EXPECT_EQ(reports[1].quantity_affected.get(), 20);

    // 3. System Invariant: The Ask book must be completely wiped out and dry!
    EXPECT_TRUE(book.getAsks().empty());
}

// ============================================================================
// 2. LIMIT ORDER EDGE CASES
// ============================================================================

TEST(MatchingEngineEdgeTest, LimitOrderNoFillInsertsIntoBook) {
    MatchingEngine engine;
    OrderBook book;
    StopOrderBook stop_book;

    // Submit a Limit Buy order below the market (No resting asks present)
    Order limit_buy = makeOrder(1, Side::BUY, Price{95}, Qty{10}, OrderType::LIMIT);
    auto reports = engine.submit(std::move(limit_buy), book, stop_book);

    // Expected behavior: No matches occur, no reports returned, order safely rests in the tree
    EXPECT_TRUE(reports.empty());
    
    auto& bids = book.getBids();
    ASSERT_FALSE(bids.empty());
    EXPECT_EQ(bids.begin()->first, Price{95});
    EXPECT_FALSE(bids.begin()->second.isEmpty());
}

TEST(MatchingEngineEdgeTest, LimitOrderPartialFillRestsLeftovers) {
    MatchingEngine engine;
    OrderBook book;
    StopOrderBook stop_book;

    // Inject resting passive sell liquidity: 5 units at $100
    book.addRestingOrder(makeOrder(10, Side::SELL, Price{100}, Qty{5}), Side::SELL);

    // Submit an aggressive Buy Limit order for 15 units at $100
    Order aggressive_buy = makeOrder(1, Side::BUY, Price{100}, Qty{15}, OrderType::LIMIT);
    auto reports = engine.submit(std::move(aggressive_buy), book, stop_book);

    // Expected behavior: 
    // - One PARTIALLY_FILLED report returned for the 5 units filled.
    // - Leftover 10 units are allowed to rest in the bid map (DAY order behavior).
    ASSERT_EQ(reports.size(), 1u);
    EXPECT_EQ(reports[0].type, ExecutionType::PARTIALLY_FILLED);
    EXPECT_EQ(reports[0].quantity_affected.get(), 10); // 10 remaining
    EXPECT_EQ(reports[0].fills.size(), 1u);

    // Check resting state of the leftover volume
    auto& bids = book.getBids();
    ASSERT_FALSE(bids.empty());
    EXPECT_EQ(bids.begin()->first, Price{100});
    EXPECT_EQ(bids.begin()->second.begin()->quantity.get(), 10);
    EXPECT_EQ(bids.begin()->second.begin()->order_id, OrderId{1});
}

// ============================================================================
// 3. STOP AND STOP-LIMIT TRIGGERS & TRANSFORMATION
// ============================================================================

TEST(MatchingEngineEdgeTest, StopOrderTriggersAndConvertsToMarket) {
    MatchingEngine engine;
    OrderBook book;
    StopOrderBook stop_book;

    // 1. Establish resting passive ask liquidity
    book.addRestingOrder(makeOrder(10, Side::SELL, Price{100}, Qty{10}), Side::SELL);
    book.addRestingOrder(makeOrder(11, Side::SELL, Price{105}, Qty{10}), Side::SELL); // Liquidity for converted market order

    // 2. Put a resting Buy Stop order in place to trigger at $100
    // (Note: your makeOrder maps the 'price' arg to both stop_price and price)
    stop_book.add(makeOrder(2, Side::BUY, Price{100}, Qty{5}, OrderType::STOP));

    // 3. Print a trade at $100 to trigger the stop order
    Order trigger_order = makeOrder(3, Side::BUY, Price{100}, Qty{10}, OrderType::LIMIT);
    auto reports = engine.submit(std::move(trigger_order), book, stop_book);

    // Flat processing queue expects sequential execution tracking nodes:
    // Report 0: Taker order 3 filled completely at $100 against order 10
    // Report 1: Stop order 2 transformed to MARKET buy, triggered, walked up, and filled at $105 against order 11
    ASSERT_EQ(reports.size(), 2u);

    EXPECT_EQ(reports[0].order_id, OrderId{3});
    EXPECT_EQ(reports[0].type, ExecutionType::FILLED);
    EXPECT_EQ(reports[0].fills[0].price, Price{100});

    EXPECT_EQ(reports[1].order_id, OrderId{2});
    EXPECT_EQ(reports[1].type, ExecutionType::FILLED);
    ASSERT_FALSE(reports[1].fills.empty());
    EXPECT_EQ(reports[1].fills[0].price, Price{105}); // Verified conversion to MARKET sweep mechanics
}

TEST(MatchingEngineEdgeTest, StopLimitOrderTriggersAndConvertsToLimit) {
    MatchingEngine engine;
    OrderBook book;
    StopOrderBook stop_book;

    // 1. Establish initial trade liquidity tier
    book.addRestingOrder(makeOrder(10, Side::SELL, Price{100}, Qty{10}), Side::SELL);

    // 2. Add Buy Stop Limit order: trigger at $100, limit price constraint $102
    Order stop_limit = makeOrder(2, Side::BUY, Price{102}, Qty{5}, OrderType::STOP_LIMIT, TimeInForce::DAY, FillPolicy::NORMAL, Price{100});
    stop_book.add(std::move(stop_limit));

    // 3. Print a trade at $100 to wake it up
    Order trigger_order = makeOrder(3, Side::BUY, Price{100}, Qty{10}, OrderType::LIMIT);
    auto reports = engine.submit(std::move(trigger_order), book, stop_book);

    // Report 0 handles the trigger match pass
    ASSERT_EQ(reports.size(), 1u);
    EXPECT_EQ(reports[0].order_id, OrderId{3});

    // Invariant Verification: Stop-Limit order 2 triggered, mutated to LIMIT, 
    // but because there are no asks <= $102, it must now sit resting in the bid book.
    auto& bids = book.getBids();
    ASSERT_FALSE(bids.empty());
    EXPECT_EQ(bids.begin()->first, Price{102}); // Verified conversion to LIMIT instead of MARKET
    EXPECT_EQ(bids.begin()->second.begin()->order_id, OrderId{2});
}

// ============================================================================
// 4. COMPLEX ARCHITECTURAL INTEGRATION & RISK BOUNDARIES
// ============================================================================

TEST(MatchingEngineEdgeTest, FillOrKillPreCheckRejection) {
    MatchingEngine engine;
    OrderBook book;
    StopOrderBook stop_book;

    // Inject insufficient liquidity: 5 units available at $100
    book.addRestingOrder(makeOrder(10, Side::SELL, Price{100}, Qty{5}), Side::SELL);

    // Submit an FOK Limit Buy order for 10 units
    Order fok_order = makeOrder(1, Side::BUY, Price{100}, Qty{10}, OrderType::LIMIT, TimeInForce::FILL_OR_KILL);
    auto reports = engine.submit(std::move(fok_order), book, stop_book);

    // Expected behavior: Blocked on entry, zero matches, instantly returns a CANCELED notification report
    ASSERT_EQ(reports.size(), 1u);
    EXPECT_EQ(reports[0].order_id, OrderId{1});
    EXPECT_EQ(reports[0].type, ExecutionType::CANCELED);
    EXPECT_EQ(reports[0].quantity_affected.get(), 10);
    EXPECT_TRUE(reports[0].fills.empty());

    // Passive liquidity must be completely untouched and un-mutated
    ASSERT_FALSE(book.getAsks().empty());
    EXPECT_EQ(book.getAsks().begin()->second.begin()->quantity.get(), 5);
}

TEST(MatchingEngineEdgeTest, MarketAllOrNonePreCheckRejection) {
    MatchingEngine engine;
    OrderBook book;
    StopOrderBook stop_book;

    // Empty book pool
    Order aon_market = makeOrder(1, Side::BUY, Price{0}, Qty{50}, OrderType::MARKET, TimeInForce::DAY, FillPolicy::ALL_OR_NONE);
    auto reports = engine.submit(std::move(aon_market), book, stop_book);

    // Your custom top-level guard block condition handles this:
    ASSERT_EQ(reports.size(), 1u);
    EXPECT_EQ(reports[0].type, ExecutionType::CANCELED);
    EXPECT_EQ(reports[0].quantity_affected.get(), 50);
}