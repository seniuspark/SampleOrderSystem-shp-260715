#pragma once

#include <iostream>
#include <limits>
#include <string>

#include "View/MenuOption.h"
#include "View/SummaryRenderer.h"

// 메인 메뉴/서브 메뉴 선택 입력과 요약 정보 출력을 담당한다. 각 도메인 메뉴의
// 실제 입력 폼(시료 등록 값, 주문 값 등)은 ISampleView/IOrderView 등 별도
// 인터페이스가 담당하므로, 이 클래스는 메뉴 탐색(어떤 화면으로 갈지)만 다룬다.
class ConsoleAppView
{
public:
    ConsoleAppView(std::istream& in = std::cin, std::ostream& out = std::cout)
        : in_(in), out_(out)
    {
    }

    MainMenuOption ReadMainMenuChoice()
    {
        out_ << "\n=== 메인 메뉴 ===\n"
             << static_cast<int>(MainMenuOption::SampleManagement) << ". 시료 관리\n"
             << static_cast<int>(MainMenuOption::PlaceOrder) << ". 시료 주문\n"
             << static_cast<int>(MainMenuOption::ApproveOrRejectOrder) << ". 주문 승인/거절\n"
             << static_cast<int>(MainMenuOption::Monitoring) << ". 모니터링\n"
             << static_cast<int>(MainMenuOption::ProductionLine) << ". 생산라인 조회\n"
             << static_cast<int>(MainMenuOption::ReleaseOrder) << ". 출고 처리\n"
             << static_cast<int>(MainMenuOption::Exit) << ". 종료\n"
             << "선택: ";
        return static_cast<MainMenuOption>(ReadInt());
    }

    SampleMenuOption ReadSampleMenuChoice()
    {
        out_ << "\n-- 시료 관리 --\n"
             << static_cast<int>(SampleMenuOption::Register) << ". 등록\n"
             << static_cast<int>(SampleMenuOption::List) << ". 목록 조회\n"
             << static_cast<int>(SampleMenuOption::Search) << ". 이름 검색\n"
             << static_cast<int>(SampleMenuOption::Back) << ". 이전 메뉴로\n"
             << "선택: ";
        return static_cast<SampleMenuOption>(ReadInt());
    }

    ApprovalDecision ReadApprovalDecision()
    {
        out_ << "\n주문 ID를 확인하고 처리를 선택하세요.\n"
             << static_cast<int>(ApprovalDecision::Approve) << ". 승인\n"
             << static_cast<int>(ApprovalDecision::Reject) << ". 거절\n"
             << "선택: ";
        return static_cast<ApprovalDecision>(ReadInt());
    }

    void ShowSummary(const MainMenuSummary& summary)
    {
        out_ << RenderMainMenuSummary(summary);
    }

    void ShowMessage(const std::string& message)
    {
        out_ << message << '\n';
    }

private:
    static constexpr int InvalidChoice = -1;

    int ReadInt()
    {
        int choice = 0;
        if (!(in_ >> choice))
        {
            in_.clear();
            choice = InvalidChoice;
        }
        in_.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return choice;
    }

    std::istream& in_;
    std::ostream& out_;
};
