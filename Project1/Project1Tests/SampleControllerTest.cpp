#include "gtest/gtest.h"

#include <filesystem>
#include <string>

#include "Controller/SampleController.h"
#include "Repository/SampleRepository.h"
#include "View/FakeSampleView.h"

namespace
{

std::filesystem::path MakeSampleRepositoryFilePath(const std::string& testName)
{
    std::filesystem::path dir = std::filesystem::temp_directory_path() / "Project1Tests" / "SampleControllerTest" / testName;
    std::filesystem::remove_all(dir);
    return dir / "samples.json";
}

}  // namespace

TEST(SampleControllerTest, RegisterSample_ValidInput_AddsToRepositoryAndShowsSuccessMessage)
{
    SampleRepository repository(MakeSampleRepositoryFilePath("RegisterSample_ValidInput_AddsToRepositoryAndShowsSuccessMessage"));
    FakeSampleView view;
    SampleController controller(repository, view);

    view.EnqueueNewSampleInput(NewSampleInput{ "S-001", "테스트시료", 10.0, 0.9 });
    controller.RegisterSample();

    std::optional<Sample> added = repository.FindById("S-001");
    ASSERT_TRUE(added.has_value());
    EXPECT_EQ(added->Name, "테스트시료");
    EXPECT_EQ(added->Stock, 0);
    EXPECT_NE(view.LastMessage().find("S-001"), std::string::npos);
}

TEST(SampleControllerTest, RegisterSample_DuplicateSampleId_ShowsFailureAndRepositoryUnchanged)
{
    SampleRepository repository(MakeSampleRepositoryFilePath("RegisterSample_DuplicateSampleId_ShowsFailureAndRepositoryUnchanged"));
    repository.Add(Sample("S-001", "원래시료", 10.0, 0.9, 0));
    FakeSampleView view;
    SampleController controller(repository, view);

    view.EnqueueNewSampleInput(NewSampleInput{ "S-001", "중복시료", 5.0, 0.8 });
    controller.RegisterSample();

    EXPECT_EQ(repository.GetAll().size(), 1u);
    EXPECT_EQ(repository.FindById("S-001")->Name, "원래시료");
    EXPECT_FALSE(view.LastMessage().empty());
}

TEST(SampleControllerTest, RegisterSample_InvalidYield_RejectedWithErrorMessage)
{
    SampleRepository repository(MakeSampleRepositoryFilePath("RegisterSample_InvalidYield_RejectedWithErrorMessage"));
    FakeSampleView view;
    SampleController controller(repository, view);

    view.EnqueueNewSampleInput(NewSampleInput{ "S-002", "잘못된수율시료", 10.0, 1.5 });
    controller.RegisterSample();

    EXPECT_TRUE(repository.GetAll().empty());
    EXPECT_FALSE(view.LastMessage().empty());
}

TEST(SampleControllerTest, ListSamples_ReturnsAllRegisteredSamplesWithStock)
{
    SampleRepository repository(MakeSampleRepositoryFilePath("ListSamples_ReturnsAllRegisteredSamplesWithStock"));
    repository.Add(Sample("S-001", "시료A", 10.0, 0.9, 5));
    repository.Add(Sample("S-002", "시료B", 20.0, 0.8, 3));
    FakeSampleView view;
    SampleController controller(repository, view);

    controller.ListSamples();

    ASSERT_EQ(view.LastShownSamples().size(), 2u);
    EXPECT_EQ(view.LastShownSamples()[0].Stock, 5);
    EXPECT_EQ(view.LastShownSamples()[1].Stock, 3);
}

TEST(SampleControllerTest, ListSamples_NoSamplesRegistered_ShowsEmptyListMessage)
{
    SampleRepository repository(MakeSampleRepositoryFilePath("ListSamples_NoSamplesRegistered_ShowsEmptyListMessage"));
    FakeSampleView view;
    SampleController controller(repository, view);

    controller.ListSamples();

    EXPECT_TRUE(view.LastShownSamples().empty());
    EXPECT_FALSE(view.LastMessage().empty());
}

TEST(SampleControllerTest, SearchSamplesByName_MatchingKeyword_ReturnsOnlyMatchedSamples)
{
    SampleRepository repository(MakeSampleRepositoryFilePath("SearchSamplesByName_MatchingKeyword_ReturnsOnlyMatchedSamples"));
    repository.Add(Sample("S-001", "웨이퍼A", 10.0, 0.9, 5));
    repository.Add(Sample("S-002", "다이오드B", 20.0, 0.8, 3));
    FakeSampleView view;
    SampleController controller(repository, view);

    view.EnqueueSearchKeyword("웨이퍼");
    controller.SearchSamplesByName();

    ASSERT_EQ(view.LastShownSamples().size(), 1u);
    EXPECT_EQ(view.LastShownSamples()[0].SampleId, "S-001");
}

TEST(SampleControllerTest, SearchSamplesByName_NoMatch_ShowsNoResultMessage)
{
    SampleRepository repository(MakeSampleRepositoryFilePath("SearchSamplesByName_NoMatch_ShowsNoResultMessage"));
    repository.Add(Sample("S-001", "웨이퍼A", 10.0, 0.9, 5));
    FakeSampleView view;
    SampleController controller(repository, view);

    view.EnqueueSearchKeyword("존재하지않음");
    controller.SearchSamplesByName();

    EXPECT_TRUE(view.LastShownSamples().empty());
    EXPECT_FALSE(view.LastMessage().empty());
}

TEST(SampleControllerTest, RegisterSample_AlwaysInitializesStockToZero)
{
    SampleRepository repository(MakeSampleRepositoryFilePath("RegisterSample_AlwaysInitializesStockToZero"));
    FakeSampleView view;
    SampleController controller(repository, view);

    view.EnqueueNewSampleInput(NewSampleInput{ "S-003", "재고테스트시료", 15.0, 0.7 });
    controller.RegisterSample();

    ASSERT_TRUE(repository.FindById("S-003").has_value());
    EXPECT_EQ(repository.FindById("S-003")->Stock, 0);
}
