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

    // First, check and trigger any stop orders
    triggerStopOrders();

    // Process all orders that can potentially match
    while (!bids.empty() && !asks.empty()) {
        auto bid_it = bids.begin();
        auto ask_it = asks.begin();
        
        // Get the best bid and ask prices
        Price best_bid = bid_it->first;
        Price best_ask = ask_it->first;
        
        // Check if there's a cross (bid >= ask)
        if (best_bid < best_ask) break;
        
        PriceLevel &bidPL = bid_it->second;
        PriceLevel &askPL = ask_it->second;

        if (bidPL.isEmpty() || askPL.isEmpty()) break;

        Order &bidOrder = bidPL.front();
        Order &askOrder = askPL.front();
        
        // Check fill policies - AON orders can only match if entire quantity can be filled
        if (bidOrder.fill == FillPolicy::ALL_OR_NONE && bidOrder.quantity > askPL.getQuantity()) {
            continue; // Skip this bid order
        }
        if (askOrder.fill == FillPolicy::ALL_OR_NONE && askOrder.quantity > bidPL.getQuantity()) {
            continue; // Skip this ask order
        }

        int bid_id = bidOrder.order_id;
        int ask_id = askOrder.order_id;
        
        // Determine trade quantity
        int tradeQty = std::min(bidOrder.quantity, askOrder.quantity);
        
        // For AON orders, trade quantity must be the full order quantity
        if (bidOrder.fill == FillPolicy::ALL_OR_NONE) {
            tradeQty = bidOrder.quantity;
        }
        if (askOrder.fill == FillPolicy::ALL_OR_NONE) {
            tradeQty = askOrder.quantity;
        }
        
        // Determine trade price based on order types
        Price tradePrice;
        if (bidOrder.order_type == OrderType::MARKET || askOrder.order_type == OrderType::MARKET) {
            // Market orders execute at the best available price
            tradePrice = (bidOrder.order_type == OrderType::MARKET) ? best_ask : best_bid;
        } else {
            // Limit orders execute at the best available price (price improvement)
            tradePrice = (bidOrder.price >= askOrder.price) ? askOrder.price : bidOrder.price;
        }

        std::cout << "Trade executed: qty=" << tradeQty << " price=" << tradePrice << "\n";

        // Update quantities
        bidOrder.quantity -= tradeQty;
        bidPL.setQuantity(bidPL.getQuantity() - tradeQty);
        askOrder.quantity -= tradeQty;
        askPL.setQuantity(askPL.getQuantity() - tradeQty);

        // Remove filled orders
        if (bidOrder.quantity == 0) {
            order_index.erase(bid_id);
            bidPL.popFront();
        }
        if (askOrder.quantity == 0) {
            order_index.erase(ask_id);
            askPL.popFront();
        }

        // Remove empty price levels
        if (bidPL.isEmpty()) bids.erase(bid_it);
        if (askPL.isEmpty()) asks.erase(ask_it);
        
        fills.push_back({bid_id, 0, tradeQty, tradePrice}); // buy
        fills.push_back({ask_id, 1, tradeQty, tradePrice}); // sell
    }

    // Handle FOK orders - remove any unfilled FOK orders
    std::vector<int> fok_orders_to_remove;
    for (auto& [price, pl] : bids) {
        for (auto& order : pl.getOrderQueue()) {
            if (order.tif == TimeInForce::FILL_OR_KILL && order.quantity > 0) {
                fok_orders_to_remove.push_back(order.order_id);
            }
        }
    }
    for (auto& [price, pl] : asks) {
        for (auto& order : pl.getOrderQueue()) {
            if (order.tif == TimeInForce::FILL_OR_KILL && order.quantity > 0) {
                fok_orders_to_remove.push_back(order.order_id);
            }
        }
    }
    
    for (int order_id : fok_orders_to_remove) {
        cancelOrder(order_id);
        // FOK orders that don't fill are cancelled and reported to client via server
        // Server checks if order exists after addOrder; if FOK and doesn't exist with no fills, sends cancellation report
    }

    std::cout << "Returning traded price" << std::endl;
    return fills;
}

void OrderBook::triggerStopOrders() {
    if (bids.empty() && asks.empty()) return;
    
    Price best_bid = bids.empty() ? 0 : bids.begin()->first;
    Price best_ask = asks.empty() ? INT32_MAX : asks.begin()->first;
    
    // Check buy stop orders (stop-buy becomes market buy when price drops to stop level)
    std::vector<std::pair<Price, int>> stop_orders_to_trigger;
    for (auto& [price, pl] : bids) {
        for (auto& order : pl.getOrderQueue()) {
            if (order.order_type == OrderType::STOP && order.side == Side::BUY) {
                // Buy stop: trigger when best ask <= stop price
                if (!asks.empty() && best_ask <= order.stop_price) {
                    stop_orders_to_trigger.push_back({price, order.order_id});
                }
            } else if (order.order_type == OrderType::STOP_LIMIT && order.side == Side::BUY) {
                // Buy stop-limit: trigger when best ask <= stop price, then becomes limit order
                if (!asks.empty() && best_ask <= order.stop_price) {
                    stop_orders_to_trigger.push_back({price, order.order_id});
                }
            }
        }
    }
    
    // Check sell stop orders (stop-sell becomes market sell when price rises to stop level)
    for (auto& [price, pl] : asks) {
        for (auto& order : pl.getOrderQueue()) {
            if (order.order_type == OrderType::STOP && order.side == Side::SELL) {
                // Sell stop: trigger when best bid >= stop price
                if (!bids.empty() && best_bid >= order.stop_price) {
                    stop_orders_to_trigger.push_back({price, order.order_id});
                }
            } else if (order.order_type == OrderType::STOP_LIMIT && order.side == Side::SELL) {
                // Sell stop-limit: trigger when best bid >= stop price, then becomes limit order
                if (!bids.empty() && best_bid >= order.stop_price) {
                    stop_orders_to_trigger.push_back({price, order.order_id});
                }
            }
        }
    }
    
    // Trigger the stop orders
    for (auto& [old_price, order_id] : stop_orders_to_trigger) {
        auto& order = findOrder(order_id);
        if (order.order_type == OrderType::STOP) {
            // Convert to market order
            order.order_type = OrderType::MARKET;
            order.price = 0; // Market orders don't have a specific price
        } else if (order.order_type == OrderType::STOP_LIMIT) {
            // Convert to limit order at the stop price
            order.order_type = OrderType::LIMIT;
            order.price = order.stop_price;
        }
        // Move to appropriate price level if needed
        if (order.order_type == OrderType::MARKET) {
            // Market orders go to the best available price level
            Price new_price = (order.side == Side::BUY) ? best_ask : best_bid;
            if (new_price != old_price) {
                // Remove from old price level and add to new one
                cancelOrder(order_id);
                order.price = new_price;
                addOrder(order);
            }
        }
    }
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