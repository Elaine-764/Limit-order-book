#include "../include/lob/exchange.h"
#include <iostream>

std::vector<FillResult> Exchange::addOrder(Order &order) {
    std::string ticker = order.ticker;
    
    if (books.find(ticker) == books.end()) {
        ++num_assets;   // only increment for new tickers
    }
    order_id_to_ticker[order.order_id] = ticker;

    std::vector<FillResult> fills = books[ticker].addOrder(order);
    // std::cout << "Starting match" << std::endl;
    // books[ticker].match();
    std::cout << "Finished match (from exchange)" << std::endl;
    if (!books[ticker].checkIfOrderExists(order.order_id)) {
        order_id_to_ticker.erase(order.order_id);
    }
    std::cout << "added to order_id_to_ticker map" << std::endl;
    return fills;
}

void Exchange::cancelOrder(int order_id) {
    try {
        CheckIfOrderExists(order_id); 
        std::string tic = order_id_to_ticker[order_id];
        books[tic].cancelOrder(order_id);
        order_id_to_ticker.erase(order_id);
    } catch (...) {
        throw std::invalid_argument("unknown order");
    }
}

std::vector<FillResult> Exchange::modifyOrder(int order_id, Order &new_order) {
    try {
        CheckIfOrderExists(order_id); 
        std::string tic = order_id_to_ticker[order_id];
        std::vector<FillResult> fills = books[tic].modifyOrder(order_id, new_order);
        return fills;
        // books[tic].match();
    } catch (...) {
        throw std::invalid_argument("unknown order");
    }
}

bool Exchange::CheckIfOrderExists(const int order_id) {
    auto it = order_id_to_ticker.find(order_id);
    if (it == order_id_to_ticker.end()) return false;
    return true;
}

Price Exchange::getBestAsk(std::string ticker) const {
    auto tic = books.find(ticker);
    if (tic != books.end()) {
        return tic->second.getBestAsk();
    } else {
        throw std::invalid_argument("no orders exist for this ticker");
    }
}

Price Exchange::getBestBid(std::string ticker) const {
    auto tic = books.find(ticker);
    if (tic != books.end()) {
        return tic->second.getBestBid();
    } else {
        throw std::invalid_argument("no orders exist for this ticker");
    }
}

Price Exchange::getSpread(std::string ticker) const {
    auto tic = books.find(ticker);
    if (tic != books.end()) {
        return tic->second.getSpread();
    } else {
        throw std::invalid_argument("no orders exist for this ticker");
    }
}

Order& Exchange::findOrder(const int order_id) {
    try {
        std::string ticker = order_id_to_ticker[order_id];
        auto it = books.find(ticker);
        return it->second.findOrder(order_id);

    } catch (...) {
        throw std::invalid_argument("invalid order id");
    }
}

const Order& Exchange::findOrder(const int order_id) const {
    try {
        std::string ticker = order_id_to_ticker.at(order_id);
        auto it = books.find(ticker);
        return it->second.findOrder(order_id);

    } catch (...) {
        throw std::invalid_argument("invalid order id");
    }
}

void Exchange::print() const {
    std::cout << "=== Exchange (" << books.size() << " tickers) ===\n";
    for (const auto& [ticker, book] : books) {
        std::cout << "--- " << ticker << " ---\n";
        book.print();
    }
    std::cout << "  order_id_to_ticker:\n";
    for (const auto& [id, ticker] : order_id_to_ticker) {
        std::cout << "    " << id << " -> " << ticker << "\n";
    }
    std::cout << "========================\n";
}

int Exchange::getRemainingQty(int order_id) const {
    auto &order = findOrder(order_id);
    return order.quantity;
}