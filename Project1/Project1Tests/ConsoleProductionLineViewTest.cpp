#include "gtest/gtest.h"

#include <chrono>
#include <sstream>

#include "Controller/ProductionQueueItem.h"
#include "View/ConsoleProductionLineView.h"

TEST(ConsoleProductionLineViewTest, ShowProductionQueueMarksFrontItemAsInProgress)
{
    std::ostringstream out;
    ConsoleProductionLineView view(out);

    std::vector<ProductionQueueItem> items{
        ProductionQueueItem{ "ORD-1", "S-001", 10, 10, 12, 300.0, std::chrono::system_clock::now() },
        ProductionQueueItem{ "ORD-2", "S-002", 5, 5, 6, 150.0, std::chrono::system_clock::now() },
    };

    view.ShowProductionQueue(items);

    std::string output = out.str();
    EXPECT_NE(output.find("ORD-1"), std::string::npos);
    EXPECT_NE(output.find("ORD-2"), std::string::npos);
    EXPECT_NE(output.find("생산중"), std::string::npos);
    EXPECT_NE(output.find("대기"), std::string::npos);
}

TEST(ConsoleProductionLineViewTest, ShowMessageWritesMessageToOutputStream)
{
    std::ostringstream out;
    ConsoleProductionLineView view(out);

    view.ShowMessage("생산 대기 중인 항목이 없습니다.");

    EXPECT_NE(out.str().find("생산 대기 중인 항목이 없습니다."), std::string::npos);
}
