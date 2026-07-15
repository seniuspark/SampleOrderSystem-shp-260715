#include "gtest/gtest.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>
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

TEST(SampleRepositoryTest, Add_DuplicateId_ThrowsAndFileUnchanged)
{
    std::filesystem::path filePath = MakeSampleRepositoryFilePath("Add_DuplicateId_ThrowsAndFileUnchanged");
    SampleRepository repository(filePath);
    repository.Add(Sample("S-004", "FirstSample", 3.0, 0.9, 0));

    std::ifstream beforeInput(filePath);
    std::string contentBefore((std::istreambuf_iterator<char>(beforeInput)), std::istreambuf_iterator<char>());
    beforeInput.close();

    EXPECT_THROW(repository.Add(Sample("S-004", "DuplicateSample", 4.0, 0.8, 0)), std::invalid_argument);

    std::ifstream afterInput(filePath);
    std::string contentAfter((std::istreambuf_iterator<char>(afterInput)), std::istreambuf_iterator<char>());
    EXPECT_EQ(contentBefore, contentAfter);
}

TEST(SampleRepositoryTest, Reload_NewInstance_ReturnsSameData)
{
    std::filesystem::path filePath = MakeSampleRepositoryFilePath("Reload_NewInstance_ReturnsSameData");
    {
        SampleRepository repository(filePath);
        repository.Add(Sample("S-005", "ReloadSample", 6.0, 0.85, 3));
    }

    SampleRepository reloadedRepository(filePath);
    std::vector<Sample> all = reloadedRepository.GetAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].SampleId, "S-005");
    EXPECT_EQ(all[0].Name, "ReloadSample");
    EXPECT_DOUBLE_EQ(all[0].AvgProductionTime, 6.0);
    EXPECT_DOUBLE_EQ(all[0].Yield, 0.85);
    EXPECT_EQ(all[0].Stock, 3);
}

TEST(SampleRepositoryTest, Reload_MultipleEntries_PreservesOrderAndContent)
{
    std::filesystem::path filePath = MakeSampleRepositoryFilePath("Reload_MultipleEntries_PreservesOrderAndContent");
    {
        SampleRepository repository(filePath);
        repository.Add(Sample("S-006", "First", 1.0, 0.5, 1));
        repository.Add(Sample("S-007", "Second", 2.0, 0.6, 2));
        repository.Add(Sample("S-008", "Third", 3.0, 0.7, 3));
    }

    SampleRepository reloadedRepository(filePath);
    std::vector<Sample> all = reloadedRepository.GetAll();
    ASSERT_EQ(all.size(), 3u);
    EXPECT_EQ(all[0].SampleId, "S-006");
    EXPECT_EQ(all[1].SampleId, "S-007");
    EXPECT_EQ(all[2].SampleId, "S-008");
}

TEST(SampleRepositoryTest, Update_ThenReload_ChangesArePersisted)
{
    std::filesystem::path filePath = MakeSampleRepositoryFilePath("Update_ThenReload_ChangesArePersisted");
    {
        SampleRepository repository(filePath);
        repository.Add(Sample("S-009", "Original", 5.0, 0.9, 0));

        bool updated = repository.Update(Sample("S-009", "Updated", 7.5, 0.75, 20));
        EXPECT_TRUE(updated);
    }

    SampleRepository reloadedRepository(filePath);
    std::optional<Sample> found = reloadedRepository.FindById("S-009");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->Name, "Updated");
    EXPECT_DOUBLE_EQ(found->AvgProductionTime, 7.5);
    EXPECT_DOUBLE_EQ(found->Yield, 0.75);
    EXPECT_EQ(found->Stock, 20);
}

TEST(SampleRepositoryTest, Update_UnknownId_ReturnsFalseAndFileUnchanged)
{
    std::filesystem::path filePath = MakeSampleRepositoryFilePath("Update_UnknownId_ReturnsFalseAndFileUnchanged");
    SampleRepository repository(filePath);
    repository.Add(Sample("S-010", "Existing", 5.0, 0.9, 0));

    std::ifstream beforeInput(filePath);
    std::string contentBefore((std::istreambuf_iterator<char>(beforeInput)), std::istreambuf_iterator<char>());
    beforeInput.close();

    bool updated = repository.Update(Sample("S-999", "Unknown", 1.0, 0.5, 0));
    EXPECT_FALSE(updated);

    std::ifstream afterInput(filePath);
    std::string contentAfter((std::istreambuf_iterator<char>(afterInput)), std::istreambuf_iterator<char>());
    EXPECT_EQ(contentBefore, contentAfter);
}

TEST(SampleRepositoryTest, Delete_ThenReload_RemovesEntry)
{
    std::filesystem::path filePath = MakeSampleRepositoryFilePath("Delete_ThenReload_RemovesEntry");
    {
        SampleRepository repository(filePath);
        repository.Add(Sample("S-011", "ToDelete", 5.0, 0.9, 0));

        bool deleted = repository.Delete("S-011");
        EXPECT_TRUE(deleted);
    }

    SampleRepository reloadedRepository(filePath);
    EXPECT_TRUE(reloadedRepository.GetAll().empty());
    EXPECT_FALSE(reloadedRepository.FindById("S-011").has_value());
}

TEST(SampleRepositoryTest, MalformedJsonSyntax_GetAllReturnsEmptyAndFileUnchanged)
{
    std::filesystem::path filePath = MakeSampleRepositoryFilePath("MalformedJsonSyntax_GetAllReturnsEmptyAndFileUnchanged");
    std::filesystem::create_directories(filePath.parent_path());
    std::string malformedContent = "{ \"samples\": [ { \"sampleId\": \"S-001\", ";
    {
        std::ofstream output(filePath);
        output << malformedContent;
    }

    SampleRepository repository(filePath);

    EXPECT_TRUE(repository.GetAll().empty());

    std::ifstream afterInput(filePath);
    std::string contentAfter((std::istreambuf_iterator<char>(afterInput)), std::istreambuf_iterator<char>());
    EXPECT_EQ(contentAfter, malformedContent);
}

TEST(SampleRepositoryTest, TopLevelIsArray_GetAllReturnsEmpty)
{
    std::filesystem::path filePath = MakeSampleRepositoryFilePath("TopLevelIsArray_GetAllReturnsEmpty");
    std::filesystem::create_directories(filePath.parent_path());
    {
        std::ofstream output(filePath);
        output << "[ { \"sampleId\": \"S-001\" } ]";
    }

    SampleRepository repository(filePath);

    EXPECT_TRUE(repository.GetAll().empty());
}

TEST(SampleRepositoryTest, MissingSamplesKey_GetAllReturnsEmpty)
{
    std::filesystem::path filePath = MakeSampleRepositoryFilePath("MissingSamplesKey_GetAllReturnsEmpty");
    std::filesystem::create_directories(filePath.parent_path());
    {
        std::ofstream output(filePath);
        output << "{ \"unexpectedKey\": [] }";
    }

    SampleRepository repository(filePath);

    EXPECT_TRUE(repository.GetAll().empty());
}

TEST(SampleRepositoryTest, OneElementMissingRequiredField_SkipsThatElementOnly)
{
    std::filesystem::path filePath = MakeSampleRepositoryFilePath("OneElementMissingRequiredField_SkipsThatElementOnly");
    std::filesystem::create_directories(filePath.parent_path());
    {
        std::ofstream output(filePath);
        output << R"({
            "samples": [
                { "sampleId": "S-101", "name": "Valid", "avgProductionTime": 5.0, "yield": 0.9, "stock": 10 },
                { "sampleId": "S-102", "avgProductionTime": 5.0, "yield": 0.9, "stock": 10 }
            ]
        })";
    }

    SampleRepository repository(filePath);

    std::vector<Sample> all = repository.GetAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].SampleId, "S-101");
}

TEST(SampleRepositoryTest, Delete_UnknownId_ReturnsFalseAndFileUnchanged)
{
    std::filesystem::path filePath = MakeSampleRepositoryFilePath("Delete_UnknownId_ReturnsFalseAndFileUnchanged");
    SampleRepository repository(filePath);
    repository.Add(Sample("S-012", "Existing", 5.0, 0.9, 0));

    std::ifstream beforeInput(filePath);
    std::string contentBefore((std::istreambuf_iterator<char>(beforeInput)), std::istreambuf_iterator<char>());
    beforeInput.close();

    bool deleted = repository.Delete("S-999");
    EXPECT_FALSE(deleted);

    std::ifstream afterInput(filePath);
    std::string contentAfter((std::istreambuf_iterator<char>(afterInput)), std::istreambuf_iterator<char>());
    EXPECT_EQ(contentBefore, contentAfter);
}
