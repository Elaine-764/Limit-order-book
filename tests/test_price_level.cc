#include <gtest/gtest.h>
#include "../include/orderbook.h"
#include "../include/data.h"
#include "../include/types.h"
#include "test_helpers.h"

TEST(PriceLevelTest, DefaultConstructorStartsEmptyWithZeroQty) {
    PriceLevel level;
    EXPECT_TRUE(level.isEmpty());
    EXPECT_EQ(level.getQty().get(), 0);
}

TEST(PriceLevelTest, PriceConstructorSetsPriceAndStartsEmpty) {
    PriceLevel level{Price{100}};
    EXPECT_EQ(level.getPrice().get(), 100);
    EXPECT_TRUE(level.isEmpty());
    EXPECT_EQ(level.getQty().get(), 0);
}

TEST(PriceLevelTest, AddOrderIncreasesQtyAndClearsEmpty) {
    PriceLevel level{Price{100}};
    level.addOrder(makeOrder(1, Side::BUY, Price{100}, Qty{10}));

    EXPECT_FALSE(level.isEmpty());
    EXPECT_EQ(level.getQty().get(), 10);
}

TEST(PriceLevelTest, MultipleAddOrdersAccumulateQty) {
    PriceLevel level{Price{100}};
    level.addOrder(makeOrder(1, Side::BUY, Price{100}, Qty{10}));
    level.addOrder(makeOrder(2, Side::BUY, Price{100}, Qty{5}));
    level.addOrder(makeOrder(3, Side::BUY, Price{100}, Qty{7}));

    EXPECT_EQ(level.getQty().get(), 22);
}

TEST(PriceLevelTest, AddOrderReturnsIteratorToJustInsertedOrder) {
    PriceLevel level{Price{100}};
    level.addOrder(makeOrder(1, Side::BUY, Price{100}, Qty{10}));
    auto it2 = level.addOrder(makeOrder(2, Side::BUY, Price{100}, Qty{5}));

    EXPECT_EQ(it2->order_id.get(), 2);
    EXPECT_EQ(it2->quantity.get(), 5);
}

TEST(PriceLevelTest, RemoveOrderDecreasesQtyByThatOrdersAmount) {
    PriceLevel level{Price{100}};
    auto it1 = level.addOrder(makeOrder(1, Side::BUY, Price{100}, Qty{10}));
    level.addOrder(makeOrder(2, Side::BUY, Price{100}, Qty{5}));

    level.removeOrder(it1);

    EXPECT_EQ(level.getQty().get(), 5);   // only order 2's qty remains
    EXPECT_FALSE(level.isEmpty());
}

TEST(PriceLevelTest, RemoveMiddleOrderLeavesOthersIntact) {
    PriceLevel level{Price{100}};
    auto it1 = level.addOrder(makeOrder(1, Side::BUY, Price{100}, Qty{10}));
    auto it2 = level.addOrder(makeOrder(2, Side::BUY, Price{100}, Qty{5}));
    level.addOrder(makeOrder(3, Side::BUY, Price{100}, Qty{7}));

    level.removeOrder(it2);   // erase the middle one

    // it1 must still be valid and point at the correct data —
    // this is the actual claim we care about: erasing one list node
    // does not invalidate iterators to *other* nodes.
    EXPECT_EQ(it1->order_id.get(), 1);
    EXPECT_EQ(level.getQty().get(), 17);   // 10 + 7, order 2's 5 removed
}

TEST(PriceLevelTest, RemovingLastOrderMakesLevelEmpty) {
    PriceLevel level{Price{100}};
    auto it = level.addOrder(makeOrder(1, Side::BUY, Price{100}, Qty{10}));

    level.removeOrder(it);

    EXPECT_TRUE(level.isEmpty());
    EXPECT_EQ(level.getQty().get(), 0);
}

TEST(PriceLevelTest, RemovingAllOrdersOneByOneLeavesConsistentQty) {
    PriceLevel level{Price{100}};
    auto it1 = level.addOrder(makeOrder(1, Side::BUY, Price{100}, Qty{10}));
    auto it2 = level.addOrder(makeOrder(2, Side::BUY, Price{100}, Qty{5}));

    level.removeOrder(it1);
    EXPECT_EQ(level.getQty().get(), 5);

    level.removeOrder(it2);
    EXPECT_EQ(level.getQty().get(), 0);
    EXPECT_TRUE(level.isEmpty());
}

TEST(PriceLevelTest, OrdersPreserveFifoInsertionOrder) {
    PriceLevel level{Price{100}};
    level.addOrder(makeOrder(1, Side::BUY, Price{100}, Qty{10}));
    level.addOrder(makeOrder(2, Side::BUY, Price{100}, Qty{5}));
    level.addOrder(makeOrder(3, Side::BUY, Price{100}, Qty{7}));

    EXPECT_EQ(level.front().order_id.get(), 1);
}

TEST(PriceLevelTest, FifoOrderSurvivesAMiddleRemoval) {
    PriceLevel level{Price{100}};
    auto it1 = level.addOrder(makeOrder(1, Side::BUY, Price{100}, Qty{10}));
    auto it2 = level.addOrder(makeOrder(2, Side::BUY, Price{100}, Qty{5}));
    level.addOrder(makeOrder(3, Side::BUY, Price{100}, Qty{7}));

    level.removeOrder(it2);

    // front should still be order 1; iterate to confirm order 3 follows directly
    auto it = level.begin();
    EXPECT_EQ(it->order_id.get(), 1);
    ++it;
    EXPECT_EQ(it->order_id.get(), 3);
}