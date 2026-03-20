#ifndef __LOB_TYPES_H__
#define __LOB_TYPES_H__

#include <cstdint>
#include <string>
#include <stdexcept>

enum Side { BUY = 0, SELL = 1};

enum OrderType { MARKET = 0, LIMIT, STOP, STOP_LIMIT };

enum TimeInForce { NONE = 0, DAY, GOOD_TIL_CANCELLED, FILL_OR_KILL };

enum FillPolicy { NORMAL, ALL_OR_NONE };

using Price = int32_t;

class WrongPriceException : public std::runtime_error {
public:
    explicit WrongPriceException(const std::string& message)
        : std::runtime_error(message) {}
};

// Client input
struct __attribute__((packed)) OrderMessage {
    uint32_t order_id;     // null or 0 if order type is "A" for add
    uint8_t  msg_type;      // 'A' add, 'C' cancel, 'M' modify
    uint8_t  side;          // 0 = BUY, 1 = SELL
    int32_t  price;         // integer ticks, ignored for MARKET orders
    int32_t  stop_value;      // if fixed: tick price
                              // if percentage: e.g. 500 = 5.00%
    uint32_t quantity;
    uint8_t  order_type;    // 0 = MARKET, 1 = LIMIT, 2 = STOP, 3 = STOP_LIMIT
    uint8_t  time_in_force; // 0 = DAY, 1 = GTC, 2 = FOK
    uint8_t  fill_policy;   // 0 = NORMAL, 1 = FOK, 2 = AON
    char     ticker[8];
};

// LOB output
struct __attribute__((packed)) ExecutionReport {
    uint32_t order_id;
    uint8_t  side; // 0 - buy, 1 - sell
    uint8_t  status;      // 'F' filled, 'P' partial, 'A' acknowledged
    int32_t  fill_price;
    uint32_t fill_qty;
    uint32_t remaining_qty;
    char     ticker[8]; 
};

struct FillResult {
    int     bid_id;
    int     ask_id;
    int     qty;
    Price   price;
};


#endif