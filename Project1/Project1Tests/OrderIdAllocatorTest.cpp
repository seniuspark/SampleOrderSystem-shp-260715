#include "gtest/gtest.h"
#include "Model/OrderIdAllocator.h"

#include <string>
#include <vector>

TEST(OrderIdAllocatorTest, AllocatesFirstSequenceForTodayWhenNoExistingOrders)
{
    const std::vector<std::string> existingOrderIds;

    EXPECT_EQ(AllocateOrderId(existingOrderIds, "20260715"), "ORD-20260715-0001");
}

TEST(OrderIdAllocatorTest, AllocatesNextSequenceAfterExistingMaximumForSameDate)
{
    const std::vector<std::string> existingOrderIds{ "ORD-20260715-0001", "ORD-20260715-0007", "ORD-20260715-0003" };

    EXPECT_EQ(AllocateOrderId(existingOrderIds, "20260715"), "ORD-20260715-0008");
}

TEST(OrderIdAllocatorTest, RestartsSequenceFromOneWhenDateChanges)
{
    const std::vector<std::string> existingOrderIds{ "ORD-20260714-0009" };

    EXPECT_EQ(AllocateOrderId(existingOrderIds, "20260715"), "ORD-20260715-0001");
}

TEST(OrderIdAllocatorTest, IgnoresOrderIdsThatFailToParse)
{
    const std::vector<std::string> existingOrderIds{ "ORD-20260715-0002", "malformed-id", "ORD-BADSEQ", "" };

    EXPECT_EQ(AllocateOrderId(existingOrderIds, "20260715"), "ORD-20260715-0003");
}
