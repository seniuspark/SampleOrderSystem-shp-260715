#pragma once

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

#include "Model/Sample.h"
#include "Repository/SampleRepository.h"
#include "View/ISampleView.h"

class SampleController
{
public:
    SampleController(SampleRepository& repository, ISampleView& view)
        : repository_(repository), view_(view)
    {
    }

    void RegisterSample()
    {
        const NewSampleInput input = view_.ReadNewSampleInput();

        try
        {
            Sample sample(input.SampleId, input.Name, input.AvgProductionTime, input.Yield, InitialStock);
            repository_.Add(sample);
            view_.ShowMessage("시료가 등록되었습니다: " + sample.SampleId);
        }
        catch (const std::invalid_argument& error)
        {
            view_.ShowMessage(std::string("시료 등록에 실패했습니다: ") + error.what());
        }
    }

    void ListSamples()
    {
        const std::vector<Sample> samples = repository_.GetAll();
        if (samples.empty())
        {
            view_.ShowMessage("등록된 시료가 없습니다.");
            return;
        }
        view_.ShowSampleList(samples);
    }

    void SearchSamplesByName()
    {
        const std::string keyword = view_.ReadSearchKeyword();
        const std::vector<Sample> allSamples = repository_.GetAll();

        std::vector<Sample> matched;
        std::copy_if(allSamples.begin(), allSamples.end(), std::back_inserter(matched), [&keyword](const Sample& sample)
            {
                return sample.Name.find(keyword) != std::string::npos;
            });

        if (matched.empty())
        {
            view_.ShowMessage("검색 결과가 없습니다.");
            return;
        }
        view_.ShowSampleList(matched);
    }

private:
    static constexpr int InitialStock = 0;

    SampleRepository& repository_;
    ISampleView& view_;
};
