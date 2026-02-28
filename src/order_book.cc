// matching engine, add/cancel/modify logic
#include "lob/order_book.h"

void OrderBook::addOrder(Order& order) {
    // build in place?? - no for this, overkill, could consider for deque
}

void OrderBook::modifyOrder(int order_id, Order& new_order) {

    auto [side, price] = order_index.at(order_id);
    auto& p_level = (new_order.side == Side::BUY) ? bids.at(price) : asks.at(price);
    auto& existing = p_level.find(order_id);
    bool price_changed  = new_order.price != existing.price;
    bool qty_decreased  = new_order.quantity < existing.quantity;
    bool qty_increased  = new_order.quantity > existing.quantity;
    bool policy_changed = new_order.tif != existing.tif 
                       || new_order.fill != existing.fill;

    if (price_changed || qty_increased || policy_changed) {
        // full cancel + reinsert as new order
        removeOrder(order_id, p_level);
        addOrder(new_order);  
    } else if (qty_decreased) {
        // in-place modification, preserve position
        p_level.decreaseQuantity(order_id, new_order.quantity);
    }
}