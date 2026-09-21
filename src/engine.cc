#include "../include/orderbook.h"
#include "../include/engine.h"
#include "../include/data.h"
#include "../include/types.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <optional>
/*
How order type, time in force, and fill policy are handled...

Order Type:
- Market: any not filled quantity is cancelled

Time In Force:
- FOK: check liquidity; if not enough orders, give Cancel report for the volume of the entire incoming order
       fill entire order, or cancel immediately
- GTC: "normal"
- Day: not responsible for by this loop
- IOC: fill whatever volume is possible, cancel the rest immediately

Fill Policy:
- AON: check liquidity; if not enough orders, give Cancel report for the volume of the entire incoming order
*/
bool MatchingEngine::hasEnoughLiquidity(const Order& order, OrderBook& book) const {
    Qty available{0};

    auto accumulate = [&](auto& book_side) {
        for (const auto& [price, level] : book_side) {
            if (order.type == OrderType::LIMIT) {
                bool inReach = (order.side == Side::BUY) ? (price <= order.price)
                                                          : (price >= order.price);
                if (!inReach) break;   // map is ordered — nothing further can be in reach either
            }
            available += level.getQty();
            if (available.get() >= order.quantity.get()) return;
        }
    };

    if (order.side == Side::BUY) accumulate(book.getAsks());
    else                          accumulate(book.getBids());

    return available.get() >= order.quantity.get();
}
// note: the liquidity check assumes all resting quantity is consumable; interactions between multiple AON orders at the same level aren't modeled precisely

std::vector<ExecutionReport> MatchingEngine::submit(Order order, OrderBook& book, StopOrderBook& stop_book) {
    std::vector<ExecutionReport> reports;

    std::deque<Order> order_queue;
    order_queue.push_back(std::move(order));

    size_t queue_index = 0;
    while (queue_index < order_queue.size()) {
        Order& cur_order = order_queue[queue_index++];   // stays valid across push_back — deque guarantee

        bool requiresFullFill = (cur_order.tif == TimeInForce::FILL_OR_KILL) ||
                                (cur_order.fill == FillPolicy::ALL_OR_NONE);

        if (requiresFullFill && !hasEnoughLiquidity(cur_order, book)) {
            if (cur_order.tif == TimeInForce::FILL_OR_KILL || cur_order.type == OrderType::MARKET) {
                // FOK always kills outright. AON on a MARKET order also kills outright,
                // because market orders never rest — there's nowhere for it to wait.
                reports.push_back(ExecutionReport{
                    cur_order.order_id, ExecutionType::CANCELED, cur_order.quantity, Price{0}, {}
                });
            } else {
                // AON on a LIMIT order: can't fill now, so it rests and waits for future liquidity.
                book.addRestingOrder(cur_order, cur_order.side, reports);   // emits NEW
            }
            continue;
        }

        std::vector<Fill> fills;
        switch (cur_order.type) {
            case OrderType::MARKET:
                fills = matchMarket(cur_order, book);
                break;
            case OrderType::LIMIT:
                fills = matchLimit(cur_order, book);
                break;
            case OrderType::STOP:
            case OrderType::STOP_LIMIT:
                stop_book.add(cur_order, reports);
                continue;
        }

        Price last_price(0);
        if (!fills.empty()) {
            last_price = fills.back().price;

            reports.emplace_back(
                cur_order.order_id,
                (cur_order.quantity.get() == 0) ? ExecutionType::FILLED : ExecutionType::PARTIALLY_FILLED,
                cur_order.quantity,
                last_price,
                fills
            );

            std::vector<Order> triggered = stop_book.triggerUpTo(last_price);
            for (auto& t : triggered) {
                reports.emplace_back(
                    t.order_id,
                    ExecutionType::TRIGGERED,
                    t.quantity,
                    last_price,
                    std::vector<Fill>{}
                );
                t.type = (t.type == OrderType::STOP) ? OrderType::MARKET : OrderType::LIMIT;
                order_queue.push_back(std::move(t));   // safe: cur_order reference above is unaffected
            }
        }

        if (cur_order.quantity.get() > 0) {
            if (cur_order.type == OrderType::MARKET ||
                cur_order.tif == TimeInForce::IMMEDIATE_OR_CANCEL) {
                reports.emplace_back(
                    cur_order.order_id,
                    ExecutionType::CANCELED,
                    cur_order.quantity,
                    last_price,
                    std::vector<Fill>{}
                );
            } else if (cur_order.type == OrderType::LIMIT &&
                       (cur_order.tif == TimeInForce::DAY || cur_order.tif == TimeInForce::GOOD_TIL_CANCELED || cur_order.tif == TimeInForce::NONE)) {
                book.addRestingOrder(cur_order, cur_order.side, reports);
            }
        }
    }
    return reports;
}

std::vector<Fill> MatchingEngine::matchMarket(Order& order, OrderBook& book) {
    std::vector<Fill> fills;
    Qty remaining = order.quantity;

    auto sweep = [&](auto& book_side) {
        while (remaining.get() > 0) {
            if (book_side.empty()) break;

            auto& [price, price_level] = *book_side.begin();

            while (!price_level.isEmpty() && remaining.get() > 0) {
                auto transacted_order_it = price_level.begin();
                Qty transacted_qty = std::min(remaining, transacted_order_it->quantity);
                Qty resting_remaining = transacted_order_it->quantity - transacted_qty;

                fills.emplace_back(
                    transacted_order_it->order_id,
                    order.order_id,
                    price,
                    transacted_qty,
                    resting_remaining,
                    std::chrono::steady_clock::now()
                );

                remaining -= transacted_qty;
                transacted_order_it->quantity = resting_remaining;
                price_level.reduceQty(transacted_qty);

                if (resting_remaining.get() == 0) {
                    book.removeFilledOrder(transacted_order_it->order_id);
                }
            }

            if (price_level.isEmpty()) {
                book_side.erase(price);
            }
        }
    };

    if (order.side == Side::BUY) sweep(book.getAsks());
    else                          sweep(book.getBids());

    order.quantity = remaining;
    return fills;
}

std::vector<Fill> MatchingEngine::matchLimit(Order& order, OrderBook& book) {
    std::vector<Fill> fills;
    Qty remaining = order.quantity;

    auto sweep = [&](auto& book_side) {
        while (remaining.get() > 0) {
            if (book_side.empty()) break;

            auto& [price, price_level] = *book_side.begin();

            // very important, after merging the buy and sell logic with lambda
            bool outOfReach = (order.side == Side::BUY) ? (price > order.price)
                                                      : (price < order.price);
            if (outOfReach) break;

            while (!price_level.isEmpty() && remaining.get() > 0) {
                auto transacted_order_it = price_level.begin();
                if (transacted_order_it->fill == FillPolicy::ALL_OR_NONE &&
                    remaining.get() < transacted_order_it->quantity.get()) {
                    break;   // stop sweeping this level; don't touch this order at all
                }
                Qty transacted_qty = std::min(remaining, transacted_order_it->quantity);
                Qty resting_remaining = transacted_order_it->quantity - transacted_qty;

                fills.emplace_back(
                    transacted_order_it->order_id,
                    order.order_id,
                    price,
                    transacted_qty,
                    resting_remaining,
                    std::chrono::steady_clock::now()
                );

                remaining -= transacted_qty;
                transacted_order_it->quantity = resting_remaining;
                price_level.reduceQty(transacted_qty);

                if (resting_remaining.get() == 0) {
                    book.removeFilledOrder(transacted_order_it->order_id);
                }
            }

            if (price_level.isEmpty()) {
                book_side.erase(price);
            }
        }
    };

    if (order.side == Side::BUY) sweep(book.getAsks());
    else                          sweep(book.getBids());

    order.quantity = remaining;
    return fills;
}
