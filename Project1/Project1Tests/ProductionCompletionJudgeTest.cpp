#include "gtest/gtest.h"
#include "Model/ProductionCompletionJudge.h"
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

TEST(ProductionCompletionJudgeTest, JudgesCompleteWhenCurrentTimeIsPastCompletionTime)
{
    const auto productionStartedAt = std::chrono::system_clock::time_point{ std::chrono::seconds{ 1700000000 } };
    const double totalProductionTimeMinutes = 30.0;
    const auto currentTime = productionStartedAt + std::chrono::minutes{ 40 };
    FixedClock clock(currentTime);

    EXPECT_TRUE(IsProductionComplete(productionStartedAt, totalProductionTimeMinutes, clock));
}

TEST(ProductionCompletionJudgeTest, JudgesNotCompleteWhenCurrentTimeIsBeforeCompletionTime)
{
    const auto productionStartedAt = std::chrono::system_clock::time_point{ std::chrono::seconds{ 1700000000 } };
    const double totalProductionTimeMinutes = 30.0;
    const auto currentTime = productionStartedAt + std::chrono::minutes{ 20 };
    FixedClock clock(currentTime);

    EXPECT_FALSE(IsProductionComplete(productionStartedAt, totalProductionTimeMinutes, clock));
}

TEST(ProductionCompletionJudgeTest, JudgesCompleteWhenCurrentTimeExactlyEqualsCompletionTime)
{
    const auto productionStartedAt = std::chrono::system_clock::time_point{ std::chrono::seconds{ 1700000000 } };
    const double totalProductionTimeMinutes = 30.0;
    const auto currentTime = productionStartedAt + std::chrono::minutes{ 30 };
    FixedClock clock(currentTime);

    EXPECT_TRUE(IsProductionComplete(productionStartedAt, totalProductionTimeMinutes, clock));
}
