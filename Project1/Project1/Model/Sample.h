#pragma once

#include <stdexcept>
#include <string>

class Sample
{
public:
    static constexpr double MinYieldExclusive = 0.0;
    static constexpr double MaxYieldInclusive = 1.0;

    Sample(std::string sampleId, std::string name, double avgProductionTime, double yield, int stock)
        : SampleId(std::move(sampleId)), Name(std::move(name)), AvgProductionTime(avgProductionTime), Yield(yield), Stock(stock)
    {
        if (Yield <= MinYieldExclusive || Yield > MaxYieldInclusive)
        {
            throw std::invalid_argument("Yield must be within (0.0, 1.0]");
        }
    }

    std::string SampleId;
    std::string Name;
    double AvgProductionTime;
    double Yield;
    int Stock;
};
