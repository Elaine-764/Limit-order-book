#include <gtest/gtest.h>
#include "../include/orderbook.h"   
#include "../include/data.h"
#include "../include/types.h"
#include "test_helpers.h"

TEST(StopOrderBookTest, EmptyBookTriggersNothing) {
    StopOrderBook stops;
    auto triggered = stops.triggerUpTo(Price{100});
    EXPECT_TRUE(triggered.empty());
}

TEST(StopOrderBookTest, BuyStopTriggersWhenPriceRisesToItsStopPrice) {
    StopOrderBook stops;
    stops.add(makeOrder(1, Side::BUY, Price{100}, Qty{10}, OrderType::STOP));
    // assuming stop_price is set via the order's price field per your multimap key,
    // or a dedicated stop_price field — adjust makeOrder args to match your Order struct.

    auto triggered = stops.triggerUpTo(Price{100});

    ASSERT_EQ(triggered.size(), 1u);
    EXPECT_EQ(triggered[0].order_id.get(), 1);
}

TEST(StopOrderBookTest, BuyStopDoesNotTriggerBelowItsStopPrice) {
    StopOrderBook stops;
    stops.add(makeOrder(1, Side::BUY, Price{105}, Qty{10}, OrderType::STOP));

    auto triggered = stops.triggerUpTo(Price{100});   // hasn't reached 105 yet

    EXPECT_TRUE(triggered.empty());
}

TEST(StopOrderBookTest, OnlyBuyStopsAtOrBelowLastTradeTrigger) {
    StopOrderBook stops;
    stops.add(makeOrder(1, Side::BUY, Price{100}, Qty{10}, OrderType::STOP));
    stops.add(makeOrder(2, Side::BUY, Price{105}, Qty{10}, OrderType::STOP));

    auto triggered = stops.triggerUpTo(Price{102});   // crosses 100, not 105

    ASSERT_EQ(triggered.size(), 1u);
    EXPECT_EQ(triggered[0].order_id.get(), 1);
}

TEST(StopOrderBookTest, SellStopTriggersWhenPriceFallsToItsStopPrice) {
    StopOrderBook stops;
    stops.add(makeOrder(1, Side::SELL, Price{95}, Qty{10}, OrderType::STOP));

    auto triggered = stops.triggerUpTo(Price{95});

    ASSERT_EQ(triggered.size(), 1u);
    EXPECT_EQ(triggered[0].order_id.get(), 1);
}

TEST(StopOrderBookTest, SellStopDoesNotTriggerAbovePriceItsStopPrice) {
    StopOrderBook stops;
    stops.add(makeOrder(1, Side::SELL, Price{95}, Qty{10}, OrderType::STOP));

    auto triggered = stops.triggerUpTo(Price{100});   // hasn't fallen to 95 yet

    EXPECT_TRUE(triggered.empty());
}

TEST(StopOrderBookTest, OnlySellStopsAtOrAboveLastTradeTrigger) {
    StopOrderBook stops;
    stops.add(makeOrder(1, Side::SELL, Price{100}, Qty{10}, OrderType::STOP));
    stops.add(makeOrder(2, Side::SELL, Price{95}, Qty{10}, OrderType::STOP));

    auto triggered = stops.triggerUpTo(Price{97});   // crosses 100 downward, not 95

    ASSERT_EQ(triggered.size(), 1u);
    EXPECT_EQ(triggered[0].order_id.get(), 1);
}

TEST(StopOrderBookTest, TriggeredOrdersAreConsumedNotReTriggered) {
    StopOrderBook stops;
    stops.add(makeOrder(1, Side::BUY, Price{100}, Qty{10}, OrderType::STOP));

    auto first = stops.triggerUpTo(Price{102});
    ASSERT_EQ(first.size(), 1u);

    auto second = stops.triggerUpTo(Price{200});   // way past the stop again
    EXPECT_TRUE(second.empty());                    // must not fire twice
}

TEST(StopOrderBookTest, BuyAndSellStopsAreIndependent) {
    StopOrderBook stops;
    stops.add(makeOrder(1, Side::BUY, Price{100}, Qty{10}, OrderType::STOP));
    stops.add(makeOrder(2, Side::SELL, Price{95}, Qty{10}, OrderType::STOP));

    auto triggered = stops.triggerUpTo(Price{100});   // triggers the buy stop only

    ASSERT_EQ(triggered.size(), 1u);
    EXPECT_EQ(triggered[0].order_id.get(), 1);
}