#pragma once

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace OrderIdFormat
{
constexpr const char* Prefix = "ORD-";
constexpr int DateLength = 8;
constexpr int SequenceWidth = 4;
}  // namespace OrderIdFormat

namespace
{

bool IsAllDigits(const std::string& text)
{
    return !text.empty() && std::all_of(text.begin(), text.end(), [](unsigned char character) {
        return std::isdigit(character) != 0;
    });
}

std::optional<int> TryParseSequenceForDate(const std::string& orderId, const std::string& date)
{
    const std::string prefix = OrderIdFormat::Prefix;
    const size_t expectedLength = prefix.size() + OrderIdFormat::DateLength + 1 + OrderIdFormat::SequenceWidth;

    if (orderId.size() != expectedLength)
    {
        return std::nullopt;
    }

    if (orderId.compare(0, prefix.size(), prefix) != 0)
    {
        return std::nullopt;
    }

    const std::string orderDate = orderId.substr(prefix.size(), OrderIdFormat::DateLength);
    if (!IsAllDigits(orderDate) || orderDate != date)
    {
        return std::nullopt;
    }

    const size_t separatorIndex = prefix.size() + OrderIdFormat::DateLength;
    if (orderId[separatorIndex] != '-')
    {
        return std::nullopt;
    }

    const std::string sequenceText = orderId.substr(separatorIndex + 1);
    if (!IsAllDigits(sequenceText))
    {
        return std::nullopt;
    }

    return std::stoi(sequenceText);
}

}  // namespace

inline std::string AllocateOrderId(const std::vector<std::string>& existingOrderIds, const std::string& today)
{
    int maxSequence = 0;
    for (const std::string& orderId : existingOrderIds)
    {
        const std::optional<int> sequence = TryParseSequenceForDate(orderId, today);
        if (sequence.has_value())
        {
            maxSequence = std::max(maxSequence, *sequence);
        }
    }

    std::ostringstream orderIdStream;
    orderIdStream << OrderIdFormat::Prefix << today << "-" << std::setw(OrderIdFormat::SequenceWidth)
                  << std::setfill('0') << (maxSequence + 1);
    return orderIdStream.str();
}
