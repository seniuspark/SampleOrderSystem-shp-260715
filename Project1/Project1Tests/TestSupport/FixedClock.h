#pragma once

#include <chrono>

#include "Model/IClock.h"

class FixedClock : public IClock
{
public:
    explicit FixedClock(std::chrono::system_clock::time_point fixedTime)
        : fixedTime_(fixedTime)
    {
    }

    std::chrono::system_clock::time_point Now() const override
    {
        return fixedTime_;
    }

    void Advance(std::chrono::system_clock::duration duration)
    {
        fixedTime_ += duration;
    }

private:
    std::chrono::system_clock::time_point fixedTime_;
};
