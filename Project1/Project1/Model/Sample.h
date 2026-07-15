#pragma once

#include <string>

class Sample
{
public:
    Sample(std::string sampleId, std::string name, double avgProductionTime, double yield, int stock)
        : SampleId(std::move(sampleId)), Name(std::move(name)), AvgProductionTime(avgProductionTime), Yield(yield), Stock(stock)
    {
    }

    std::string SampleId;
    std::string Name;
    double AvgProductionTime;
    double Yield;
    int Stock;
};
