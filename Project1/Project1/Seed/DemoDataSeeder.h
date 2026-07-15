#pragma once

#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <iomanip>
#include <optional>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "Model/IClock.h"
#include "Model/Order.h"
#include "Model/OrderIdAllocator.h"
#include "Model/OrderStatus.h"
#include "Model/Sample.h"
#include "Repository/JsonCodec.h"
#include "Repository/OrderRepository.h"
#include "Repository/SampleRepository.h"

// Dummy/ PoC(SampleGenerator/OrderGenerator/IdAllocator)에서 검증한 시드 기반
// 생성 설계를 재구현한다: 시드가 주입된 std::mt19937만으로 도메인 제약을
// 만족하는 값을 만드는 순수 함수 + Repository에 반영하는 조립 로직.
namespace DemoDataSeeder
{

namespace Detail
{

inline std::optional<int> ParseSampleIdSequence(const std::string& sampleId)
{
    const std::string prefix = "S-";
    if (sampleId.size() <= prefix.size() || sampleId.compare(0, prefix.size(), prefix) != 0)
    {
        return std::nullopt;
    }

    const std::string digits = sampleId.substr(prefix.size());
    const bool allDigits = !digits.empty() && std::all_of(digits.begin(), digits.end(), [](unsigned char c) {
        return std::isdigit(c) != 0;
    });
    if (!allDigits)
    {
        return std::nullopt;
    }

    try
    {
        return std::stoi(digits);
    }
    catch (const std::exception&)
    {
        return std::nullopt;
    }
}

inline std::string NextSampleId(const std::vector<std::string>& existingSampleIds)
{
    static constexpr int SequenceWidth = 3;

    int maxSequence = 0;
    for (const std::string& sampleId : existingSampleIds)
    {
        const std::optional<int> sequence = ParseSampleIdSequence(sampleId);
        if (sequence.has_value())
        {
            maxSequence = std::max(maxSequence, *sequence);
        }
    }

    std::ostringstream stream;
    stream << "S-" << std::setw(SequenceWidth) << std::setfill('0') << (maxSequence + 1);
    return stream.str();
}

inline std::string FormatDate(std::chrono::system_clock::time_point timePoint)
{
    static constexpr int YearOffset = 0;
    static constexpr int YearLength = 4;
    static constexpr int MonthOffset = 5;
    static constexpr int MonthLength = 2;
    static constexpr int DayOffset = 8;
    static constexpr int DayLength = 2;

    const std::string iso8601 = JsonCodec::FormatIso8601(timePoint);
    return iso8601.substr(YearOffset, YearLength) +
        iso8601.substr(MonthOffset, MonthLength) +
        iso8601.substr(DayOffset, DayLength);
}

}  // namespace Detail

inline Sample GenerateSample(std::mt19937& rng, const std::string& sampleId)
{
    static const std::array<const char*, 5> NamePrefixes = {
        "SiC-Wafer", "GaN-Substrate", "Si-Ingot", "SiGe-Epi", "GaAs-Wafer",
    };

    std::uniform_int_distribution<std::size_t> prefixDist(0, NamePrefixes.size() - 1);
    std::uniform_int_distribution<int> suffixDist(1, 999);
    std::uniform_real_distribution<double> yieldDist(0.5, 1.0);
    std::uniform_real_distribution<double> productionTimeDist(1.0, 120.0);
    std::uniform_int_distribution<int> stockDist(0, 200);

    const std::string name = std::string(NamePrefixes[prefixDist(rng)]) + "-" + std::to_string(suffixDist(rng));
    return Sample(sampleId, name, productionTimeDist(rng), yieldDist(rng), stockDist(rng));
}

inline Order GenerateOrder(
    std::mt19937& rng,
    const std::vector<Sample>& samples,
    const std::string& orderId,
    std::chrono::system_clock::time_point now)
{
    if (samples.empty())
    {
        throw std::invalid_argument("GenerateOrder: samples must not be empty");
    }

    static const std::array<const char*, 5> CustomerNamePrefixes = {
        "SK-Hynix", "Samsung-Elec", "LG-Innotek", "DB-HiTek", "Amkor-Tech",
    };
    static constexpr std::array<OrderStatus, 5> AllStatuses = {
        OrderStatus::RESERVED,
        OrderStatus::REJECTED,
        OrderStatus::PRODUCING,
        OrderStatus::CONFIRMED,
        OrderStatus::RELEASE,
    };

    std::uniform_int_distribution<std::size_t> sampleDist(0, samples.size() - 1);
    std::uniform_int_distribution<std::size_t> customerDist(0, CustomerNamePrefixes.size() - 1);
    std::uniform_int_distribution<int> suffixDist(1, 999);
    std::uniform_int_distribution<int> quantityDist(1, 50);
    std::uniform_int_distribution<std::size_t> statusDist(0, AllStatuses.size() - 1);

    const std::string sampleId = samples[sampleDist(rng)].SampleId;
    const std::string customerName = std::string(CustomerNamePrefixes[customerDist(rng)]) + "-" + std::to_string(suffixDist(rng));
    const int quantity = quantityDist(rng);

    Order order(orderId, sampleId, customerName, quantity, now);
    order.Status = AllStatuses[statusDist(rng)];
    return order;
}

// 지정한 개수만큼 Sample/Order를 생성해 Repository에 추가(append)한다.
// 기존 데이터는 보존되며, sampleCount/orderCount는 이번 호출로 새로 추가될
// 개수다. 시료가 하나도 없으면(신규 생성분 포함) 주문을 만들 수 없으므로
// orderCount는 무시된다.
inline void SeedDemoData(
    SampleRepository& sampleRepository,
    OrderRepository& orderRepository,
    int sampleCount,
    int orderCount,
    unsigned seed,
    const IClock& clock)
{
    std::mt19937 rng(seed);

    for (int i = 0; i < sampleCount; ++i)
    {
        std::vector<std::string> existingSampleIds;
        for (const Sample& sample : sampleRepository.GetAll())
        {
            existingSampleIds.push_back(sample.SampleId);
        }
        sampleRepository.Add(GenerateSample(rng, Detail::NextSampleId(existingSampleIds)));
    }

    const std::vector<Sample> samples = sampleRepository.GetAll();
    if (samples.empty())
    {
        return;
    }

    for (int i = 0; i < orderCount; ++i)
    {
        std::vector<std::string> existingOrderIds;
        for (const Order& order : orderRepository.GetAll())
        {
            existingOrderIds.push_back(order.OrderId);
        }
        const std::string orderId = AllocateOrderId(existingOrderIds, Detail::FormatDate(clock.Now()));
        orderRepository.Add(GenerateOrder(rng, samples, orderId, clock.Now()));
    }
}

}  // namespace DemoDataSeeder
