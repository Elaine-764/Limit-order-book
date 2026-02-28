#ifndef ___ORDER_BOOK___H__
#define ___ORDER_BOOK___H__

#include "lob/price_level.h"
#include <unordered_map>
#include <map>

class OrderBook {
    std::map<Price, PriceLevel, std::greater<Price>> bids;
    std::map<Price, PriceLevel, std::less<Price>> asks;
    std::unordered_map<Price, std::pair<Side, Price>> order_index;

  public:
    // add/remove order
    void addOrder(Order &order);
    void removeOrder(int order_id, PriceLevel& p_level);
    void modifyOrder(int order_id, Order& new_order);

    // bid ask spread
    Price getSpread();
    
    // make transaction 
    void match();
    // a lot more methods
};

#endif