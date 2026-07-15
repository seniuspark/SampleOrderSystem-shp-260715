#include "gtest/gtest.h"
#include "Model/ProductionQueueCalculator.h"

TEST(ProductionQueueCalculatorTest, ShortageIsZeroWhenStockMeetsOrExceedsQuantity)
{
    const int orderQuantity = 5;
    const int stockEqualToQuantity = 5;
    const int stockGreaterThanQuantity = 10;

    EXPECT_EQ(CalculateShortage(orderQuantity, stockEqualToQuantity), 0);
    EXPECT_EQ(CalculateShortage(orderQuantity, stockGreaterThanQuantity), 0);
}

TEST(ProductionQueueCalculatorTest, ActualProductionQuantityRoundsUpWhenNotDivisible)
{
    EXPECT_EQ(CalculateActualProductionQuantity(10, 0.9), 12);
}

TEST(ProductionQueueCalculatorTest, ZeroShortageProducesZeroQuantityAndZeroTime)
{
    EXPECT_EQ(CalculateActualProductionQuantity(0, 0.9), 0);
    EXPECT_DOUBLE_EQ(CalculateTotalProductionTime(10.0, 0), 0.0);
}

TEST(ProductionQueueCalculatorTest, TotalProductionTimeIsAvgTimeTimesActualQuantity)
{
    EXPECT_DOUBLE_EQ(CalculateTotalProductionTime(10.0, 12), 120.0);
}

TEST(ProductionQueueCalculatorTest, ActualProductionQuantityEqualsShortageWhenYieldIsOne)
{
    EXPECT_EQ(CalculateActualProductionQuantity(7, 1.0), 7);
}
