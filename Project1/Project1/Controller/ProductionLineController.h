#pragma once

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
    }

    void EnqueueProductionItem(const Order& order, const Sample& sample)
    {
        const int shortage = CalculateShortage(order.Quantity, sample.Stock);
        const int actualProductionQuantity = CalculateActualProductionQuantity(shortage, sample.Yield);
        const double totalProductionTimeMinutes = CalculateTotalProductionTime(sample.AvgProductionTime, actualProductionQuantity);

        queue_.push_back(ProductionQueueItem{
            order.OrderId,
            order.SampleId,
            order.Quantity,
            shortage,
            actualProductionQuantity,
            totalProductionTimeMinutes,
            clock_.Now(),
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

private:
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
