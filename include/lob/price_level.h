#ifndef ___PRICE_LEVEL___H__
#define ___PRICE_LEVEL___H__

#include "order.h"
#include <deque>

class PriceLevel {
    Price price;
    std::deque<Order> order_queue;
    int quantity;
  public:
    // Default constructor
    PriceLevel() : price{0}, quantity{0} {} 
    PriceLevel(Price p) : price {p}, quantity {0} {}

    // add to price level
    void addOrder(Order& order);

    // remove from price level
    void cancelOrder(int id);
    void modifyOrder(int order_id, const Order& new_order);
    void decreaseQuantity(int order_id, int new_quantity);

    int getQuantity() {return quantity;}
    void setQuantity(int n) {quantity = n;}

    Price getPrice() {return price;}

    Order& front();
    Order& find(int order_id);
    void popFront();
    bool isEmpty() const;

    void print() const;
};

#endif