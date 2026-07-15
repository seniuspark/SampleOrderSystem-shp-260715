#pragma once

#include <string>
#include <vector>

#include "Controller/ProductionQueueItem.h"

class IProductionLineView
{
public:
    virtual ~IProductionLineView() = default;

    virtual void ShowProductionQueue(const std::vector<ProductionQueueItem>& items) = 0;
    virtual void ShowMessage(const std::string& message) = 0;
};
