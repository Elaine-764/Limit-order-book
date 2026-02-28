#ifndef ___ORDER___H__
#define ___ORDER___H__

#include "lob/types.h"
#include <chrono>

struct Order {
    int order_id;
    Side side;
    Price price;
    Price stop_price;
    OrderType order_type; // market vs limit vs stop vs stop-limit
    TimeInForce tif; // normal expiration, day, good 'til cancelled
    FillPolicy fill; // normal, fill or kill, all or none
    int quantity;
    std::chrono::steady_clock::time_point timestamp;
    std::chrono::steady_clock::time_point expiration;
  
    Order(Price price, Side side, int quantity, 
          OrderType type, TimeInForce tif, FillPolicy fill, 
          Price stop_price = 0)
        : order_id{next_id++}
        , price{price}
        , stop_price{stop_price}
        , side{side}
        , order_type{type}
        , tif{tif}
        , fill{fill}
        , quantity{quantity}
        , timestamp{std::chrono::steady_clock::now()}
    {}
  private:
    static int next_id;

};

#endif