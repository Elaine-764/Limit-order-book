#include <gtest/gtest.h>
#include "../include/orderbook.h"
#include "../include/types.h"
#include "../include/data.h"
#include "test_helpers.h"

class OrderBookTest : public ::testing::Test {
protected:
    OrderBook book;
};

TEST_F(OrderBookTest, RestingBuyOrderBecomesBestBid) {
    book.addRestingOrder(makeOrder(1, Side::BUY, Price{100}, Qty{10}), Side::BUY);
    ASSERT_TRUE(book.bestBid().has_value());
    EXPECT_EQ(book.bestBid()->get(), 100);
}

TEST_F(OrderBookTest, RestingSellOrderBecomesBestAsk) {
    book.addRestingOrder(makeOrder(1, Side::SELL, Price{105}, Qty{10}), Side::SELL);
    ASSERT_TRUE(book.bestAsk().has_value());
    EXPECT_EQ(book.bestAsk()->get(), 105);
}

TEST_F(OrderBookTest, BidsOrderedHighestFirst) {
    book.addRestingOrder(makeOrder(1, Side::BUY, Price{100}, Qty{1}), Side::BUY);
    book.addRestingOrder(makeOrder(2, Side::BUY, Price{102}, Qty{1}), Side::BUY);
    book.addRestingOrder(makeOrder(3, Side::BUY, Price{101}, Qty{1}), Side::BUY);

    EXPECT_EQ(book.bestBid()->get(), 102);   // tests std::greater<Price> ordering
}

TEST_F(OrderBookTest, AsksOrderedLowestFirst) {
    book.addRestingOrder(makeOrder(1, Side::SELL, Price{105}, Qty{1}), Side::SELL);
    book.addRestingOrder(makeOrder(2, Side::SELL, Price{102}, Qty{1}), Side::SELL);
    book.addRestingOrder(makeOrder(3, Side::SELL, Price{103}, Qty{1}), Side::SELL);

    EXPECT_EQ(book.bestAsk()->get(), 102);   // tests std::less<Price> ordering
}

TEST_F(OrderBookTest, MultipleOrdersAtSamePriceAccumulateQty) {
    book.addRestingOrder(makeOrder(1, Side::BUY, Price{100}, Qty{10}), Side::BUY);
    book.addRestingOrder(makeOrder(2, Side::BUY, Price{100}, Qty{5}), Side::BUY);

    EXPECT_EQ(book.quantityAt(Side::BUY, Price{100}).get(), 15);
    EXPECT_EQ(book.bidLevelCount(), 1u);   // still one price level, not two
}

TEST_F(OrderBookTest, CancelRemovesOrderFromIndex) {
    book.addRestingOrder(makeOrder(1, Side::BUY, Price{100}, Qty{10}), Side::BUY);
    EXPECT_TRUE(book.hasOrder(OrderId{1}));

    bool result = book.cancel(OrderId{1});

    EXPECT_TRUE(result);
    EXPECT_FALSE(book.hasOrder(OrderId{1}));
}

TEST_F(OrderBookTest, CancelingOnlyOrderAtPriceRemovesTheLevel) {
    book.addRestingOrder(makeOrder(1, Side::BUY, Price{100}, Qty{10}), Side::BUY);
    book.cancel(OrderId{1});

    EXPECT_FALSE(book.bestBid().has_value());
    EXPECT_EQ(book.bidLevelCount(), 0u);
}

TEST_F(OrderBookTest, CancelingOneOfSeveralOrdersLeavesLevelAndSiblingsIntact) {
    book.addRestingOrder(makeOrder(1, Side::BUY, Price{100}, Qty{10}), Side::BUY);
    book.addRestingOrder(makeOrder(2, Side::BUY, Price{100}, Qty{5}), Side::BUY);

    book.cancel(OrderId{1});

    EXPECT_EQ(book.bidLevelCount(), 1u);                       // level survives
    EXPECT_EQ(book.quantityAt(Side::BUY, Price{100}).get(), 5); // only order 2 remains
    EXPECT_TRUE(book.hasOrder(OrderId{2}));
    EXPECT_FALSE(book.hasOrder(OrderId{1}));
}

TEST_F(OrderBookTest, EmptyingBestPriceRevealsNextBestBid) {
    book.addRestingOrder(makeOrder(1, Side::BUY, Price{100}, Qty{10}), Side::BUY);
    book.addRestingOrder(makeOrder(2, Side::BUY, Price{99}, Qty{10}), Side::BUY);

    book.cancel(OrderId{1});   // remove the 100 level entirely

    EXPECT_EQ(book.bestBid()->get(), 99);
}

TEST_F(OrderBookTest, CancelingUnknownOrderIdReturnsFalseNotCrash) {
    EXPECT_FALSE(book.cancel(OrderId{999}));
}

TEST_F(OrderBookTest, CancelingSameOrderTwiceIsSafeAndReturnsFalseSecondTime) {
    book.addRestingOrder(makeOrder(1, Side::BUY, Price{100}, Qty{10}), Side::BUY);

    EXPECT_TRUE(book.cancel(OrderId{1}));
    EXPECT_FALSE(book.cancel(OrderId{1}));   // already gone — must not double-erase or crash
}

TEST_F(OrderBookTest, BidsAndAsksAreIndependent) {
    book.addRestingOrder(makeOrder(1, Side::BUY, Price{100}, Qty{10}), Side::BUY);
    book.addRestingOrder(makeOrder(2, Side::SELL, Price{101}, Qty{10}), Side::SELL);

    EXPECT_EQ(book.bidLevelCount(), 1u);
    EXPECT_EQ(book.askLevelCount(), 1u);

    book.cancel(OrderId{1});
    EXPECT_EQ(book.bidLevelCount(), 0u);
    EXPECT_EQ(book.askLevelCount(), 1u);   // untouched by the bid-side cancel
}