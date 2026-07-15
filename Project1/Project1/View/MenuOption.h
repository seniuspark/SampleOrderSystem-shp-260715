#pragma once

// 메뉴 번호를 상수화해 Controller/View/main.cpp에 매직 넘버가 흩어지지 않게 한다.
enum class MainMenuOption
{
    Exit = 0,
    SampleManagement = 1,
    PlaceOrder = 2,
    ApproveOrRejectOrder = 3,
    Monitoring = 4,
    ProductionLine = 5,
    ReleaseOrder = 6,
};

enum class SampleMenuOption
{
    Back = 0,
    Register = 1,
    List = 2,
    Search = 3,
};

enum class ApprovalDecision
{
    Approve = 1,
    Reject = 2,
};
