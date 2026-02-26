#ifndef ___ORDER_BOOK___H__
#define ___ORDER_BOOK___H__

#include <price_level.h>

class OrderBook {
    std:map<double, PriceLevel, std::greater<double>> bids;
    std:map<double, PriceLevel, std::greater<double>> asks;
    std::unordered_map<int, std::pair<Side, double>> orders;

  public:
    // add/remove order
    // bid ask spread
    // make transaction 
    // a lot more methods
}

#endif