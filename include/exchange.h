#pragma once

#include "engine.h"
#include "orderbook.h"
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <optional>

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

    struct OrderMetaData {
        uint32_t ticker_id;
        OrderType type; // Useful to differentiate STOP vs LIMIT instantly
    };
    std::unordered_map<OrderId, OrderMetaData, OrderIdHash> global_order_registry_;

  public:
    std::vector<ExecutionReport> submitOrder(Order order);
    std::vector<ExecutionReport> cancelOrder(CancelRequest req);
    std::vector<ExecutionReport> modifyOrder(const ModifyRequest& req);
    std::vector<ExecutionReport> runEndOfDaySession();
    std::optional<Price> bestBid(uint32_t ticker_id) {
      auto book_it = books_.find(ticker_id);
      if (book_it == books_.end()) return std::nullopt;
      return book_it->second.bestBid();
    }
    std::optional<Price> bestAsk(uint32_t ticker_id) {
      auto book_it = books_.find(ticker_id);
      if (book_it == books_.end()) return std::nullopt;
      return book_it->second.bestAsk();
    }

    // for testing
    std::optional<Order> getFirstBuyStop(uint32_t ticker_id) const noexcept {
      auto book_it = stop_books_.find(ticker_id);
      if (book_it == stop_books_.end()) return std::nullopt;
      return book_it->second.getTopBuyStop();
    }
    std::optional<Order> getFirstSellStop(uint32_t ticker_id) const noexcept {
      auto book_it = stop_books_.find(ticker_id);
      if (book_it == stop_books_.end()) return std::nullopt;
      return book_it->second.getTopSellStop();
    }
    Qty getQty(uint32_t ticker_id, Side side, Price price) const noexcept {
      auto book_it = books_.find(ticker_id);
      if (book_it == books_.end()) return Qty{0};
      return book_it->second.quantityAt(side, price);
    }
    bool orderExists(uint32_t ticker_id, OrderId order_id) const noexcept {
      auto order_it = global_order_registry_.find(order_id);
      if (order_it == global_order_registry_.end()) return false;
      if (order_it->second.ticker_id != ticker_id) return false;
      return true;
    }

    std::size_t bidLevelCount(uint32_t ticker_id) const noexcept {
      auto book_it = books_.find(ticker_id);
      if (book_it == books_.end()) return 0;
      return book_it->second.bidLevelCount();
    }
    std::size_t askLevelCount(uint32_t ticker_id) const noexcept {
      auto book_it = books_.find(ticker_id);
      if (book_it == books_.end()) return 0;
      return book_it->second.askLevelCount();
    }

};
