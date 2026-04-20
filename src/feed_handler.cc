#include "../include/lob/feed_handler.h"
#include "../include/lob/types.h"
#include "../include/lob/utilities.h"
#include <unistd.h>

Order deserialize(const OrderMessage& msg) {
    // parse input
    if (msg.order_type > 3)   throw std::invalid_argument("unknown order type");
    if (msg.time_in_force > 2) throw std::invalid_argument("unknown time in force");
    if (msg.fill_policy > 1)  throw std::invalid_argument("unknown fill policy");
    if (msg.side > 1)         throw std::invalid_argument("unknown side");
    
    Price stop_price;
    if (msg.order_type == 2) {
        stop_price = msg.stop_value;
    } else if (msg.order_type == 3) {
        double pct = msg.stop_value / 10000.0;
        Price last_price = find_last_price(msg.ticker);
        stop_price = (msg.side == SELL) ? last_price * (1.0 - pct) : last_price * (1.0 + pct);
    }

    return Order {
        static_cast<std::string>(msg.ticker),
        static_cast<Price>(msg.price),
        static_cast<Side>(msg.side),
        static_cast<int>(msg.quantity),
        static_cast<OrderType>(msg.order_type),       // extend later for msg_type variants
        static_cast<TimeInForce>(msg.time_in_force),
        static_cast<FillPolicy>(msg.fill_policy)
    };
}

ExecutionReport buildReport(uint32_t order_id, uint8_t side, char status,
                            int32_t fill_price, uint32_t fill_qty,
                            uint32_t remaining_qty, char tic[]) {
    ExecutionReport report{};
    report.order_id      = order_id;
    report.side          = side;
    report.status        = status;
    report.fill_price    = fill_price;
    report.fill_qty      = fill_qty;
    report.remaining_qty = remaining_qty;
    strncpy(report.ticker, tic, 8);
    return report;
}
