// order book implementation: add/remove/modify and matching
#include "../include/lob/order_book.h"
#include <algorithm>
#include <iostream>
#include <stdexcept>

int Order::next_id = 1; // initialize order count

std::vector<FillResult> OrderBook::addOrder(Order& order) {
    if (order.side == Side::BUY) {
        std::cout << "Successfully running buy side add order" << "\n";
        auto it = bids.find(order.price);
        if (it != bids.end()) {
            it->second.addOrder(order);
        } else {
            auto [new_it, _] = bids.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(order.price),     // key
                std::forward_as_tuple(order.price)      // PriceLevel constructor arg
            );
            new_it->second.addOrder(order);
        }
    } else {
        auto it = asks.find(order.price);
        if (it != asks.end()) {
            it->second.addOrder(order);
        } else {
            auto [new_it, _] = asks.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(order.price),     // key
                std::forward_as_tuple(order.price)      // PriceLevel constructor arg
            );
            new_it->second.addOrder(order);
        }
    }
    order_index[order.order_id] = {order.side, order.price};

    std::cout << "Starting matching !" << std::endl;
    std::vector<FillResult> fills = match();
    return fills;
}

void OrderBook::cancelOrder(int order_id) {
    auto idx_it = order_index.find(order_id);
    if (idx_it == order_index.end())
        return; // nothing to remove

    Side s = idx_it->second.first;
    Price p = idx_it->second.second;
    if (s == Side::BUY) {
        auto pl_it = bids.find(p);
        if (pl_it != bids.end()) {
            try {
                pl_it->second.cancelOrder(order_id);
                if (pl_it->second.isEmpty())
                    bids.erase(pl_it);
            } catch (const std::exception& e) {
                std::cerr << "Exception in cancelOrder: " << e.what() << std::endl;
            }
        }
    } else {
        auto pl_it = asks.find(p);
        if (pl_it != asks.end()) {
            try {
                pl_it->second.cancelOrder(order_id);
                if (pl_it->second.isEmpty())
                    asks.erase(pl_it);
            } catch (const std::exception& e) {
                std::cerr << "Exception in cancelOrder: " << e.what() << std::endl;
            }
        }
    }
    order_index.erase(idx_it);
}

std::vector<FillResult> OrderBook::modifyOrder(int order_id, Order& new_order) {
    auto [side, price] = order_index.at(order_id);
    PriceLevel* p_level_ptr = nullptr;
    if (side == Side::BUY) {
        p_level_ptr = &bids.at(price);
    } else {
        p_level_ptr = &asks.at(price);
    }
    auto& p_level = *p_level_ptr;
    auto& existing = p_level.find(order_id);
    bool price_changed  = new_order.price != existing.price;
    bool qty_decreased  = new_order.quantity < existing.quantity;
    bool qty_increased  = new_order.quantity > existing.quantity;
    bool policy_changed = new_order.tif != existing.tif 
                       || new_order.fill != existing.fill;

    if (price_changed || qty_increased || policy_changed) {
        // full cancel + reinsert as new order
        cancelOrder(order_id);
        addOrder(new_order);  
    } else if (qty_decreased) {
        // in-place modification, preserve position
        int old_qty = existing.quantity;
        existing.quantity = new_order.quantity;
        p_level.setQuantity(p_level.getQuantity() - old_qty + new_order.quantity);
    }
    std::vector<FillResult> fills = match();
    return fills;
}

Price OrderBook::getBestBid() const {
    if (bids.empty())
        throw std::runtime_error("No bids available");
    return bids.begin()->first;
}

Price OrderBook::getBestAsk() const {
    if (asks.empty())
        throw std::runtime_error("No asks available");
    return asks.begin()->first;
}

Price OrderBook::getSpread() const {
    if (bids.empty() || asks.empty())
        throw std::runtime_error("Cannot compute spread on empty book");
    return getBestAsk() - getBestBid();
}

std::vector<FillResult> OrderBook::match() {
    std::vector<FillResult> fills;

    while (!bids.empty() && !asks.empty()) {
        auto bid_it = bids.begin();
        auto ask_it = asks.begin();
        // check if match condition exists
        if (bid_it->first < ask_it->first) break;
        std::cout << "match condition found" << std::endl;

        PriceLevel &bidPL = bid_it->second;
        PriceLevel &askPL = ask_it->second;

        if (bidPL.isEmpty() || askPL.isEmpty()) break; 
        std::cout << "no empty PLs" << std::endl;

        Order &bidOrder = bidPL.front();
        Order &askOrder = askPL.front();
        std::cout << "got orders" << std::endl;
        int bid_id  = bidOrder.order_id;
        int ask_id  = askOrder.order_id;
        int tradeQty = std::min(bidOrder.quantity, askOrder.quantity);
        Price tradePrice = askPL.getPrice();

        std::cout << "Trade executed: qty=" << tradeQty
                  << " price=" << tradePrice << "\n";

        bidOrder.quantity -= tradeQty;
        bidPL.setQuantity(bidPL.getQuantity() - tradeQty);
        askOrder.quantity -= tradeQty;
        askPL.setQuantity(askPL.getQuantity() - tradeQty);

        std::cout << "Removing order" << std::endl;
        if (bidOrder.quantity == 0) {
            order_index.erase(bid_id);
            bidPL.popFront();
        }
        if (askOrder.quantity == 0) {
            order_index.erase(ask_id);
            askPL.popFront();
        }
        std::cout << "finished removing orders" << std::endl;

        if (bidPL.isEmpty()) bids.erase(bid_it);
        if (askPL.isEmpty()) asks.erase(ask_it);
        
        fills.push_back({bid_id, 0, tradeQty, tradePrice}); // buy
        fills.push_back({ask_id, 1, tradeQty, tradePrice}); // sell
    }

    std::cout << "Returning traded price" << std::endl;
    return fills;
}

Order& OrderBook::findOrder(int order_id) {
    auto it = order_index.find(order_id);
    if (it != order_index.end()) {
        if (it->second.first == SELL) {
            auto pl = asks[it->second.second];
            return pl.find(order_id);
        } else {
            auto pl = bids[it->second.second];
            return pl.find(order_id);
        }
    } else {
        throw std::runtime_error("Order ID not found");
    }
}

const Order& OrderBook::findOrder(int order_id) const {
    auto it = order_index.find(order_id);
    if (it != order_index.end()) {
        if (it->second.first == SELL) {
            auto pl = asks.at(it->second.second);
            return pl.find(order_id);
        } else {
            auto pl = bids.at(it->second.second);
            return pl.find(order_id);
        }
    } else {
        throw std::runtime_error("Order ID not found");
    }
}

int OrderBook::checkIfOrderExists(const int order_id) {
    auto it = order_index.find(order_id);
    if (it != order_index.end()) return 1;
    else return 0;
}

void OrderBook::print() const {
    std::cout << "  ASKS (lowest first):\n";
    for (const auto& [price, pl] : asks) {
        pl.print();
    }
    std::cout << "  BIDS (highest first):\n";
    for (const auto& [price, pl] : bids) {
        pl.print();
    }
}

int OrderBook::getRemainingQty(int order_id) const {
    auto& order = findOrder(order_id);
    return order.quantity;
}