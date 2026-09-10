#pragma once

#include<concepts>
#include<compare>

template <typename Tag, typename T>
class StrongType {
  public:
    explicit constexpr StrongType(T v): value_(v) {}
    constexpr T get() const { return value_; }

    auto operator<=>(const StrongType&) const = default;
    bool operator==(const StrongType&) const = default;

    StrongType& operator+=(const StrongType& rhs) 
        requires std::same_as<Tag, struct QtyTag> 
    {   
        value_ += rhs.value_; 
        return *this; 
    } 

    StrongType& operator-=(const StrongType& rhs) 
      requires (std::same_as<Tag, struct QtyTag> || std::same_as<Tag, struct PriceTag>)
    { 
        value_ -= rhs.value_; 
        return *this; 
    }

    // binary operators + and -
    [[nodiscard]] friend constexpr StrongType operator+(StrongType lhs, StrongType rhs) 
      requires std::same_as<Tag, struct QtyTag>
    {
      lhs += rhs;
      return lhs;
    }

    [[nodiscard]] friend constexpr StrongType operator-(StrongType lhs, StrongType rhs) 
      requires (std::same_as<Tag, struct QtyTag> || std::same_as<Tag, struct PriceTag>)
    {
      lhs -= rhs;
      return lhs;
    }

    [[nodiscard]] friend constexpr StrongType operator-(const StrongType& obj) 
      requires (std::same_as<Tag, struct QtyTag> || std::same_as<Tag, struct PriceTag>)
    {
      return StrongType(-obj.value_);
    }

    // comparisons against raw underlying types
    [[nodiscard]] friend constexpr bool operator==(const StrongType& lhs, const T& rhs) noexcept {
      return lhs.value_ == rhs;
    }
    [[nodiscard]] friend constexpr auto operator<=>(const StrongType& lhs, const T& rhs) noexcept {
      return lhs.value_ <=> rhs;
    }
  private:
    T value_;
};

struct PriceTag{}; struct QtyTag{}; struct OrderIdTag{};

using Price = StrongType<PriceTag, std::int64_t>;
using Qty = StrongType<QtyTag, std::int32_t>;
using OrderId = StrongType<OrderIdTag, std::int64_t>;

// bool operator==(const OrderId& lhs, const OrderId& rhs) {
//     return lhs.get() == rhs.get();
// }
struct OrderIdHash {
  std::size_t operator()(OrderId id) const noexcept {
    return std::hash<std::int64_t>{}(id.get());
  }
};

enum Side { BUY = 0, SELL = 1};
enum OrderType { MARKET = 0, LIMIT, STOP, STOP_LIMIT };
enum TimeInForce { NONE = 0, 
                   DAY, 
                   GOOD_TIL_CANCELED, 
                   IMMEDIATE_OR_CANCEL, // fill whatever is available immediately, cancel the rest
                   FILL_OR_KILL // fill the entire quantity, cancel if not possible
                  };
enum FillPolicy { NORMAL = 0, 
                  ALL_OR_NONE // combining IOC + all or none = FOK order
                };

enum class ExecutionType {
  FILLED = 0,
  PARTIALLY_FILLED,
  CANCELED, // Used when Market orders run out of liquidity
  REJECTED,
  MODIFIED}
;
