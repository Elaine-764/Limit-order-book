#include "lob/order_book.h"
#include <iostream>

int main() {
    OrderBook book;
    std::cout << "Limit Order Book Tests\n";
    
    // Test addOrder
    Order buy_order(100, Side::BUY, 10, OrderType::LIMIT, TimeInForce::DAY, FillPolicy::NORMAL);
    book.addOrder(buy_order);
    std::cout << "Added buy order\n";
    
    Order sell_order(99, Side::SELL, 5, OrderType::LIMIT, TimeInForce::DAY, FillPolicy::NORMAL);
    book.addOrder(sell_order);
    std::cout << "Added sell order\n";
    
    // Run matching
    book.match();
    std::cout << "Matching complete\n";
    
    return 0;
}
