#include "gtest/gtest.h"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace
{

// 이 소스 파일이 위치한 디렉터리를 컴파일 시점 매크로 __FILE__로 계산한다.
// 런타임 작업 디렉터리(cwd)가 빌드 방식에 따라 달라져도 항상 같은 소스
// 파일을 가리키도록 하기 위함이다.
std::string TestSourceDir()
{
    std::string file = __FILE__;
    auto pos = file.find_last_of("/\\");
    return pos == std::string::npos ? std::string(".") : file.substr(0, pos);
}

std::string ReadFileContent(const std::string& pathRelativeToThisFile)
{
    std::string fullPath = TestSourceDir() + "/" + pathRelativeToThisFile;
    std::ifstream file(fullPath);
    if (!file)
    {
        ADD_FAILURE() << "파일을 열 수 없습니다: " << fullPath;
        return {};
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool ContainsAnyPattern(const std::string& content, const std::vector<std::string>& patterns)
{
    for (const auto& pattern : patterns)
    {
        if (content.find(pattern) != std::string::npos)
        {
            return true;
        }
    }
    return false;
}

const std::vector<std::string> kConsoleIoPatterns{
    "<iostream>", "std::cin", "std::cout", "std::cerr", "printf(",
};

const std::vector<std::string> kFileIoPatterns{
    "std::ifstream", "std::ofstream", "<fstream>",
};

// 재고/수율 비교 등 도메인 판단으로 의심되는 패턴. 뒤에 공백을 붙여
// "<< ..." 같은 스트림 출력 연산자와 구분한다. 완벽한 정적 검증은 불가능
// 하므로 휴리스틱이며, 최종 판단은 코드 리뷰로 보완한다.
const std::vector<std::string> kBusinessLogicPatterns{
    "Stock < ", "Stock > ", "Stock <= ", "Stock >= ",
    "Yield < ", "Yield > ", "Yield <= ", "Yield >= ",
};

const std::vector<std::string> kModelFiles{
    "../../Project1/Model/Sample.h",
    "../../Project1/Model/Order.h",
    "../../Project1/Model/OrderStatus.h",
    "../../Project1/Model/ProductionQueueCalculator.h",
    "../../Project1/Model/StockStatus.h",
    "../../Project1/Model/IClock.h",
    "../../Project1/Model/AvailableStockCalculator.h",
    "../../Project1/Model/ProductionCompletionJudge.h",
    "../../Project1/Model/OrderIdAllocator.h",
};

const std::vector<std::string> kControllerFiles{
    "../../Project1/Controller/SampleController.h",
    "../../Project1/Controller/OrderController.h",
    "../../Project1/Controller/ProductionLineController.h",
    "../../Project1/Controller/MonitoringController.h",
    "../../Project1/Controller/ProductionQueueItem.h",
};

const std::vector<std::string> kViewFiles{
    "../../Project1/View/ConsoleSampleView.h",
    "../../Project1/View/ConsoleOrderView.h",
    "../../Project1/View/ConsoleProductionLineView.h",
    "../../Project1/View/ConsoleMonitoringView.h",
    "../../Project1/View/ConsoleAppView.h",
};

const std::vector<std::string> kRepositoryFiles{
    "../../Project1/Repository/JsonCodec.h",
    "../../Project1/Repository/SampleRepository.h",
    "../../Project1/Repository/OrderRepository.h",
};

}  // namespace

namespace LayerBoundaryTest
{

TEST(ContainsAnyPatternHelperTest, FindsPatternWhenPresent)
{
    EXPECT_TRUE(ContainsAnyPattern("void f() { std::cout << 1; }", kConsoleIoPatterns));
}

TEST(ContainsAnyPatternHelperTest, ReturnsFalseWhenAbsent)
{
    EXPECT_FALSE(ContainsAnyPattern("void f() { return; }", kConsoleIoPatterns));
}

TEST(LayerBoundaryTest, ModelSourcesContainNoConsoleIo)
{
    for (const auto& path : kModelFiles)
    {
        std::string content = ReadFileContent(path);
        EXPECT_FALSE(ContainsAnyPattern(content, kConsoleIoPatterns))
            << path << " 에 콘솔 입출력 관련 코드가 있습니다.";
    }
}

TEST(LayerBoundaryTest, ControllerSourcesContainNoConsoleIo)
{
    for (const auto& path : kControllerFiles)
    {
        std::string content = ReadFileContent(path);
        EXPECT_FALSE(ContainsAnyPattern(content, kConsoleIoPatterns))
            << path << " 에 콘솔 입출력 관련 코드가 있습니다.";
    }
}

TEST(LayerBoundaryTest, ModelSourcesContainNoFileIo)
{
    for (const auto& path : kModelFiles)
    {
        std::string content = ReadFileContent(path);
        EXPECT_FALSE(ContainsAnyPattern(content, kFileIoPatterns))
            << path << " 에 파일 입출력 관련 코드가 있습니다(영속성은 Repository 계층의 책임).";
    }
}

TEST(LayerBoundaryTest, ControllerSourcesContainNoFileIo)
{
    for (const auto& path : kControllerFiles)
    {
        std::string content = ReadFileContent(path);
        EXPECT_FALSE(ContainsAnyPattern(content, kFileIoPatterns))
            << path << " 에 파일 입출력 관련 코드가 있습니다(영속성은 Repository 계층의 책임).";
    }
}

TEST(LayerBoundaryTest, ViewSourcesContainNoFileIo)
{
    for (const auto& path : kViewFiles)
    {
        std::string content = ReadFileContent(path);
        EXPECT_FALSE(ContainsAnyPattern(content, kFileIoPatterns))
            << path << " 에 파일 입출력 관련 코드가 있습니다(영속성은 Repository 계층의 책임).";
    }
}

TEST(LayerBoundaryTest, RepositorySourcesAreTheOnlyPlaceExpectedToUseFileIo)
{
    bool anyRepositoryUsesFileIo = false;
    for (const auto& path : kRepositoryFiles)
    {
        std::string content = ReadFileContent(path);
        if (ContainsAnyPattern(content, kFileIoPatterns))
        {
            anyRepositoryUsesFileIo = true;
        }
    }
    EXPECT_TRUE(anyRepositoryUsesFileIo) << "Repository 계층 어디에도 파일 입출력이 없습니다 — 검사 대상 목록이 최신 상태인지 확인하세요.";
}

TEST(LayerBoundaryTest, ViewSourcesContainNoStockOrYieldBusinessComparison)
{
    for (const auto& path : kViewFiles)
    {
        std::string content = ReadFileContent(path);
        EXPECT_FALSE(ContainsAnyPattern(content, kBusinessLogicPatterns))
            << path << " 에 재고/수율 비교 등 비즈니스 판단으로 의심되는 코드가 있습니다. "
            << "(휴리스틱 검사이므로 최종 확인은 코드 리뷰로 보완할 것)";
    }
}

}  // namespace LayerBoundaryTest
