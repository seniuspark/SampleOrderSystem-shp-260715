#pragma once

#include <algorithm>
#include <chrono>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "Model/AvailableStockCalculator.h"
#include "Model/IClock.h"
#include "Model/Order.h"
#include "Model/OrderIdAllocator.h"
#include "Model/OrderStatus.h"
#include "Model/Sample.h"
#include "Controller/ProductionLineController.h"
#include "Repository/OrderRepository.h"
#include "Repository/SampleRepository.h"
#include "Repository/JsonCodec.h"
#include "View/IOrderView.h"

class OrderController
{
public:
    OrderController(
        OrderRepository& orderRepository,
        SampleRepository& sampleRepository,
        IOrderView& view,
        const IClock& clock,
        ProductionLineController& productionLineController)
        : orderRepository_(orderRepository)
        , sampleRepository_(sampleRepository)
        , view_(view)
        , clock_(clock)
        , productionLineController_(productionLineController)
    {
    }

    void PlaceOrder()
    {
        const NewOrderInput input = view_.ReadNewOrderInput();

        if (!sampleRepository_.FindById(input.SampleId).has_value())
        {
            view_.ShowMessage("존재하지 않는 시료 ID입니다: " + input.SampleId);
            return;
        }

        try
        {
            const std::string orderId = AllocateOrderId(CollectExistingOrderIds(), FormatDate(clock_.Now()));
            Order order(orderId, input.SampleId, input.CustomerName, input.Quantity, clock_.Now());
            orderRepository_.Add(order);
            view_.ShowMessage("주문이 접수되었습니다: " + orderId);
        }
        catch (const std::invalid_argument& error)
        {
            view_.ShowMessage(std::string("주문 접수에 실패했습니다: ") + error.what());
        }
    }

    void ShowReservedOrdersForApproval()
    {
        const std::vector<Order> reservedOrders = FindOrdersByStatus(OrderStatus::RESERVED);
        if (reservedOrders.empty())
        {
            view_.ShowMessage("승인 대기 중인 주문이 없습니다.");
            return;
        }
        view_.ShowOrderList(reservedOrders);
    }

    void ApproveOrder()
    {
        const std::string orderId = view_.ReadOrderId();
        std::optional<Order> order = FindOrderOrShowNotFoundMessage(orderId);
        if (!order.has_value())
        {
            return;
        }
        if (order->Status != OrderStatus::RESERVED)
        {
            view_.ShowMessage("이미 처리된 주문은 승인할 수 없습니다: " + orderId);
            return;
        }

        std::optional<Sample> sample = sampleRepository_.FindById(order->SampleId);
        if (!sample.has_value())
        {
            view_.ShowMessage("존재하지 않는 시료입니다: " + order->SampleId);
            return;
        }

        const int unreleasedConfirmedQuantity = SumUnreleasedConfirmedQuantity(order->SampleId);
        const int availableStock = CalculateAvailableStock(sample->Stock, unreleasedConfirmedQuantity);

        if (availableStock >= order->Quantity)
        {
            order->Status = OrderStatus::CONFIRMED;
            orderRepository_.Update(*order);
            view_.ShowMessage("주문이 승인되어 출고 대기 상태입니다: " + orderId);
        }
        else
        {
            // 생산 큐의 부족분/실생산량 계산은 물리적 Stock이 아니라 가용재고
            // 기준이어야 하므로(docs/PRD.md 2-1~2-3), 큐 등록 시 전달하는
            // Sample의 Stock을 가용재고로 대체한다. order를 PRODUCING으로
            // 전환하고 Repository에 반영하는 것은 EnqueueProductionItem이
            // 담당한다(생산 큐 복원에 필요한 메타데이터도 함께 영속화).
            Sample sampleWithAvailableStock = *sample;
            sampleWithAvailableStock.Stock = availableStock;
            productionLineController_.EnqueueProductionItem(*order, sampleWithAvailableStock);
            view_.ShowMessage("재고 부족으로 생산 큐에 등록되었습니다: " + orderId);
        }
    }

    void RejectOrder()
    {
        const std::string orderId = view_.ReadOrderId();
        std::optional<Order> order = FindOrderOrShowNotFoundMessage(orderId);
        if (!order.has_value())
        {
            return;
        }
        if (order->Status != OrderStatus::RESERVED)
        {
            view_.ShowMessage("이미 처리된 주문은 거절할 수 없습니다: " + orderId);
            return;
        }

        order->Status = OrderStatus::REJECTED;
        orderRepository_.Update(*order);
        view_.ShowMessage("주문이 거절되었습니다: " + orderId);
    }

    void ShowConfirmedOrdersForRelease()
    {
        const std::vector<Order> confirmedOrders = FindOrdersByStatus(OrderStatus::CONFIRMED);
        if (confirmedOrders.empty())
        {
            view_.ShowMessage("출고 대상 주문이 없습니다.");
            return;
        }
        view_.ShowOrderList(confirmedOrders);
    }

    void ReleaseOrder()
    {
        const std::string orderId = view_.ReadOrderId();
        std::optional<Order> order = FindOrderOrShowNotFoundMessage(orderId);
        if (!order.has_value())
        {
            return;
        }
        if (order->Status != OrderStatus::CONFIRMED)
        {
            view_.ShowMessage("출고 대기 상태가 아닌 주문은 출고할 수 없습니다: " + orderId);
            return;
        }

        std::optional<Sample> sample = sampleRepository_.FindById(order->SampleId);
        if (!sample.has_value())
        {
            view_.ShowMessage("존재하지 않는 시료입니다: " + order->SampleId);
            return;
        }

        order->Status = OrderStatus::RELEASE;
        orderRepository_.Update(*order);

        sample->Stock -= order->Quantity;
        sampleRepository_.Update(*sample);

        view_.ShowMessage("출고가 완료되었습니다: " + orderId);
    }

private:
    std::optional<Order> FindOrderOrShowNotFoundMessage(const std::string& orderId)
    {
        std::optional<Order> order = orderRepository_.FindById(orderId);
        if (!order.has_value())
        {
            view_.ShowMessage("존재하지 않는 주문 ID입니다: " + orderId);
        }
        return order;
    }

    std::vector<std::string> CollectExistingOrderIds() const
    {
        std::vector<std::string> orderIds;
        for (const Order& order : orderRepository_.GetAll())
        {
            orderIds.push_back(order.OrderId);
        }
        return orderIds;
    }

    std::vector<Order> FindOrdersByStatus(OrderStatus status) const
    {
        std::vector<Order> matched;
        for (const Order& order : orderRepository_.GetAll())
        {
            if (order.Status == status)
            {
                matched.push_back(order);
            }
        }
        return matched;
    }

    int SumUnreleasedConfirmedQuantity(const std::string& sampleId) const
    {
        int total = 0;
        for (const Order& order : orderRepository_.GetAll())
        {
            if (order.SampleId == sampleId && order.Status == OrderStatus::CONFIRMED)
            {
                total += order.Quantity;
            }
        }
        return total;
    }

    static std::string FormatDate(std::chrono::system_clock::time_point timePoint)
    {
        static constexpr int YearOffset = 0;
        static constexpr int YearLength = 4;
        static constexpr int MonthOffset = 5;
        static constexpr int MonthLength = 2;
        static constexpr int DayOffset = 8;
        static constexpr int DayLength = 2;

        const std::string iso8601 = JsonCodec::FormatIso8601(timePoint);
        return iso8601.substr(YearOffset, YearLength) +
            iso8601.substr(MonthOffset, MonthLength) +
            iso8601.substr(DayOffset, DayLength);
    }

    OrderRepository& orderRepository_;
    SampleRepository& sampleRepository_;
    IOrderView& view_;
    const IClock& clock_;
    ProductionLineController& productionLineController_;
};
