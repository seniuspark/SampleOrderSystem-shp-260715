#pragma once

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "nlohmann/json.hpp"

#include "Model/Order.h"
#include "Model/OrderStatus.h"
#include "Model/Sample.h"

namespace JsonCodec
{

constexpr const char* Iso8601Format = "%Y-%m-%dT%H:%M:%SZ";

inline std::string FormatIso8601(std::chrono::system_clock::time_point timePoint)
{
    std::time_t time = std::chrono::system_clock::to_time_t(timePoint);
    std::tm utcTime{};
    gmtime_s(&utcTime, &time);

    std::ostringstream stream;
    stream << std::put_time(&utcTime, Iso8601Format);
    return stream.str();
}

inline std::chrono::system_clock::time_point ParseIso8601(const std::string& text)
{
    std::tm utcTime{};
    std::istringstream stream(text);
    stream >> std::get_time(&utcTime, Iso8601Format);

    // std::mktime/std::timegm 표준에는 UTC 기준 tm -> time_t 변환 함수가 없어
    // MSVC 전용 _mkgmtime을 사용한다. 이 프로젝트는 Visual Studio/MSVC 빌드만
    // 지원하므로(CLAUDE.md 기술 스택) 플랫폼 종속성을 감수한다.
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

inline std::vector<nlohmann::json> ReadJsonArray(const std::filesystem::path& filePath, const std::string& arrayKey)
{
    std::vector<nlohmann::json> elements;

    std::ifstream input(filePath);
    if (!input.is_open())
    {
        return elements;
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    std::string content = buffer.str();
    if (content.empty())
    {
        return elements;
    }

    nlohmann::json root;
    try
    {
        root = nlohmann::json::parse(content);
    }
    catch (const nlohmann::json::parse_error&)
    {
        return elements;
    }

    if (!root.is_object() || !root.contains(arrayKey))
    {
        return elements;
    }

    for (const nlohmann::json& element : root.at(arrayKey))
    {
        elements.push_back(element);
    }
    return elements;
}

inline void WriteJsonArray(const std::filesystem::path& filePath, const std::string& arrayKey, const nlohmann::json& arrayJson)
{
    std::filesystem::create_directories(filePath.parent_path());

    nlohmann::json root;
    root[arrayKey] = arrayJson;

    std::ofstream output(filePath);
    output << root.dump(2);
}

}
