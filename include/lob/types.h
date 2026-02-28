#ifndef __LOB_TYPES_H__
#define __LOB_TYPES_H__

#include <cstdint>

enum Side { BUY = 0, SELL = 1};

enum OrderType { MARKET = 0, LIMIT, STOP, STOP_LIMIT };

enum TimeInForce { NONE = 0, DAY, GOOD_TIL_CANCELLED };

enum FillPolicy { NORMAL, FILL_OR_KILL, ALL_OR_NONE };

struct OrderMessage {
    uint8_t  msg_type;    // 'A' add, 'C' cancel, 'M' modify
    uint32_t order_id;
    uint8_t  side;
    int32_t  price;       // integer ticks
    uint32_t quantity;
} __attribute__((packed)); // ensure each message is the same number of bytes

using Price = int32_t;

class WrongPriceException : public std::runtime_error {
public:
    explicit WrongPriceException(const std::string& message)
        : std::runtime_error(message) {}
};


#endif