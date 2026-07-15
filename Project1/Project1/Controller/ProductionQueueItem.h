#pragma once

#include <chrono>
#include <string>

struct ProductionQueueItem
{
    std::string OrderId;
    std::string SampleId;
    int OrderQuantity;
    int Shortage;
    int ActualProductionQuantity;
    double TotalProductionTimeMinutes;
    std::chrono::system_clock::time_point ProductionStartedAt;
};
