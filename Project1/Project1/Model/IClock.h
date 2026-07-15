#pragma once

#include <chrono>

class IClock
{
public:
    virtual ~IClock() = default;

    virtual std::chrono::system_clock::time_point Now() const = 0;
};

class SystemClock : public IClock
{
public:
    std::chrono::system_clock::time_point Now() const override
    {
        return std::chrono::system_clock::now();
    }
};
