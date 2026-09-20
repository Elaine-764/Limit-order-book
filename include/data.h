#pragma once

#include <chrono>
#include "types.h"

struct Order {
  OrderId order_id;
  std::uint32_t ticker_id;
  Side side;
  OrderType type;
  TimeInForce tif;
  FillPolicy fill;
  Price price;
  Price stop_price;
  Qty quantity;
  std::chrono::steady_clock::time_point timestamp;
  std::chrono::steady_clock::time_point expiration;
};

struct Fill {
  OrderId resting_order_id;
  OrderId incoming_order_id;
  Price price;
  Qty quantity;
  Qty resting_remaining_qty;
  std::chrono::steady_clock::time_point timestamp;
};

struct ExecutionReport {
  OrderId order_id;
  ExecutionType type;
  Qty quantity_remaining;
  Price price;
  std::vector<Fill> fills;

  ExecutionReport(OrderId id, ExecutionType t, Qty q, Price p, std::vector<Fill> f)
    : order_id(id), type(t), quantity_remaining(q), price(p), fills(std::move(f)) {}
};

struct CancelRequest {
  OrderId order_id;
  std::uint32_t ticker_id;
};

struct ModifyRequest {
    OrderId order_id;
    std::uint32_t ticker_id;
    Price new_price;
    Qty new_quantity;
};