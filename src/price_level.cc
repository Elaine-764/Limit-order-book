#include "lob/price_level.h"
#include "lob/order.h"
#include <algorithm>
#include <deque>
#include <stdexcept>
#include <format>
#include <string>
#include <iostream>

// price level method definitions
// add to price level
void PriceLevel::addOrder(Order& order) {
    // append to deque
    order_queue.push_back(order);

    // update price level quantity
    setQuantity(getQuantity() + order.quantity);
}

// remove from price level
void PriceLevel::removeOrder(int id) {
    auto it = std::find_if(order_queue.begin(), order_queue.end(), [id](const Order& order) {
        return order.order_id == id;
    });
    if (it != order_queue.end()) {
        // Update quantity before erasing
        setQuantity(getQuantity() - it->quantity);
        order_queue.erase(it);
    } else {
        throw std::invalid_argument(std::format("Order ID {} does not exist.", id));
    }
}

void PriceLevel::modifyOrder(int id, const Order& new_order) {

    Price new_price = new_order.price;
    int new_quantity = new_order.quantity;
    auto it = std::find_if(order_queue.begin(), order_queue.end(), [id](const Order& order) {
        return order.order_id == id;
    });
    int old_quantity = it->quantity;
    if (it != order_queue.end()) {
        if (new_price != it->price) {
            // remove order
            setQuantity(getQuantity() - old_quantity);
            order_queue.erase(it);
            throw WrongPriceException("Price change.");
        } else if (new_quantity == old_quantity) {
            return;
        } else if (new_quantity < old_quantity) {
            setQuantity(getQuantity() - old_quantity + new_quantity);
            it->quantity = new_quantity;
        } else {
            setQuantity(getQuantity() - old_quantity + new_quantity);
            order_queue.erase(it);
            order_queue.push_back(new_order);
        }
    } else {
        throw std::invalid_argument(std::format("Order ID {} does not exist.", id));
    }
}

void PriceLevel::decreaseQuantity(int order_id, int new_quantity) {
    auto it = std::find_if(order_queue.begin(), order_queue.end(), [order_id](const Order& order) {
        return order.order_id == order_id;
    });
    it->quantity = new_quantity;
    return;
}

Order& PriceLevel::find(int order_id) {
    auto it = std::find_if(order_queue.begin(), order_queue.end(), [order_id](const Order& order) {
        return order.order_id == order_id;
    });
    if (it != order_queue.end()) {
        return *it;
    } else {
        throw std::invalid_argument(std::format("Order ID {} does not exist.", std::to_string(order_id)));
    }
}