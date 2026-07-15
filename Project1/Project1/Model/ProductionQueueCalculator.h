#pragma once

#include <cmath>

inline int CalculateShortage(int orderQuantity, int stock)
{
    const int shortage = orderQuantity - stock;
    return shortage > 0 ? shortage : 0;
}

inline int CalculateActualProductionQuantity(int shortage, double yield)
{
    if (shortage <= 0)
    {
        return 0;
    }
    return static_cast<int>(std::ceil(shortage / yield));
}

inline double CalculateTotalProductionTime(double avgProductionTime, int actualProductionQuantity)
{
    return avgProductionTime * actualProductionQuantity;
}
