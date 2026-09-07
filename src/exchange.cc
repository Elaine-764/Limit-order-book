#include "../include/exchange.h"
#include "../include/data.h"
#include "../include/types.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>
#include <utility>
#include <stdexcept>

std::uint32_t SymbolTable::intern(const std::string& ticker);
    auto it = to_id_.find(ticker);
    if (it != to_id_.end()) {
        return it->second;
    }
    if (to_name_.size() >= std::numeric_limits<std::uint32_t>::max()) {
        throw std::runtime_error("Symbol table capacity exceeded.");
    }

    std::uint32_t new_id = static_cast<std::uint32_t>(to_name.size())

    to_name_.push_back(ticker);
    to_id_[to_name.back()] = new_id;
    return new_id;
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

const std::string& SymbolTable::name(std::utin32_t id) {
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
    bool success = book.cancel(req.order_id);
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
    std::vector<ExecutionReport> reports.
    auto& book = books_.find(order.ticker_id);
    auto& stop_book = stop_books_.find(order.ticker_id);
    if (book == books_.end() || stop_book == stop_books_.end()) {
        reports.push_back(ExecutionReport{order.order_id, ExecutionType::REJECTED, Qty{0}, Price{0}, {}});
        return reports
    }

    return engine_.submit(order, book, stop_book);
}

std::vector<ExecutionReport> Exchange::modifyOrder(const ModifyRequest& req) {
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
    // first check if new order is the same as existing order
    const Order* exising_order = book.findOrder(req.order_id);
    if (!existing_order) {
        reports.push_back(ExecutionReport{req.order_id, ExecutionType::REJECTED, Qty{0}, Price{0}, {}});
        return reports;
    }
    if (existing_order->price == req.new_price && existing_order->quantity == req.new_quantity) {
        reports.push_back(ExecutionReport{req.order_id, ExecutionType::REJECTED, Qty{0}, Price{0}, {}});
        return reports;
    }

    bool success = book.modify(req, reports, engine_, stop_books_[req.ticker_id]);
    if (!success) {
        reports.push_back(ExecutionReport{req.order_id, ExecutionType::REJECTED, Qty{0}, Price{0}, {}});
    }
    return reports;
}