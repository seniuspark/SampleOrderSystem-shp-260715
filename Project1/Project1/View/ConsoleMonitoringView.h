#pragma once

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "Model/OrderStatus.h"
#include "Model/StockStatus.h"
#include "View/IMonitoringView.h"

class ConsoleMonitoringView : public IMonitoringView
{
public:
    explicit ConsoleMonitoringView(std::ostream& out = std::cout)
        : out_(out)
    {
    }

    void ShowOrderStatusCounts(const std::vector<OrderStatusCount>& counts) override
    {
        out_ << "=== 주문 상태별 건수 ===\n";
        for (const OrderStatusCount& count : counts)
        {
            out_ << ToString(count.Status) << ": " << count.Count << "건\n";
        }
    }

    void ShowSampleStockLevels(const std::vector<SampleStockLevel>& levels) override
    {
        out_ << "=== 시료별 재고 현황 ===\n";
        for (const SampleStockLevel& level : levels)
        {
            out_ << level.SampleId << ": " << level.Stock << " (" << StockStatusLabel(level.Status) << ")\n";
        }
    }

    void ShowMessage(const std::string& message) override
    {
        out_ << message << '\n';
    }

private:
    // 재고 여유/부족/고갈 판정 자체는 Model(JudgeStockStatus)이 이미 내린 결과이며,
    // 여기서는 그 결과를 화면에 표시할 한글 라벨로 옮기는 순수 서식 변환만 한다.
    static std::string StockStatusLabel(StockStatus status)
    {
        switch (status)
        {
        case StockStatus::DEPLETED:
            return "고갈";
        case StockStatus::SHORTAGE:
            return "부족";
        case StockStatus::SUFFICIENT:
            return "여유";
        }
        throw std::invalid_argument("Unknown StockStatus");
    }

    std::ostream& out_;
};
