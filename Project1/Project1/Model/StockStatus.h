#pragma once

#include <algorithm>

enum class StockStatus
{
    DEPLETED,
    SHORTAGE,
    SUFFICIENT,
};

inline StockStatus JudgeStockStatus(int stock, int demand)
{
    const int clampedStock = std::max(stock, 0);
    const int clampedDemand = std::max(demand, 0);

    if (clampedStock == 0)
    {
        return StockStatus::DEPLETED;
    }

    if (clampedStock < clampedDemand)
    {
        return StockStatus::SHORTAGE;
    }

    return StockStatus::SUFFICIENT;
}
