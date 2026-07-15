#pragma once

#include <sstream>
#include <string>

struct MainMenuSummary
{
    int SampleCount;
    int TotalStock;
    int TotalOrderCount;
    int ProductionQueueWaitingCount;
};

inline std::string RenderMainMenuSummary(const MainMenuSummary& summary)
{
    std::ostringstream out;
    out << "\n=== 전체 요약 ===\n"
        << "등록 시료 수: " << summary.SampleCount << '\n'
        << "총 재고: " << summary.TotalStock << '\n'
        << "전체 주문 수: " << summary.TotalOrderCount << '\n'
        << "생산라인 대기: " << summary.ProductionQueueWaitingCount << '\n';
    return out.str();
}
