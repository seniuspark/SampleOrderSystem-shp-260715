#include "gtest/gtest.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <stdexcept>
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

TEST(OrderRepositoryTest, Add_DuplicateId_ThrowsAndFileUnchanged)
{
    std::filesystem::path filePath = MakeOrderRepositoryFilePath("Add_DuplicateId_ThrowsAndFileUnchanged");
    OrderRepository repository(filePath);
    repository.Add(Order("ORD-20260715-0004", "S-001", "CustomerA", 10, std::chrono::system_clock::now()));

    std::ifstream beforeInput(filePath);
    std::string contentBefore((std::istreambuf_iterator<char>(beforeInput)), std::istreambuf_iterator<char>());
    beforeInput.close();

    EXPECT_THROW(repository.Add(Order("ORD-20260715-0004", "S-002", "CustomerB", 5, std::chrono::system_clock::now())), std::invalid_argument);

    std::ifstream afterInput(filePath);
    std::string contentAfter((std::istreambuf_iterator<char>(afterInput)), std::istreambuf_iterator<char>());
    EXPECT_EQ(contentBefore, contentAfter);
}

TEST(OrderRepositoryTest, Reload_NewInstance_ReturnsSameData)
{
    std::filesystem::path filePath = MakeOrderRepositoryFilePath("Reload_NewInstance_ReturnsSameData");
    {
        OrderRepository repository(filePath);
        repository.Add(Order("ORD-20260715-0005", "S-005", "CustomerReload", 7, std::chrono::system_clock::now()));
    }

    OrderRepository reloadedRepository(filePath);
    std::vector<Order> all = reloadedRepository.GetAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].OrderId, "ORD-20260715-0005");
    EXPECT_EQ(all[0].SampleId, "S-005");
    EXPECT_EQ(all[0].CustomerName, "CustomerReload");
    EXPECT_EQ(all[0].Quantity, 7);
    EXPECT_EQ(all[0].Status, OrderStatus::RESERVED);
}

TEST(OrderRepositoryTest, Reload_MultipleEntries_PreservesOrderAndContent)
{
    std::filesystem::path filePath = MakeOrderRepositoryFilePath("Reload_MultipleEntries_PreservesOrderAndContent");
    {
        OrderRepository repository(filePath);
        repository.Add(Order("ORD-20260715-0006", "S-006", "CustomerFirst", 1, std::chrono::system_clock::now()));
        repository.Add(Order("ORD-20260715-0007", "S-007", "CustomerSecond", 2, std::chrono::system_clock::now()));
        repository.Add(Order("ORD-20260715-0008", "S-008", "CustomerThird", 3, std::chrono::system_clock::now()));
    }

    OrderRepository reloadedRepository(filePath);
    std::vector<Order> all = reloadedRepository.GetAll();
    ASSERT_EQ(all.size(), 3u);
    EXPECT_EQ(all[0].OrderId, "ORD-20260715-0006");
    EXPECT_EQ(all[1].OrderId, "ORD-20260715-0007");
    EXPECT_EQ(all[2].OrderId, "ORD-20260715-0008");
}

TEST(OrderRepositoryTest, Update_ThenReload_ChangesArePersisted)
{
    std::filesystem::path filePath = MakeOrderRepositoryFilePath("Update_ThenReload_ChangesArePersisted");
    {
        OrderRepository repository(filePath);
        Order order("ORD-20260715-0009", "S-009", "CustomerOriginal", 4, std::chrono::system_clock::now());
        repository.Add(order);

        order.Status = OrderStatus::CONFIRMED;
        bool updated = repository.Update(order);
        EXPECT_TRUE(updated);
    }

    OrderRepository reloadedRepository(filePath);
    std::optional<Order> found = reloadedRepository.FindById("ORD-20260715-0009");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->Status, OrderStatus::CONFIRMED);
}

TEST(OrderRepositoryTest, Update_UnknownId_ReturnsFalseAndFileUnchanged)
{
    std::filesystem::path filePath = MakeOrderRepositoryFilePath("Update_UnknownId_ReturnsFalseAndFileUnchanged");
    OrderRepository repository(filePath);
    repository.Add(Order("ORD-20260715-0010", "S-010", "CustomerExisting", 2, std::chrono::system_clock::now()));

    std::ifstream beforeInput(filePath);
    std::string contentBefore((std::istreambuf_iterator<char>(beforeInput)), std::istreambuf_iterator<char>());
    beforeInput.close();

    bool updated = repository.Update(Order("ORD-20260715-9999", "S-999", "CustomerUnknown", 1, std::chrono::system_clock::now()));
    EXPECT_FALSE(updated);

    std::ifstream afterInput(filePath);
    std::string contentAfter((std::istreambuf_iterator<char>(afterInput)), std::istreambuf_iterator<char>());
    EXPECT_EQ(contentBefore, contentAfter);
}

TEST(OrderRepositoryTest, Delete_ThenReload_RemovesEntry)
{
    std::filesystem::path filePath = MakeOrderRepositoryFilePath("Delete_ThenReload_RemovesEntry");
    {
        OrderRepository repository(filePath);
        repository.Add(Order("ORD-20260715-0011", "S-011", "CustomerToDelete", 3, std::chrono::system_clock::now()));

        bool deleted = repository.Delete("ORD-20260715-0011");
        EXPECT_TRUE(deleted);
    }

    OrderRepository reloadedRepository(filePath);
    EXPECT_TRUE(reloadedRepository.GetAll().empty());
    EXPECT_FALSE(reloadedRepository.FindById("ORD-20260715-0011").has_value());
}

TEST(OrderRepositoryTest, MalformedJsonSyntax_GetAllReturnsEmptyAndFileUnchanged)
{
    std::filesystem::path filePath = MakeOrderRepositoryFilePath("MalformedJsonSyntax_GetAllReturnsEmptyAndFileUnchanged");
    std::filesystem::create_directories(filePath.parent_path());
    std::string malformedContent = "{ \"orders\": [ { \"orderId\": \"ORD-20260715-0100\", ";
    {
        std::ofstream output(filePath);
        output << malformedContent;
    }

    OrderRepository repository(filePath);

    EXPECT_TRUE(repository.GetAll().empty());

    std::ifstream afterInput(filePath);
    std::string contentAfter((std::istreambuf_iterator<char>(afterInput)), std::istreambuf_iterator<char>());
    EXPECT_EQ(contentAfter, malformedContent);
}

TEST(OrderRepositoryTest, TopLevelIsArray_GetAllReturnsEmpty)
{
    std::filesystem::path filePath = MakeOrderRepositoryFilePath("TopLevelIsArray_GetAllReturnsEmpty");
    std::filesystem::create_directories(filePath.parent_path());
    {
        std::ofstream output(filePath);
        output << "[ { \"orderId\": \"ORD-20260715-0101\" } ]";
    }

    OrderRepository repository(filePath);

    EXPECT_TRUE(repository.GetAll().empty());
}

TEST(OrderRepositoryTest, MissingOrdersKey_GetAllReturnsEmpty)
{
    std::filesystem::path filePath = MakeOrderRepositoryFilePath("MissingOrdersKey_GetAllReturnsEmpty");
    std::filesystem::create_directories(filePath.parent_path());
    {
        std::ofstream output(filePath);
        output << "{ \"unexpectedKey\": [] }";
    }

    OrderRepository repository(filePath);

    EXPECT_TRUE(repository.GetAll().empty());
}

TEST(OrderRepositoryTest, OneElementMissingRequiredField_SkipsThatElementOnly)
{
    std::filesystem::path filePath = MakeOrderRepositoryFilePath("OneElementMissingRequiredField_SkipsThatElementOnly");
    std::filesystem::create_directories(filePath.parent_path());
    {
        std::ofstream output(filePath);
        output << R"({
            "orders": [
                { "orderId": "ORD-20260715-0102", "sampleId": "S-102", "customerName": "CustomerValid", "quantity": 3, "status": "RESERVED", "createdAt": "2026-07-15T00:00:00Z" },
                { "orderId": "ORD-20260715-0103", "sampleId": "S-103", "quantity": 3, "status": "RESERVED", "createdAt": "2026-07-15T00:00:00Z" }
            ]
        })";
    }

    OrderRepository repository(filePath);

    std::vector<Order> all = repository.GetAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].OrderId, "ORD-20260715-0102");
}

TEST(OrderRepositoryTest, OneElementUnknownStatus_SkipsThatElementOnly)
{
    std::filesystem::path filePath = MakeOrderRepositoryFilePath("OneElementUnknownStatus_SkipsThatElementOnly");
    std::filesystem::create_directories(filePath.parent_path());
    {
        std::ofstream output(filePath);
        output << R"({
            "orders": [
                { "orderId": "ORD-20260715-0104", "sampleId": "S-104", "customerName": "CustomerValid", "quantity": 3, "status": "RESERVED", "createdAt": "2026-07-15T00:00:00Z" },
                { "orderId": "ORD-20260715-0105", "sampleId": "S-105", "customerName": "CustomerInvalid", "quantity": 3, "status": "UNKNOWN_STATUS", "createdAt": "2026-07-15T00:00:00Z" }
            ]
        })";
    }

    OrderRepository repository(filePath);

    std::vector<Order> all = repository.GetAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].OrderId, "ORD-20260715-0104");
}

TEST(OrderRepositoryTest, Delete_UnknownId_ReturnsFalseAndFileUnchanged)
{
    std::filesystem::path filePath = MakeOrderRepositoryFilePath("Delete_UnknownId_ReturnsFalseAndFileUnchanged");
    OrderRepository repository(filePath);
    repository.Add(Order("ORD-20260715-0012", "S-012", "CustomerExisting", 2, std::chrono::system_clock::now()));

    std::ifstream beforeInput(filePath);
    std::string contentBefore((std::istreambuf_iterator<char>(beforeInput)), std::istreambuf_iterator<char>());
    beforeInput.close();

    bool deleted = repository.Delete("ORD-20260715-9999");
    EXPECT_FALSE(deleted);

    std::ifstream afterInput(filePath);
    std::string contentAfter((std::istreambuf_iterator<char>(afterInput)), std::istreambuf_iterator<char>());
    EXPECT_EQ(contentBefore, contentAfter);
}
