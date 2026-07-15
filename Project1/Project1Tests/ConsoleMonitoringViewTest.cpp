#include "gtest/gtest.h"

#include <sstream>

#include "Model/OrderStatus.h"
#include "Model/StockStatus.h"
#include "View/ConsoleMonitoringView.h"
#include "View/IMonitoringView.h"

TEST(ConsoleMonitoringViewTest, ShowOrderStatusCountsFormatsEachStatus)
{
    std::ostringstream out;
    ConsoleMonitoringView view(out);

    std::vector<OrderStatusCount> counts{
        OrderStatusCount{ OrderStatus::RESERVED, 3 },
        OrderStatusCount{ OrderStatus::CONFIRMED, 1 },
    };

    view.ShowOrderStatusCounts(counts);

    std::string output = out.str();
    EXPECT_NE(output.find("RESERVED"), std::string::npos);
    EXPECT_NE(output.find("3"), std::string::npos);
    EXPECT_NE(output.find("CONFIRMED"), std::string::npos);
}

TEST(ConsoleMonitoringViewTest, ShowSampleStockLevelsUsesKoreanLabelsForEachStatus)
{
    std::ostringstream out;
    ConsoleMonitoringView view(out);

    std::vector<SampleStockLevel> levels{
        SampleStockLevel{ "S-001", 0, StockStatus::DEPLETED },
        SampleStockLevel{ "S-002", 3, StockStatus::SHORTAGE },
        SampleStockLevel{ "S-003", 10, StockStatus::SUFFICIENT },
    };

    view.ShowSampleStockLevels(levels);

    std::string output = out.str();
    EXPECT_NE(output.find("고갈"), std::string::npos);
    EXPECT_NE(output.find("부족"), std::string::npos);
    EXPECT_NE(output.find("여유"), std::string::npos);
}

TEST(ConsoleMonitoringViewTest, ShowMessageWritesMessageToOutputStream)
{
    std::ostringstream out;
    ConsoleMonitoringView view(out);

    view.ShowMessage("표시할 데이터가 없습니다.");

    EXPECT_NE(out.str().find("표시할 데이터가 없습니다."), std::string::npos);
}
