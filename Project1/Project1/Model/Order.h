#pragma once

#include <chrono>
#include <stdexcept>
#include <string>

#include "OrderStatus.h"

class Order
{
public:
    Order(std::string orderId, std::string sampleId, std::string customerName, int quantity, std::chrono::system_clock::time_point createdAt)
        : OrderId(std::move(orderId)), SampleId(std::move(sampleId)), CustomerName(std::move(customerName)), Quantity(quantity), Status(OrderStatus::RESERVED), CreatedAt(createdAt)
    {
        ValidateInvariants();
    }

    std::string OrderId;
    std::string SampleId;
    std::string CustomerName;
    int Quantity;
    OrderStatus Status;
    std::chrono::system_clock::time_point CreatedAt;

private:
    static constexpr int MinQuantity = 1;

    void ValidateInvariants() const
    {
        if (Quantity < MinQuantity)
        {
            throw std::invalid_argument("Quantity must be at least 1");
        }
    }
};
