#include "gtest/gtest.h"

#include <sstream>

#include "View/ConsoleAppView.h"

TEST(ConsoleAppViewTest, ReadMainMenuChoiceParsesInteger)
{
    std::istringstream in("2\n");
    std::ostringstream out;
    ConsoleAppView view(in, out);

    EXPECT_EQ(view.ReadMainMenuChoice(), MainMenuOption::PlaceOrder);
}

TEST(ConsoleAppViewTest, ReadMainMenuChoiceNonNumericInputYieldsInvalidChoice)
{
    std::istringstream in("abc\n");
    std::ostringstream out;
    ConsoleAppView view(in, out);

    MainMenuOption choice = view.ReadMainMenuChoice();

    EXPECT_NE(choice, MainMenuOption::Exit);
    EXPECT_NE(choice, MainMenuOption::SampleManagement);
    EXPECT_NE(choice, MainMenuOption::PlaceOrder);
    EXPECT_NE(choice, MainMenuOption::ApproveOrRejectOrder);
    EXPECT_NE(choice, MainMenuOption::Monitoring);
    EXPECT_NE(choice, MainMenuOption::ProductionLine);
    EXPECT_NE(choice, MainMenuOption::ReleaseOrder);
}

TEST(ConsoleAppViewTest, ReadSampleMenuChoiceParsesInteger)
{
    std::istringstream in("1\n");
    std::ostringstream out;
    ConsoleAppView view(in, out);

    EXPECT_EQ(view.ReadSampleMenuChoice(), SampleMenuOption::Register);
}

TEST(ConsoleAppViewTest, ReadApprovalDecisionParsesInteger)
{
    std::istringstream in("2\n");
    std::ostringstream out;
    ConsoleAppView view(in, out);

    EXPECT_EQ(view.ReadApprovalDecision(), ApprovalDecision::Reject);
}

TEST(ConsoleAppViewTest, ShowSummaryWritesRenderedSummaryToOutputStream)
{
    std::istringstream in;
    std::ostringstream out;
    ConsoleAppView view(in, out);

    view.ShowSummary(MainMenuSummary{ 1, 2, 3, 4 });

    EXPECT_NE(out.str().find("전체 요약"), std::string::npos);
}

TEST(ConsoleAppViewTest, ShowMessageWritesMessageToOutputStream)
{
    std::istringstream in;
    std::ostringstream out;
    ConsoleAppView view(in, out);

    view.ShowMessage("hello");

    EXPECT_NE(out.str().find("hello"), std::string::npos);
}
