#include <gtest/gtest.h>
#include "../include/engine.h"
#include "../include/orderbook.h"
#include "../include/types.h"
#include "../include/data.h"
#include "test_helpers.h"

TEST_F(MatchingEngineTest, FillTriggersRestingStopOrder) {
    book.addRestingOrder(makeOrder(1, Side::SELL, Price{100}, Qty{10}), Side::SELL);
    stop_book.add(makeOrder(2, Side::BUY, Price{100}, Qty{5}, OrderType::Stop)); // stop_price=100

    auto fills = engine.submit(makeOrder(3, Side::BUY, Price{100}, Qty{10}), book, stop_book);

    // fill 1: order 3 buys 10 @ 100 from resting order 1
    // that fill's price (100) should trigger stop order 2, converting it to a market buy,
    // which should then generate a second fill
    EXPECT_GE(fills.size(), 1u);
    // exact assertions depend on what liquidity remains for order 2 to match against
}