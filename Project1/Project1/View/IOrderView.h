#pragma once

#include <string>
#include <vector>

#include "Model/Order.h"

struct NewOrderInput
{
    std::string SampleId;
    std::string CustomerName;
    int Quantity;
};

class IOrderView
{
public:
    virtual ~IOrderView() = default;

    virtual NewOrderInput ReadNewOrderInput() = 0;
    virtual std::string ReadOrderId() = 0;
    virtual void ShowOrderList(const std::vector<Order>& orders) = 0;
    virtual void ShowMessage(const std::string& message) = 0;
};
