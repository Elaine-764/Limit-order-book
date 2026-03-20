#pragma once
#include "types.h"
#include "order.h"

Order deserialize(const OrderMessage& msg);

ExecutionReport buildReport(uint32_t order_id, uint8_t side, char status, 
                            int32_t fill_price, uint32_t fill_qty,
                            uint32_t remaining_qty, char tic[]);