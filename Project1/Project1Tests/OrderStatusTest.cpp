#include "gtest/gtest.h"
#include "Model/OrderStatus.h"

TEST(OrderStatusTest, RoundTripsAllFiveStatusesThroughStringConversion)
{
    const OrderStatus statuses[] = {
        OrderStatus::RESERVED,
        OrderStatus::REJECTED,
        OrderStatus::PRODUCING,
        OrderStatus::CONFIRMED,
        OrderStatus::RELEASE,
    };

    for (OrderStatus status : statuses)
    {
        EXPECT_EQ(ParseOrderStatus(ToString(status)), status);
    }
}

TEST(OrderStatusTest, ThrowsOnUnknownStatusString)
{
    EXPECT_THROW(ParseOrderStatus("UNKNOWN"), std::invalid_argument);
}
