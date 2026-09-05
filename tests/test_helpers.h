// tests/test_helpers.h
#pragma once
#include "../include/data.h"
#include "../include/types.h"
#include <chrono>

inline Order makeOrder(
    int id,
    Side side              = Side::BUY,
    Price price             = Price{100},
    Qty quantity            = Qty{10},
    OrderType type          = OrderType::LIMIT,
    TimeInForce tif         = TimeInForce::DAY,
    FillPolicy fill         = FillPolicy::NORMAL,
    Price stop_price        = Price{0},
    std::uint32_t ticker_id = 1
) {
    auto now = std::chrono::steady_clock::now();
    return Order{
        .order_id   = OrderId{id},
        .ticker_id  = ticker_id,
        .side       = side,
        .type       = type,
        .tif        = tif,
        .fill       = fill,
        .price      = price,
        .stop_price = stop_price,
        .quantity   = quantity,
        .timestamp  = now,
        .expiration = now + std::chrono::hours(24),
    };
}