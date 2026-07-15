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

    // 예: shortage=21, yield=0.7일 때 21/0.7이 부동소수점 연산에서
    // 30.000000000000003...으로 계산되어 ceil이 30이 아닌 31을 반환하는
    // 오차를 막기 위해 epsilon만큼 보정한다.
    constexpr double Epsilon = 1e-9;
    return static_cast<int>(std::ceil(shortage / yield - Epsilon));
}

inline double CalculateTotalProductionTime(double avgProductionTime, int actualProductionQuantity)
{
    return avgProductionTime * actualProductionQuantity;
}
