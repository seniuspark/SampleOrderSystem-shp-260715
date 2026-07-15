#pragma once

#include <string>
#include <vector>

#include "View/IMonitoringView.h"

class FakeMonitoringView : public IMonitoringView
{
public:
    void ShowOrderStatusCounts(const std::vector<OrderStatusCount>& counts) override
    {
        lastCounts_ = counts;
    }

    void ShowSampleStockLevels(const std::vector<SampleStockLevel>& levels) override
    {
        lastLevels_ = levels;
    }

    void ShowMessage(const std::string& message) override
    {
        lastMessage_ = message;
    }

    const std::vector<OrderStatusCount>& LastCounts() const { return lastCounts_; }
    const std::vector<SampleStockLevel>& LastLevels() const { return lastLevels_; }
    const std::string& LastMessage() const { return lastMessage_; }

private:
    std::vector<OrderStatusCount> lastCounts_;
    std::vector<SampleStockLevel> lastLevels_;
    std::string lastMessage_;
};
