#pragma once

#include <algorithm>

inline int CalculateAvailableStock(int stock, int unreleasedConfirmedQuantity)
{
    return std::max(stock - unreleasedConfirmedQuantity, 0);
}
