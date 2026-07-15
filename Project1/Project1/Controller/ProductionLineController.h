#pragma once

#include <algorithm>
#include <deque>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "Model/IClock.h"
#include "Model/Order.h"
#include "Model/OrderStatus.h"
#include "Model/ProductionCompletionJudge.h"
#include "Model/ProductionQueueCalculator.h"
#include "Model/Sample.h"
#include "Controller/ProductionQueueItem.h"
#include "Repository/OrderRepository.h"
#include "Repository/SampleRepository.h"
#include "View/IProductionLineView.h"

class ProductionLineController
{
public:
    ProductionLineController(OrderRepository& orderRepository, SampleRepository& sampleRepository, const IClock& clock, IProductionLineView& view)
        : orderRepository_(orderRepository), sampleRepository_(sampleRepository), clock_(clock), view_(view)
    {
        RebuildQueueFromPersistedOrders();
    }

    // 부족분/실생산량/총생산시간을 계산해 order를 PRODUCING으로 전환하고,
    // 그 값들을 order에 함께 기록해 Repository에 영속화한다(생산 큐 자체는
    // 프로세스 메모리에만 있으므로, 앱 재시작 후에도 진행 중이던 항목을
    // 복원하려면 이 값들이 파일에 남아 있어야 한다).
    void EnqueueProductionItem(Order order, const Sample& sample)
    {
        const int shortage = CalculateShortage(order.Quantity, sample.Stock);
        const int actualProductionQuantity = CalculateActualProductionQuantity(shortage, sample.Yield);
        const double totalProductionTimeMinutes = CalculateTotalProductionTime(sample.AvgProductionTime, actualProductionQuantity);
        const std::chrono::system_clock::time_point startedAt = clock_.Now();

        order.Status = OrderStatus::PRODUCING;
        order.ProductionShortage = shortage;
        order.ProductionActualQuantity = actualProductionQuantity;
        order.ProductionTotalTimeMinutes = totalProductionTimeMinutes;
        order.ProductionStartedAt = startedAt;
        orderRepository_.Update(order);

        queue_.push_back(ProductionQueueItem{
            order.OrderId,
            order.SampleId,
            order.Quantity,
            shortage,
            actualProductionQuantity,
            totalProductionTimeMinutes,
            startedAt,
            });
    }

    void ShowProductionQueue()
    {
        CompleteFrontIfDue();

        if (queue_.empty())
        {
            view_.ShowMessage("생산 대기 중인 항목이 없습니다.");
            return;
        }

        view_.ShowProductionQueue(std::vector<ProductionQueueItem>(queue_.begin(), queue_.end()));
    }

    std::size_t QueueSize() const
    {
        return queue_.size();
    }

private:
    // 생성자에서 호출된다. Repository에 PRODUCING 상태로 남아있는 주문 중
    // 생산 큐 메타데이터(ProductionStartedAt 등)가 기록된 것들을 시작 시각
    // 순서(FIFO)로 복원한다. 메타데이터가 없는 PRODUCING 주문(이 복원 로직이
    // 도입되기 전 데이터)은 자동 완료 판정 대상에서 제외한다(fail-soft).
    void RebuildQueueFromPersistedOrders()
    {
        std::vector<Order> producingOrders;
        for (const Order& order : orderRepository_.GetAll())
        {
            if (order.Status == OrderStatus::PRODUCING && order.ProductionStartedAt.has_value())
            {
                producingOrders.push_back(order);
            }
        }

        std::sort(producingOrders.begin(), producingOrders.end(), [](const Order& left, const Order& right) {
            return *left.ProductionStartedAt < *right.ProductionStartedAt;
        });

        for (const Order& order : producingOrders)
        {
            queue_.push_back(ProductionQueueItem{
                order.OrderId,
                order.SampleId,
                order.Quantity,
                *order.ProductionShortage,
                *order.ProductionActualQuantity,
                *order.ProductionTotalTimeMinutes,
                *order.ProductionStartedAt,
                });
        }
    }

    void CompleteFrontIfDue()
    {
        if (queue_.empty())
        {
            return;
        }

        const ProductionQueueItem& frontItem = queue_.front();
        if (!IsProductionComplete(frontItem.ProductionStartedAt, frontItem.TotalProductionTimeMinutes, clock_))
        {
            return;
        }

        std::optional<Order> order = orderRepository_.FindById(frontItem.OrderId);
        std::optional<Sample> sample = sampleRepository_.FindById(frontItem.SampleId);
        if (!order.has_value() || !sample.has_value())
        {
            throw std::runtime_error("생산 큐 항목에 해당하는 주문 또는 시료를 찾을 수 없습니다.");
        }

        order->Status = OrderStatus::CONFIRMED;
        orderRepository_.Update(*order);

        sample->Stock += frontItem.ActualProductionQuantity;
        sampleRepository_.Update(*sample);

        queue_.pop_front();
    }

    OrderRepository& orderRepository_;
    SampleRepository& sampleRepository_;
    const IClock& clock_;
    IProductionLineView& view_;
    std::deque<ProductionQueueItem> queue_;
};
