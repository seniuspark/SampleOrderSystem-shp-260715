# Phase 0 — 테스트 하네스 구성

## 목표

Main의 실행 파일 프로젝트(`Project1`)와 소스를 공유하는 GoogleTest 테스트
프로젝트(`Project1Tests`)를 구성하고, 로컬에서 실행 가능한 테스트 환경을
확보한다. 이후 모든 phase는 이 하네스 위에서 Red → Green을 반복한다.

## 참고하는 PoC 설계

- `MVC/Project1/Project1Tests`: 별도 vcxproj로 테스트 프로젝트를 구성하되,
  `Project1` 실행 파일의 소스(`.cpp`)를 `AdditionalIncludeDirectories`(`..\Project1`)
  와 `ClCompile` 상대경로로 직접 공유해 라이브러리 분리 없이 동일 소스를
  양쪽에서 컴파일하는 방식. `HarnessSanityTest.cpp`로 하네스 자체 동작을
  먼저 확인한 패턴을 그대로 따른다.
- `Json/Project1/Project1Tests`, `Dummy/Project1/Project1Tests`: `packages.config`로
  `Microsoft.googletest.v140.windesktop.msvcstl.static.rt-dyn`와
  `nlohmann.json`을 함께 설치하는 구성(Main도 JSON 파싱이 필요하므로 동일
  패키지 조합을 채택).

**코드는 복사하지 않는다.** vcxproj의 `packages.config` 패키지 이름/버전과
프로젝트 참조 구조(소스 공유 방식)만 동일한 설계로 재구성한다.

## 작성할 테스트 목록 (Red 단계)

1. `HarnessSanityTest` — gtest가 정상적으로 빌드/실행되는지 확인하는 자명한
   assertion 1개 (예: `EXPECT_EQ(1 + 1, 2)`).

## 구현할 파일 목록

- `Project1/Project1Tests/Project1Tests.vcxproj` (NuGet: googletest, nlohmann.json)
- `Project1/Project1Tests/packages.config`
- `Project1/Project1Tests/HarnessSanityTest.cpp`
- 기존 `Project1/Project1.slnx`에 `Project1Tests` 프로젝트 추가

## 완료 기준 (DoD)

- `msbuild`로 `Project1Tests`가 빌드되고, 실행 시 `HarnessSanityTest` 1건이
  통과한다.
- Visual Studio 테스트 탐색기에서도 동일하게 인식/실행된다.

## 다음 phase와의 연결점

Phase 1(Model)부터는 이 하네스에 테스트 파일을 추가하는 방식으로 진행한다.
Model 계층은 아직 JSON 파싱을 쓰지 않지만, Repository(Phase 2)에서 바로 쓸 수
있도록 `nlohmann.json` 패키지를 이 시점에 미리 구성해 둔다.
