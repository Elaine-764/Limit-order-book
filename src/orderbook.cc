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

bool OrderBook::cancel(OrderId order_id, std::vector<ExecutionReport>& reports) {
    // find OrderLocation
    auto order_it = order_index_.find(order_id);
    if (order_it != order_index_.end()) {
        // remove order from bid/ask
        OrderLocation& loc = order_it->second;
        PriceLevel* level = loc.level;

        Qty canceled_qty = loc.order_it->quantity;
        level->removeOrder(loc.order_it);
        reports.push_back(ExecutionReport{
                order_id,
                ExecutionType::CANCELED,
                canceled_qty,
                level->getPrice(),
                std::vector<Fill>{}
            });
    
        // check if any orders remain at this price level, if not, remove
        if (level->isEmpty()) {
            if (loc.side == Side::BUY) {
                bids_.erase(level->getPrice());
            } else {
                asks_.erase(level->getPrice());
            }
        }

        // remove from order_index_
        order_index_.erase(order_it);
    } else {
        // give error message
        std::cerr << "WARNING: Cancel failed. OrderId " << order_id.get() << " not found.\n";
        return false;
    }
    return true;
}

const Order* OrderBook::findOrder(const OrderId order_id) const {
    auto it = order_index_.find(order_id);
    if (it == order_index_.end()) {
        return nullptr;
    }
    const OrderLocation& loc = it->second;
    return &(*loc.order_it);
}

bool OrderBook::modify(const ModifyRequest& req, std::vector<ExecutionReport>& reports, MatchingEngine& engine, StopOrderBook& stop_book) {
    auto index_it = order_index_.find(req.order_id);
    if (index_it == order_index_.end()) return false;

    OrderLocation& loc = index_it->second;
    PriceLevel* level = loc.level;
    auto order_it = loc.order_it;

    // if price is unchanged and quantity decreased 
    if (level->getPrice() == req.new_price && req.new_quantity < order_it->quantity) {
        Qty diff = order_it->quantity - req.new_quantity;
        order_it->quantity -= diff;
        level->adjustQty(-diff);
        reports.push_back(ExecutionReport{
            req.order_id, 
            ExecutionType::PARTIALLY_FILLED, // Standard protocol for size adjustment
            req.new_quantity, 
            req.new_price, 
            {}
        });
        return true;
    }
    Order backup_order = *order_it;
    backup_order.price = req.new_price;
    backup_order.quantity = req.new_quantity;

    this->cancel(req.order_id, reports);

    // resubmit an order
    std::vector<ExecutionReport> match_reports = engine.submitOrder(std::move(backup_order), *this, stop_book);

    reports.insert(reports.end(), match_reports.begin(), match_reports.end());
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