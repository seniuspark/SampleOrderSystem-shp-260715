#include "gtest/gtest.h"

#include <chrono>
#include <filesystem>
#include <string>

#include "Controller/MonitoringController.h"
#include "Repository/OrderRepository.h"
#include "Repository/SampleRepository.h"
#include "View/FakeMonitoringView.h"

namespace
{

std::filesystem::path MakeRepositoryDir(const std::string& testName)
{
    std::filesystem::path dir = std::filesystem::temp_directory_path() / "Project1Tests" / "MonitoringControllerTest" / testName;
    std::filesystem::remove_all(dir);
    return dir;
}

int CountForStatus(const std::vector<OrderStatusCount>& counts, OrderStatus status)
{
    for (const OrderStatusCount& count : counts)
    {
        if (count.Status == status)
        {
            return count.Count;
        }
    }
    return -1;
}

}  // namespace

TEST(MonitoringControllerTest, CountsOrdersByStatusExcludingRejected)
{
    std::filesystem::path dir = MakeRepositoryDir("CountsOrdersByStatusExcludingRejected");
    SampleRepository sampleRepository(dir / "samples.json");
    OrderRepository orderRepository(dir / "orders.json");

    Order reserved("ORD-20260715-0001", "S-001", "고객A", 1, std::chrono::system_clock::now());
    orderRepository.Add(reserved);

    Order confirmed("ORD-20260715-0002", "S-001", "고객B", 2, std::chrono::system_clock::now());
    confirmed.Status = OrderStatus::CONFIRMED;
    orderRepository.Add(confirmed);

    Order producing("ORD-20260715-0003", "S-001", "고객C", 3, std::chrono::system_clock::now());
    producing.Status = OrderStatus::PRODUCING;
    orderRepository.Add(producing);

    Order release("ORD-20260715-0004", "S-001", "고객D", 4, std::chrono::system_clock::now());
    release.Status = OrderStatus::RELEASE;
    orderRepository.Add(release);

    Order rejected("ORD-20260715-0005", "S-001", "고객E", 5, std::chrono::system_clock::now());
    rejected.Status = OrderStatus::REJECTED;
    orderRepository.Add(rejected);

    sampleRepository.Add(Sample("S-001", "시료A", 10.0, 0.9, 10));
    FakeMonitoringView view;
    MonitoringController controller(orderRepository, sampleRepository, view);

    controller.ShowMonitoring();

    const std::vector<OrderStatusCount>& counts = view.LastCounts();
    EXPECT_EQ(CountForStatus(counts, OrderStatus::RESERVED), 1);
    EXPECT_EQ(CountForStatus(counts, OrderStatus::CONFIRMED), 1);
    EXPECT_EQ(CountForStatus(counts, OrderStatus::PRODUCING), 1);
    EXPECT_EQ(CountForStatus(counts, OrderStatus::RELEASE), 1);
    for (const OrderStatusCount& count : counts)
    {
        EXPECT_NE(count.Status, OrderStatus::REJECTED);
    }
}

TEST(MonitoringControllerTest, StockLevelsMatchPureJudgmentFunction)
{
    std::filesystem::path dir = MakeRepositoryDir("StockLevelsMatchPureJudgmentFunction");
    SampleRepository sampleRepository(dir / "samples.json");
    OrderRepository orderRepository(dir / "orders.json");

    sampleRepository.Add(Sample("S-001", "여유시료", 10.0, 0.9, 100));
    sampleRepository.Add(Sample("S-002", "부족시료", 10.0, 0.9, 5));
    sampleRepository.Add(Sample("S-003", "고갈시료", 10.0, 0.9, 0));

    Order orderForShortage("ORD-20260715-0001", "S-002", "고객A", 10, std::chrono::system_clock::now());
    orderRepository.Add(orderForShortage);

    FakeMonitoringView view;
    MonitoringController controller(orderRepository, sampleRepository, view);

    controller.ShowMonitoring();

    const std::vector<SampleStockLevel>& levels = view.LastLevels();
    ASSERT_EQ(levels.size(), 3u);

    auto findLevel = [&levels](const std::string& sampleId) {
        for (const SampleStockLevel& level : levels)
        {
            if (level.SampleId == sampleId)
            {
                return level;
            }
        }
        throw std::runtime_error("not found");
    };

    EXPECT_EQ(findLevel("S-001").Status, JudgeStockStatus(100, 0));
    EXPECT_EQ(findLevel("S-002").Status, JudgeStockStatus(5, 10));
    EXPECT_EQ(findLevel("S-003").Status, JudgeStockStatus(0, 0));
}

TEST(MonitoringControllerTest, NoOrdersAndNoSamples_ShowsNoDataMessage)
{
    std::filesystem::path dir = MakeRepositoryDir("NoOrdersAndNoSamples_ShowsNoDataMessage");
    SampleRepository sampleRepository(dir / "samples.json");
    OrderRepository orderRepository(dir / "orders.json");
    FakeMonitoringView view;
    MonitoringController controller(orderRepository, sampleRepository, view);

    controller.ShowMonitoring();

    EXPECT_TRUE(view.LastCounts().empty());
    EXPECT_TRUE(view.LastLevels().empty());
    EXPECT_FALSE(view.LastMessage().empty());
}
