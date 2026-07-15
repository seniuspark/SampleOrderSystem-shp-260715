#include "gtest/gtest.h"
#include "Model/Sample.h"

TEST(SampleTest, CreatesWithGivenFieldsRetained)
{
    Sample sample("S-001", "TestSample", 10.0, 0.9, 5);

    EXPECT_EQ(sample.SampleId, "S-001");
    EXPECT_EQ(sample.Name, "TestSample");
    EXPECT_DOUBLE_EQ(sample.AvgProductionTime, 10.0);
    EXPECT_DOUBLE_EQ(sample.Yield, 0.9);
    EXPECT_EQ(sample.Stock, 5);
}

TEST(SampleTest, RejectsYieldOutOfRange)
{
    EXPECT_THROW(Sample("S-002", "TestSample", 10.0, 0.0, 5), std::invalid_argument);
    EXPECT_THROW(Sample("S-002", "TestSample", 10.0, 1.1, 5), std::invalid_argument);
}
