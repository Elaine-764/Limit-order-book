#pragma once
#include "order_book.h"
#include <stdexcept>

class Exchange {
    std::unordered_map<std::string, OrderBook> books;
    std::unordered_map<int, std::string> order_id_to_ticker;
    int num_assets;
  public:
    Exchange(): books {std::unordered_map<std::string, OrderBook> {}}, num_assets {0} {} 

    int getNumAssets() { return num_assets; }
    std::vector<FillResult> addOrder(Order &order);
    void cancelOrder(int order_id);
    std::vector<FillResult> modifyOrder(int order_id, Order &new_order);

    Price getBestBid(std::string ticker) const;
    Price getBestAsk(std::string ticker) const;
    
    Price getSpread(std::string ticker) const;

    Order& findOrder(const int order_id);
    const Order& findOrder(const int order_id) const;

    bool CheckIfOrderExists(const int order_id) const;
    int getRemainingQty(int order_id) const;

    void print() const;
};