#pragma once

#include "engine.h"
#include <string>
#include <vector>
#include <map>

class SymbolTable {
  private:
    std::unordered_map<std::string, std::uint32_t> to_id_;
    std::vector<std::string> to_name;
  public:
    std::uint32_t intern(const std::string& ticker);
    const std::string& name(std::utin32_t id) const;
};

class Exchange {
  private:
    std::unordered_map<std::uint32_t, OrderBook> books_;
    std::unordered_map<std::uint32_t, StopOrderBook> stop_books_;
    SymbolTable symbols_;
    MatchingEngine engine_;
  public:
    std::vector<Fill> submitOrder(Order);
};
