#pragma once

#include <string>
#include <vector>

#include "Model/OrderStatus.h"
#include "Model/StockStatus.h"

struct OrderStatusCount
{
    OrderStatus Status;
    int Count;
};

struct SampleStockLevel
{
    std::string SampleId;
    int Stock;
    StockStatus Status;
};

class IMonitoringView
{
public:
    virtual ~IMonitoringView() = default;

    virtual void ShowOrderStatusCounts(const std::vector<OrderStatusCount>& counts) = 0;
    virtual void ShowSampleStockLevels(const std::vector<SampleStockLevel>& levels) = 0;
    virtual void ShowMessage(const std::string& message) = 0;
};
