#ifndef ___ORDER_BOOK___H__
#define ___ORDER_BOOK___H__

#include "price_level.h"
#include <unordered_map>
#include <map>
#include <stdexcept>

class OrderBook {
    std::string ticker;
    std::map<Price, PriceLevel, std::greater<Price>> bids;
    std::map<Price, PriceLevel, std::less<Price>> asks;
    std::unordered_map<int, std::pair<Side, Price>> order_index;

  public:
    // add/cancel order
    std::vector<FillResult> addOrder(Order &order);
    void cancelOrder(int order_id);
    std::vector<FillResult> modifyOrder(int order_id, Order& new_order);

    // bid/ask helpers
    Price getBestBid() const;
    Price getBestAsk() const;
    // bid ask spread
    Price getSpread() const;
    
    // matching engine
    std::vector<FillResult> match();

    // a lot more methods
    Order& findOrder(int order_id);
    const Order& findOrder(int order_id) const;

    int checkIfOrderExists(const int order_id);
    int getRemainingQty(int order_id) const;

    void print() const;
    
};

#endif