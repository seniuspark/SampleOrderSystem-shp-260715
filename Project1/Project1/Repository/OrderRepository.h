#pragma once

#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
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

private:
    static constexpr const char* OrdersKey = "orders";

    std::filesystem::path filePath_;
    std::vector<Order> orders_;

    static std::vector<Order> Load(const std::filesystem::path& filePath)
    {
        std::vector<Order> orders;

        std::ifstream input(filePath);
        if (!input.is_open())
        {
            return orders;
        }

        std::ostringstream buffer;
        buffer << input.rdbuf();
        std::string content = buffer.str();
        if (content.empty())
        {
            return orders;
        }

        nlohmann::json root = nlohmann::json::parse(content);
        for (const nlohmann::json& element : root.at(OrdersKey))
        {
            orders.push_back(JsonCodec::OrderFromJson(element));
        }
        return orders;
    }

    void Save() const
    {
        std::filesystem::create_directories(filePath_.parent_path());

        nlohmann::json root;
        root[OrdersKey] = nlohmann::json::array();
        for (const Order& order : orders_)
        {
            root[OrdersKey].push_back(JsonCodec::ToJson(order));
        }

        std::ofstream output(filePath_);
        output << root.dump(2);
    }
};
