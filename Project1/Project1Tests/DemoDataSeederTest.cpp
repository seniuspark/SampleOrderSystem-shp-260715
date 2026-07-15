#include "gtest/gtest.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <random>
#include <vector>

#include "Model/Sample.h"
#include "Repository/OrderRepository.h"
#include "Repository/SampleRepository.h"
#include "Seed/DemoDataSeeder.h"
#include "TestSupport/FixedClock.h"

namespace
{

std::filesystem::path UniqueTempPath(const std::string& name)
{
    return std::filesystem::temp_directory_path() / ("DemoDataSeederTest_" + name + ".json");
}

class TempFileGuard
{
public:
    explicit TempFileGuard(std::filesystem::path path) : path_(std::move(path))
    {
        std::filesystem::remove(path_);
    }
    ~TempFileGuard()
    {
        std::filesystem::remove(path_);
    }
    const std::filesystem::path& Path() const { return path_; }

private:
    std::filesystem::path path_;
};

}  // namespace

TEST(DemoDataSeederTest, GenerateSampleAlwaysSatisfiesDomainConstraintsAcrossManySeeds)
{
    for (unsigned seed = 0; seed < 200; ++seed)
    {
        std::mt19937 rng(seed);
        EXPECT_NO_THROW({
            Sample sample = DemoDataSeeder::GenerateSample(rng, "S-001");
            EXPECT_GT(sample.Yield, 0.0);
            EXPECT_LE(sample.Yield, 1.0);
            EXPECT_GT(sample.AvgProductionTime, 0.0);
            EXPECT_GE(sample.Stock, 0);
        }) << "seed=" << seed;
    }
}

TEST(DemoDataSeederTest, GenerateOrderAlwaysReferencesAnExistingSampleIdAcrossManySeeds)
{
    std::vector<Sample> samples{
        Sample("S-001", "Wafer-A", 10.0, 0.9, 5),
        Sample("S-002", "Wafer-B", 20.0, 0.8, 0),
    };
    const std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

    for (unsigned seed = 0; seed < 200; ++seed)
    {
        std::mt19937 rng(seed);
        Order order = DemoDataSeeder::GenerateOrder(rng, samples, "ORD-20260416-0001", now);

        const bool referencesKnownSample = std::any_of(samples.begin(), samples.end(), [&order](const Sample& sample) {
            return sample.SampleId == order.SampleId;
        });
        EXPECT_TRUE(referencesKnownSample) << "seed=" << seed << " sampleId=" << order.SampleId;
    }
}

TEST(DemoDataSeederTest, GenerateOrderThrowsWhenSampleListIsEmpty)
{
    std::mt19937 rng(1);
    EXPECT_THROW(DemoDataSeeder::GenerateOrder(rng, {}, "ORD-20260416-0001", std::chrono::system_clock::now()), std::invalid_argument);
}

TEST(DemoDataSeederTest, SeedDemoDataAddsRequestedCountsAndPreservesExistingData)
{
    TempFileGuard sampleFile(UniqueTempPath("samples"));
    TempFileGuard orderFile(UniqueTempPath("orders"));
    FixedClock clock(std::chrono::system_clock::now());

    SampleRepository sampleRepository(sampleFile.Path());
    OrderRepository orderRepository(orderFile.Path());
    sampleRepository.Add(Sample("S-900", "PreExisting", 15.0, 0.7, 3));

    DemoDataSeeder::SeedDemoData(sampleRepository, orderRepository, 5, 8, /*seed=*/42, clock);

    SampleRepository reloadedSamples(sampleFile.Path());
    OrderRepository reloadedOrders(orderFile.Path());

    EXPECT_EQ(reloadedSamples.GetAll().size(), 6u);
    EXPECT_TRUE(reloadedSamples.FindById("S-900").has_value());
    EXPECT_EQ(reloadedOrders.GetAll().size(), 8u);
}
