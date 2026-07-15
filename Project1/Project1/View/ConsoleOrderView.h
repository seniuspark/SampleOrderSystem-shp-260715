#pragma once

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include "Model/Order.h"
#include "Model/OrderStatus.h"
#include "View/IOrderView.h"

class ConsoleOrderView : public IOrderView
{
public:
    ConsoleOrderView(std::istream& in = std::cin, std::ostream& out = std::cout)
        : in_(in), out_(out)
    {
    }

    NewOrderInput ReadNewOrderInput() override
    {
        NewOrderInput input;

        out_ << "SampleId: ";
        std::getline(in_, input.SampleId);

        out_ << "CustomerName: ";
        std::getline(in_, input.CustomerName);

        out_ << "Quantity: ";
        in_ >> input.Quantity;
        in_.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        return input;
    }

    std::string ReadOrderId() override
    {
        out_ << "OrderId: ";
        std::string orderId;
        std::getline(in_, orderId);
        return orderId;
    }

    void ShowOrderList(const std::vector<Order>& orders) override
    {
        out_ << "OrderId\tSampleId\tCustomerName\tQuantity\tStatus\tCreatedAt\n";
        for (const Order& order : orders)
        {
            out_ << order.OrderId << '\t'
                 << order.SampleId << '\t'
                 << order.CustomerName << '\t'
                 << order.Quantity << '\t'
                 << ToString(order.Status) << '\t'
                 << FormatForDisplay(order.CreatedAt) << '\n';
        }
    }

    void ShowMessage(const std::string& message) override
    {
        out_ << message << '\n';
    }

private:
    static constexpr const char* DisplayTimeFormat = "%Y-%m-%d %H:%M";

    // 시간 표시는 순수 서식 변환일 뿐 재고/수율 비교 같은 도메인 판단이 아니므로
    // View 계층에 두되, Repository의 JsonCodec에는 의존하지 않는다(계층 경계 유지).
    static std::string FormatForDisplay(std::chrono::system_clock::time_point timePoint)
    {
        std::time_t time = std::chrono::system_clock::to_time_t(timePoint);
        std::tm utcTime{};
        gmtime_s(&utcTime, &time);

        std::ostringstream stream;
        stream << std::put_time(&utcTime, DisplayTimeFormat);
        return stream.str();
    }

    std::istream& in_;
    std::ostream& out_;
};
