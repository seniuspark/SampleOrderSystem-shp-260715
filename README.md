# SampleOrderSystem-shp-260715

반도체 시료 생산주문관리 콘솔 시스템. 시료(Sample) 등록/조회, 주문 접수,
승인/거절, 재고·생산 모니터링, 생산라인 조회, 출고 처리를 하나의 콘솔
애플리케이션에서 처리한다.

## 1. 사용자가 알아야 할 정책

이 시스템은 아래 정책들을 내부적으로 자동 적용한다. 화면에 별도로 안내되지
않는 항목도 있으므로, 담당자는 아래 내용을 알고 있어야 한다.

### 1-1. 재고(Stock) 증감 시점

물리적 재고는 **생산완료 시점**과 **출고 시점**, 단 두 곳에서만 바뀐다.

| 시점 | 재고 변화 |
|---|---|
| 시료 등록 | 항상 `0`으로 시작 (등록 화면에는 재고 입력란이 없음) |
| 주문 승인(재고 충분 → 즉시 출고대기) | **변화 없음** — 물리적 재고는 그대로, 내부적으로만 "예약" 처리 |
| 생산 완료(생산중 → 출고대기 자동 전환) | `+= 실생산량` (재고 입고) |
| 출고 처리(출고대기 → 출고완료) | `-= 주문 수량` (재고 출고) |

즉, "주문을 승인했더니 재고가 줄었다"는 일어나지 않는다. 재고는 오직
생산이 끝나서 늘거나, 실제로 출고할 때만 준다.

### 1-2. 주문 승인 시 재고 충분/부족 판정 기준

승인 시 "충분/부족"은 **재고 수량 그 자체**가 아니라 **가용재고**로
판정한다.

```
가용재고 = 현재 재고(Stock) − 아직 출고되지 않은 승인(CONFIRMED) 주문들의 수량 합
```

이미 승인은 났지만 아직 출고 전인 물량은 "이미 확보된 것"으로 보고 다른
주문이 같은 재고를 중복으로 확보하지 못하게 막기 위함이다.

- 가용재고 ≥ 주문 수량 → 즉시 **출고대기(CONFIRMED)**
- 가용재고 < 주문 수량 → **생산중(PRODUCING)**으로 전환, 생산 큐에 자동 등록

### 1-3. 생산 완료는 자동으로 처리된다

생산완료를 수동으로 처리하는 메뉴는 없다. 생산 큐 선두 항목의 "생산
시작 시각 + 총생산시간"이 현재 시각을 지났으면, **생산라인 조회 메뉴에
들어갈 때마다** 자동으로 완료 처리되어 상태가 출고대기로 바뀌고 재고에
실생산량이 반영된다. 즉 완료 여부를 보려면 5번(생산라인 조회) 메뉴에
다시 들어가야 한다.

- 실생산량 = `ceil(부족분 / 수율)` — 수율이 1 미만이면 주문 수량보다
  많이 생산되며(초과분), 이 초과분은 생산완료 즉시 가용재고 계산에
  자동으로 반영된다(별도 처리 불필요).
- 생산 큐는 주문(Order) 1건당 1개 항목이며, FIFO(선입선출)로 처리된다.

### 1-4. 수율(Yield)은 등록 시 입력한 값으로 고정된다

시료 등록 시 입력한 수율은 이후 생산 실적에 따라 자동으로 갱신되지
않는다. 실측 수율을 반영하려면 사람이 직접 값을 다시 확인해 새 시료로
등록하거나 정책 변경이 필요하다(현재 버전은 갱신 기능 없음).

### 1-5. 주문번호(OrderId) 채번 규칙

`ORD-YYYYMMDD-####` 형식으로 시스템이 자동 채번한다. 순번은 **같은
날짜 안에서만** 누적되며, 날짜가 바뀌면(자정 경과) `0001`부터 다시
시작한다. 사용자가 직접 주문번호를 입력하는 화면은 없다.

### 1-6. 데이터는 항상 즉시 저장되고, 강제 종료에도 안전하다

메뉴 동작 하나(등록/주문/승인/거절/출고 등)가 끝나면 그 결과는 즉시
`data/samples.json`, `data/orders.json` 파일에 저장된다. 프로그램을
정상 종료(`0. 종료`)하지 않고 창을 강제로 닫거나 프로세스를 죽여도,
그 시점까지 완료된 작업은 파일에 남아 있고 다음 실행 시 정상적으로
이어서 사용할 수 있다(생산 큐에 등록된 항목도 포함).

### 1-7. 데이터 파일이 손상되었을 때

`data/samples.json` 또는 `data/orders.json`이 없거나, 비어 있거나,
JSON 문법이 깨졌거나, 일부 항목의 필드가 잘못되어 있어도 프로그램은
**죽지 않는다**. 문제가 있는 항목만 조용히 건너뛰고 나머지 정상 데이터로
계속 동작한다(fail-soft). 단, 이미 존재하는 시료/주문 ID로 다시 등록을
시도하면 그 등록 자체는 실패 처리된다(오류 메시지 표시).

## 2. 소프트웨어 구조

Model(도메인) → Repository(영속성) → Controller(흐름 제어) → View(콘솔
입출력) 4계층 구조다. Controller는 실제 콘솔 클래스가 아니라 View
추상 인터페이스에만 의존하므로, 콘솔 I/O 없이도 Controller 단위 테스트가
가능하다.

```
Project1/
├── Project1.slnx                       # Visual Studio 솔루션
├── Project1/                           # 실행 파일 프로젝트
│   ├── main.cpp                        # Repository/Controller/View 조립, 메인 루프
│   ├── Model/                          # 콘솔·파일 I/O 없는 순수 도메인 로직
│   │   ├── Sample.h, Order.h, OrderStatus.h        # 엔티티 + 불변식 검증
│   │   ├── IClock.h                                # 시간 소스 추상화(SystemClock/테스트용 FixedClock)
│   │   ├── ProductionQueueCalculator.h             # 부족분/실생산량/총생산시간 계산
│   │   ├── StockStatus.h                           # 재고 여유/부족/고갈 판정
│   │   ├── AvailableStockCalculator.h              # 가용재고 계산
│   │   ├── ProductionCompletionJudge.h             # 생산완료 여부 판정(IClock 기준)
│   │   └── OrderIdAllocator.h                      # OrderId 일자별 채번
│   ├── Repository/                     # JSON 파일 기반 영속성 (파일 I/O는 여기에만 존재)
│   │   ├── JsonCodec.h                             # Sample/Order ↔ JSON 변환, 손상 데이터 fail-soft 처리
│   │   ├── SampleRepository.h, OrderRepository.h   # GetAll/FindById/Add/Update/Delete
│   ├── Controller/                     # 메뉴별 흐름 제어 (View 인터페이스에만 의존)
│   │   ├── SampleController.h                      # 시료 등록/조회/검색
│   │   ├── OrderController.h                       # 주문 접수/승인/거절/출고
│   │   ├── ProductionLineController.h              # 생산 큐(FIFO), 자동 완료 판정/복원
│   │   └── MonitoringController.h                  # 상태별 집계, 재고 판정
│   ├── View/                            # 콘솔 입출력(이 계층에만 std::cin/std::cout 존재)
│   │   ├── I*View.h                                # Controller가 의존하는 추상 인터페이스
│   │   ├── Console*View.h                          # 실제 콘솔 구현체(istream/ostream 주입 가능)
│   │   ├── ConsoleAppView.h                        # 메인/서브 메뉴 탐색, 요약 정보 출력
│   │   ├── MenuOption.h                            # 메뉴 번호 enum class
│   │   └── SummaryRenderer.h                       # 메인 메뉴 요약 정보 렌더링
│   └── Seed/DemoDataSeeder.h            # 최초 실행 시 데모 데이터 자동 생성
└── Project1Tests/                       # gtest 테스트 프로젝트
    ├── *Test.cpp                        # 계층별 단위/통합 테스트
    └── BoundaryTests/LayerBoundaryTest.cpp  # 계층 경계(콘솔 I/O, 파일 I/O 위치) 자동 검증
```

핵심 정책 결정의 배경과 대안 비교는 [`docs/PRD.md`](docs/PRD.md), 개발
단계별 설계 근거는 [`docs/Plan.md`](docs/Plan.md)와 `docs/phase*.md`에
남아 있다.

## 3. 빌드 방법

- 요구 사항: Visual Studio(v145 툴셋), C++20, NuGet 패키지 복원
  (`nlohmann.json` 3.11.3, `Microsoft.googletest.v140.windesktop.msvcstl.dyn.rt-dyn` 1.8.1.8)
- `Project1/Project1.slnx`를 Visual Studio로 열거나, MSBuild로 빌드한다.

```
MSBuild.exe Project1/Project1.slnx -t:Rebuild -p:Configuration=Debug -p:Platform=x64
```

빌드가 끝나면 `Project1/x64/Debug/Project1.exe`(실행 파일)와
`Project1/x64/Debug/Project1Tests.exe`(테스트)가 생성된다.

## 4. 테스트 방법

```
Project1/x64/Debug/Project1Tests.exe
```

Model/Repository/Controller/View 전 계층에 대한 단위 테스트와, 계층
경계(콘솔 I/O가 View 밖에 없는지, 파일 I/O가 Repository 밖에 없는지)를
자동 검증하는 `LayerBoundaryTest`가 포함되어 있다. 전체 테스트가 통과해야
정상 상태다.

## 5. 사용 가이드

### 5-1. 실행

```
Project1/x64/Debug/Project1.exe
```

실행 위치 기준 상대경로 `data/samples.json`, `data/orders.json`에
데이터를 저장/로드한다. 두 파일이 모두 비어 있는 상태(최초 실행)라면
시료 5개·주문 8개짜리 데모 데이터를 자동으로 채운 뒤 시작한다.

### 5-2. 메인 메뉴

실행하면 매번 요약 정보(등록 시료 수/총 재고/전체 주문 수/생산라인
대기 수)를 먼저 보여준 뒤 메인 메뉴가 나온다.

| 번호 | 메뉴 | 설명 |
|---|---|---|
| 1 | 시료 관리 | 등록 / 목록 조회 / 이름 검색 (하위 메뉴, `0`으로 이전 메뉴 복귀) |
| 2 | 시료 주문 | 시료 ID/고객명/수량을 입력해 주문 접수(RESERVED 생성) |
| 3 | 주문 승인/거절 | 접수 대기(RESERVED) 목록 표시 후 승인/거절 선택 → 주문 ID 입력 |
| 4 | 모니터링 | 상태별 주문 건수(RESERVED/CONFIRMED/PRODUCING/RELEASE), 시료별 재고 현황(여유/부족/고갈) |
| 5 | 생산라인 조회 | 진입 시 자동 완료 판정 수행 후, 현재 생산 큐(FIFO) 표시 |
| 6 | 출고 처리 | 출고대기(CONFIRMED) 목록 표시 후 주문 ID 입력 → 출고 완료 |
| 0 | 종료 | 프로그램 종료 |

거절(REJECTED)된 주문은 정상 흐름 밖으로 취급되어 모니터링 집계에서
제외된다.

### 5-3. 데이터 초기화

데모 데이터를 다시 채우고 싶다면 `data/` 폴더를 통째로 지우고 다시
실행하면 된다(두 파일이 모두 비어 있을 때만 자동 시딩된다).
