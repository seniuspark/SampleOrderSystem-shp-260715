#pragma once

#include <chrono>
#include <optional>
#include <stdexcept>
#include <string>

#include "Model/OrderStatus.h"

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

    // PRODUCING 상태로 전환될 때 생산 큐 등록 시점에 계산된 값을 함께 저장해
    // 둔다. 생산 큐 자체는 프로세스 메모리에만 있으므로(재시작 시 소실),
    // 앱을 재시작해도 진행 중인 생산 큐를 복원할 수 있도록 이 값들을
    // Repository에 함께 영속화한다(ProductionLineController가 기록/복원).
    std::optional<int> ProductionShortage;
    std::optional<int> ProductionActualQuantity;
    std::optional<double> ProductionTotalTimeMinutes;
    std::optional<std::chrono::system_clock::time_point> ProductionStartedAt;

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
