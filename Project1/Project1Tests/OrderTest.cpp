#include "gtest/gtest.h"
#include "Model/Order.h"

namespace
{
    std::chrono::system_clock::time_point FixedTimePoint()
    {
        return std::chrono::system_clock::time_point{};
    }
}

TEST(OrderTest, RejectsQuantityLessThanOne)
{
    EXPECT_THROW(Order("ORD-20260715-0001", "S-001", "TestCustomer", 0, FixedTimePoint()), std::invalid_argument);
    EXPECT_THROW(Order("ORD-20260715-0001", "S-001", "TestCustomer", -1, FixedTimePoint()), std::invalid_argument);
}

TEST(OrderTest, StatusIsReservedImmediatelyAfterCreation)
{
    Order order("ORD-20260715-0001", "S-001", "TestCustomer", 3, FixedTimePoint());

    EXPECT_EQ(order.Status, OrderStatus::RESERVED);
}
