#include "gtest/gtest.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "Model/Order.h"
#include "Model/OrderStatus.h"
#include "Repository/OrderRepository.h"

namespace
{

std::filesystem::path MakeOrderRepositoryFilePath(const std::string& testName)
{
    std::filesystem::path dir = std::filesystem::temp_directory_path() / "Project1Tests" / "OrderRepositoryTest" / testName;
    std::filesystem::remove_all(dir);
    return dir / "orders.json";
}

}

TEST(OrderRepositoryTest, FileDoesNotExist_GetAllReturnsEmpty)
{
    OrderRepository repository(MakeOrderRepositoryFilePath("FileDoesNotExist_GetAllReturnsEmpty"));

    EXPECT_TRUE(repository.GetAll().empty());
}

TEST(OrderRepositoryTest, FileIsEmpty_GetAllReturnsEmpty)
{
    std::filesystem::path filePath = MakeOrderRepositoryFilePath("FileIsEmpty_GetAllReturnsEmpty");
    std::filesystem::create_directories(filePath.parent_path());
    std::ofstream(filePath).close();

    OrderRepository repository(filePath);

    EXPECT_TRUE(repository.GetAll().empty());
}

TEST(OrderRepositoryTest, Add_ThenGetAll_ContainsAddedOrder)
{
    OrderRepository repository(MakeOrderRepositoryFilePath("Add_ThenGetAll_ContainsAddedOrder"));
    Order order("ORD-20260715-0001", "S-001", "CustomerA", 10, std::chrono::system_clock::now());

    repository.Add(order);

    std::vector<Order> all = repository.GetAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].OrderId, "ORD-20260715-0001");
    EXPECT_EQ(all[0].SampleId, "S-001");
    EXPECT_EQ(all[0].CustomerName, "CustomerA");
    EXPECT_EQ(all[0].Quantity, 10);
    EXPECT_EQ(all[0].Status, OrderStatus::RESERVED);
}

TEST(OrderRepositoryTest, Add_ThenFindById_ReturnsOrder)
{
    OrderRepository repository(MakeOrderRepositoryFilePath("Add_ThenFindById_ReturnsOrder"));
    Order order("ORD-20260715-0002", "S-002", "CustomerB", 5, std::chrono::system_clock::now());

    repository.Add(order);

    std::optional<Order> found = repository.FindById("ORD-20260715-0002");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->CustomerName, "CustomerB");
}

TEST(OrderRepositoryTest, Add_CreatesFileOnDisk)
{
    std::filesystem::path filePath = MakeOrderRepositoryFilePath("Add_CreatesFileOnDisk");
    OrderRepository repository(filePath);

    repository.Add(Order("ORD-20260715-0003", "S-003", "CustomerC", 1, std::chrono::system_clock::now()));

    ASSERT_TRUE(std::filesystem::exists(filePath));
    std::ifstream input(filePath);
    std::string content((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    EXPECT_NE(content.find("ORD-20260715-0003"), std::string::npos);
}
