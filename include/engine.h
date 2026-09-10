#pragma once

#include "data.h"
#include <vector>

class OrderBook;
class StopOrderBook;

class MatchingEngine {
  public:
    std::vector<ExecutionReport> submit(Order order, OrderBook& book, StopOrderBook& stop_book);
  private:
    std::vector<Fill> matchMarket(Order& order, OrderBook& book);
    std::vector<Fill> matchLimit(Order& order, OrderBook& book);
};
