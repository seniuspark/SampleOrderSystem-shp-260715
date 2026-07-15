#include <filesystem>
#include <vector>

#include "Controller/MonitoringController.h"
#include "Controller/OrderController.h"
#include "Controller/ProductionLineController.h"
#include "Controller/SampleController.h"
#include "Model/IClock.h"
#include "Model/Sample.h"
#include "Repository/OrderRepository.h"
#include "Repository/SampleRepository.h"
#include "View/ConsoleAppView.h"
#include "View/ConsoleMonitoringView.h"
#include "View/ConsoleOrderView.h"
#include "View/ConsoleProductionLineView.h"
#include "View/ConsoleSampleView.h"
#include "View/MenuOption.h"
#include "View/SummaryRenderer.h"

namespace
{

MainMenuSummary BuildSummary(
    const SampleRepository& sampleRepository,
    const OrderRepository& orderRepository,
    const ProductionLineController& productionLineController)
{
    const std::vector<Sample> samples = sampleRepository.GetAll();

    int totalStock = 0;
    for (const Sample& sample : samples)
    {
        totalStock += sample.Stock;
    }

    return MainMenuSummary{
        static_cast<int>(samples.size()),
        totalStock,
        static_cast<int>(orderRepository.GetAll().size()),
        static_cast<int>(productionLineController.QueueSize()),
    };
}

void RunSampleManagementMenu(SampleController& controller, ConsoleAppView& appView)
{
    bool inSampleMenu = true;
    while (inSampleMenu)
    {
        switch (appView.ReadSampleMenuChoice())
        {
        case SampleMenuOption::Register:
            controller.RegisterSample();
            break;
        case SampleMenuOption::List:
            controller.ListSamples();
            break;
        case SampleMenuOption::Search:
            controller.SearchSamplesByName();
            break;
        case SampleMenuOption::Back:
            inSampleMenu = false;
            break;
        default:
            appView.ShowMessage("잘못된 선택입니다.");
            break;
        }
    }
}

void RunApproveOrRejectMenu(OrderController& controller, ConsoleAppView& appView)
{
    controller.ShowReservedOrdersForApproval();

    switch (appView.ReadApprovalDecision())
    {
    case ApprovalDecision::Approve:
        controller.ApproveOrder();
        break;
    case ApprovalDecision::Reject:
        controller.RejectOrder();
        break;
    default:
        appView.ShowMessage("잘못된 선택입니다.");
        break;
    }
}

void RunReleaseMenu(OrderController& controller)
{
    controller.ShowConfirmedOrdersForRelease();
    controller.ReleaseOrder();
}

}

int main()
{
    const std::filesystem::path sampleDataPath = "data/samples.json";
    const std::filesystem::path orderDataPath = "data/orders.json";

    SampleRepository sampleRepository(sampleDataPath);
    OrderRepository orderRepository(orderDataPath);
    SystemClock clock;

    ConsoleAppView appView;
    ConsoleSampleView sampleView;
    ConsoleOrderView orderView;
    ConsoleProductionLineView productionLineView;
    ConsoleMonitoringView monitoringView;

    SampleController sampleController(sampleRepository, sampleView);
    ProductionLineController productionLineController(orderRepository, sampleRepository, clock, productionLineView);
    OrderController orderController(orderRepository, sampleRepository, orderView, clock, productionLineController);
    MonitoringController monitoringController(orderRepository, sampleRepository, monitoringView);

    bool running = true;
    while (running)
    {
        appView.ShowSummary(BuildSummary(sampleRepository, orderRepository, productionLineController));

        switch (appView.ReadMainMenuChoice())
        {
        case MainMenuOption::SampleManagement:
            RunSampleManagementMenu(sampleController, appView);
            break;
        case MainMenuOption::PlaceOrder:
            orderController.PlaceOrder();
            break;
        case MainMenuOption::ApproveOrRejectOrder:
            RunApproveOrRejectMenu(orderController, appView);
            break;
        case MainMenuOption::Monitoring:
            monitoringController.ShowMonitoring();
            break;
        case MainMenuOption::ProductionLine:
            productionLineController.ShowProductionQueue();
            break;
        case MainMenuOption::ReleaseOrder:
            RunReleaseMenu(orderController);
            break;
        case MainMenuOption::Exit:
            running = false;
            break;
        default:
            appView.ShowMessage("잘못된 선택입니다.");
            break;
        }
    }

    return 0;
}
