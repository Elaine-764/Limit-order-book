#ifndef ___ORDER___H__
#define ___ORDER___H__

#include "types.h"

class Order {
    int order_id;
    Side side;
    double price;
    int quantity;
    std::chrono::stead_clock::time_point timestamp;

  public:
    // methods - shouldn't be many, except maybe fetching fields
};

#endif