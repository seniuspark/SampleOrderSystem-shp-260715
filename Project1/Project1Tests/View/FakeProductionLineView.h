#pragma once

#include <string>
#include <vector>

#include "Controller/ProductionQueueItem.h"
#include "View/IProductionLineView.h"

class FakeProductionLineView : public IProductionLineView
{
public:
    void ShowProductionQueue(const std::vector<ProductionQueueItem>& items) override
    {
        lastShownItems_ = items;
    }

    void ShowMessage(const std::string& message) override
    {
        lastMessage_ = message;
    }

    const std::vector<ProductionQueueItem>& LastShownItems() const { return lastShownItems_; }
    const std::string& LastMessage() const { return lastMessage_; }

private:
    std::vector<ProductionQueueItem> lastShownItems_;
    std::string lastMessage_;
};
