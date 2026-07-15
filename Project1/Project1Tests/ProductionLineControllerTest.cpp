#include "gtest/gtest.h"

#include <chrono>
#include <filesystem>
#include <string>

#include "Controller/ProductionLineController.h"
#include "Repository/OrderRepository.h"
#include "Repository/SampleRepository.h"
#include "TestSupport/FixedClock.h"
#include "View/FakeProductionLineView.h"

namespace
{

std::filesystem::path MakeRepositoryDir(const std::string& testName)
{
    std::filesystem::path dir = std::filesystem::temp_directory_path() / "Project1Tests" / "ProductionLineControllerTest" / testName;
    std::filesystem::remove_all(dir);
    return dir;
}

}  // namespace

TEST(ProductionLineControllerTest, EnqueueMultipleItems_ShownInFifoOrder)
{
    std::filesystem::path dir = MakeRepositoryDir("EnqueueMultipleItems_ShownInFifoOrder");
    SampleRepository sampleRepository(dir / "samples.json");
    OrderRepository orderRepository(dir / "orders.json");
    Sample sample("S-001", "시료A", 10.0, 1.0, 0);
    sampleRepository.Add(sample);
    Order first("ORD-20260715-0001", "S-001", "고객A", 5, std::chrono::system_clock::now());
    first.Status = OrderStatus::PRODUCING;
    orderRepository.Add(first);
    Order second("ORD-20260715-0002", "S-001", "고객B", 3, std::chrono::system_clock::now());
    second.Status = OrderStatus::PRODUCING;
    orderRepository.Add(second);
    FakeProductionLineView view;
    FixedClock clock(std::chrono::system_clock::now());
    ProductionLineController controller(orderRepository, sampleRepository, clock, view);

    controller.EnqueueProductionItem(first, sample);
    controller.EnqueueProductionItem(second, sample);
    controller.ShowProductionQueue();

    ASSERT_EQ(view.LastShownItems().size(), 2u);
    EXPECT_EQ(view.LastShownItems()[0].OrderId, "ORD-20260715-0001");
    EXPECT_EQ(view.LastShownItems()[1].OrderId, "ORD-20260715-0002");
}

TEST(ProductionLineControllerTest, EnqueuedItem_ShortageAndProductionTimeMatchPureFunctions)
{
    std::filesystem::path dir = MakeRepositoryDir("EnqueuedItem_ShortageAndProductionTimeMatchPureFunctions");
    SampleRepository sampleRepository(dir / "samples.json");
    OrderRepository orderRepository(dir / "orders.json");
    Sample sample("S-001", "시료A", 10.0, 0.7, 2);
    sampleRepository.Add(sample);
    Order order("ORD-20260715-0001", "S-001", "고객A", 10, std::chrono::system_clock::now());
    order.Status = OrderStatus::PRODUCING;
    orderRepository.Add(order);
    FakeProductionLineView view;
    FixedClock clock(std::chrono::system_clock::now());
    ProductionLineController controller(orderRepository, sampleRepository, clock, view);

    controller.EnqueueProductionItem(order, sample);
    controller.ShowProductionQueue();

    ASSERT_EQ(view.LastShownItems().size(), 1u);
    const ProductionQueueItem& item = view.LastShownItems()[0];
    EXPECT_EQ(item.Shortage, CalculateShortage(order.Quantity, sample.Stock));
    EXPECT_EQ(item.ActualProductionQuantity, CalculateActualProductionQuantity(item.Shortage, sample.Yield));
    EXPECT_DOUBLE_EQ(item.TotalProductionTimeMinutes, CalculateTotalProductionTime(sample.AvgProductionTime, item.ActualProductionQuantity));
}

TEST(ProductionLineControllerTest, EnteringMenu_CompletesFrontItemWhenTimeElapsed)
{
    std::filesystem::path dir = MakeRepositoryDir("EnteringMenu_CompletesFrontItemWhenTimeElapsed");
    SampleRepository sampleRepository(dir / "samples.json");
    OrderRepository orderRepository(dir / "orders.json");
    Sample sample("S-001", "시료A", 10.0, 1.0, 0);
    sampleRepository.Add(sample);
    Order order("ORD-20260715-0001", "S-001", "고객A", 5, std::chrono::system_clock::now());
    order.Status = OrderStatus::PRODUCING;
    orderRepository.Add(order);
    FakeProductionLineView view;
    FixedClock clock(std::chrono::system_clock::now());
    ProductionLineController controller(orderRepository, sampleRepository, clock, view);
    controller.EnqueueProductionItem(order, sample);

    clock.Advance(std::chrono::minutes{ 60 });
    controller.ShowProductionQueue();

    EXPECT_EQ(orderRepository.FindById("ORD-20260715-0001")->Status, OrderStatus::CONFIRMED);
    EXPECT_EQ(sampleRepository.FindById("S-001")->Stock, 5);
    EXPECT_TRUE(view.LastShownItems().empty());
}

TEST(ProductionLineControllerTest, EnteringMenu_DoesNotCompleteBeforeElapsedTime)
{
    std::filesystem::path dir = MakeRepositoryDir("EnteringMenu_DoesNotCompleteBeforeElapsedTime");
    SampleRepository sampleRepository(dir / "samples.json");
    OrderRepository orderRepository(dir / "orders.json");
    Sample sample("S-001", "시료A", 10.0, 1.0, 0);
    sampleRepository.Add(sample);
    Order order("ORD-20260715-0001", "S-001", "고객A", 5, std::chrono::system_clock::now());
    order.Status = OrderStatus::PRODUCING;
    orderRepository.Add(order);
    FakeProductionLineView view;
    FixedClock clock(std::chrono::system_clock::now());
    ProductionLineController controller(orderRepository, sampleRepository, clock, view);
    controller.EnqueueProductionItem(order, sample);

    clock.Advance(std::chrono::minutes{ 1 });
    controller.ShowProductionQueue();

    EXPECT_EQ(orderRepository.FindById("ORD-20260715-0001")->Status, OrderStatus::PRODUCING);
    EXPECT_EQ(sampleRepository.FindById("S-001")->Stock, 0);
    EXPECT_EQ(view.LastShownItems().size(), 1u);
}

TEST(ProductionLineControllerTest, CompletionSurplus_IncreasesAvailableStockBeyondOrderedQuantity)
{
    std::filesystem::path dir = MakeRepositoryDir("CompletionSurplus_IncreasesAvailableStockBeyondOrderedQuantity");
    SampleRepository sampleRepository(dir / "samples.json");
    OrderRepository orderRepository(dir / "orders.json");
    Sample sample("S-001", "시료A", 10.0, 0.7, 0);
    sampleRepository.Add(sample);
    Order order("ORD-20260715-0001", "S-001", "고객A", 10, std::chrono::system_clock::now());
    order.Status = OrderStatus::PRODUCING;
    orderRepository.Add(order);
    FakeProductionLineView view;
    FixedClock clock(std::chrono::system_clock::now());
    ProductionLineController controller(orderRepository, sampleRepository, clock, view);
    controller.EnqueueProductionItem(order, sample);

    const int actualProductionQuantity = CalculateActualProductionQuantity(CalculateShortage(order.Quantity, sample.Stock), sample.Yield);
    ASSERT_GT(actualProductionQuantity, order.Quantity);

    clock.Advance(std::chrono::minutes{ 10000 });
    controller.ShowProductionQueue();

    Sample completedSample = *sampleRepository.FindById("S-001");
    EXPECT_EQ(completedSample.Stock, actualProductionQuantity);
}

TEST(ProductionLineControllerTest, EmptyQueue_ShowsNoWaitingItemMessage)
{
    std::filesystem::path dir = MakeRepositoryDir("EmptyQueue_ShowsNoWaitingItemMessage");
    SampleRepository sampleRepository(dir / "samples.json");
    OrderRepository orderRepository(dir / "orders.json");
    FakeProductionLineView view;
    FixedClock clock(std::chrono::system_clock::now());
    ProductionLineController controller(orderRepository, sampleRepository, clock, view);

    controller.ShowProductionQueue();

    EXPECT_TRUE(view.LastShownItems().empty());
    EXPECT_FALSE(view.LastMessage().empty());
}

TEST(ProductionLineControllerTest, AutoCompletion_OnlyAppliesToFrontOfQueue)
{
    std::filesystem::path dir = MakeRepositoryDir("AutoCompletion_OnlyAppliesToFrontOfQueue");
    SampleRepository sampleRepository(dir / "samples.json");
    OrderRepository orderRepository(dir / "orders.json");
    Sample sample("S-001", "시료A", 100.0, 1.0, 0);
    sampleRepository.Add(sample);
    Order first("ORD-20260715-0001", "S-001", "고객A", 5, std::chrono::system_clock::now());
    first.Status = OrderStatus::PRODUCING;
    orderRepository.Add(first);
    Order second("ORD-20260715-0002", "S-001", "고객B", 1, std::chrono::system_clock::now());
    second.Status = OrderStatus::PRODUCING;
    orderRepository.Add(second);
    FakeProductionLineView view;
    FixedClock clock(std::chrono::system_clock::now());
    ProductionLineController controller(orderRepository, sampleRepository, clock, view);

    controller.EnqueueProductionItem(first, sample);
    clock.Advance(std::chrono::minutes{ 200 });
    controller.EnqueueProductionItem(second, sample);
    clock.Advance(std::chrono::minutes{ 150 });

    controller.ShowProductionQueue();

    EXPECT_EQ(orderRepository.FindById("ORD-20260715-0001")->Status, OrderStatus::PRODUCING);
    EXPECT_EQ(orderRepository.FindById("ORD-20260715-0002")->Status, OrderStatus::PRODUCING);
}
