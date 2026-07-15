#include "gtest/gtest.h"
#include "Model/StockStatus.h"

TEST(StockStatusJudgeTest, DepletedWhenStockIsZeroRegardlessOfDemand)
{
    EXPECT_EQ(JudgeStockStatus(0, 0), StockStatus::DEPLETED);
    EXPECT_EQ(JudgeStockStatus(0, 10), StockStatus::DEPLETED);
}

TEST(StockStatusJudgeTest, ShortageWhenStockIsPositiveButBelowDemand)
{
    EXPECT_EQ(JudgeStockStatus(3, 10), StockStatus::SHORTAGE);
}

TEST(StockStatusJudgeTest, SufficientWhenStockEqualsDemand)
{
    EXPECT_EQ(JudgeStockStatus(5, 5), StockStatus::SUFFICIENT);
}

TEST(StockStatusJudgeTest, SufficientWhenDemandIsZero)
{
    EXPECT_EQ(JudgeStockStatus(5, 0), StockStatus::SUFFICIENT);
}

TEST(StockStatusJudgeTest, NegativeStockAndDemandAreClampedToZero)
{
    EXPECT_EQ(JudgeStockStatus(-5, 10), StockStatus::DEPLETED);
    EXPECT_EQ(JudgeStockStatus(5, -5), StockStatus::SUFFICIENT);
}
