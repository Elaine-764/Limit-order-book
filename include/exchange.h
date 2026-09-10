#pragma once

#include "engine.h"
#include "orderbook.h"
#include <string>
#include <vector>
#include <map>
#include <unordered_map>

class SymbolTable {
  private:
    std::unordered_map<std::string, std::uint32_t> to_id_;
    std::vector<std::string> to_name_;
    
    // Static empty string to safely return on out-of-bounds errors without creating dangling references on the stack.
    inline static const std::string EMPTY_STRING = "";

  public:
    std::uint32_t intern(const std::string& ticker);
    std::uint32_t intern(std::string&& ticker);
    const std::string& name(std::uint32_t id) const; 
};

class Exchange {
  private:
    std::unordered_map<std::uint32_t, OrderBook> books_;
    std::unordered_map<std::uint32_t, StopOrderBook> stop_books_;
    SymbolTable symbols_;
    MatchingEngine engine_;
  public:
    std::vector<ExecutionReport> submitOrder(Order order);
    std::vector<ExecutionReport> cancelOrder(CancelRequest req);
    std::vector<ExecutionReport> modifyOrder(const ModifyRequest& req);
    std::vector<ExecutionReport> runEndOfDaySession();

};
