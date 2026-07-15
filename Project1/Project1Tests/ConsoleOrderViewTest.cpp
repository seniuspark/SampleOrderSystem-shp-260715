#include "gtest/gtest.h"

#include <chrono>
#include <sstream>

#include "Model/Order.h"
#include "View/ConsoleOrderView.h"

TEST(ConsoleOrderViewTest, ReadNewOrderInputParsesAllFields)
{
    std::istringstream in("S-001\nAcme Corp\n10\n");
    std::ostringstream out;
    ConsoleOrderView view(in, out);

    NewOrderInput input = view.ReadNewOrderInput();

    EXPECT_EQ(input.SampleId, "S-001");
    EXPECT_EQ(input.CustomerName, "Acme Corp");
    EXPECT_EQ(input.Quantity, 10);
}

TEST(ConsoleOrderViewTest, ReadOrderIdReturnsLine)
{
    std::istringstream in("ORD-20260416-0001\n");
    std::ostringstream out;
    ConsoleOrderView view(in, out);

    EXPECT_EQ(view.ReadOrderId(), "ORD-20260416-0001");
}

TEST(ConsoleOrderViewTest, ShowOrderListFormatsHeaderAndRows)
{
    std::istringstream in;
    std::ostringstream out;
    ConsoleOrderView view(in, out);

    std::vector<Order> orders{
        Order("ORD-20260416-0001", "S-001", "Acme Corp", 10, std::chrono::system_clock::now()),
    };

    view.ShowOrderList(orders);

    std::string output = out.str();
    EXPECT_NE(output.find("OrderId"), std::string::npos);
    EXPECT_NE(output.find("ORD-20260416-0001"), std::string::npos);
    EXPECT_NE(output.find("Acme Corp"), std::string::npos);
    EXPECT_NE(output.find("RESERVED"), std::string::npos);
}

TEST(ConsoleOrderViewTest, ShowMessageWritesMessageToOutputStream)
{
    std::istringstream in;
    std::ostringstream out;
    ConsoleOrderView view(in, out);

    view.ShowMessage("done");

    EXPECT_NE(out.str().find("done"), std::string::npos);
}
