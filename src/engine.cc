#include "../include/orderbook.h"
#include "../include/engine.h"
#include "../include/data.h"
#include "../include/types.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>

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

bool hasEnoughLiquidity(const Order& order, OrderBook& book) {
    Qty avail_vol(0);
    if (order.side == Side::BUY) {
        for (auto it = book.getAsks().cbegin(); it != book.getAsks().cend(); ++ it) {
            if (order.type == OrderType::LIMIT && it->first > order.price) break;
            avail_vol += it->second.getQty();
            if (avail_vol >= order.quantity) return true;
        }
    } else {
        for (auto it = book.getBids().cbegin(); it != book.getBids().cend(); ++ it) {
            if (order.type == OrderType::LIMIT && it->first < order.price) break;
            avail_vol += it->second.getQty();
            if (avail_vol >= order.quantity) return true;
        }
    }
    return false;
}

std::vector<ExecutionReport> MatchingEngine::submit(Order order, OrderBook& book, StopOrderBook& stop_book) {
    std::vector<ExecutionReport> reports;

    // flat queue to hold orders to process
    std::vector<Order> order_queue;
    order_queue.push_back(std::move(order));

    size_t queue_index = 0;
    while (queue_index < order_queue.size()) {
        Order& cur_order = order_queue[queue_index++];

        if (cur_order.tif == TimeInForce::FILL_OR_KILL || 
           cur_order.fill == FillPolicy::ALL_OR_NONE) {
            
            if (!hasEnoughLiquidity(cur_order, book)) {
                reports.push_back(ExecutionReport{
                    cur_order.order_id,
                    ExecutionType::CANCELED,
                    cur_order.quantity,
                    Price(0),
                    std::vector<Fill>{}
                });
                continue; 
            }
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
                stop_book.add(cur_order);
                break;
            case OrderType::STOP_LIMIT:
                stop_book.add(cur_order);
                break;
        }

        Price last_price(0);
        if (!fills.empty()) {
            last_price = fills.back().price;

            reports.push_back(ExecutionReport {
                cur_order.order_id,
                (cur_order.quantity == 0) ? ExecutionType::FILLED : ExecutionType::PARTIALLY_FILLED,
                cur_order.quantity,
                last_price,
                fills
            });

            std::vector<Order> triggered = stop_book.triggerUpTo(last_price);
            for (auto& t : triggered) {
                t.type = (t.type == OrderType::STOP) ? OrderType::MARKET : OrderType::LIMIT;
                order_queue.push_back(std::move(t));
            }
        } 
        // add ExecutionReport with cancellation if needed
        if (cur_order.quantity > 0) {
            if (cur_order.type == OrderType::MARKET || 
                cur_order.tif == TimeInForce::IMMEDIATE_OR_CANCEL) {
                    // Any remaining quantity must be cancled 
                    reports.push_back(ExecutionReport{
                        cur_order.order_id,
                        ExecutionType::CANCELED,
                        cur_order.quantity,
                        last_price,
                        std::vector<Fill>{}
                    });
            } else if (cur_order.type == OrderType::LIMIT && 
                      (cur_order.tif == TimeInForce::DAY || cur_order.tif == TimeInForce::GOOD_TIL_CANCELED)) {
                        book.addRestingOrder(cur_order, cur_order.side);
                        // TODO: add report here
                      }
        }
    }
    return reports;
}

std::vector<Fill> MatchingEngine::matchMarket(Order& order, OrderBook& book) {
    std::vector<Fill> fills;
    Qty remaining = order.quantity;
    // match the order based on the best existing bid/ask in the books
    if (order.side == Side::BUY) {
        auto& asks = book.getAsks();
    
        while (remaining > 0) {
            if (asks.empty()) {
                // no more orders to transact with
                break;
            }
            auto& [price, price_level] = *asks.begin();
            
            while (!price_level.isEmpty() && remaining > 0) {
                auto transacted_order_it = price_level.begin();
                Qty transacted_qty = std::min(remaining, transacted_order_it->quantity);
                fills.push_back(Fill {
                                    transacted_order_it->order_id,
                                    order.order_id,
                                    price,
                                    transacted_qty,
                                    std::chrono::steady_clock::now()
                                });
                remaining -= transacted_qty;
                transacted_order_it->quantity -= transacted_qty;
                price_level.adjustQty(-transacted_qty);

                if (transacted_order_it->quantity == 0) {
                    book.removeFilledOrder(transacted_order_it->order_id);
                }

            }
            if (price_level.isEmpty()) {
                asks.erase(price);
            }
        }
    } else { // sell side
        auto& bids = book.getBids();
        while (remaining > 0) {
            if (bids.empty()) {
                break;
            }

            auto& [price, price_level] = *bids.begin();
            while (!price_level.isEmpty() && remaining > 0) {
                auto transacted_order_it = price_level.begin();
                Qty transacted_qty = std::min(remaining, transacted_order_it->quantity);
                fills.push_back(Fill {
                                    transacted_order_it->order_id,
                                    order.order_id,
                                    price,
                                    transacted_qty,
                                    std::chrono::steady_clock::now()
                                });
                remaining -= transacted_qty;
                transacted_order_it->quantity -= transacted_qty;
                price_level.adjustQty(-transacted_qty);

                if (transacted_order_it->quantity == 0) {
                    book.removeFilledOrder(transacted_order_it->order_id);
                }
            }
            if (price_level.isEmpty()) {
                bids.erase(price);
            }
        }
    }
    order.quantity = remaining;
    return fills;
}

std::vector<Fill> MatchingEngine::matchLimit(Order& order, OrderBook& book) {
    std::vector<Fill> fills;
    Qty remaining = order.quantity;
    // match the order based on the best existing bid/ask in the books
    if (order.side == Side::BUY) {
        auto& asks = book.getAsks();
    
        while (remaining > 0) {
            if (asks.empty()) {
                // no more orders to transact with
                break;
            }
            auto& [price, price_level] = *asks.begin();
            if (price > order.price) break;
            
            while (!price_level.isEmpty() && remaining > 0) {
                auto transacted_order_it = price_level.begin();
                Qty transacted_qty = std::min(remaining, transacted_order_it->quantity);
                fills.push_back(Fill {
                                    transacted_order_it->order_id,
                                    order.order_id,
                                    price,
                                    transacted_qty,
                                    std::chrono::steady_clock::now()
                                });
                remaining -= transacted_qty;
                transacted_order_it->quantity -= transacted_qty;
                price_level.adjustQty(-transacted_qty);

                if (transacted_order_it->quantity == 0) {
                    book.removeFilledOrder(transacted_order_it->order_id);
                }

            }
            if (price_level.isEmpty()) {
                asks.erase(price);
            }
        }
    } else { // sell side
        auto& bids = book.getBids();
        while (remaining > 0) {
            if (bids.empty()) {
                break;
            }

            auto& [price, price_level] = *bids.begin();
            if (price < order.price) break;
            while (!price_level.isEmpty() && remaining > 0) {
                auto transacted_order_it = price_level.begin();
                Qty transacted_qty = std::min(remaining, transacted_order_it->quantity);
                fills.push_back(Fill {
                                    transacted_order_it->order_id,
                                    order.order_id,
                                    price,
                                    transacted_qty,
                                    std::chrono::steady_clock::now()
                                });
                remaining -= transacted_qty;
                transacted_order_it->quantity -= transacted_qty;
                price_level.adjustQty(-transacted_qty);

                if (transacted_order_it->quantity == 0) {
                    book.removeFilledOrder(transacted_order_it->order_id);
                }
            }
            if (price_level.isEmpty()) {
                bids.erase(price);
            }
        }
    }
    order.quantity = remaining;
    return fills;
}
