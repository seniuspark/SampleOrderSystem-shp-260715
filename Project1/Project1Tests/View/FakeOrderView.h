#pragma once

#include <queue>
#include <string>
#include <vector>

#include "Model/Order.h"
#include "View/IOrderView.h"

class FakeOrderView : public IOrderView
{
public:
    void EnqueueNewOrderInput(NewOrderInput input) { orderInputs_.push(std::move(input)); }
    void EnqueueOrderId(std::string orderId) { orderIds_.push(std::move(orderId)); }

    NewOrderInput ReadNewOrderInput() override
    {
        NewOrderInput input = orderInputs_.front();
        orderInputs_.pop();
        return input;
    }

    std::string ReadOrderId() override
    {
        std::string orderId = orderIds_.front();
        orderIds_.pop();
        return orderId;
    }

    void ShowOrderList(const std::vector<Order>& orders) override
    {
        lastShownOrders_ = orders;
    }

    void ShowMessage(const std::string& message) override
    {
        lastMessage_ = message;
    }

    const std::vector<Order>& LastShownOrders() const { return lastShownOrders_; }
    const std::string& LastMessage() const { return lastMessage_; }

private:
    std::queue<NewOrderInput> orderInputs_;
    std::queue<std::string> orderIds_;
    std::vector<Order> lastShownOrders_;
    std::string lastMessage_;
};
