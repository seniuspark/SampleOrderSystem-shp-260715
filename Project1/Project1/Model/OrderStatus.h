#pragma once

#include <stdexcept>
#include <string>

enum class OrderStatus
{
    RESERVED,
    REJECTED,
    PRODUCING,
    CONFIRMED,
    RELEASE,
};

inline std::string ToString(OrderStatus status)
{
    switch (status)
    {
    case OrderStatus::RESERVED:
        return "RESERVED";
    case OrderStatus::REJECTED:
        return "REJECTED";
    case OrderStatus::PRODUCING:
        return "PRODUCING";
    case OrderStatus::CONFIRMED:
        return "CONFIRMED";
    case OrderStatus::RELEASE:
        return "RELEASE";
    }

    throw std::invalid_argument("Unknown OrderStatus");
}

inline OrderStatus ParseOrderStatus(const std::string& statusText)
{
    if (statusText == "RESERVED")
    {
        return OrderStatus::RESERVED;
    }
    if (statusText == "REJECTED")
    {
        return OrderStatus::REJECTED;
    }
    if (statusText == "PRODUCING")
    {
        return OrderStatus::PRODUCING;
    }
    if (statusText == "CONFIRMED")
    {
        return OrderStatus::CONFIRMED;
    }
    if (statusText == "RELEASE")
    {
        return OrderStatus::RELEASE;
    }

    throw std::invalid_argument("Unknown OrderStatus string: " + statusText);
}
