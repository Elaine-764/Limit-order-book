#pragma once

#include "orderbook.h"
#include <vector>

class MatchingEngine {
  public:
    std::vector<Fill> submit(Order order, OrderBook& book, StopOrderBook& stop_book);
  private:
    std::vector<Fill> matchMarket(Order& order, OrderBook& book);
    std::vector<Fill> matchLimit(Order& order, OrderBook& book);
    void matchStop(Order& order, StopOrderBook& book);
    void matchStopLimit(Order& order, StopOrderBook& book);
};
