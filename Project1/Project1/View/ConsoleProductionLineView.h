#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "Controller/ProductionQueueItem.h"
#include "View/IProductionLineView.h"

class ConsoleProductionLineView : public IProductionLineView
{
public:
    explicit ConsoleProductionLineView(std::ostream& out = std::cout)
        : out_(out)
    {
    }

    void ShowProductionQueue(const std::vector<ProductionQueueItem>& items) override
    {
        out_ << "OrderId\tSampleId\tOrderQuantity\tShortage\tActualProductionQuantity\tTotalProductionTimeMinutes\tState\n";
        for (std::size_t index = 0; index < items.size(); ++index)
        {
            const ProductionQueueItem& item = items[index];
            const bool isFront = (index == 0);

            out_ << item.OrderId << '\t'
                 << item.SampleId << '\t'
                 << item.OrderQuantity << '\t'
                 << item.Shortage << '\t'
                 << item.ActualProductionQuantity << '\t'
                 << item.TotalProductionTimeMinutes << '\t'
                 << (isFront ? "생산중" : "대기") << '\n';
        }
    }

    void ShowMessage(const std::string& message) override
    {
        out_ << message << '\n';
    }

private:
    std::ostream& out_;
};
