// tests/test_exchange.cc
//
// DESIGN RULE (as of this revision): submitOrder() returns AT MOST TWO reports for
// the incoming order itself — a trade report (FILLED/PARTIALLY_FILLED, carrying every
// matched resting order as a Fill in .fills) and/or a remainder-disposition report
// (NEW if it rests, CANCELED if dropped). Fully filled -> 1 report. No fill -> 1 report.
// Partial fill -> 2 reports. Stop triggers add further reports (TRIGGERED + the
// triggered order's own report(s), reprocessed through the same rule).
//
// ASSUMPTIONS ABOUT YOUR API — confirm/rename to match reality:
//   ExecutionType: NEW, FILLED, PARTIALLY_FILLED, CANCELED, DORMANT, REJECTED, TRIGGERED, EXPIRED
//   Fill: resting_order_id, incoming_order_id, price, quantity, resting_remaining_qty, timestamp
//   Exchange::bestBid(ticker)/bestAsk(ticker)          -> std::optional<Price>
//   Exchange::getQty(ticker, Side, Price)              -> Qty
//   Exchange::getFirstBuyStop(ticker)/getFirstSellStop(ticker) -> std::optional<Order>
//   Exchange::orderExists(ticker, OrderId)             -> bool
//   Exchange::bidLevelCount(ticker)/askLevelCount(...) -> std::size_t
//   Exchange::cancelOrder(CancelRequest)
//   Exchange::runEndOfDaySession()                     -> std::vector<ExecutionReport>

#include <gtest/gtest.h>

#include "../include/exchange.h"
#include "test_helpers.h"

// ============================================================
// 1. Basic single-order behavior (empty book)
// ============================================================

TEST(ExchangeTest, BuyLimitOnEmpty) {
    Exchange exchange;
    auto reports = exchange.submitOrder(
        makeOrder(1, Side::BUY, Price{100}, Qty{5}, OrderType::LIMIT, TimeInForce::NONE,
                  FillPolicy::NORMAL, Price{0}, 7)
    );

    ASSERT_EQ(reports.size(), 1u);
    ASSERT_EQ(reports[0].type, ExecutionType::NEW);   // no fill -> single remainder-disposition report
    ASSERT_EQ(exchange.bestBid(7), Price{100});
}

TEST(ExchangeTest, SellLimitOnEmpty) {
    Exchange exchange;
    auto reports = exchange.submitOrder(
        makeOrder(1, Side::SELL, Price{100}, Qty{5}, OrderType::LIMIT, TimeInForce::NONE,
                  FillPolicy::NORMAL, Price{0}, 7)
    );

    ASSERT_EQ(reports.size(), 1u);
    ASSERT_EQ(reports[0].type, ExecutionType::NEW);
    ASSERT_EQ(exchange.bestAsk(7), Price{100});
}

TEST(ExchangeTest, BuyMarketOnEmpty) {
    Exchange exchange;
    auto reports = exchange.submitOrder(
        makeOrder(1, Side::BUY, Price{0}, Qty{5}, OrderType::MARKET, TimeInForce::NONE,
                  FillPolicy::NORMAL, Price{0}, 7)
    );

    ASSERT_EQ(reports.size(), 1u);
    ASSERT_EQ(reports[0].type, ExecutionType::CANCELED);   // no fill, dropped, not rested
    ASSERT_FALSE(exchange.bestBid(7).has_value());
    ASSERT_FALSE(exchange.bestAsk(7).has_value());
}

TEST(ExchangeTest, SellMarketOnEmpty) {
    Exchange exchange;
    auto reports = exchange.submitOrder(
        makeOrder(1, Side::SELL, Price{0}, Qty{5}, OrderType::MARKET, TimeInForce::NONE,
                  FillPolicy::NORMAL, Price{0}, 7)
    );

    ASSERT_EQ(reports.size(), 1u);
    ASSERT_EQ(reports[0].type, ExecutionType::CANCELED);
    ASSERT_EQ(exchange.bestBid(7), std::nullopt);
    ASSERT_EQ(exchange.bestAsk(7), std::nullopt);
}

TEST(ExchangeTest, BuyStopOnEmpty) {
    Exchange exchange;
    auto reports = exchange.submitOrder(
        makeOrder(1, Side::BUY, Price{0}, Qty{5}, OrderType::STOP, TimeInForce::NONE,
                  FillPolicy::NORMAL, Price{100}, 7)
    );

    ASSERT_EQ(reports.size(), 1u);
    ASSERT_EQ(reports[0].type, ExecutionType::DORMANT);
    ASSERT_EQ(exchange.bestBid(7), std::nullopt);
    ASSERT_EQ(exchange.bestAsk(7), std::nullopt);

    auto topBuyStop = exchange.getFirstBuyStop(7);
    ASSERT_TRUE(topBuyStop.has_value());
    ASSERT_EQ(topBuyStop->order_id.get(), 1);
    ASSERT_EQ(topBuyStop->stop_price.get(), 100);
}

TEST(ExchangeTest, SellStopOnEmpty) {
    Exchange exchange;
    auto reports = exchange.submitOrder(
        makeOrder(1, Side::SELL, Price{0}, Qty{5}, OrderType::STOP, TimeInForce::NONE,
                  FillPolicy::NORMAL, Price{95}, 7)
    );

    ASSERT_EQ(reports.size(), 1u);
    ASSERT_EQ(reports[0].type, ExecutionType::DORMANT);
    ASSERT_EQ(exchange.bestBid(7), std::nullopt);
    ASSERT_EQ(exchange.bestAsk(7), std::nullopt);

    auto topSellStop = exchange.getFirstSellStop(7);
    ASSERT_TRUE(topSellStop.has_value());
    ASSERT_EQ(topSellStop->order_id.get(), 1);
    ASSERT_EQ(topSellStop->stop_price.get(), 95);
}

TEST(ExchangeTest, BuyStopLimitOnEmpty) {
    Exchange exchange;
    auto reports = exchange.submitOrder(
        makeOrder(1, Side::BUY, Price{102}, Qty{5}, OrderType::STOP_LIMIT, TimeInForce::NONE,
                  FillPolicy::NORMAL, Price{100}, 7)
    );

    ASSERT_EQ(reports.size(), 1u);
    ASSERT_EQ(reports[0].type, ExecutionType::DORMANT);
    ASSERT_EQ(exchange.bestBid(7), std::nullopt);

    auto topBuyStop = exchange.getFirstBuyStop(7);
    ASSERT_TRUE(topBuyStop.has_value());
    ASSERT_EQ(topBuyStop->price.get(), 102);
    ASSERT_EQ(topBuyStop->stop_price.get(), 100);
}

TEST(ExchangeTest, SellStopLimitOnEmpty) {
    Exchange exchange;
    auto reports = exchange.submitOrder(
        makeOrder(1, Side::SELL, Price{93}, Qty{5}, OrderType::STOP_LIMIT, TimeInForce::NONE,
                  FillPolicy::NORMAL, Price{95}, 7)
    );

    ASSERT_EQ(reports.size(), 1u);
    ASSERT_EQ(reports[0].type, ExecutionType::DORMANT);
    ASSERT_EQ(exchange.bestAsk(7), std::nullopt);

    auto topSellStop = exchange.getFirstSellStop(7);
    ASSERT_TRUE(topSellStop.has_value());
    ASSERT_EQ(topSellStop->price.get(), 93);
    ASSERT_EQ(topSellStop->stop_price.get(), 95);
}

TEST(ExchangeTest, CancelOnEmpty) {
    Exchange exchange;
    auto reports = exchange.cancelOrder(CancelRequest{OrderId{1}, 7});

    ASSERT_EQ(reports.size(), 1u);
    ASSERT_EQ(reports[0].type, ExecutionType::REJECTED);
}

// ============================================================
// 2. Two orders, opposite sides, crossing
// ============================================================

// Reference test for the "one report, many fills" design.
TEST(ExchangeTest, TwoLimitsExactCross) {
    Exchange exchange;
    Order o1 = makeOrder(1, Side::SELL, Price{100}, Qty{10}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1);
    Order o2 = makeOrder(2, Side::BUY,  Price{100}, Qty{10}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1);

    exchange.submitOrder(o1);
    auto reports2 = exchange.submitOrder(o2);

    ASSERT_EQ(reports2.size(), 1u);
    ASSERT_EQ(reports2[0].type, ExecutionType::FILLED);
    ASSERT_EQ(reports2[0].fills.size(), 1u);

    ASSERT_EQ(reports2[0].fills[0].resting_order_id.get(), 1);
    ASSERT_EQ(reports2[0].fills[0].incoming_order_id.get(), 2);
    ASSERT_EQ(reports2[0].fills[0].price.get(), 100);
    ASSERT_EQ(reports2[0].fills[0].resting_remaining_qty.get(), 0);

    ASSERT_EQ(exchange.bestBid(1), std::nullopt);
    ASSERT_EQ(exchange.bestAsk(1), std::nullopt);
}

TEST(ExchangeTest, TwoLimitsCrossingFillsAtRestingPrice) {
    Exchange exchange;
    Order o1 = makeOrder(1, Side::SELL, Price{100}, Qty{10}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1);
    Order o2 = makeOrder(2, Side::BUY,  Price{101}, Qty{10}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1);

    exchange.submitOrder(o1);
    auto reports2 = exchange.submitOrder(o2);

    ASSERT_EQ(reports2.size(), 1u);   // fully filled -> single report
    ASSERT_EQ(reports2[0].type, ExecutionType::FILLED);
    ASSERT_EQ(reports2[0].fills.size(), 1u);
    ASSERT_EQ(reports2[0].fills[0].price.get(), 100);   // fill at resting price, not incoming's 101

    ASSERT_FALSE(exchange.bestBid(1).has_value());
    ASSERT_FALSE(exchange.bestAsk(1).has_value());
}

TEST(ExchangeTest, TwoLimitsMiss) {
    Exchange exchange;
    Order o1 = makeOrder(1, Side::SELL, Price{100}, Qty{10}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1);
    Order o2 = makeOrder(2, Side::BUY,  Price{99},  Qty{10}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1);

    exchange.submitOrder(o1);
    exchange.submitOrder(o2);

    ASSERT_EQ(exchange.bestAsk(1), Price{100});
    ASSERT_EQ(exchange.bestBid(1), Price{99});
}

TEST(ExchangeTest, LimitAndMarketFullFill) {
    Exchange exchange;
    Order o1 = makeOrder(1, Side::SELL, Price{100}, Qty{10}, OrderType::LIMIT,  TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1);
    Order o2 = makeOrder(2, Side::BUY,  Price{0},   Qty{10}, OrderType::MARKET, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1);

    exchange.submitOrder(o1);
    auto reports2 = exchange.submitOrder(o2);

    // CHANGED: was asserting 2 reports, one per side. The aggressor gets exactly
    // one report; the resting order's outcome lives in that report's .fills.
    ASSERT_EQ(reports2.size(), 1u);
    ASSERT_EQ(reports2[0].type, ExecutionType::FILLED);
    ASSERT_EQ(reports2[0].fills.size(), 1u);
    ASSERT_EQ(reports2[0].fills[0].resting_order_id.get(), 1);
    ASSERT_EQ(reports2[0].fills[0].resting_remaining_qty.get(), 0);
    ASSERT_EQ(exchange.bestAsk(1), std::nullopt);
}

TEST(ExchangeTest, LimitAndMarketPartialFillOnAggressor) {
    Exchange exchange;
    Order o1 = makeOrder(1, Side::SELL, Price{100}, Qty{5},  OrderType::LIMIT,  TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1);
    Order o2 = makeOrder(2, Side::BUY,  Price{0},   Qty{10}, OrderType::MARKET, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1);

    exchange.submitOrder(o1);
    auto reports2 = exchange.submitOrder(o2);

    // CHANGED: partial fill -> exactly the two-report case (trade, then remainder disposition).
    ASSERT_EQ(reports2.size(), 2u);

    ASSERT_EQ(reports2[0].type, ExecutionType::PARTIALLY_FILLED);
    ASSERT_EQ(reports2[0].fills.size(), 1u);
    ASSERT_EQ(reports2[0].fills[0].resting_order_id.get(), 1);
    ASSERT_EQ(reports2[0].fills[0].resting_remaining_qty.get(), 0);   // o1 fully consumed

    ASSERT_EQ(reports2[1].type, ExecutionType::CANCELED);   // remaining 5 of the market order, dropped
    ASSERT_TRUE(reports2[1].fills.empty());

    ASSERT_EQ(exchange.bestBid(1), std::nullopt);   // must not have rested
}

// ============================================================
// 3. Partial fills and remainder handling
// ============================================================

TEST(ExchangeTest, PartialFillLeavesCorrectRestingQty) {
    Exchange exchange;
    Order o1 = makeOrder(1, Side::SELL, Price{100}, Qty{10}, OrderType::LIMIT,  TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1);
    Order o2 = makeOrder(2, Side::BUY,  Price{0},   Qty{4},  OrderType::MARKET, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1);

    exchange.submitOrder(o1);
    auto reports2 = exchange.submitOrder(o2);

    // The INCOMING order (qty 4) is fully filled — one report — even though the
    // RESTING order (o1) is only partially consumed. That asymmetry is exactly
    // what .fills[].resting_remaining_qty exists to capture.
    ASSERT_EQ(reports2.size(), 1u);
    ASSERT_EQ(reports2[0].type, ExecutionType::FILLED);
    ASSERT_EQ(reports2[0].fills.size(), 1u);
    ASSERT_EQ(reports2[0].fills[0].resting_remaining_qty.get(), 6);

    ASSERT_EQ(exchange.getQty(1, Side::SELL, Price{100}).get(), 6);
}

TEST(ExchangeTest, RemainderRestsAtIncomingOrdersOwnLimitPrice) {
    Exchange exchange;
    Order o1 = makeOrder(1, Side::SELL, Price{100}, Qty{4},  OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1);
    Order o2 = makeOrder(2, Side::BUY,  Price{101}, Qty{10}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1);

    exchange.submitOrder(o1);
    auto reports2 = exchange.submitOrder(o2);

    // Partial fill (4 of 10) + rests remainder -> the two-report case again,
    // but the second report is NEW (resting) rather than CANCELED.
    ASSERT_EQ(reports2.size(), 2u);
    ASSERT_EQ(reports2[0].type, ExecutionType::PARTIALLY_FILLED);
    ASSERT_EQ(reports2[0].fills.size(), 1u);
    ASSERT_EQ(reports2[0].fills[0].price.get(), 100);

    ASSERT_EQ(reports2[1].type, ExecutionType::NEW);
    ASSERT_TRUE(reports2[1].fills.empty());

    ASSERT_EQ(exchange.getQty(1, Side::BUY, Price{101}).get(), 6);
    ASSERT_EQ(exchange.bestBid(1), Price{101});
    ASSERT_EQ(exchange.bestAsk(1), std::nullopt);
}

// ============================================================
// 4. Price-time priority (FIFO) within a level
// ============================================================

TEST(ExchangeTest, FifoFillsOldestFirst) {
    Exchange exchange;
    Order a = makeOrder(1, Side::SELL, Price{100}, Qty{4},  OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1);
    Order b = makeOrder(2, Side::SELL, Price{100}, Qty{10}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1);
    exchange.submitOrder(a);
    exchange.submitOrder(b);

    Order c = makeOrder(3, Side::BUY, Price{0}, Qty{6}, OrderType::MARKET, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1);
    auto reports = exchange.submitOrder(c);

    // one report, TWO fills (against a, then b), in that order
    ASSERT_EQ(reports.size(), 1u);
    ASSERT_EQ(reports[0].fills.size(), 2u);
    ASSERT_EQ(reports[0].fills[0].resting_order_id.get(), 1);
    ASSERT_EQ(reports[0].fills[0].resting_remaining_qty.get(), 0);
    ASSERT_EQ(reports[0].fills[1].resting_order_id.get(), 2);
    ASSERT_EQ(reports[0].fills[1].resting_remaining_qty.get(), 8);   // 10 - 2

    ASSERT_FALSE(exchange.orderExists(1, OrderId{1}));
    ASSERT_TRUE(exchange.orderExists(1, OrderId{2}));
    ASSERT_EQ(exchange.getQty(1, Side::SELL, Price{100}).get(), 8);
}

TEST(ExchangeTest, CancelMiddleOrderPreservesFifoOfSurvivors) {
    Exchange exchange;
    Order a = makeOrder(1, Side::SELL, Price{100}, Qty{4}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1);
    Order b = makeOrder(2, Side::SELL, Price{100}, Qty{4}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1);
    Order c = makeOrder(3, Side::SELL, Price{100}, Qty{4}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1);
    exchange.submitOrder(a);
    exchange.submitOrder(b);
    exchange.submitOrder(c);

    exchange.cancelOrder(CancelRequest{OrderId{2}, 1});

    Order incoming = makeOrder(4, Side::BUY, Price{0}, Qty{4}, OrderType::MARKET, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1);
    auto reports = exchange.submitOrder(incoming);

    ASSERT_EQ(reports.size(), 1u);
    ASSERT_EQ(reports[0].fills.size(), 1u);
    ASSERT_EQ(reports[0].fills[0].resting_order_id.get(), 1);   // only a — b is gone, c untouched

    ASSERT_FALSE(exchange.orderExists(1, OrderId{1}));
    ASSERT_TRUE(exchange.orderExists(1, OrderId{3}));
    ASSERT_EQ(exchange.getQty(1, Side::SELL, Price{100}).get(), 4);
}

// ============================================================
// 5. Sweeping multiple price levels
// ============================================================

TEST(ExchangeTest, SweepMultipleLevelsPartial) {
    Exchange exchange;
    exchange.submitOrder(makeOrder(1, Side::SELL, Price{100}, Qty{5}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));
    exchange.submitOrder(makeOrder(2, Side::SELL, Price{101}, Qty{5}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));
    exchange.submitOrder(makeOrder(3, Side::SELL, Price{102}, Qty{5}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));

    auto reports = exchange.submitOrder(makeOrder(4, Side::BUY, Price{0}, Qty{12}, OrderType::MARKET, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));

    // qty 12 fully fillable across 3 levels -> single report, THREE fills
    ASSERT_EQ(reports.size(), 1u);
    ASSERT_EQ(reports[0].type, ExecutionType::FILLED);
    ASSERT_EQ(reports[0].fills.size(), 3u);
    ASSERT_EQ(reports[0].fills[2].resting_remaining_qty.get(), 3);   // 5 - 2 left on the 102 level

    ASSERT_EQ(exchange.bestAsk(1), Price{102});
    ASSERT_EQ(exchange.getQty(1, Side::SELL, Price{102}).get(), 3);
}

TEST(ExchangeTest, SweepExactlyExhaustsAllLevels) {
    Exchange exchange;
    exchange.submitOrder(makeOrder(1, Side::SELL, Price{100}, Qty{5}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));
    exchange.submitOrder(makeOrder(2, Side::SELL, Price{101}, Qty{5}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));
    exchange.submitOrder(makeOrder(3, Side::SELL, Price{102}, Qty{5}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));

    auto reports = exchange.submitOrder(makeOrder(4, Side::BUY, Price{0}, Qty{15}, OrderType::MARKET, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));

    ASSERT_EQ(reports.size(), 1u);
    ASSERT_EQ(reports[0].type, ExecutionType::FILLED);
    ASSERT_EQ(reports[0].fills.size(), 3u);

    ASSERT_EQ(exchange.bestAsk(1), std::nullopt);
    ASSERT_EQ(exchange.askLevelCount(1), 0u);
}

TEST(ExchangeTest, MarketOrderExceedingDepthDropsRemainder) {
    Exchange exchange;
    exchange.submitOrder(makeOrder(1, Side::SELL, Price{100}, Qty{5}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));
    exchange.submitOrder(makeOrder(2, Side::SELL, Price{101}, Qty{5}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));

    auto reports = exchange.submitOrder(makeOrder(3, Side::BUY, Price{0}, Qty{20}, OrderType::MARKET, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));

    // qty 20 vs depth 10 -> partial fill + cancel remainder -> two-report case
    ASSERT_EQ(reports.size(), 2u);
    ASSERT_EQ(reports[0].type, ExecutionType::PARTIALLY_FILLED);
    ASSERT_EQ(reports[0].fills.size(), 2u);
    ASSERT_EQ(reports[1].type, ExecutionType::CANCELED);

    ASSERT_EQ(exchange.bestAsk(1), std::nullopt);
    ASSERT_EQ(exchange.bestBid(1), std::nullopt);
}

TEST(ExchangeTest, LimitOrderExceedingDepthRestsRemainder) {
    Exchange exchange;
    exchange.submitOrder(makeOrder(1, Side::SELL, Price{100}, Qty{5}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));
    exchange.submitOrder(makeOrder(2, Side::SELL, Price{101}, Qty{5}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));

    auto reports = exchange.submitOrder(makeOrder(3, Side::BUY, Price{101}, Qty{20}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));

    // partial fill + rests remainder -> two-report case, second is NEW not CANCELED
    ASSERT_EQ(reports.size(), 2u);
    ASSERT_EQ(reports[0].type, ExecutionType::PARTIALLY_FILLED);
    ASSERT_EQ(reports[0].fills.size(), 2u);
    ASSERT_EQ(reports[1].type, ExecutionType::NEW);

    ASSERT_EQ(exchange.bestAsk(1), std::nullopt);
    ASSERT_EQ(exchange.bestBid(1), Price{101});
    ASSERT_EQ(exchange.getQty(1, Side::BUY, Price{101}).get(), 10);
}

TEST(ExchangeTest, LimitOrderStopsAtOwnPriceDuringSweep) {
    Exchange exchange;
    exchange.submitOrder(makeOrder(1, Side::SELL, Price{100}, Qty{5}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));
    exchange.submitOrder(makeOrder(2, Side::SELL, Price{101}, Qty{5}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));
    exchange.submitOrder(makeOrder(3, Side::SELL, Price{102}, Qty{5}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));

    auto reports = exchange.submitOrder(makeOrder(4, Side::BUY, Price{101}, Qty{20}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));

    ASSERT_EQ(reports.size(), 2u);
    ASSERT_EQ(reports[0].fills.size(), 2u);   // only 100 and 101 levels — 102 never touched

    ASSERT_EQ(exchange.bestAsk(1), Price{102});
    ASSERT_EQ(exchange.getQty(1, Side::SELL, Price{102}).get(), 5);
    ASSERT_EQ(exchange.getQty(1, Side::BUY, Price{101}).get(), 10);
}

// ============================================================
// 6. Order type interactions (stop/stop-limit triggering)
// ============================================================

TEST(ExchangeTest, FillTriggersRestingBuyStopAndEmitsTriggeredReport) {
    Exchange exchange;
    exchange.submitOrder(makeOrder(1, Side::SELL, Price{100}, Qty{10}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));
    exchange.submitOrder(makeOrder(2, Side::SELL, Price{100}, Qty{10}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));
    exchange.submitOrder(makeOrder(3, Side::BUY, Price{0}, Qty{5}, OrderType::STOP, TimeInForce::NONE, FillPolicy::NORMAL, Price{100}, 1));

    auto reports = exchange.submitOrder(makeOrder(4, Side::BUY, Price{0}, Qty{5}, OrderType::MARKET, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));

    // order 4's own trade report, plus order 3's TRIGGERED report, plus order 3's
    // own trade report once reprocessed — three reports total, not one-per-resting-order.
    bool sawTriggered = false, sawFillAfterTrigger = false;
    for (const auto& r : reports) {
        if (r.order_id.get() != 3) continue;
        if (r.type == ExecutionType::TRIGGERED) sawTriggered = true;
        if (sawTriggered && (r.type == ExecutionType::FILLED || r.type == ExecutionType::PARTIALLY_FILLED))
            sawFillAfterTrigger = true;
    }
    ASSERT_TRUE(sawTriggered);
    ASSERT_TRUE(sawFillAfterTrigger);
    ASSERT_EQ(exchange.getFirstBuyStop(1), std::nullopt);
}

TEST(ExchangeTest, StopLimitTriggersButRestsIfMarketMovedPastLimit) {
    Exchange exchange;
    exchange.submitOrder(makeOrder(1, Side::SELL, Price{100}, Qty{10}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));
    exchange.submitOrder(makeOrder(2, Side::BUY, Price{99}, Qty{5}, OrderType::STOP_LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{100}, 1));

    auto reports = exchange.submitOrder(makeOrder(3, Side::BUY, Price{0}, Qty{10}, OrderType::MARKET, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));

    bool sawTriggered = false, sawNewAfterTrigger = false;
    for (const auto& r : reports) {
        if (r.order_id.get() != 2) continue;
        if (r.type == ExecutionType::TRIGGERED) sawTriggered = true;
        if (sawTriggered && r.type == ExecutionType::NEW) sawNewAfterTrigger = true;
    }
    ASSERT_TRUE(sawTriggered);
    ASSERT_TRUE(sawNewAfterTrigger);

    ASSERT_EQ(exchange.getFirstBuyStop(1), std::nullopt);
    ASSERT_EQ(exchange.bestBid(1), Price{99});
}

TEST(ExchangeTest, TriggeredStopReportPrecedesReprocessingReports) {
    Exchange exchange;
    exchange.submitOrder(makeOrder(1, Side::SELL, Price{100}, Qty{10}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));
    exchange.submitOrder(makeOrder(2, Side::BUY, Price{0}, Qty{5}, OrderType::STOP, TimeInForce::NONE, FillPolicy::NORMAL, Price{100}, 1));

    auto reports = exchange.submitOrder(makeOrder(3, Side::BUY, Price{0}, Qty{5}, OrderType::MARKET, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));

    int triggeredIndex = -1, filledIndex = -1;
    for (size_t i = 0; i < reports.size(); ++i) {
        if (reports[i].order_id.get() != 2) continue;
        if (reports[i].type == ExecutionType::TRIGGERED && triggeredIndex == -1) triggeredIndex = static_cast<int>(i);
        if (reports[i].type == ExecutionType::FILLED) filledIndex = static_cast<int>(i);
    }
    ASSERT_NE(triggeredIndex, -1);
    ASSERT_NE(filledIndex, -1);
    ASSERT_LT(triggeredIndex, filledIndex);
}

TEST(ExchangeTest, TypeDispatchSmoke) {
    Exchange exchange;
    exchange.submitOrder(makeOrder(1, Side::BUY, Price{90}, Qty{5}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));
    exchange.submitOrder(makeOrder(3, Side::SELL, Price{0}, Qty{5}, OrderType::STOP, TimeInForce::NONE, FillPolicy::NORMAL, Price{80}, 1));

    ASSERT_EQ(exchange.bestBid(1), Price{90});
    ASSERT_TRUE(exchange.getFirstSellStop(1).has_value());
}

// ============================================================
// 7. Fill policies (FOK / AON)
// ============================================================

TEST(ExchangeTest, FokFullyFillableSucceeds) {
    Exchange exchange;
    exchange.submitOrder(makeOrder(1, Side::SELL, Price{100}, Qty{10}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));

    auto reports = exchange.submitOrder(
        makeOrder(2, Side::BUY, Price{100}, Qty{10}, OrderType::LIMIT, TimeInForce::FILL_OR_KILL, FillPolicy::NORMAL, Price{0}, 1)
    );

    ASSERT_EQ(reports.size(), 1u);
    ASSERT_EQ(reports[0].type, ExecutionType::FILLED);
    ASSERT_EQ(reports[0].fills.size(), 1u);
}

TEST(ExchangeTest, FokPartiallyFillableIsKilledEntirely) {
    Exchange exchange;
    exchange.submitOrder(makeOrder(1, Side::SELL, Price{100}, Qty{5}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));

    auto reports = exchange.submitOrder(
        makeOrder(2, Side::BUY, Price{100}, Qty{10}, OrderType::LIMIT, TimeInForce::FILL_OR_KILL, FillPolicy::NORMAL, Price{0}, 1)
    );

    ASSERT_EQ(reports.size(), 1u);
    ASSERT_EQ(reports[0].type, ExecutionType::CANCELED);
    ASSERT_TRUE(reports[0].fills.empty());   // gated before matching — no partial fill leaked
    ASSERT_TRUE(exchange.orderExists(1, OrderId{1}));
    ASSERT_EQ(exchange.getQty(1, Side::SELL, Price{100}).get(), 5);
}

TEST(ExchangeTest, FokSumsLiquidityAcrossLevelsBeforeCommitting) {
    Exchange exchange;
    exchange.submitOrder(makeOrder(1, Side::SELL, Price{100}, Qty{5}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));
    exchange.submitOrder(makeOrder(2, Side::SELL, Price{101}, Qty{5}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));

    auto reports = exchange.submitOrder(
        makeOrder(3, Side::BUY, Price{101}, Qty{10}, OrderType::LIMIT, TimeInForce::FILL_OR_KILL, FillPolicy::NORMAL, Price{0}, 1)
    );

    ASSERT_EQ(reports.size(), 1u);
    ASSERT_EQ(reports[0].type, ExecutionType::FILLED);
    ASSERT_EQ(reports[0].fills.size(), 2u);
    ASSERT_EQ(exchange.bestAsk(1), std::nullopt);
}

TEST(ExchangeTest, AonRestsIfUnfillableInsteadOfKilling) {
    Exchange exchange;
    exchange.submitOrder(makeOrder(1, Side::SELL, Price{100}, Qty{5}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));

    auto reports = exchange.submitOrder(
        makeOrder(2, Side::BUY, Price{100}, Qty{10}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::ALL_OR_NONE, Price{0}, 1)
    );

    ASSERT_EQ(reports.size(), 1u);
    ASSERT_EQ(reports[0].type, ExecutionType::NEW);
    ASSERT_TRUE(exchange.orderExists(1, OrderId{2}));
    ASSERT_TRUE(exchange.orderExists(1, OrderId{1}));
}

// ============================================================
// 8. Time-in-force
// ============================================================

TEST(ExchangeTest, DayOrderExpiredAtEndOfDayNotCanceled) {
    Exchange exchange;
    exchange.submitOrder(makeOrder(1, Side::BUY, Price{100}, Qty{5}, OrderType::LIMIT, TimeInForce::DAY, FillPolicy::NORMAL, Price{0}, 1));

    auto reports = exchange.runEndOfDaySession();

    bool sawExpired = false;
    for (const auto& r : reports) if (r.order_id.get() == 1 && r.type == ExecutionType::EXPIRED) sawExpired = true;
    ASSERT_TRUE(sawExpired);

    ASSERT_FALSE(exchange.orderExists(1, OrderId{1}));
    ASSERT_EQ(exchange.bestBid(1), std::nullopt);
}

TEST(ExchangeTest, DayStopOrderExpiredAtEndOfDay) {
    Exchange exchange;
    exchange.submitOrder(makeOrder(1, Side::BUY, Price{0}, Qty{5}, OrderType::STOP, TimeInForce::DAY, FillPolicy::NORMAL, Price{100}, 1));

    auto reports = exchange.runEndOfDaySession();

    bool sawExpired = false;
    for (const auto& r : reports) if (r.order_id.get() == 1 && r.type == ExecutionType::EXPIRED) sawExpired = true;
    ASSERT_TRUE(sawExpired);
    ASSERT_EQ(exchange.getFirstBuyStop(1), std::nullopt);
}

TEST(ExchangeTest, GtcOrderSurvivesEndOfDay) {
    Exchange exchange;
    exchange.submitOrder(makeOrder(1, Side::BUY, Price{100}, Qty{5}, OrderType::LIMIT, TimeInForce::GOOD_TIL_CANCELED, FillPolicy::NORMAL, Price{0}, 1));

    auto reports = exchange.runEndOfDaySession();

    for (const auto& r : reports) ASSERT_NE(r.order_id.get(), 1);
    ASSERT_TRUE(exchange.orderExists(1, OrderId{1}));
    ASSERT_EQ(exchange.bestBid(1), Price{100});
}

TEST(ExchangeTest, MixedDayAndGtcOnlyDayIsPurged) {
    Exchange exchange;
    exchange.submitOrder(makeOrder(1, Side::BUY, Price{100}, Qty{5}, OrderType::LIMIT, TimeInForce::DAY, FillPolicy::NORMAL, Price{0}, 1));
    exchange.submitOrder(makeOrder(2, Side::BUY, Price{99},  Qty{5}, OrderType::LIMIT, TimeInForce::GOOD_TIL_CANCELED, FillPolicy::NORMAL, Price{0}, 1));

    exchange.runEndOfDaySession();

    ASSERT_FALSE(exchange.orderExists(1, OrderId{1}));
    ASSERT_TRUE(exchange.orderExists(1, OrderId{2}));
    ASSERT_EQ(exchange.bestBid(1), Price{99});
}

TEST(ExchangeTest, ImmediateOrCancelDropsUnfilledRemainder) {
    Exchange exchange;
    exchange.submitOrder(makeOrder(1, Side::SELL, Price{100}, Qty{5}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));

    auto reports = exchange.submitOrder(
        makeOrder(2, Side::BUY, Price{100}, Qty{10}, OrderType::LIMIT, TimeInForce::IMMEDIATE_OR_CANCEL, FillPolicy::NORMAL, Price{0}, 1)
    );

    // partial fill (5 of 10) + drop remainder -> two-report case
    ASSERT_EQ(reports.size(), 2u);
    ASSERT_EQ(reports[0].type, ExecutionType::PARTIALLY_FILLED);
    ASSERT_EQ(reports[1].type, ExecutionType::CANCELED);
    ASSERT_EQ(exchange.bestBid(1), std::nullopt);
}

// NOTE: GTD-with-past-expiration and GTD-actually-expiring-over-time still need a design
// decision on how expiration is evaluated. Recommend an
// Exchange::expireOrdersAsOf(time_point) hook purely for testability.

// ============================================================
// 9. Cancel interactions
// ============================================================
// (Scope note: cancelOrder() reports are unaffected by the submitOrder() consolidation
// rule above — a cancel always concerns exactly one order and produces exactly one report.)

TEST(ExchangeTest, CancelAlreadyFilledOrderIsRejected) {
    Exchange exchange;
    exchange.submitOrder(makeOrder(1, Side::SELL, Price{100}, Qty{5}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));
    exchange.submitOrder(makeOrder(2, Side::BUY,  Price{100}, Qty{5}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));

    auto reports = exchange.cancelOrder(CancelRequest{OrderId{1}, 1});
    ASSERT_EQ(reports.size(), 1u);
    ASSERT_EQ(reports[0].type, ExecutionType::REJECTED);
}

TEST(ExchangeTest, CancelPartiallyFilledOrderCancelsRemainderOnly) {
    Exchange exchange;
    exchange.submitOrder(makeOrder(1, Side::SELL, Price{100}, Qty{10}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));
    exchange.submitOrder(makeOrder(2, Side::BUY,  Price{0},   Qty{4},  OrderType::MARKET, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));

    auto reports = exchange.cancelOrder(CancelRequest{OrderId{1}, 1});

    ASSERT_EQ(reports.size(), 1u);
    ASSERT_EQ(reports[0].type, ExecutionType::CANCELED);
    ASSERT_FALSE(exchange.orderExists(1, OrderId{1}));
    ASSERT_EQ(exchange.bestAsk(1), std::nullopt);
}

TEST(ExchangeTest, DoubleCancelSecondCallIsRejected) {
    Exchange exchange;
    exchange.submitOrder(makeOrder(1, Side::BUY, Price{100}, Qty{5}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));

    auto first = exchange.cancelOrder(CancelRequest{OrderId{1}, 1});
    auto second = exchange.cancelOrder(CancelRequest{OrderId{1}, 1});

    ASSERT_EQ(first[0].type, ExecutionType::CANCELED);
    ASSERT_EQ(second[0].type, ExecutionType::REJECTED);
}

TEST(ExchangeTest, ResubmitAtSamePriceAfterCancelStartsClean) {
    Exchange exchange;
    exchange.submitOrder(makeOrder(1, Side::BUY, Price{100}, Qty{5}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));
    exchange.cancelOrder(CancelRequest{OrderId{1}, 1});

    exchange.submitOrder(makeOrder(2, Side::BUY, Price{100}, Qty{7}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));

    ASSERT_EQ(exchange.getQty(1, Side::BUY, Price{100}).get(), 7);
    ASSERT_EQ(exchange.bidLevelCount(1), 1u);
}

TEST(ExchangeTest, CancelStopOrderPreventsItFromEverTriggering) {
    Exchange exchange;
    exchange.submitOrder(makeOrder(1, Side::BUY, Price{0}, Qty{5}, OrderType::STOP, TimeInForce::NONE, FillPolicy::NORMAL, Price{100}, 1));

    exchange.cancelOrder(CancelRequest{OrderId{1}, 1});
    ASSERT_EQ(exchange.getFirstBuyStop(1), std::nullopt);

    exchange.submitOrder(makeOrder(2, Side::SELL, Price{100}, Qty{10}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));
    auto reports = exchange.submitOrder(makeOrder(3, Side::BUY, Price{0}, Qty{10}, OrderType::MARKET, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));

    for (const auto& r : reports) ASSERT_NE(r.order_id.get(), 1);
}

// ============================================================
// 10. Self-crossing / edge quantity cases
// ============================================================

TEST(ExchangeTest, LargeQuantityDoesNotOverflowCachedTotal) {
    Exchange exchange;
    exchange.submitOrder(makeOrder(1, Side::BUY, Price{100}, Qty{1'000'000}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));
    exchange.submitOrder(makeOrder(2, Side::BUY, Price{100}, Qty{1'000'000}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));

    ASSERT_EQ(exchange.getQty(1, Side::BUY, Price{100}).get(), 2'000'000);
}

// NOTE: negative price/quantity, zero-quantity submission, and self-trade prevention
// still depend on validation decisions you haven't made yet.

// ============================================================
// 11. Multi-ticker isolation
// ============================================================

TEST(ExchangeTest, SamePriceDifferentTickersDoNotCross) {
    Exchange exchange;
    exchange.submitOrder(makeOrder(1, Side::SELL, Price{100}, Qty{10}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, /*ticker*/1));
    auto reports = exchange.submitOrder(makeOrder(2, Side::BUY, Price{100}, Qty{10}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, /*ticker*/2));

    ASSERT_EQ(reports.size(), 1u);
    ASSERT_EQ(reports[0].type, ExecutionType::NEW);
    ASSERT_EQ(exchange.bestAsk(1), Price{100});
    ASSERT_EQ(exchange.bestBid(2), Price{100});
}

TEST(ExchangeTest, CancelWithWrongTickerIsRejectedNotMisrouted) {
    Exchange exchange;
    exchange.submitOrder(makeOrder(1, Side::BUY, Price{100}, Qty{5}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, /*ticker*/1));

    auto reports = exchange.cancelOrder(CancelRequest{OrderId{1}, /*wrong ticker*/2});

    ASSERT_EQ(reports[0].type, ExecutionType::REJECTED);
    ASSERT_TRUE(exchange.orderExists(1, OrderId{1}));
}

TEST(ExchangeTest, EndOfDayPurgesOnlyMatchingTickersDayOrders) {
    Exchange exchange;
    exchange.submitOrder(makeOrder(1, Side::BUY, Price{100}, Qty{5}, OrderType::LIMIT, TimeInForce::DAY, FillPolicy::NORMAL, Price{0}, 1));
    exchange.submitOrder(makeOrder(2, Side::BUY, Price{100}, Qty{5}, OrderType::LIMIT, TimeInForce::DAY, FillPolicy::NORMAL, Price{0}, 2));

    auto reports = exchange.runEndOfDaySession();

    int expiredCount = 0;
    for (const auto& r : reports) if (r.type == ExecutionType::EXPIRED) ++expiredCount;
    ASSERT_EQ(expiredCount, 2);
    ASSERT_FALSE(exchange.orderExists(1, OrderId{1}));
    ASSERT_FALSE(exchange.orderExists(2, OrderId{2}));
}

// ============================================================
// Regression test: matchLimit must respect the seller's price floor
// (caught via simulate.cc trace — a SELL LIMIT at $1039 was incorrectly
// matching resting bids at $38 and $48)
// ============================================================

TEST(ExchangeTest, SellLimitDoesNotCrossBelowItsLimitPrice) {
    Exchange exchange;
    exchange.submitOrder(makeOrder(1, Side::BUY, Price{38}, Qty{280}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));
    exchange.submitOrder(makeOrder(3, Side::BUY, Price{48}, Qty{131}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));

    auto reports = exchange.submitOrder(
        makeOrder(6, Side::SELL, Price{1039}, Qty{161}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1)
    );

    // best bid ($48) is nowhere near the seller's floor ($1039) — must NOT cross
    ASSERT_EQ(reports.size(), 1u);
    ASSERT_EQ(reports[0].type, ExecutionType::NEW);
    ASSERT_TRUE(reports[0].fills.empty());

    ASSERT_EQ(exchange.bestAsk(1), Price{1039});   // rests as the new best ask
    ASSERT_EQ(exchange.bestBid(1), Price{48});      // both resting buys untouched
    ASSERT_EQ(exchange.getQty(1, Side::BUY, Price{48}).get(), 131);
    ASSERT_EQ(exchange.getQty(1, Side::BUY, Price{38}).get(), 280);
}

// ============================================================
// bestBid / bestAsk behavior
// ============================================================

TEST(ExchangeTest, BestBidUpdatesAsHigherOrdersArriveAndAreRemoved) {
    Exchange exchange;
    ASSERT_EQ(exchange.bestBid(1), std::nullopt);

    exchange.submitOrder(makeOrder(1, Side::BUY, Price{100}, Qty{5}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));
    ASSERT_EQ(exchange.bestBid(1), Price{100});

    // a higher bid arrives — becomes the new best
    exchange.submitOrder(makeOrder(2, Side::BUY, Price{105}, Qty{5}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));
    ASSERT_EQ(exchange.bestBid(1), Price{105});

    // a lower bid arrives — does NOT become best
    exchange.submitOrder(makeOrder(3, Side::BUY, Price{90}, Qty{5}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));
    ASSERT_EQ(exchange.bestBid(1), Price{105});

    // removing the current best reveals the next-highest
    exchange.cancelOrder(CancelRequest{OrderId{2}, 1});
    ASSERT_EQ(exchange.bestBid(1), Price{100});

    exchange.cancelOrder(CancelRequest{OrderId{1}, 1});
    ASSERT_EQ(exchange.bestBid(1), Price{90});

    exchange.cancelOrder(CancelRequest{OrderId{3}, 1});
    ASSERT_EQ(exchange.bestBid(1), std::nullopt);   // back to empty
}

TEST(ExchangeTest, BestAskUpdatesAsLowerOrdersArriveAndAreRemoved) {
    Exchange exchange;
    ASSERT_EQ(exchange.bestAsk(1), std::nullopt);

    exchange.submitOrder(makeOrder(1, Side::SELL, Price{100}, Qty{5}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));
    ASSERT_EQ(exchange.bestAsk(1), Price{100});

    // a lower ask arrives — becomes the new best (lowest wins for asks)
    exchange.submitOrder(makeOrder(2, Side::SELL, Price{95}, Qty{5}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));
    ASSERT_EQ(exchange.bestAsk(1), Price{95});

    // a higher ask arrives — does NOT become best
    exchange.submitOrder(makeOrder(3, Side::SELL, Price{110}, Qty{5}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));
    ASSERT_EQ(exchange.bestAsk(1), Price{95});

    exchange.cancelOrder(CancelRequest{OrderId{2}, 1});
    ASSERT_EQ(exchange.bestAsk(1), Price{100});

    exchange.cancelOrder(CancelRequest{OrderId{1}, 1});
    ASSERT_EQ(exchange.bestAsk(1), Price{110});

    exchange.cancelOrder(CancelRequest{OrderId{3}, 1});
    ASSERT_EQ(exchange.bestAsk(1), std::nullopt);
}

TEST(ExchangeTest, BestBidAskAreIsolatedPerTicker) {
    Exchange exchange;
    exchange.submitOrder(makeOrder(1, Side::BUY,  Price{50}, Qty{5}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));
    exchange.submitOrder(makeOrder(2, Side::SELL, Price{60}, Qty{5}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));

    exchange.submitOrder(makeOrder(3, Side::BUY,  Price{999}, Qty{5}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 2));
    exchange.submitOrder(makeOrder(4, Side::SELL, Price{1000}, Qty{5}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 2));

    ASSERT_EQ(exchange.bestBid(1), Price{50});
    ASSERT_EQ(exchange.bestAsk(1), Price{60});
    ASSERT_EQ(exchange.bestBid(2), Price{999});
    ASSERT_EQ(exchange.bestAsk(2), Price{1000});
}

TEST(ExchangeTest, BestBidAskReflectPartialFillNotJustTopLevelPresence) {
    Exchange exchange;
    exchange.submitOrder(makeOrder(1, Side::SELL, Price{100}, Qty{5}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));
    exchange.submitOrder(makeOrder(2, Side::SELL, Price{101}, Qty{5}, OrderType::LIMIT, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));

    // fully consumes the $100 level, best ask should move to $101
    exchange.submitOrder(makeOrder(3, Side::BUY, Price{0}, Qty{5}, OrderType::MARKET, TimeInForce::NONE, FillPolicy::NORMAL, Price{0}, 1));

    ASSERT_EQ(exchange.bestAsk(1), Price{101});
    ASSERT_EQ(exchange.askLevelCount(1), 1u);
}