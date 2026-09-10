#include <gtest/gtest.h>

#include "../include/engine.h"
#include "../include/orderbook.h"
#include "test_helpers.h"

TEST(MatchingEngineCurrentTest, LimitOrderMatchesAtPriceTimePriority) {
    MatchingEngine engine;
    OrderBook book;
    StopOrderBook stops;

    book.addRestingOrder(makeOrder(1, Side::SELL, Price{100}, Qty{5}), Side::SELL);
    book.addRestingOrder(makeOrder(2, Side::SELL, Price{100}, Qty{5}), Side::SELL);

    auto reports = engine.submit(
        makeOrder(3, Side::BUY, Price{100}, Qty{7}), book, stops);

    ASSERT_EQ(reports.size(), 1u);
    EXPECT_EQ(reports[0].type, ExecutionType::FILLED);
    ASSERT_EQ(reports[0].fills.size(), 2u);
    EXPECT_EQ(reports[0].fills[0].resting_order_id, OrderId{1});
    EXPECT_EQ(reports[0].fills[0].quantity, Qty{5});
    EXPECT_EQ(reports[0].fills[1].resting_order_id, OrderId{2});
    EXPECT_EQ(reports[0].fills[1].quantity, Qty{2});
    EXPECT_EQ(book.quantityAt(Side::SELL, Price{100}), Qty{3});
    EXPECT_FALSE(book.hasOrder(OrderId{1}));
    EXPECT_TRUE(book.hasOrder(OrderId{2}));
}

TEST(MatchingEngineCurrentTest, MarketOrderSweepsMultipleLevelsAndCancelsRemainder) {
    MatchingEngine engine;
    OrderBook book;
    StopOrderBook stops;

    book.addRestingOrder(makeOrder(10, Side::SELL, Price{100}, Qty{3}), Side::SELL);
    book.addRestingOrder(makeOrder(11, Side::SELL, Price{101}, Qty{4}), Side::SELL);

    auto reports = engine.submit(
        makeOrder(20, Side::BUY, Price{0}, Qty{10}, OrderType::MARKET), book, stops);

    ASSERT_EQ(reports.size(), 2u);
    EXPECT_EQ(reports[0].type, ExecutionType::PARTIALLY_FILLED);
    EXPECT_EQ(reports[0].quantity_remaining, Qty{3});
    EXPECT_EQ(reports[1].type, ExecutionType::CANCELED);
    EXPECT_EQ(reports[1].quantity_remaining, Qty{3});
    EXPECT_TRUE(book.getAsks().empty());
}

TEST(MatchingEngineCurrentTest, IOCLimitCancelsOnlyUnfilledRemainder) {
    MatchingEngine engine;
    OrderBook book;
    StopOrderBook stops;

    book.addRestingOrder(makeOrder(10, Side::SELL, Price{100}, Qty{4}), Side::SELL);

    auto reports = engine.submit(
        makeOrder(20, Side::BUY, Price{100}, Qty{7}, OrderType::LIMIT,
                  TimeInForce::IMMEDIATE_OR_CANCEL),
        book, stops);

    ASSERT_EQ(reports.size(), 2u);
    EXPECT_EQ(reports[0].type, ExecutionType::PARTIALLY_FILLED);
    EXPECT_EQ(reports[0].quantity_remaining, Qty{3});
    EXPECT_EQ(reports[1].type, ExecutionType::CANCELED);
    EXPECT_EQ(reports[1].quantity_remaining, Qty{3});
    EXPECT_TRUE(book.getBids().empty());
}

TEST(MatchingEngineCurrentTest, FOKRejectsWithoutConsumingInsufficientLiquidity) {
    MatchingEngine engine;
    OrderBook book;
    StopOrderBook stops;

    book.addRestingOrder(makeOrder(10, Side::SELL, Price{100}, Qty{4}), Side::SELL);

    auto reports = engine.submit(
        makeOrder(20, Side::BUY, Price{100}, Qty{5}, OrderType::LIMIT,
                  TimeInForce::FILL_OR_KILL),
        book, stops);

    ASSERT_EQ(reports.size(), 1u);
    EXPECT_EQ(reports[0].type, ExecutionType::CANCELED);
    EXPECT_EQ(reports[0].quantity_remaining, Qty{5});
    EXPECT_EQ(book.quantityAt(Side::SELL, Price{100}), Qty{4});
    EXPECT_TRUE(book.hasOrder(OrderId{10}));
}

TEST(MatchingEngineCurrentTest, AllOrNoneLimitRejectsWithoutConsumingLiquidity) {
    MatchingEngine engine;
    OrderBook book;
    StopOrderBook stops;

    book.addRestingOrder(makeOrder(10, Side::SELL, Price{100}, Qty{4}), Side::SELL);

    auto reports = engine.submit(
        makeOrder(20, Side::BUY, Price{100}, Qty{5}, OrderType::LIMIT,
                  TimeInForce::DAY, FillPolicy::ALL_OR_NONE),
        book, stops);

    ASSERT_EQ(reports.size(), 1u);
    EXPECT_EQ(reports[0].type, ExecutionType::CANCELED);
    EXPECT_EQ(book.quantityAt(Side::SELL, Price{100}), Qty{4});
}

TEST(MatchingEngineCurrentTest, UnfilledGTCOrderRestsInBook) {
    MatchingEngine engine;
    OrderBook book;
    StopOrderBook stops;

    auto reports = engine.submit(
        makeOrder(20, Side::BUY, Price{99}, Qty{6}, OrderType::LIMIT,
                  TimeInForce::GOOD_TIL_CANCELED),
        book, stops);

    EXPECT_TRUE(reports.empty());
    EXPECT_TRUE(book.hasOrder(OrderId{20}));
    EXPECT_EQ(book.quantityAt(Side::BUY, Price{99}), Qty{6});
}

TEST(MatchingEngineCurrentTest, BuyStopTriggersAndIsRemovedFromStopBook) {
    MatchingEngine engine;
    OrderBook book;
    StopOrderBook stops;

    book.addRestingOrder(makeOrder(10, Side::SELL, Price{100}, Qty{2}), Side::SELL);
    book.addRestingOrder(makeOrder(11, Side::SELL, Price{105}, Qty{3}), Side::SELL);
    stops.add(makeOrder(20, Side::BUY, Price{0}, Qty{3}, OrderType::STOP,
                        TimeInForce::DAY, FillPolicy::NORMAL, Price{100}));

    auto reports = engine.submit(
        makeOrder(30, Side::BUY, Price{100}, Qty{2}), book, stops);

    ASSERT_EQ(reports.size(), 2u);
    EXPECT_EQ(reports[0].order_id, OrderId{30});
    EXPECT_EQ(reports[1].order_id, OrderId{20});
    EXPECT_EQ(reports[1].type, ExecutionType::FILLED);
    EXPECT_FALSE(stops.contains(OrderId{20}));
}

TEST(MatchingEngineCurrentTest, SellStopsTriggerForPricesAtOrBelowTheirThreshold) {
    StopOrderBook stops;
    stops.add(makeOrder(1, Side::SELL, Price{0}, Qty{1}, OrderType::STOP,
                        TimeInForce::DAY, FillPolicy::NORMAL, Price{100}));
    stops.add(makeOrder(2, Side::SELL, Price{0}, Qty{1}, OrderType::STOP,
                        TimeInForce::DAY, FillPolicy::NORMAL, Price{95}));

    auto triggered = stops.triggerUpTo(Price{97});

    ASSERT_EQ(triggered.size(), 1u);
    EXPECT_EQ(triggered[0].order_id, OrderId{1});
    EXPECT_FALSE(stops.contains(OrderId{1}));
    EXPECT_TRUE(stops.contains(OrderId{2}));
}
