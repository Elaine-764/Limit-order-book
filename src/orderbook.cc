#include "../include/orderbook.h"
#include <iostream>

// PriceLevel
std::list<Order>::iterator PriceLevel::addOrder(const Order& order) {
    // order is passed from the network that receives the object
    // then a new object is allocated on the heap, to be owned by the PriceLevel
    // the intially submitted order goes out of scope - irrelevant to the order stored in PriceLevel
    orders_.push_back(order);
    total_qty_ += order.quantity;

    return std::prev(orders_.end());
}

void PriceLevel::removeOrder(std::list<Order>::iterator it) {
    total_qty_ -= it->quantity;
    orders_.erase(it);
}

// OrderBook
void OrderBook::addRestingOrder(Order order, Side side) {
    // add order to the correct price level - create price level if needed
    PriceLevel* pricelevel = nullptr;
    if (side == Side::BUY) {
        auto [map_it, inserted] =  bids_.emplace(order.price, order.price);
        pricelevel = &map_it->second;
    } else {
        auto [map_it, inserted] =  asks_.emplace(order.price, order.price);
        pricelevel = &map_it->second;
    }
    std::list<Order>::iterator it = pricelevel->addOrder(order);

    // create OrderLocation - fast version - put order and order location into order_index_
    order_index_.try_emplace(order.order_id, side, pricelevel, it);
}

bool OrderBook::cancel(OrderId order_id) {
    // find OrderLocation
    auto order_it = order_index_.find(order_id);
    if (order_it != order_index_.end()) {
        // remove order from bid/ask
        OrderLocation location = order_it->second;
        Side side = location.side;
        PriceLevel* level = location.level;
        level->removeOrder(location.order_it);
        // remove from order_index_
        order_index_.erase(order_it);

        // check if any orders remain at this price level, if not, remove
        if (level->isEmpty()) {
            if (side == Side::BUY) {
                bids_.erase(level->getPrice());
            } else {
                asks_.erase(level->getPrice());
            }
        }
    } else {
        // give error message
        std::cerr << "WARNING: Cancel failed. OrderId " << order_id.get() << " not found.\n";
        return false;
    }
    return true;
}

// StopOrderBook
void StopOrderBook::add(Order order) {
    auto& book = (order.side == Side::BUY) ? buy_stops_ : sell_stops_;
    book.emplace(order.stop_price, order);
}

std::vector<Order> StopOrderBook::triggerUpTo(Price last_trade_price) {
    std::vector<Order> triggered;
    // check buys
    auto it_end_buys = buy_stops_.upper_bound(last_trade_price);
    for (auto it = buy_stops_.begin(); it != it_end_buys; ++it) {
        auto order = it->second;
        triggered.push_back(order);
    }
    buy_stops_.erase(buy_stops_.begin(), it_end_buys);
    // check sells
    auto it_end_sells = sell_stops_.lower_bound(last_trade_price);
    for (auto it = sell_stops_.begin(); it != it_end_sells; ++it) {
        auto order = it->second;
        triggered.push_back(order);
    }
    sell_stops_.erase(sell_stops_.begin(), it_end_sells);
    return triggered;
}