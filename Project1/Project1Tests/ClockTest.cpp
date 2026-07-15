#include "gtest/gtest.h"
#include "Model/IClock.h"

#include <chrono>

namespace
{

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

private:
    std::chrono::system_clock::time_point fixedTime_;
};

}  // namespace

TEST(ClockTest, FixedClockAlwaysReturnsInjectedTime)
{
    const auto injectedTime = std::chrono::system_clock::time_point{ std::chrono::seconds{ 1700000000 } };
    FixedClock clock(injectedTime);

    EXPECT_EQ(clock.Now(), injectedTime);
    EXPECT_EQ(clock.Now(), injectedTime);
}
