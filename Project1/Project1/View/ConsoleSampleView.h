#pragma once

#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "Model/Sample.h"
#include "View/ISampleView.h"

class ConsoleSampleView : public ISampleView
{
public:
    ConsoleSampleView(std::istream& in = std::cin, std::ostream& out = std::cout)
        : in_(in), out_(out)
    {
    }

    NewSampleInput ReadNewSampleInput() override
    {
        NewSampleInput input;

        out_ << "SampleId: ";
        std::getline(in_, input.SampleId);

        out_ << "Name: ";
        std::getline(in_, input.Name);

        out_ << "AvgProductionTime: ";
        in_ >> input.AvgProductionTime;

        out_ << "Yield: ";
        in_ >> input.Yield;
        in_.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        return input;
    }

    std::string ReadSearchKeyword() override
    {
        out_ << "검색어: ";
        std::string keyword;
        std::getline(in_, keyword);
        return keyword;
    }

    void ShowSampleList(const std::vector<Sample>& samples) override
    {
        out_ << "SampleId\tName\tAvgProductionTime\tYield\tStock\n";
        for (const Sample& sample : samples)
        {
            out_ << sample.SampleId << '\t'
                 << sample.Name << '\t'
                 << sample.AvgProductionTime << '\t'
                 << sample.Yield << '\t'
                 << sample.Stock << '\n';
        }
    }

    void ShowMessage(const std::string& message) override
    {
        out_ << message << '\n';
    }

private:
    std::istream& in_;
    std::ostream& out_;
};
