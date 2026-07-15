#pragma once

#include <string>
#include <vector>

#include "Model/Sample.h"

struct NewSampleInput
{
    std::string SampleId;
    std::string Name;
    double AvgProductionTime;
    double Yield;
};

class ISampleView
{
public:
    virtual ~ISampleView() = default;

    virtual NewSampleInput ReadNewSampleInput() = 0;
    virtual std::string ReadSearchKeyword() = 0;
    virtual void ShowSampleList(const std::vector<Sample>& samples) = 0;
    virtual void ShowMessage(const std::string& message) = 0;
};
