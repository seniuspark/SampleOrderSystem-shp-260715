#pragma once

#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
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

private:
    static constexpr const char* SamplesKey = "samples";

    std::filesystem::path filePath_;
    std::vector<Sample> samples_;

    static std::vector<Sample> Load(const std::filesystem::path& filePath)
    {
        std::vector<Sample> samples;

        std::ifstream input(filePath);
        if (!input.is_open())
        {
            return samples;
        }

        std::ostringstream buffer;
        buffer << input.rdbuf();
        std::string content = buffer.str();
        if (content.empty())
        {
            return samples;
        }

        nlohmann::json root = nlohmann::json::parse(content);
        for (const nlohmann::json& element : root.at(SamplesKey))
        {
            samples.push_back(JsonCodec::SampleFromJson(element));
        }
        return samples;
    }

    void Save() const
    {
        std::filesystem::create_directories(filePath_.parent_path());

        nlohmann::json root;
        root[SamplesKey] = nlohmann::json::array();
        for (const Sample& sample : samples_)
        {
            root[SamplesKey].push_back(JsonCodec::ToJson(sample));
        }

        std::ofstream output(filePath_);
        output << root.dump(2);
    }
};
