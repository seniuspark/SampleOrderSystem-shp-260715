#pragma once

#include <filesystem>
#include <optional>
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
        samples_.push_back(sample);
        Save();
    }

    bool Update(const Sample& sample)
    {
        return false;
    }

    bool Delete(const std::string& sampleId)
    {
        return false;
    }

private:
    static constexpr const char* SamplesKey = "samples";

    std::filesystem::path filePath_;
    std::vector<Sample> samples_;

    static std::vector<Sample> Load(const std::filesystem::path& filePath)
    {
        std::vector<Sample> samples;
        for (const nlohmann::json& element : JsonCodec::ReadJsonArray(filePath, SamplesKey))
        {
            samples.push_back(JsonCodec::SampleFromJson(element));
        }
        return samples;
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
