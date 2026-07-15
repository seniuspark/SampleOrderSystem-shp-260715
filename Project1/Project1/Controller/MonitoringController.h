#pragma once

#include <array>
#include <string>
#include <vector>

#include "Model/Order.h"
#include "Model/OrderStatus.h"
#include "Model/Sample.h"
#include "Model/StockStatus.h"
#include "Repository/OrderRepository.h"
#include "Repository/SampleRepository.h"
#include "View/IMonitoringView.h"

class MonitoringController
{
public:
    MonitoringController(const OrderRepository& orderRepository, const SampleRepository& sampleRepository, IMonitoringView& view)
        : orderRepository_(orderRepository), sampleRepository_(sampleRepository), view_(view)
    {
    }

    void ShowMonitoring()
    {
        const std::vector<Order> orders = orderRepository_.GetAll();
        const std::vector<Sample> samples = sampleRepository_.GetAll();

        if (orders.empty() && samples.empty())
        {
            view_.ShowMessage("표시할 데이터가 없습니다.");
            return;
        }

        view_.ShowOrderStatusCounts(CountOrdersByStatus(orders));
        view_.ShowSampleStockLevels(BuildSampleStockLevels(orders, samples));
    }

private:
    static constexpr std::array<OrderStatus, 4> MonitoredStatuses = {
        OrderStatus::RESERVED,
        OrderStatus::CONFIRMED,
        OrderStatus::PRODUCING,
        OrderStatus::RELEASE,
    };

    static std::vector<OrderStatusCount> CountOrdersByStatus(const std::vector<Order>& orders)
    {
        std::vector<OrderStatusCount> counts;
        for (OrderStatus status : MonitoredStatuses)
        {
            int count = 0;
            for (const Order& order : orders)
            {
                if (order.Status == status)
                {
                    ++count;
                }
            }
            counts.push_back(OrderStatusCount{ status, count });
        }
        return counts;
    }

    static int CalculateDemand(const std::string& sampleId, const std::vector<Order>& orders)
    {
        int demand = 0;
        for (const Order& order : orders)
        {
            if (order.SampleId != sampleId)
            {
                continue;
            }
            if (order.Status == OrderStatus::RESERVED || order.Status == OrderStatus::CONFIRMED || order.Status == OrderStatus::PRODUCING)
            {
                demand += order.Quantity;
            }
        }
        return demand;
    }

    static std::vector<SampleStockLevel> BuildSampleStockLevels(const std::vector<Order>& orders, const std::vector<Sample>& samples)
    {
        std::vector<SampleStockLevel> levels;
        for (const Sample& sample : samples)
        {
            const int demand = CalculateDemand(sample.SampleId, orders);
            levels.push_back(SampleStockLevel{ sample.SampleId, sample.Stock, JudgeStockStatus(sample.Stock, demand) });
        }
        return levels;
    }

    const OrderRepository& orderRepository_;
    const SampleRepository& sampleRepository_;
    IMonitoringView& view_;
};
