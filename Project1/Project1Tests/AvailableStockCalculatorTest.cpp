#include "gtest/gtest.h"
#include "Model/AvailableStockCalculator.h"

TEST(AvailableStockCalculatorTest, SubtractsUnreleasedConfirmedQuantityFromStock)
{
    EXPECT_EQ(CalculateAvailableStock(100, 30), 70);
}

TEST(AvailableStockCalculatorTest, EqualsStockWhenNoUnreleasedConfirmedQuantity)
{
    EXPECT_EQ(CalculateAvailableStock(50, 0), 50);
}

TEST(AvailableStockCalculatorTest, ClampsToZeroWhenUnreleasedConfirmedQuantityExceedsStock)
{
    EXPECT_EQ(CalculateAvailableStock(10, 25), 0);
}

TEST(AvailableStockCalculatorTest, ProductionCompletionSurplusIsReflectedByFormulaAlone)
{
    const int stockBeforeCompletion = 0;
    const int unreleasedConfirmedQuantity = 10;
    const int availableStockBeforeCompletion = CalculateAvailableStock(stockBeforeCompletion, unreleasedConfirmedQuantity);

    const int actualProductionQuantity = 12;
    const int orderQuantity = 10;
    const int stockAfterCompletion = stockBeforeCompletion + actualProductionQuantity;
    const int availableStockAfterCompletion = CalculateAvailableStock(stockAfterCompletion, unreleasedConfirmedQuantity);

    const int surplus = actualProductionQuantity - orderQuantity;
    EXPECT_EQ(availableStockAfterCompletion - availableStockBeforeCompletion, surplus);
}
