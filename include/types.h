#pragma once

template <typename Tag, typename T>
class StrongType {
  public:
    explicit constexpr StrongType(T v): value_(v) {}
    constexpr T get() const { return value_; }
    auto operator<=>(const StrongType&) const = default;
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
enum TimeInForce { NONE = 0, DAY, GOOD_TIL_CANCELLED, FILL_OR_KILL };
enum FillPolicy { NORMAL, ALL_OR_NONE };
