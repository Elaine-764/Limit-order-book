#include "../include/exchange.h"
#include "../include/data.h"
#include "../include/types.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>
#include <utility>
#include <stdexcept>
#include <iostream>

std::uint32_t SymbolTable::intern(const std::string& ticker) { 
    auto it = to_id_.find(ticker);
    if (it != to_id_.end()) {
        return it->second;
    }
    if (to_name_.size() >= std::numeric_limits<std::uint32_t>::max()) {
        throw std::runtime_error("Symbol table capacity exceeded.");
    }

    std::uint32_t new_id = static_cast<std::uint32_t>(to_name_.size()); 

    to_name_.push_back(ticker);
    to_id_[to_name_.back()] = new_id; 
    return new_id;
}

// 2. RValue Move Overload
std::uint32_t SymbolTable::intern(std::string&& ticker) {

    auto it = to_id_.find(ticker);
    if (it != to_id_.end()) {
        return it->second;
    }
    if (to_name_.size() >= std::numeric_limits<std::uint32_t>::max()) {
        throw std::runtime_error("Symbol table capacity exceeded.");
    }

    std::uint32_t new_id = static_cast<std::uint32_t>(to_name_.size());
    to_name_.push_back(std::move(ticker)); 
    to_id_[to_name_.back()] = new_id;
    
    return new_id;
}


const std::string& SymbolTable::name(std::uint32_t id) const { 
    if (id >= to_name_.size()) {
        std::cerr << "Warning: Ticker ID is invalid." << std::endl;
        return EMPTY_STRING; 
    }
    return to_name_[id];
}

std::vector<ExecutionReport> Exchange::cancelOrder(CancelRequest req) {
    std::vector<ExecutionReport> reports;

    // verify ownership via global registry
    auto registry_it = global_order_registry_.find(req.order_id);
    if (registry_it == global_order_registry_.end()) {
        reports.push_back(ExecutionReport{req.order_id, ExecutionType::REJECTED, Qty{0}, Price{0}, {}});
        return reports;
    }

    OrderMetaData& meta = registry_it->second;
    // check if ticker id matches
    if (meta.ticker_id != req.ticker_id) {
        std::cerr << "WARNING: Malicious or corrupt cancel attempt. Order ID " 
                  << req.order_id.get() << " belongs to Ticker " << meta.ticker_id 
                  << ", but client requested Ticker " << req.ticker_id << ".\n";
        reports.push_back(ExecutionReport{req.order_id, ExecutionType::REJECTED, Qty{0}, Price{0}, {}});
        return reports;
    }
    
    // check if order is a stop/stop limit order
    if (meta.type == OrderType::STOP || meta.type == OrderType::STOP_LIMIT) {
        auto book_it = stop_books_.find(req.ticker_id);
        if (book_it == stop_books_.end()) {
            reports.push_back(ExecutionReport {req.order_id, ExecutionType::REJECTED, Qty{0}, Price{0}, std::vector<Fill>{}});
            return reports;
        }
        StopOrderBook& book = book_it->second;
        if (book.cancel(req.order_id, reports)) {
            global_order_registry_.erase(registry_it);
        } else {
            reports.push_back(ExecutionReport {req.order_id, ExecutionType::REJECTED, Qty{0}, Price{0}, std::vector<Fill>{}});
        }
    } else {
        auto book_it = books_.find(req.ticker_id);
        if (book_it == books_.end()) {
            reports.push_back(ExecutionReport {req.order_id, ExecutionType::REJECTED, Qty{0}, Price{0}, std::vector<Fill>{}});
            return reports;
        }
        OrderBook& book = book_it->second;
        if (book.cancel(req.order_id, reports)) {
            global_order_registry_.erase(registry_it);
        } else {
            reports.push_back(ExecutionReport {req.order_id, ExecutionType::REJECTED, Qty{0}, Price{0}, std::vector<Fill>{}});
        }
    }
   
    return reports;
}

std::vector<ExecutionReport> Exchange::submitOrder(Order order) {
    std::vector<ExecutionReport> reports;

    // Global Pre-Trade Validation
    if ((order.quantity <= 0 || // invalid quantity
        (order.type != OrderType::MARKET && order.type != OrderType::STOP &&
         order.price <= 0) ||
        ((order.type == OrderType::STOP || order.type == OrderType::STOP_LIMIT) &&
         order.stop_price <= 0)) || 
       (order.side != Side::BUY && order.side != Side::SELL) ||  // invalid side
       (order.stop_price <= 0 && order.type == OrderType::STOP) || // stop orders without trigger price 
       (order.type == OrderType::STOP_LIMIT && (order.stop_price <= 0 || order.price <= 0)))  // stop limit orders without both trigger + limit price
    { 
        reports.push_back(ExecutionReport{order.order_id, ExecutionType::REJECTED, Qty{0}, Price{0}, {}});
        return reports;
    }

    auto [book_it, book_inserted] = books_.try_emplace(order.ticker_id, order.ticker_id);
    auto [stop_book_it, stop_inserted] = stop_books_.try_emplace(order.ticker_id, order.ticker_id);

    reports = engine_.submit(order, book_it->second, stop_book_it->second);

    for (const auto& report : reports) {
        for (const auto& fill : report.fills) {
            if (!book_it->second.hasOrder(fill.resting_order_id)) {
                global_order_registry_.erase(fill.resting_order_id);
            }
        }
    }

    const bool is_stop = order.type == OrderType::STOP || order.type == OrderType::STOP_LIMIT;
    const bool remains_live = is_stop
        ? stop_book_it->second.contains(order.order_id)
        : book_it->second.hasOrder(order.order_id);

    if (remains_live) {
        global_order_registry_[order.order_id] = OrderMetaData{order.ticker_id, order.type};
        if (reports.empty()) {
            reports.emplace_back(
                order.order_id,
                is_stop ? ExecutionType::DORMANT : ExecutionType::NEW,
                order.quantity,
                order.price,
                std::vector<Fill>{}
            );
        }
    } else {
        global_order_registry_.erase(order.order_id);
    }

    return reports;
}

std::vector<ExecutionReport> Exchange::modifyOrder(const ModifyRequest& req) {
    std::vector<ExecutionReport> reports;

    // Global Pre-Trade Validation  - valid prices and quantity
    if (req.new_quantity <= 0 || req.new_price <= 0) {
        reports.push_back(ExecutionReport{req.order_id, ExecutionType::REJECTED, Qty{0}, Price{0}, {}});
        return reports;
    }

    // verify ownership via global registry
    auto registry_it = global_order_registry_.find(req.order_id);
    if (registry_it == global_order_registry_.end()) {
        reports.push_back(ExecutionReport{req.order_id, ExecutionType::REJECTED, Qty{0}, Price{0}, {}});
        return reports;
    }

    OrderMetaData& meta = registry_it->second;
    // check if ticker id matches
    if (meta.ticker_id != req.ticker_id) {
        std::cerr << "WARNING: Malicious or corrupt modification attempt. Order ID " 
                  << req.order_id.get() << " belongs to Ticker " << meta.ticker_id 
                  << ", but client requested Ticker " << req.ticker_id << ".\n";
        reports.push_back(ExecutionReport{req.order_id, ExecutionType::REJECTED, Qty{0}, Price{0}, {}});
        return reports;
    }
    
    // check if order is a stop/stop limit order
    if (meta.type == OrderType::STOP || meta.type == OrderType::STOP_LIMIT) {
        auto book_it = stop_books_.find(req.ticker_id);
        if (book_it == stop_books_.end()) {
            reports.push_back(ExecutionReport {req.order_id, ExecutionType::REJECTED, Qty{0}, Price{0}, std::vector<Fill>{}});
            return reports;
        }
        StopOrderBook& stop_book = book_it->second;
        bool success = stop_book.modify(req, reports, *this);
        if (!success) {
            reports.push_back(ExecutionReport{req.order_id, ExecutionType::REJECTED, Qty{0}, Price{0}, {}});
        }
    } else {
        auto book_it = books_.find(req.ticker_id);
        if (book_it == books_.end()) {
            reports.push_back(ExecutionReport {req.order_id, ExecutionType::REJECTED, Qty{0}, Price{0}, std::vector<Fill>{}});
            return reports;
        }
        OrderBook& book = book_it->second;
        bool success = book.modify(req, reports, *this);
        if (!success) {
            reports.push_back(ExecutionReport{req.order_id, ExecutionType::REJECTED, Qty{0}, Price{0}, {}});
        }
    }
    
    return reports;
}

std::vector<ExecutionReport> Exchange::runEndOfDaySession() {
    std::vector<ExecutionReport> cancellations;
    cancellations.reserve(1024); 

    for (auto& [ticker_id, book] : books_) {
        book.purgeDayOrders(cancellations);
    }
    for (auto& [ticker_id, stop_book] : stop_books_) {
        stop_book.purgeDayOrders(cancellations);
    }
    for (const auto& report : cancellations) {
        global_order_registry_.erase(report.order_id);
    }
    return cancellations;
    
}