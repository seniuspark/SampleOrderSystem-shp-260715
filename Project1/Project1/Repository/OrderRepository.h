#pragma once

#include <algorithm>
#include <filesystem>
#include <optional>
#include <stdexcept>
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
        if (FindById(order.OrderId).has_value())
        {
            throw std::invalid_argument("Order with duplicate OrderId: " + order.OrderId);
        }
        orders_.push_back(order);
        Save();
    }

    bool Update(const Order& order)
    {
        auto it = std::find_if(orders_.begin(), orders_.end(), [&order](const Order& existing)
            {
                return existing.OrderId == order.OrderId;
            });
        if (it == orders_.end())
        {
            return false;
        }
        *it = order;
        Save();
        return true;
    }

    bool Delete(const std::string& orderId)
    {
        auto it = std::find_if(orders_.begin(), orders_.end(), [&orderId](const Order& existing)
            {
                return existing.OrderId == orderId;
            });
        if (it == orders_.end())
        {
            return false;
        }
        orders_.erase(it);
        Save();
        return true;
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
            try
            {
                orders.push_back(JsonCodec::OrderFromJson(element));
            }
            catch (const std::exception&)
            {
                continue;
            }
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
