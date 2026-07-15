#pragma once

#include <queue>
#include <string>
#include <vector>

#include "Model/Sample.h"
#include "View/ISampleView.h"

class FakeSampleView : public ISampleView
{
public:
    void EnqueueNewSampleInput(NewSampleInput input) { sampleInputs_.push(std::move(input)); }
    void EnqueueSearchKeyword(std::string keyword) { searchKeywords_.push(std::move(keyword)); }

    NewSampleInput ReadNewSampleInput() override
    {
        NewSampleInput input = sampleInputs_.front();
        sampleInputs_.pop();
        return input;
    }

    std::string ReadSearchKeyword() override
    {
        std::string keyword = searchKeywords_.front();
        searchKeywords_.pop();
        return keyword;
    }

    void ShowSampleList(const std::vector<Sample>& samples) override
    {
        lastShownSamples_ = samples;
    }

    void ShowMessage(const std::string& message) override
    {
        lastMessage_ = message;
    }

    const std::vector<Sample>& LastShownSamples() const { return lastShownSamples_; }
    const std::string& LastMessage() const { return lastMessage_; }

private:
    std::queue<NewSampleInput> sampleInputs_;
    std::queue<std::string> searchKeywords_;
    std::vector<Sample> lastShownSamples_;
    std::string lastMessage_;
};
