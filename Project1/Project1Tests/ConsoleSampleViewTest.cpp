#include "gtest/gtest.h"

#include <sstream>

#include "Model/Sample.h"
#include "View/ConsoleSampleView.h"

TEST(ConsoleSampleViewTest, ReadNewSampleInputParsesAllFields)
{
    std::istringstream in("S-010\nWafer-Z\n7.5\n0.9\n");
    std::ostringstream out;
    ConsoleSampleView view(in, out);

    NewSampleInput input = view.ReadNewSampleInput();

    EXPECT_EQ(input.SampleId, "S-010");
    EXPECT_EQ(input.Name, "Wafer-Z");
    EXPECT_DOUBLE_EQ(input.AvgProductionTime, 7.5);
    EXPECT_DOUBLE_EQ(input.Yield, 0.9);
}

TEST(ConsoleSampleViewTest, ReadSearchKeywordReturnsLine)
{
    std::istringstream in("Wafer\n");
    std::ostringstream out;
    ConsoleSampleView view(in, out);

    EXPECT_EQ(view.ReadSearchKeyword(), "Wafer");
}

TEST(ConsoleSampleViewTest, ShowSampleListFormatsHeaderAndRows)
{
    std::istringstream in;
    std::ostringstream out;
    ConsoleSampleView view(in, out);

    std::vector<Sample> samples{
        Sample("S-001", "Wafer-A", 12.5, 0.95, 100),
        Sample("S-002", "Wafer-B", 5.0, 0.8, 50),
    };

    view.ShowSampleList(samples);

    std::string output = out.str();
    EXPECT_NE(output.find("SampleId"), std::string::npos);
    EXPECT_NE(output.find("S-001"), std::string::npos);
    EXPECT_NE(output.find("Wafer-A"), std::string::npos);
    EXPECT_NE(output.find("S-002"), std::string::npos);
    EXPECT_NE(output.find("Wafer-B"), std::string::npos);
}

TEST(ConsoleSampleViewTest, ShowMessageWritesMessageToOutputStream)
{
    std::istringstream in;
    std::ostringstream out;
    ConsoleSampleView view(in, out);

    view.ShowMessage("hello world");

    EXPECT_NE(out.str().find("hello world"), std::string::npos);
}
