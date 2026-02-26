#ifndef ___PRICE_LEVEL___H__
#define ___PRICE_LEVEL___H__

#include "order.h"
#include <deque>

class PriceLevel {
    double price;
    std::deque<Order> order_queue;
    int quantity;
  public:
    // add to price level
    // remove from price level

};

#endif