#pragma once

#include <chrono>

#include "Model/IClock.h"

inline bool IsProductionComplete(
    std::chrono::system_clock::time_point productionStartedAt,
    double totalProductionTimeMinutes,
    const IClock& clock)
{
    const auto totalProductionTime =
        std::chrono::duration_cast<std::chrono::system_clock::duration>(
            std::chrono::duration<double, std::ratio<60>>(totalProductionTimeMinutes));
    const auto completionTime = productionStartedAt + totalProductionTime;

    return clock.Now() >= completionTime;
}
