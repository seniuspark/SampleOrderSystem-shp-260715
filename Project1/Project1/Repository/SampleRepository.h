#pragma once

#include <algorithm>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "nlohmann/json.hpp"

#include "Model/Sample.h"
#include "Repository/JsonCodec.h"

class SampleRepository
{
public:
    explicit SampleRepository(std::filesystem::path filePath)
        : filePath_(std::move(filePath)), samples_(Load(filePath_))
    {
    }

    std::vector<Sample> GetAll() const
    {
        return samples_;
    }

    std::optional<Sample> FindById(const std::string& sampleId) const
    {
        for (const Sample& sample : samples_)
        {
            if (sample.SampleId == sampleId)
            {
                return sample;
            }
        }
        return std::nullopt;
    }

    void Add(const Sample& sample)
    {
        if (FindById(sample.SampleId).has_value())
        {
            throw std::invalid_argument("Sample with duplicate SampleId: " + sample.SampleId);
        }
        samples_.push_back(sample);
        Save();
    }

    bool Update(const Sample& sample)
    {
        auto it = std::find_if(samples_.begin(), samples_.end(), [&sample](const Sample& existing)
            {
                return existing.SampleId == sample.SampleId;
            });
        if (it == samples_.end())
        {
            return false;
        }
        *it = sample;
        Save();
        return true;
    }

    bool Delete(const std::string& sampleId)
    {
        auto it = std::find_if(samples_.begin(), samples_.end(), [&sampleId](const Sample& existing)
            {
                return existing.SampleId == sampleId;
            });
        if (it == samples_.end())
        {
            return false;
        }
        samples_.erase(it);
        Save();
        return true;
    }

private:
    static constexpr const char* SamplesKey = "samples";

    std::filesystem::path filePath_;
    std::vector<Sample> samples_;

    static std::vector<Sample> Load(const std::filesystem::path& filePath)
    {
        return JsonCodec::LoadElementsSkippingInvalid<Sample>(filePath, SamplesKey, JsonCodec::SampleFromJson);
    }

    void Save() const
    {
        nlohmann::json array = nlohmann::json::array();
        for (const Sample& sample : samples_)
        {
            array.push_back(JsonCodec::ToJson(sample));
        }
        JsonCodec::WriteJsonArray(filePath_, SamplesKey, array);
    }
};
