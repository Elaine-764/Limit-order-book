#ifndef ___PRICE_LEVEL___H__
#define ___PRICE_LEVEL___H__

#include "lob/order.h"
#include <deque>

class PriceLevel {
    Price price;
    std::deque<Order> order_queue;
    int quantity;
  public:
    // add to price level
    void addOrder(Order& order);

    // remove from price level
    void removeOrder(int id);
    void modifyOrder(int order_id, const Order& new_order);
    void decreaseQuantity(int order_id, int new_quantity);

    int getQuantity() {return quantity;}
    void setQuantity(int n) {quantity = n;}

    Price getPrice() {return price;}

    Order& front();
    Order& find(int order_id);
    void popFront();
    bool isEmpty() {return quantity == 0;}
};

#endif