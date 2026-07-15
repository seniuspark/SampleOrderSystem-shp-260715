#pragma once

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

#include "nlohmann/json.hpp"

#include "Model/Order.h"
#include "Model/OrderStatus.h"
#include "Model/Sample.h"

namespace JsonCodec
{

inline std::string FormatIso8601(std::chrono::system_clock::time_point timePoint)
{
    std::time_t time = std::chrono::system_clock::to_time_t(timePoint);
    std::tm utcTime{};
    gmtime_s(&utcTime, &time);

    std::ostringstream stream;
    stream << std::put_time(&utcTime, "%Y-%m-%dT%H:%M:%SZ");
    return stream.str();
}

inline std::chrono::system_clock::time_point ParseIso8601(const std::string& text)
{
    std::tm utcTime{};
    std::istringstream stream(text);
    stream >> std::get_time(&utcTime, "%Y-%m-%dT%H:%M:%SZ");

    return std::chrono::system_clock::from_time_t(_mkgmtime(&utcTime));
}

inline nlohmann::json ToJson(const Sample& sample)
{
    return nlohmann::json{
        {"sampleId", sample.SampleId},
        {"name", sample.Name},
        {"avgProductionTime", sample.AvgProductionTime},
        {"yield", sample.Yield},
        {"stock", sample.Stock},
    };
}

inline Sample SampleFromJson(const nlohmann::json& json)
{
    return Sample(
        json.at("sampleId").get<std::string>(),
        json.at("name").get<std::string>(),
        json.at("avgProductionTime").get<double>(),
        json.at("yield").get<double>(),
        json.at("stock").get<int>());
}

inline nlohmann::json ToJson(const Order& order)
{
    return nlohmann::json{
        {"orderId", order.OrderId},
        {"sampleId", order.SampleId},
        {"customerName", order.CustomerName},
        {"quantity", order.Quantity},
        {"status", ToString(order.Status)},
        {"createdAt", FormatIso8601(order.CreatedAt)},
    };
}

inline Order OrderFromJson(const nlohmann::json& json)
{
    Order order(
        json.at("orderId").get<std::string>(),
        json.at("sampleId").get<std::string>(),
        json.at("customerName").get<std::string>(),
        json.at("quantity").get<int>(),
        ParseIso8601(json.at("createdAt").get<std::string>()));
    order.Status = ParseOrderStatus(json.at("status").get<std::string>());
    return order;
}

}
