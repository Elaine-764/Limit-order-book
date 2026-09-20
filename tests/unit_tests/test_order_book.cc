#include <gtest/gtest.h>
#include "../include/orderbook.h"
#include "../include/types.h"
#include "../include/data.h"
#include "test_helpers.h"

class OrderBookTest : public ::testing::Test {
protected:
    OrderBook book;
    OrderBookTest() : book(1) {} 
    void SetUp() override {
        // Any additional test harness setup goes here...
    }
};

TEST_F(OrderBookTest, RestingBuyOrderBecomesBestBid) {
    std::vector<ExecutionReport> reports;
    book.addRestingOrder(makeOrder(1, Side::BUY, Price{100}, Qty{10}), Side::BUY, reports);
    ASSERT_TRUE(book.bestBid().has_value());
    EXPECT_EQ(book.bestBid()->get(), 100);
}

TEST_F(OrderBookTest, RestingSellOrderBecomesBestAsk) {
    std::vector<ExecutionReport> reports;
    book.addRestingOrder(makeOrder(1, Side::SELL, Price{105}, Qty{10}), Side::SELL, reports);
    ASSERT_TRUE(book.bestAsk().has_value());
    EXPECT_EQ(book.bestAsk()->get(), 105);
}

TEST_F(OrderBookTest, BidsOrderedHighestFirst) {
    std::vector<ExecutionReport> reports;
    book.addRestingOrder(makeOrder(1, Side::BUY, Price{100}, Qty{1}), Side::BUY, reports);
    book.addRestingOrder(makeOrder(2, Side::BUY, Price{102}, Qty{1}), Side::BUY, reports);
    book.addRestingOrder(makeOrder(3, Side::BUY, Price{101}, Qty{1}), Side::BUY, reports);

    EXPECT_EQ(book.bestBid()->get(), 102);   // tests std::greater<Price> ordering
}

TEST_F(OrderBookTest, AsksOrderedLowestFirst) {
    std::vector<ExecutionReport> reports;
    book.addRestingOrder(makeOrder(1, Side::SELL, Price{105}, Qty{1}), Side::SELL, reports);
    book.addRestingOrder(makeOrder(2, Side::SELL, Price{102}, Qty{1}), Side::SELL, reports);
    book.addRestingOrder(makeOrder(3, Side::SELL, Price{103}, Qty{1}), Side::SELL, reports);

    EXPECT_EQ(book.bestAsk()->get(), 102);   // tests std::less<Price> ordering
}

TEST_F(OrderBookTest, MultipleOrdersAtSamePriceAccumulateQty) {
    std::vector<ExecutionReport> reports;
    book.addRestingOrder(makeOrder(1, Side::BUY, Price{100}, Qty{10}), Side::BUY, reports);
    book.addRestingOrder(makeOrder(2, Side::BUY, Price{100}, Qty{5}), Side::BUY, reports);

    EXPECT_EQ(book.quantityAt(Side::BUY, Price{100}).get(), 15);
    EXPECT_EQ(book.bidLevelCount(), 1u);   // still one price level, not two
}

TEST_F(OrderBookTest, CancelRemovesOrderFromIndex) {
    std::vector<ExecutionReport> reports;
    book.addRestingOrder(makeOrder(1, Side::BUY, Price{100}, Qty{10}), Side::BUY, reports);
    EXPECT_TRUE(book.hasOrder(OrderId{1}));

    bool result = book.cancel(OrderId{1}, reports);

    EXPECT_TRUE(result);
    EXPECT_FALSE(book.hasOrder(OrderId{1}));
}

TEST_F(OrderBookTest, CancelingOnlyOrderAtPriceRemovesTheLevel) {
    std::vector<ExecutionReport> reports;
    book.addRestingOrder(makeOrder(1, Side::BUY, Price{100}, Qty{10}), Side::BUY, reports);
    book.cancel(OrderId{1}, reports);

    EXPECT_FALSE(book.bestBid().has_value());
    EXPECT_EQ(book.bidLevelCount(), 0u);
}

TEST_F(OrderBookTest, CancelingOneOfSeveralOrdersLeavesLevelAndSiblingsIntact) {
    std::vector<ExecutionReport> reports;

    book.addRestingOrder(makeOrder(1, Side::BUY, Price{100}, Qty{10}), Side::BUY, reports);
    book.addRestingOrder(makeOrder(2, Side::BUY, Price{100}, Qty{5}), Side::BUY, reports);

    book.cancel(OrderId{1}, reports);

    EXPECT_EQ(book.bidLevelCount(), 1u);                       // level survives
    EXPECT_EQ(book.quantityAt(Side::BUY, Price{100}).get(), 5); // only order 2 remains
    EXPECT_TRUE(book.hasOrder(OrderId{2}));
    EXPECT_FALSE(book.hasOrder(OrderId{1}));
}

TEST_F(OrderBookTest, EmptyingBestPriceRevealsNextBestBid) {
    std::vector<ExecutionReport> reports;
    book.addRestingOrder(makeOrder(1, Side::BUY, Price{100}, Qty{10}), Side::BUY, reports);
    book.addRestingOrder(makeOrder(2, Side::BUY, Price{99}, Qty{10}), Side::BUY, reports);

    book.cancel(OrderId{1}, reports);   // remove the 100 level entirely

    EXPECT_EQ(book.bestBid()->get(), 99);
}

TEST_F(OrderBookTest, CancelingUnknownOrderIdReturnsFalseNotCrash) {
    std::vector<ExecutionReport> reports;
    EXPECT_FALSE(book.cancel(OrderId{999}, reports));
}

TEST_F(OrderBookTest, CancelingSameOrderTwiceIsSafeAndReturnsFalseSecondTime) {
    std::vector<ExecutionReport> reports;
    book.addRestingOrder(makeOrder(1, Side::BUY, Price{100}, Qty{10}), Side::BUY, reports);

    EXPECT_TRUE(book.cancel(OrderId{1}, reports));
    EXPECT_FALSE(book.cancel(OrderId{1}, reports));   // already gone — must not double-erase or crash
}

TEST_F(OrderBookTest, BidsAndAsksAreIndependent) {
    std::vector<ExecutionReport> reports;
    book.addRestingOrder(makeOrder(1, Side::BUY, Price{100}, Qty{10}), Side::BUY, reports);
    book.addRestingOrder(makeOrder(2, Side::SELL, Price{101}, Qty{10}), Side::SELL, reports);

    EXPECT_EQ(book.bidLevelCount(), 1u);
    EXPECT_EQ(book.askLevelCount(), 1u);

    book.cancel(OrderId{1}, reports);
    EXPECT_EQ(book.bidLevelCount(), 0u);
    EXPECT_EQ(book.askLevelCount(), 1u);   // untouched by the bid-side cancel
}