#include "gtest/gtest.h"

#include "View/SummaryRenderer.h"

TEST(SummaryRendererTest, RenderIncludesAllFourCounts)
{
    MainMenuSummary summary{ 3, 42, 7, 2 };

    std::string rendered = RenderMainMenuSummary(summary);

    EXPECT_NE(rendered.find("3"), std::string::npos);
    EXPECT_NE(rendered.find("42"), std::string::npos);
    EXPECT_NE(rendered.find("7"), std::string::npos);
    EXPECT_NE(rendered.find("2"), std::string::npos);
}
