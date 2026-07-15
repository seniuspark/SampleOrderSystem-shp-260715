#include "gtest/gtest.h"

#include <chrono>
#include <filesystem>
#include <string>

#include "Controller/OrderController.h"
#include "Controller/ProductionLineController.h"
#include "Repository/JsonCodec.h"
#include "Repository/OrderRepository.h"
#include "Repository/SampleRepository.h"
#include "TestSupport/FixedClock.h"
#include "View/FakeOrderView.h"
#include "View/FakeProductionLineView.h"

namespace
{

std::filesystem::path MakeRepositoryDir(const std::string& testName)
{
    std::filesystem::path dir = std::filesystem::temp_directory_path() / "Project1Tests" / "OrderControllerTest" / testName;
    std::filesystem::remove_all(dir);
    return dir;
}

}  // namespace

TEST(OrderControllerTest, PlaceOrder_ExistingSampleId_CreatesReservedOrder)
{
    std::filesystem::path dir = MakeRepositoryDir("PlaceOrder_ExistingSampleId_CreatesReservedOrder");
    SampleRepository sampleRepository(dir / "samples.json");
    OrderRepository orderRepository(dir / "orders.json");
    sampleRepository.Add(Sample("S-001", "시료A", 10.0, 0.9, 0));
    FakeOrderView orderView;
    FakeProductionLineView productionView;
    FixedClock clock(std::chrono::system_clock::time_point{ std::chrono::seconds{ 1752537600 } });
    ProductionLineController productionLineController(orderRepository, sampleRepository, clock, productionView);
    OrderController controller(orderRepository, sampleRepository, orderView, clock, productionLineController);

    orderView.EnqueueNewOrderInput(NewOrderInput{ "S-001", "고객A", 5 });
    controller.PlaceOrder();

    std::vector<Order> orders = orderRepository.GetAll();
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0].Status, OrderStatus::RESERVED);
    EXPECT_EQ(orders[0].SampleId, "S-001");
    EXPECT_EQ(orders[0].Quantity, 5);
}

TEST(OrderControllerTest, PlaceOrder_UnknownSampleId_RejectedAndNoOrderCreated)
{
    std::filesystem::path dir = MakeRepositoryDir("PlaceOrder_UnknownSampleId_RejectedAndNoOrderCreated");
    SampleRepository sampleRepository(dir / "samples.json");
    OrderRepository orderRepository(dir / "orders.json");
    FakeOrderView orderView;
    FakeProductionLineView productionView;
    FixedClock clock(std::chrono::system_clock::time_point{ std::chrono::seconds{ 1752537600 } });
    ProductionLineController productionLineController(orderRepository, sampleRepository, clock, productionView);
    OrderController controller(orderRepository, sampleRepository, orderView, clock, productionLineController);

    orderView.EnqueueNewOrderInput(NewOrderInput{ "S-UNKNOWN", "고객A", 5 });
    controller.PlaceOrder();

    EXPECT_TRUE(orderRepository.GetAll().empty());
    EXPECT_FALSE(orderView.LastMessage().empty());
}

TEST(OrderControllerTest, PlaceOrder_QuantityLessThanOne_Rejected)
{
    std::filesystem::path dir = MakeRepositoryDir("PlaceOrder_QuantityLessThanOne_Rejected");
    SampleRepository sampleRepository(dir / "samples.json");
    OrderRepository orderRepository(dir / "orders.json");
    sampleRepository.Add(Sample("S-001", "시료A", 10.0, 0.9, 0));
    FakeOrderView orderView;
    FakeProductionLineView productionView;
    FixedClock clock(std::chrono::system_clock::time_point{ std::chrono::seconds{ 1752537600 } });
    ProductionLineController productionLineController(orderRepository, sampleRepository, clock, productionView);
    OrderController controller(orderRepository, sampleRepository, orderView, clock, productionLineController);

    orderView.EnqueueNewOrderInput(NewOrderInput{ "S-001", "고객A", 0 });
    controller.PlaceOrder();

    EXPECT_TRUE(orderRepository.GetAll().empty());
    EXPECT_FALSE(orderView.LastMessage().empty());
}

TEST(OrderControllerTest, PlaceOrder_AllocatesOrderIdByDailySequence)
{
    std::filesystem::path dir = MakeRepositoryDir("PlaceOrder_AllocatesOrderIdByDailySequence");
    SampleRepository sampleRepository(dir / "samples.json");
    OrderRepository orderRepository(dir / "orders.json");
    sampleRepository.Add(Sample("S-001", "시료A", 10.0, 0.9, 0));
    orderRepository.Add(Order("ORD-20260416-0007", "S-001", "기존고객", 1, std::chrono::system_clock::now()));
    FakeOrderView orderView;
    FakeProductionLineView productionView;
    FixedClock clock(JsonCodec::ParseIso8601("2026-04-16T12:00:00Z"));
    ProductionLineController productionLineController(orderRepository, sampleRepository, clock, productionView);
    OrderController controller(orderRepository, sampleRepository, orderView, clock, productionLineController);

    orderView.EnqueueNewOrderInput(NewOrderInput{ "S-001", "신규고객", 2 });
    controller.PlaceOrder();

    std::vector<Order> orders = orderRepository.GetAll();
    ASSERT_EQ(orders.size(), 2u);
    EXPECT_EQ(orders[1].OrderId, "ORD-20260416-0008");
}

TEST(OrderControllerTest, ShowReservedOrdersForApproval_ShowsOnlyReservedOrders)
{
    std::filesystem::path dir = MakeRepositoryDir("ShowReservedOrdersForApproval_ShowsOnlyReservedOrders");
    SampleRepository sampleRepository(dir / "samples.json");
    OrderRepository orderRepository(dir / "orders.json");
    Order reservedOrder("ORD-20260715-0001", "S-001", "고객A", 3, std::chrono::system_clock::now());
    orderRepository.Add(reservedOrder);
    Order rejectedOrder("ORD-20260715-0002", "S-001", "고객B", 2, std::chrono::system_clock::now());
    rejectedOrder.Status = OrderStatus::REJECTED;
    orderRepository.Add(rejectedOrder);
    FakeOrderView orderView;
    FakeProductionLineView productionView;
    FixedClock clock(std::chrono::system_clock::now());
    ProductionLineController productionLineController(orderRepository, sampleRepository, clock, productionView);
    OrderController controller(orderRepository, sampleRepository, orderView, clock, productionLineController);

    controller.ShowReservedOrdersForApproval();

    ASSERT_EQ(orderView.LastShownOrders().size(), 1u);
    EXPECT_EQ(orderView.LastShownOrders()[0].OrderId, "ORD-20260715-0001");
}

TEST(OrderControllerTest, ApproveOrder_SufficientAvailableStock_TransitionsToConfirmedWithoutStockChange)
{
    std::filesystem::path dir = MakeRepositoryDir("ApproveOrder_SufficientAvailableStock_TransitionsToConfirmedWithoutStockChange");
    SampleRepository sampleRepository(dir / "samples.json");
    OrderRepository orderRepository(dir / "orders.json");
    sampleRepository.Add(Sample("S-001", "시료A", 10.0, 0.9, 10));
    orderRepository.Add(Order("ORD-20260715-0001", "S-001", "고객A", 5, std::chrono::system_clock::now()));
    FakeOrderView orderView;
    FakeProductionLineView productionView;
    FixedClock clock(std::chrono::system_clock::now());
    ProductionLineController productionLineController(orderRepository, sampleRepository, clock, productionView);
    OrderController controller(orderRepository, sampleRepository, orderView, clock, productionLineController);

    orderView.EnqueueOrderId("ORD-20260715-0001");
    controller.ApproveOrder();

    EXPECT_EQ(orderRepository.FindById("ORD-20260715-0001")->Status, OrderStatus::CONFIRMED);
    EXPECT_EQ(sampleRepository.FindById("S-001")->Stock, 10);
}

TEST(OrderControllerTest, ApproveOrder_SecondOrderAfterFirstConfirmed_PreventsOverbooking)
{
    std::filesystem::path dir = MakeRepositoryDir("ApproveOrder_SecondOrderAfterFirstConfirmed_PreventsOverbooking");
    SampleRepository sampleRepository(dir / "samples.json");
    OrderRepository orderRepository(dir / "orders.json");
    sampleRepository.Add(Sample("S-001", "시료A", 10.0, 0.9, 10));
    orderRepository.Add(Order("ORD-20260715-0001", "S-001", "고객A", 7, std::chrono::system_clock::now()));
    orderRepository.Add(Order("ORD-20260715-0002", "S-001", "고객B", 5, std::chrono::system_clock::now()));
    FakeOrderView orderView;
    FakeProductionLineView productionView;
    FixedClock clock(std::chrono::system_clock::now());
    ProductionLineController productionLineController(orderRepository, sampleRepository, clock, productionView);
    OrderController controller(orderRepository, sampleRepository, orderView, clock, productionLineController);

    orderView.EnqueueOrderId("ORD-20260715-0001");
    controller.ApproveOrder();
    EXPECT_EQ(orderRepository.FindById("ORD-20260715-0001")->Status, OrderStatus::CONFIRMED);

    orderView.EnqueueOrderId("ORD-20260715-0002");
    controller.ApproveOrder();

    EXPECT_EQ(orderRepository.FindById("ORD-20260715-0002")->Status, OrderStatus::PRODUCING);
}

TEST(OrderControllerTest, ApproveOrder_InsufficientAvailableStock_TransitionsToProducingAndEnqueues)
{
    std::filesystem::path dir = MakeRepositoryDir("ApproveOrder_InsufficientAvailableStock_TransitionsToProducingAndEnqueues");
    SampleRepository sampleRepository(dir / "samples.json");
    OrderRepository orderRepository(dir / "orders.json");
    sampleRepository.Add(Sample("S-001", "시료A", 10.0, 1.0, 2));
    orderRepository.Add(Order("ORD-20260715-0001", "S-001", "고객A", 5, std::chrono::system_clock::now()));
    FakeOrderView orderView;
    FakeProductionLineView productionView;
    FixedClock clock(std::chrono::system_clock::now());
    ProductionLineController productionLineController(orderRepository, sampleRepository, clock, productionView);
    OrderController controller(orderRepository, sampleRepository, orderView, clock, productionLineController);

    orderView.EnqueueOrderId("ORD-20260715-0001");
    controller.ApproveOrder();

    EXPECT_EQ(orderRepository.FindById("ORD-20260715-0001")->Status, OrderStatus::PRODUCING);

    productionLineController.ShowProductionQueue();
    ASSERT_EQ(productionView.LastShownItems().size(), 1u);
    EXPECT_EQ(productionView.LastShownItems()[0].Shortage, 3);
    EXPECT_EQ(productionView.LastShownItems()[0].ActualProductionQuantity, 3);
}

TEST(OrderControllerTest, ApproveOrder_OrderNotReserved_Rejected)
{
    std::filesystem::path dir = MakeRepositoryDir("ApproveOrder_OrderNotReserved_Rejected");
    SampleRepository sampleRepository(dir / "samples.json");
    OrderRepository orderRepository(dir / "orders.json");
    sampleRepository.Add(Sample("S-001", "시료A", 10.0, 0.9, 10));
    Order order("ORD-20260715-0001", "S-001", "고객A", 5, std::chrono::system_clock::now());
    order.Status = OrderStatus::CONFIRMED;
    orderRepository.Add(order);
    FakeOrderView orderView;
    FakeProductionLineView productionView;
    FixedClock clock(std::chrono::system_clock::now());
    ProductionLineController productionLineController(orderRepository, sampleRepository, clock, productionView);
    OrderController controller(orderRepository, sampleRepository, orderView, clock, productionLineController);

    orderView.EnqueueOrderId("ORD-20260715-0001");
    controller.ApproveOrder();

    EXPECT_FALSE(orderView.LastMessage().empty());
    EXPECT_EQ(orderRepository.FindById("ORD-20260715-0001")->Status, OrderStatus::CONFIRMED);
}

TEST(OrderControllerTest, RejectOrder_ReservedOrder_TransitionsToRejected)
{
    std::filesystem::path dir = MakeRepositoryDir("RejectOrder_ReservedOrder_TransitionsToRejected");
    SampleRepository sampleRepository(dir / "samples.json");
    OrderRepository orderRepository(dir / "orders.json");
    sampleRepository.Add(Sample("S-001", "시료A", 10.0, 0.9, 10));
    orderRepository.Add(Order("ORD-20260715-0001", "S-001", "고객A", 5, std::chrono::system_clock::now()));
    FakeOrderView orderView;
    FakeProductionLineView productionView;
    FixedClock clock(std::chrono::system_clock::now());
    ProductionLineController productionLineController(orderRepository, sampleRepository, clock, productionView);
    OrderController controller(orderRepository, sampleRepository, orderView, clock, productionLineController);

    orderView.EnqueueOrderId("ORD-20260715-0001");
    controller.RejectOrder();

    EXPECT_EQ(orderRepository.FindById("ORD-20260715-0001")->Status, OrderStatus::REJECTED);
}

TEST(OrderControllerTest, ApproveOrRejectOrder_UnknownOrderId_ShowsErrorMessage)
{
    std::filesystem::path dir = MakeRepositoryDir("ApproveOrRejectOrder_UnknownOrderId_ShowsErrorMessage");
    SampleRepository sampleRepository(dir / "samples.json");
    OrderRepository orderRepository(dir / "orders.json");
    FakeOrderView orderView;
    FakeProductionLineView productionView;
    FixedClock clock(std::chrono::system_clock::now());
    ProductionLineController productionLineController(orderRepository, sampleRepository, clock, productionView);
    OrderController controller(orderRepository, sampleRepository, orderView, clock, productionLineController);

    orderView.EnqueueOrderId("ORD-UNKNOWN");
    controller.ApproveOrder();
    EXPECT_FALSE(orderView.LastMessage().empty());

    orderView.EnqueueOrderId("ORD-UNKNOWN");
    controller.RejectOrder();
    EXPECT_FALSE(orderView.LastMessage().empty());
}

TEST(OrderControllerTest, ReleaseOrder_ConfirmedOrder_TransitionsToReleaseAndDecrementsStock)
{
    std::filesystem::path dir = MakeRepositoryDir("ReleaseOrder_ConfirmedOrder_TransitionsToReleaseAndDecrementsStock");
    SampleRepository sampleRepository(dir / "samples.json");
    OrderRepository orderRepository(dir / "orders.json");
    sampleRepository.Add(Sample("S-001", "시료A", 10.0, 0.9, 10));
    Order order("ORD-20260715-0001", "S-001", "고객A", 4, std::chrono::system_clock::now());
    order.Status = OrderStatus::CONFIRMED;
    orderRepository.Add(order);
    FakeOrderView orderView;
    FakeProductionLineView productionView;
    FixedClock clock(std::chrono::system_clock::now());
    ProductionLineController productionLineController(orderRepository, sampleRepository, clock, productionView);
    OrderController controller(orderRepository, sampleRepository, orderView, clock, productionLineController);

    orderView.EnqueueOrderId("ORD-20260715-0001");
    controller.ReleaseOrder();

    EXPECT_EQ(orderRepository.FindById("ORD-20260715-0001")->Status, OrderStatus::RELEASE);
    EXPECT_EQ(sampleRepository.FindById("S-001")->Stock, 6);
}

TEST(OrderControllerTest, ReleaseOrder_NotConfirmed_RejectedAndStockUnchanged)
{
    std::filesystem::path dir = MakeRepositoryDir("ReleaseOrder_NotConfirmed_RejectedAndStockUnchanged");
    SampleRepository sampleRepository(dir / "samples.json");
    OrderRepository orderRepository(dir / "orders.json");
    sampleRepository.Add(Sample("S-001", "시료A", 10.0, 0.9, 10));
    orderRepository.Add(Order("ORD-20260715-0001", "S-001", "고객A", 4, std::chrono::system_clock::now()));
    FakeOrderView orderView;
    FakeProductionLineView productionView;
    FixedClock clock(std::chrono::system_clock::now());
    ProductionLineController productionLineController(orderRepository, sampleRepository, clock, productionView);
    OrderController controller(orderRepository, sampleRepository, orderView, clock, productionLineController);

    orderView.EnqueueOrderId("ORD-20260715-0001");
    controller.ReleaseOrder();

    EXPECT_EQ(orderRepository.FindById("ORD-20260715-0001")->Status, OrderStatus::RESERVED);
    EXPECT_EQ(sampleRepository.FindById("S-001")->Stock, 10);
}

TEST(OrderControllerTest, ReleaseOrder_UnknownOrderId_ShowsErrorMessage)
{
    std::filesystem::path dir = MakeRepositoryDir("ReleaseOrder_UnknownOrderId_ShowsErrorMessage");
    SampleRepository sampleRepository(dir / "samples.json");
    OrderRepository orderRepository(dir / "orders.json");
    FakeOrderView orderView;
    FakeProductionLineView productionView;
    FixedClock clock(std::chrono::system_clock::now());
    ProductionLineController productionLineController(orderRepository, sampleRepository, clock, productionView);
    OrderController controller(orderRepository, sampleRepository, orderView, clock, productionLineController);

    orderView.EnqueueOrderId("ORD-UNKNOWN");
    controller.ReleaseOrder();

    EXPECT_FALSE(orderView.LastMessage().empty());
}

TEST(OrderControllerTest, ShowConfirmedOrdersForRelease_NoConfirmedOrders_ShowsNoTargetMessage)
{
    std::filesystem::path dir = MakeRepositoryDir("ShowConfirmedOrdersForRelease_NoConfirmedOrders_ShowsNoTargetMessage");
    SampleRepository sampleRepository(dir / "samples.json");
    OrderRepository orderRepository(dir / "orders.json");
    FakeOrderView orderView;
    FakeProductionLineView productionView;
    FixedClock clock(std::chrono::system_clock::now());
    ProductionLineController productionLineController(orderRepository, sampleRepository, clock, productionView);
    OrderController controller(orderRepository, sampleRepository, orderView, clock, productionLineController);

    controller.ShowConfirmedOrdersForRelease();

    EXPECT_TRUE(orderView.LastShownOrders().empty());
    EXPECT_FALSE(orderView.LastMessage().empty());
}
