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
    nlohmann::json json{
        {"orderId", order.OrderId},
        {"sampleId", order.SampleId},
        {"customerName", order.CustomerName},
        {"quantity", order.Quantity},
        {"status", ToString(order.Status)},
        {"createdAt", FormatIso8601(order.CreatedAt)},
    };

    // 생산 큐 복원에 필요한 값들은 PRODUCING 상태일 때만 의미가 있으므로,
    // 값이 없으면 필드를 아예 생략한다(fail-soft 정책과 동일하게, 이
    // 필드가 없는 예전 데이터를 읽어도 OrderFromJson이 문제없이 동작해야 함).
    if (order.ProductionShortage.has_value())
    {
        json["productionShortage"] = *order.ProductionShortage;
    }
    if (order.ProductionActualQuantity.has_value())
    {
        json["productionActualQuantity"] = *order.ProductionActualQuantity;
    }
    if (order.ProductionTotalTimeMinutes.has_value())
    {
        json["productionTotalTimeMinutes"] = *order.ProductionTotalTimeMinutes;
    }
    if (order.ProductionStartedAt.has_value())
    {
        json["productionStartedAt"] = FormatIso8601(*order.ProductionStartedAt);
    }

    return json;
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

    if (json.contains("productionShortage"))
    {
        order.ProductionShortage = json.at("productionShortage").get<int>();
    }
    if (json.contains("productionActualQuantity"))
    {
        order.ProductionActualQuantity = json.at("productionActualQuantity").get<int>();
    }
    if (json.contains("productionTotalTimeMinutes"))
    {
        order.ProductionTotalTimeMinutes = json.at("productionTotalTimeMinutes").get<double>();
    }
    if (json.contains("productionStartedAt"))
    {
        order.ProductionStartedAt = ParseIso8601(json.at("productionStartedAt").get<std::string>());
    }

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

template <typename T, typename Converter>
inline std::vector<T> LoadElementsSkippingInvalid(const std::filesystem::path& filePath, const std::string& arrayKey, Converter convert)
{
    std::vector<T> elements;
    for (const nlohmann::json& element : ReadJsonArray(filePath, arrayKey))
    {
        try
        {
            elements.push_back(convert(element));
        }
        catch (const std::exception&)
        {
            continue;
        }
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
