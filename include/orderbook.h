#pragma once

#include <list>
#include <map>
#include <unordered_map> 
#include "data.h"
#include "types.h"

class Exchange;

class PriceLevel {
    Price price_;
    std::list<Order> orders_;
    Qty total_qty_{0}; // initalize to 0
  public:
    PriceLevel() : price_(0), total_qty_(0) {}
    PriceLevel(Price price) : price_(price), total_qty_(0) {}
    std::list<Order>::iterator addOrder(const Order& order);
    void removeOrder(std::list<Order>::iterator it);
    Price getPrice() const { return price_; }
    Qty getQty() const { return total_qty_; }
    void adjustQty(Qty new_qty) { total_qty_ += new_qty; }
    bool isEmpty() const { return orders_.empty(); } 
    // for viewing FIFO behavior
    const Order& front() const { return orders_.front(); }
    std::list<Order>::iterator begin() { return orders_.begin(); }
    std::list<Order>::iterator end() { return orders_.end(); }
    std::list<Order>::const_iterator cbegin() const { return orders_.cbegin(); }
    std::list<Order>::const_iterator cend() const { return orders_.cend(); }

};

struct OrderLocation {
  Side side;
  PriceLevel* level;
  std::list<Order>::iterator order_it;
};

class OrderBook {
  private:
    std::uint32_t ticker_id;
    std::map<Price, PriceLevel, std::greater<Price>> bids_;
    std::map<Price, PriceLevel, std::less<Price>> asks_;
    std::unordered_map<OrderId, OrderLocation, OrderIdHash> order_index_;
  public:
    OrderBook(std::uint32_t ticker_id): ticker_id(ticker_id) {
      order_index_.reserve(100000);
      order_index_.max_load_factor(0.5);
    }

    auto& getAsks() { return asks_; }
    auto& getBids() { return bids_; }
    void addRestingOrder(Order order, Side side);
    const Order* findOrder(const OrderId order_id) const;
    bool cancel(OrderId order_id, std::vector<ExecutionReport>& reports);
    bool modify(const ModifyRequest& req, std::vector<ExecutionReport>& reports, Exchange& exchange);
    bool removeFilledOrder(OrderId order_id);
    std::uint32_t getTickerId() const { return ticker_id; }

    // for observability
    std::optional<Price> bestBid() const {
      return (bids_.empty()) ? std::nullopt : std::optional{bids_.begin()->first};
    }
    std::optional<Price> bestAsk() const {
      return (asks_.empty()) ? std::nullopt : std::optional{asks_.begin()->first};
    }
    Qty quantityAt(Side side, Price price) const {
      if (side == Side::BUY) {
        auto it = bids_.find(price);
        return (it == bids_.end()) ? Qty{0} : it->second.getQty();
      } else {
        auto it = asks_.find(price);
        return (it == asks_.end()) ? Qty{0} : it->second.getQty();
      }
    }
    bool hasOrder(OrderId id) const {
      return order_index_.contains(id);
    }
    std::size_t bidLevelCount() const { return bids_.size(); }
    std::size_t askLevelCount() const { return asks_.size(); }

    void purgeDayOrders(std::vector<ExecutionReport>& cancellations) noexcept;

};

class StopOrderBook {
  private:
    std::uint32_t ticker_id;
    std::multimap<Price, Order> buy_stops_;
    std::multimap<Price, Order> sell_stops_;
    
    struct StopLocation {
      Side side;
      std::multimap<Price, Order>::iterator it;
    };
    std::unordered_map<OrderId, StopLocation, OrderIdHash> stop_index_;

  public:
    StopOrderBook(std::uint32_t ticker_id): ticker_id(ticker_id) {
      stop_index_.reserve(100000);
      stop_index_.max_load_factor(0.5);
    }
    void add(Order order);
    bool cancel(OrderId order_id, std::vector<ExecutionReport>& reports);
    std::vector<Order> triggerUpTo(Price last_trade_price);
    bool modify(const ModifyRequest& req, std::vector<ExecutionReport>& reports, Exchange& exchange);
    void purgeDayOrders(std::vector<ExecutionReport>& cancellations) noexcept;

    [[nodiscard]] bool contains(OrderId order_id) const noexcept {
      return stop_index_.contains(order_id);
    }

};
