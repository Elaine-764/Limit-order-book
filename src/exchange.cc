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

    auto book_it = books_.find(req.ticker_id);
    if (book_it == books_.end()) {
        reports.push_back(ExecutionReport {
            req.order_id, 
            ExecutionType::REJECTED,
            Qty{0},
            Price{0},
            std::vector<Fill>{}
        });
        return reports;
    }

    OrderBook& book = book_it->second;
    bool success = book.cancel(req.order_id, reports);
    if (success) return reports;

    reports.push_back(ExecutionReport {
            req.order_id, 
            ExecutionType::REJECTED,
            Qty{0},
            Price{0},
            std::vector<Fill>{}
        });
    return reports;
}

std::vector<ExecutionReport> Exchange::submitOrder(Order order) {
    std::vector<ExecutionReport> reports;

    // Global Pre-Trade Validation
    if (order.quantity <= 0 ||
        (order.type != OrderType::MARKET && order.type != OrderType::STOP &&
         order.price <= 0) ||
        ((order.type == OrderType::STOP || order.type == OrderType::STOP_LIMIT) &&
         order.stop_price <= 0)) {
        reports.push_back(ExecutionReport{order.order_id, ExecutionType::REJECTED, Qty{0}, Price{0}, {}});
        return reports;
    }

    auto [book_it, book_inserted] = books_.try_emplace(order.ticker_id);
    auto [stop_book_it, stop_inserted] = stop_books_.try_emplace(order.ticker_id);

    return engine_.submit(order, book_it->second, stop_book_it->second);
}

std::vector<ExecutionReport> Exchange::modifyOrder(const ModifyRequest& req) {
    std::vector<ExecutionReport> reports;
    // Global Pre-Trade Validation
    if (req.new_quantity <= 0 || req.new_price <= 0) {
        reports.push_back(ExecutionReport{req.order_id, ExecutionType::REJECTED, Qty{0}, Price{0}, {}});
        return reports;
    }

    auto book_it = books_.find(req.ticker_id);
    if (book_it == books_.end()) {
        reports.push_back(ExecutionReport {
            req.order_id, ExecutionType::REJECTED, Qty{0}, Price{0}, {}
        });
        return reports;
    }

    OrderBook& limit_book = book_it->second;
    StopOrderBook& stop_book = stop_books_[req.ticker_id];

    if (limit_book.hasOrder(req.order_id)) {
        bool success = limit_book.modify(req, reports, *this);
        if (!success) {
            reports.push_back(ExecutionReport{req.order_id, ExecutionType::REJECTED, Qty{0}, Price{0}, {}});
        }
    } else if (stop_book.contains(req.order_id)) {
        bool success = stop_book.modify(req, reports, *this);
        if (!success) {
            reports.push_back(ExecutionReport{req.order_id, ExecutionType::REJECTED, Qty{0}, Price{0}, {}});
        }
    } else {
        reports.push_back(ExecutionReport{req.order_id, ExecutionType::REJECTED, Qty{0}, Price{0}, {}});
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
    return cancellations;
    
}