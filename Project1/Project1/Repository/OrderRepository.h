#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "nlohmann/json.hpp"

#include "Model/Order.h"
#include "Repository/JsonCodec.h"

class OrderRepository
{
public:
    explicit OrderRepository(std::filesystem::path filePath)
        : filePath_(std::move(filePath)), orders_(Load(filePath_))
    {
    }

    std::vector<Order> GetAll() const
    {
        return orders_;
    }

    std::optional<Order> FindById(const std::string& orderId) const
    {
        for (const Order& order : orders_)
        {
            if (order.OrderId == orderId)
            {
                return order;
            }
        }
        return std::nullopt;
    }

    void Add(const Order& order)
    {
        orders_.push_back(order);
        Save();
    }

    bool Update(const Order& order)
    {
        return false;
    }

    bool Delete(const std::string& orderId)
    {
        return false;
    }

private:
    static constexpr const char* OrdersKey = "orders";

    std::filesystem::path filePath_;
    std::vector<Order> orders_;

    static std::vector<Order> Load(const std::filesystem::path& filePath)
    {
        std::vector<Order> orders;
        for (const nlohmann::json& element : JsonCodec::ReadJsonArray(filePath, OrdersKey))
        {
            orders.push_back(JsonCodec::OrderFromJson(element));
        }
        return orders;
    }

    void Save() const
    {
        nlohmann::json array = nlohmann::json::array();
        for (const Order& order : orders_)
        {
            array.push_back(JsonCodec::ToJson(order));
        }
        JsonCodec::WriteJsonArray(filePath_, OrdersKey, array);
    }
};
