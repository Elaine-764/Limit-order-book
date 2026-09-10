#include <gtest/gtest.h>

#include "../include/exchange.h"
#include "test_helpers.h"

TEST(ExchangeCurrentTest, LazilyCreatesBookAndCancelsRestingOrder) {
    Exchange exchange;

    auto submit_reports = exchange.submitOrder(
        makeOrder(1, Side::BUY, Price{100}, Qty{5}, OrderType::LIMIT,
                  TimeInForce::GOOD_TIL_CANCELED, FillPolicy::NORMAL, Price{0}, 7));
    EXPECT_TRUE(submit_reports.empty());

    auto cancel_reports = exchange.cancelOrder(CancelRequest{OrderId{1}, 7});
    ASSERT_EQ(cancel_reports.size(), 1u);
    EXPECT_EQ(cancel_reports[0].type, ExecutionType::CANCELED);
    EXPECT_EQ(cancel_reports[0].quantity_remaining, Qty{5});
}

TEST(ExchangeCurrentTest, MarketOrderWithZeroPriceIsAccepted) {
    Exchange exchange;

    exchange.submitOrder(makeOrder(1, Side::SELL, Price{100}, Qty{4}, OrderType::LIMIT,
                                    TimeInForce::DAY, FillPolicy::NORMAL, Price{0}, 1));
    auto reports = exchange.submitOrder(
        makeOrder(2, Side::BUY, Price{0}, Qty{4}, OrderType::MARKET,
                  TimeInForce::IMMEDIATE_OR_CANCEL, FillPolicy::NORMAL, Price{0}, 1));

    ASSERT_EQ(reports.size(), 1u);
    EXPECT_EQ(reports[0].type, ExecutionType::FILLED);
    ASSERT_EQ(reports[0].fills.size(), 1u);
    EXPECT_EQ(reports[0].fills[0].price, Price{100});
}

TEST(ExchangeCurrentTest, InvalidQuantityAndLimitPriceAreRejected) {
    Exchange exchange;

    auto zero_quantity = exchange.submitOrder(
        makeOrder(1, Side::BUY, Price{100}, Qty{0}));
    auto zero_limit_price = exchange.submitOrder(
        makeOrder(2, Side::BUY, Price{0}, Qty{1}));

    ASSERT_EQ(zero_quantity.size(), 1u);
    ASSERT_EQ(zero_limit_price.size(), 1u);
    EXPECT_EQ(zero_quantity[0].type, ExecutionType::REJECTED);
    EXPECT_EQ(zero_limit_price[0].type, ExecutionType::REJECTED);
}

TEST(ExchangeCurrentTest, QuantityReductionModificationPreservesOrder) {
    Exchange exchange;
    exchange.submitOrder(makeOrder(1, Side::BUY, Price{100}, Qty{10}, OrderType::LIMIT,
                                    TimeInForce::GOOD_TIL_CANCELED));

    auto reports = exchange.modifyOrder(ModifyRequest{OrderId{1}, 1, Price{100}, Qty{6}});

    ASSERT_EQ(reports.size(), 1u);
    EXPECT_EQ(reports[0].type, ExecutionType::MODIFIED);
    EXPECT_EQ(reports[0].quantity_remaining, Qty{6});
}

TEST(ExchangeCurrentTest, EndOfDayCancelsDayOrdersButKeepsGTCOrders) {
    Exchange exchange;
    exchange.submitOrder(makeOrder(1, Side::BUY, Price{100}, Qty{3}, OrderType::LIMIT,
                                    TimeInForce::DAY));
    exchange.submitOrder(makeOrder(2, Side::BUY, Price{99}, Qty{4}, OrderType::LIMIT,
                                    TimeInForce::GOOD_TIL_CANCELED));

    auto eod_reports = exchange.runEndOfDaySession();

    ASSERT_EQ(eod_reports.size(), 1u);
    EXPECT_EQ(eod_reports[0].order_id, OrderId{1});
    EXPECT_EQ(eod_reports[0].type, ExecutionType::CANCELED);
    EXPECT_EQ(eod_reports[0].quantity_remaining, Qty{3});

    auto gtc_cancel = exchange.cancelOrder(CancelRequest{OrderId{2}, 1});
    ASSERT_EQ(gtc_cancel.size(), 1u);
    EXPECT_EQ(gtc_cancel[0].type, ExecutionType::CANCELED);
}

TEST(ExchangeCurrentTest, EndOfDayCancelsDayStopOrders) {
    Exchange exchange;
    exchange.submitOrder(makeOrder(1, Side::BUY, Price{0}, Qty{3}, OrderType::STOP,
                                    TimeInForce::DAY, FillPolicy::NORMAL, Price{100}, 1));

    auto reports = exchange.runEndOfDaySession();

    ASSERT_EQ(reports.size(), 1u);
    EXPECT_EQ(reports[0].order_id, OrderId{1});
    EXPECT_EQ(reports[0].type, ExecutionType::CANCELED);
    EXPECT_EQ(reports[0].quantity_remaining, Qty{3});
}

TEST(ExchangeCurrentTest, UnknownCancelAndModifyAreRejected) {
    Exchange exchange;

    auto cancel_reports = exchange.cancelOrder(CancelRequest{OrderId{99}, 42});
    auto modify_reports = exchange.modifyOrder(
        ModifyRequest{OrderId{99}, 42, Price{100}, Qty{1}});

    ASSERT_EQ(cancel_reports.size(), 1u);
    ASSERT_EQ(modify_reports.size(), 1u);
    EXPECT_EQ(cancel_reports[0].type, ExecutionType::REJECTED);
    EXPECT_EQ(modify_reports[0].type, ExecutionType::REJECTED);
}
