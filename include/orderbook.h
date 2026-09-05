#pragma once

#include <list>
#include <map>
#include <unordered_map> 
#include "data.h"
#include "types.h"

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
    bool isEmpty() const { return orders_.empty(); } 
    // for viewing FIFO behavior
    const Order& front() const { return orders_.front(); }
    std::list<Order>::const_iterator begin() const { return orders_.cbegin(); }
    std::list<Order>::const_iterator end() const { return orders_.cend(); }

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
    void addRestingOrder(Order order, Side side);
    bool cancel(OrderId order_id);
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
};

class StopOrderBook {
  private:
    std::multimap<Price, Order> buy_stops_;
    std::multimap<Price, Order> sell_stops_;
  public:
    void add(Order order);
    std::vector<Order> triggerUpTo(Price last_trade_price);
};
