#include "../include/orderbook.h"
#include "../include/exchange.h"
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
        reports.emplace_back(ExecutionReport{
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
        reports.emplace_back(ExecutionReport{order_id, ExecutionType::REJECTED, Qty{0}, Price{0}, std::vector<Fill>{}});
        return false;
    }
    return true;
}

bool OrderBook::removeFilledOrder(OrderId order_id) {
    auto order_it = order_index_.find(order_id);
    if (order_it == order_index_.end()) return false;

    OrderLocation& loc = order_it->second;
    PriceLevel* level = loc.level;
    level->removeOrder(loc.order_it);
    order_index_.erase(order_it);
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

bool OrderBook::modify(const ModifyRequest& req, std::vector<ExecutionReport>& reports, Exchange& exchange) {
    auto index_it = order_index_.find(req.order_id);
    if (index_it == order_index_.end()) return false;

    OrderLocation& loc = index_it->second;
    PriceLevel* level = loc.level;
    auto order_it = loc.order_it;

    // if both price and qty are left unchanged
    if (order_it->price == req.new_price && order_it->quantity == req.new_quantity) {
        reports.push_back(ExecutionReport{req.order_id, ExecutionType::REJECTED, Qty{0}, Price{0}, {}});
        return false; 
    }

    // Case A: Price is unchanged and quantity decreased (Queue priority maintained!)
    if (level->getPrice() == req.new_price && req.new_quantity < order_it->quantity) {
        Qty diff = order_it->quantity - req.new_quantity;
        order_it->quantity -= diff;
        level->adjustQty(-diff);
        
        reports.emplace_back(ExecutionReport{
            req.order_id, 
            ExecutionType::MODIFIED, // Fixed Protocol: Size reduction is a standard modification event
            req.new_quantity, 
            req.new_price, 
            {}
        });
        return true;
    }
    
    // Case B: Price changed or size increased (Loses queue priority, cancel & re-submit)
    Order backup_order = *order_it; // Copy from underlying list entry value
    backup_order.price = req.new_price;
    backup_order.quantity = req.new_quantity;

    this->cancel(req.order_id, reports);

    std::vector<ExecutionReport> match_reports = exchange.submitOrder(std::move(backup_order));
    reports.insert(reports.end(), match_reports.begin(), match_reports.end());
    
    return true;
}

void OrderBook::purgeDayOrders(std::vector<ExecutionReport>& cancellations) noexcept {
    std::vector<OrderId> targets_to_kill;
    for (const auto& [price, level] : bids_) {
        for (auto order_it = level.cbegin(); order_it != level.cend(); ++order_it) {
            if (order_it->tif == TimeInForce::DAY) targets_to_kill.push_back(order_it->order_id);
        }
    }
    for (const auto& [price, level] : asks_) {
        for (auto order_it = level.cbegin(); order_it != level.cend(); ++order_it) {
            if (order_it->tif == TimeInForce::DAY) targets_to_kill.push_back(order_it->order_id);
        }
    }
    for (OrderId id : targets_to_kill) cancel(id, cancellations);
}

// StopOrderBook
void StopOrderBook::add(Order order) {
    const OrderId order_id = order.order_id;
    if (order.side == Side::BUY) {
        auto it = buy_stops_.emplace(order.stop_price, std::move(order));
        stop_index_[order_id] = StopLocation{Side::BUY, it};
    } else {
        auto it = sell_stops_.emplace(order.stop_price, std::move(order));
        stop_index_[order_id] = StopLocation(Side::SELL, it);
    }
}

std::vector<Order> StopOrderBook::triggerUpTo(Price last_trade_price) {
    std::vector<Order> triggered;
    // check buys
    auto it_end_buys = buy_stops_.upper_bound(last_trade_price);
    for (auto it = buy_stops_.begin(); it != it_end_buys; ++it) {
        auto order = it->second;
        triggered.push_back(order);
        stop_index_.erase(order.order_id);
    }
    buy_stops_.erase(buy_stops_.begin(), it_end_buys);
    // check sells
    auto it_start_sells = sell_stops_.lower_bound(last_trade_price);
    for (auto it = it_start_sells; it != sell_stops_.end(); ++it) {
        auto order = it->second;
        triggered.push_back(order);
        stop_index_.erase(order.order_id);
    }
    sell_stops_.erase(it_start_sells, sell_stops_.end());
    return triggered;
}

bool StopOrderBook::cancel(OrderId order_id, std::vector<ExecutionReport>& reports) {
    auto order_it = stop_index_.find(order_id);
    if (order_it == stop_index_.end()) {
        reports.emplace_back(ExecutionReport{order_id, ExecutionType::REJECTED, Qty{0}, Price{0}, std::vector<Fill>{}});
        return false;
    }
    const StopLocation& loc = order_it->second;
    const Qty canceled_quantity = loc.it->second.quantity;
    const Price stop_price = loc.it->second.stop_price;
    if (loc.side == Side::BUY) {
        buy_stops_.erase(loc.it);
    } else {
        sell_stops_.erase(loc.it);
    }

    stop_index_.erase(order_it);
    reports.emplace_back(ExecutionReport{order_id, ExecutionType::CANCELED,
                                         canceled_quantity,
                                         stop_price,
                                         std::vector<Fill>{}});
    return true;
}

bool StopOrderBook::modify(const ModifyRequest& req, std::vector<ExecutionReport>& reports, Exchange& exchange) {
    auto order_it = stop_index_.find(req.order_id);
    if (order_it == stop_index_.end()) return false;

    StopLocation& loc = order_it->second;
    Order& order = (loc.it)->second;

    // 1. Check if both parameters are identical (Nothing to change)
    // Note: If ModifyRequest contains a new trigger target, compare against stop_price.
    // If your ModifyRequest object uses 'req.new_price' as the new trigger price for stops:
    if (order.stop_price == req.new_price && order.quantity == req.new_quantity) {
        reports.push_back(ExecutionReport{req.order_id, ExecutionType::REJECTED, Qty{0}, Price{0}, {}});
        return false; 
    }
    
    // 1. Size reduction with UNCHANGED trigger price (Safe to modify in-place without losing priority)
    if (order.stop_price == req.new_price && order.quantity > req.new_quantity) {
        order.quantity = req.new_quantity;
        reports.emplace_back(ExecutionReport{
            order.order_id, ExecutionType::MODIFIED, order.quantity, order.price, {}
        });
        return true;
    } 
    
    // 2. Trigger price changed OR size increased (Must delete and re-insert to preserve chronological sorting)
    Order backup_order = order;
    backup_order.quantity = req.new_quantity;
    backup_order.stop_price = req.new_price; // 🟢 Update the trigger boundary, not the execution limit!

    bool success = this->cancel(req.order_id, reports);
    if (success) {
        // Strip out the internal CANCELED report step so the client only sees a single clean update event
        if (!reports.empty() && reports.back().type == ExecutionType::CANCELED) {
            reports.pop_back(); 
        }
        
        this->add(std::move(backup_order));
        reports.emplace_back(ExecutionReport{
            req.order_id, ExecutionType::MODIFIED, req.new_quantity, req.new_price, {}
        });
        return true;
    } 
    
    return false;
}

void StopOrderBook::purgeDayOrders(std::vector<ExecutionReport>& cancellations) noexcept {
    std::vector<OrderId> targets_to_kill;
    for (auto it = buy_stops_.begin(); it != buy_stops_.end(); /* advanced manually */) {
        if (it->second.tif == TimeInForce::DAY) {
            targets_to_kill.push_back(it->second.order_id);
            ++it;
        } else {
            ++it;
        }
    }

    for (auto it = sell_stops_.begin(); it != sell_stops_.end(); /* advanced manually */) {
        if (it->second.tif == TimeInForce::DAY) {
            targets_to_kill.push_back(it->second.order_id);
            ++it;
        } else {
            ++it;
        }
    }
    for (OrderId id : targets_to_kill) cancel(id, cancellations);
}