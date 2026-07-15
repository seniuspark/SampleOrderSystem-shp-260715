#include "gtest/gtest.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "Model/Sample.h"
#include "Repository/SampleRepository.h"

namespace
{

std::filesystem::path MakeSampleRepositoryFilePath(const std::string& testName)
{
    std::filesystem::path dir = std::filesystem::temp_directory_path() / "Project1Tests" / "SampleRepositoryTest" / testName;
    std::filesystem::remove_all(dir);
    return dir / "samples.json";
}

}

TEST(SampleRepositoryTest, FileDoesNotExist_GetAllReturnsEmpty)
{
    SampleRepository repository(MakeSampleRepositoryFilePath("FileDoesNotExist_GetAllReturnsEmpty"));

    EXPECT_TRUE(repository.GetAll().empty());
}

TEST(SampleRepositoryTest, FileIsEmpty_GetAllReturnsEmpty)
{
    std::filesystem::path filePath = MakeSampleRepositoryFilePath("FileIsEmpty_GetAllReturnsEmpty");
    std::filesystem::create_directories(filePath.parent_path());
    std::ofstream(filePath).close();

    SampleRepository repository(filePath);

    EXPECT_TRUE(repository.GetAll().empty());
}

TEST(SampleRepositoryTest, Add_ThenGetAll_ContainsAddedSample)
{
    SampleRepository repository(MakeSampleRepositoryFilePath("Add_ThenGetAll_ContainsAddedSample"));
    Sample sample("S-001", "TestSample", 12.5, 0.95, 0);

    repository.Add(sample);

    std::vector<Sample> all = repository.GetAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].SampleId, "S-001");
    EXPECT_EQ(all[0].Name, "TestSample");
    EXPECT_DOUBLE_EQ(all[0].AvgProductionTime, 12.5);
    EXPECT_DOUBLE_EQ(all[0].Yield, 0.95);
    EXPECT_EQ(all[0].Stock, 0);
}

TEST(SampleRepositoryTest, Add_ThenFindById_ReturnsSample)
{
    SampleRepository repository(MakeSampleRepositoryFilePath("Add_ThenFindById_ReturnsSample"));
    Sample sample("S-002", "AnotherSample", 5.0, 0.8, 0);

    repository.Add(sample);

    std::optional<Sample> found = repository.FindById("S-002");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->Name, "AnotherSample");
}

TEST(SampleRepositoryTest, Add_CreatesFileOnDisk)
{
    std::filesystem::path filePath = MakeSampleRepositoryFilePath("Add_CreatesFileOnDisk");
    SampleRepository repository(filePath);

    repository.Add(Sample("S-003", "DiskSample", 3.0, 0.9, 0));

    ASSERT_TRUE(std::filesystem::exists(filePath));
    std::ifstream input(filePath);
    std::string content((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    EXPECT_NE(content.find("S-003"), std::string::npos);
}
