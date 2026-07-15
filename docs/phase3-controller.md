# Phase 3 — Controller 계층

## 목표

6개 메인 메뉴(시료관리/시료주문/승인·거절/모니터링/생산라인/출고)의 흐름을
Controller에서 구현하고, 콘솔 없이 FakeView를 주입해 TDD로 검증한다. Controller는
`ISampleView`/`IOrderView` 등 추상 View 인터페이스에만 의존하고 `std::cin`/
`std::cout`을 직접 호출하지 않는다.

## 참고하는 PoC 설계

- `MVC/Project1/Project1/Controller/SampleController.h/.cpp`,
  `View/ISampleView.h`, `Project1Tests/View/FakeSampleView.h`: Controller가
  View 인터페이스에만 의존하는 의존성 역전 패턴, `MenuOption` enum class로
  메뉴 번호를 매직 넘버 없이 표현하는 방식, FakeView가 입력을 큐에 채워두고
  마지막 출력을 검사하게 하는 테스트 더블 패턴.
- `Json/`의 Repository API(Phase 2에서 재구현) — Controller가 `Add`의
  `invalid_argument`, `Update`/`Delete`의 bool 반환을 어떻게 처리하는지.
- `Monitor/Project1/Project1/OrderAggregation.h/.cpp`,
  `StockStatusJudge.h/.cpp`, `MonitorAssembly.h/.cpp`: 모니터링 메뉴(4번)에서
  상태별 집계(`CountOrdersByStatus`류, REJECTED 제외)와 시료별 재고 판정
  (`BuildStockLevels`류, demand = RESERVED+CONFIRMED+PRODUCING 수량 합)를
  재구현.
- 상위 `../../CLAUDE.md`/`../CLAUDE.md`의 메뉴별 기능 명세와 상태 전이 규칙.
- `docs/PRD.md`의 정책 결정 현황(2-1~2-8) — 가용재고 계산, 승인 판정 기준,
  생산완료 자동 트리거, `OrderId` 채번, 재고 초기값 등. 이 phase의 3-2/3-3/3-4/
  3-6 테스트 목록은 해당 결정을 전제로 작성되었다.

서브페이즈 구성(권장, 필요 시 조정 가능):
- 3-1 시료 관리, 3-2 시료 주문, 3-3 승인/거절 + 생산 큐 등록,
  3-4 생산라인 처리(FIFO), 3-5 모니터링, 3-6 출고.

## 작성할 테스트 목록 (Red 단계) — 6개 메뉴의 모든 분기

### 3-1 시료 관리
1. 유효한 입력으로 시료 등록 성공 시 Repository에 반영되고 성공 메시지가 View로 전달된다.
2. 중복 SampleId로 등록 시 실패 메시지가 View로 전달되고 Repository는 변경되지 않는다.
3. 수율/생산시간 등 도메인 제약 위반 입력 시 등록이 거부되고 오류 메시지가 표시된다.
4. 시료 목록 조회 시 등록된 모든 시료 + 현재 재고가 View에 전달된다.
5. 시료가 하나도 없을 때 목록 조회 시 빈 목록 메시지가 표시된다.
6. 이름으로 검색 시 일치하는 시료만 반환된다.
7. 검색어에 해당하는 시료가 없을 때 "검색 결과 없음" 메시지가 표시된다.
7-1. 시료 등록 성공 시 `Stock`은 항상 0으로 초기화된다(등록 입력 폼에 재고
     필드 없음, `docs/PRD.md` 2-5 결정).

### 3-2 시료 주문
8. 존재하는 SampleId로 주문 접수 시 `RESERVED` 상태의 Order가 생성되어 저장된다.
9. 존재하지 않는 SampleId로 주문 시 거부되고 오류 메시지가 표시된다(Order 미생성).
10. 주문 수량이 1 미만이면 거부된다.
10-1. 주문 접수 시 `OrderId`는 Phase 1의 `OrderIdAllocator`로 일자별 순번
      채번되어 저장된다(`docs/PRD.md` 2-7 결정) — 같은 날짜에 기존 주문이
      있으면 최댓값+1, 날짜가 바뀌면 `0001`부터 재시작함을 Controller
      수준에서도 확인한다(Repository에서 조회한 기존 `OrderId` 목록을
      Allocator에 전달하는 조립이 올바른지 검증).

### 3-3 승인/거절 (`docs/PRD.md` 2-1~2-3 결정 반영)
11. `RESERVED` 목록이 View에 올바르게 전달된다.
12. 승인 시 **가용재고**(`Stock − 미출고 CONFIRMED 수량 합`, Phase 1의
    `AvailableStockCalculator`로 계산)가 주문 수량 이상이면 즉시 `CONFIRMED`로
    전환된다. 이때 물리적 `Stock`은 변경하지 않는다(이 주문은 "미출고
    CONFIRMED"로 집계되어 이후 다른 주문의 가용재고 계산에서 자동으로
    차감된 것처럼 반영됨).
12-1. 승인 직후 같은 시료에 대해 다른 주문을 승인 판정할 때, 방금 CONFIRMED된
      주문의 수량이 가용재고 계산에 반영되어 중복 확보(오버부킹)가 방지된다
      (동시 예약 문제 회귀 테스트).
13. 가용재고가 주문 수량보다 부족하면 `PRODUCING`으로 전환되고 생산 큐(FIFO)에
    등록된다(부족분/실생산량 계산은 가용재고 기준).
14. 승인 대상 주문이 `RESERVED`가 아니면(이미 처리됨) 거부되고 오류 메시지가 표시된다.
15. 거절 시 `REJECTED`로 전환되고, 이후 모니터링/생산라인/출고 어디에도 나타나지 않는다.
16. 존재하지 않는 OrderId로 승인/거절 시도 시 오류 메시지가 표시된다.

### 3-4 생산라인 (FIFO, `docs/PRD.md` 2-4 결정 반영)
17. 여러 건이 `PRODUCING`으로 등록된 순서대로 FIFO 큐에서 조회된다.
18. 생산 큐 조회 시 부족분/실생산량/총생산시간이 Phase 1의 순수 함수 결과와 일치한다.
19. **생산라인 메뉴 진입 시마다** 큐 선두 항목에 대해 Phase 1의
    `ProductionCompletionJudge`(현재 시각 vs 완료 예정 시각)로 자동 완료
    여부를 판정한다. 완료로 판정되면 해당 주문이 `PRODUCING -> CONFIRMED`로
    전환되고, 물리적 `Stock`에 실생산량이 `+=` 반영된다(수동 "생산 완료 처리"
    메뉴 액션은 두지 않는다).
19-1. 완료 예정 시각이 아직 지나지 않았으면 메뉴 진입 시 판정해도 상태가
      바뀌지 않는다.
19-2. 실생산량이 주문 수량보다 큰 경우(수율 < 1), 완료 처리 직후 가용재고가
      초과분만큼 늘어난다(Phase 1 #23 회귀 확인).
20. 생산 큐가 비어 있을 때 "대기 항목 없음" 메시지가 표시된다.
21. 자동 완료 판정은 큐의 맨 앞(FIFO 선두) 항목에만 적용된다(선두가 아직
    완료 전이면 뒤쪽 항목은 먼저 완료되지 않는다 — FIFO 순서 보장).

### 3-5 모니터링
22. 상태별(RESERVED/CONFIRMED/PRODUCING/RELEASE) 주문 건수가 정확히 집계되고
    `REJECTED`는 집계에서 제외된다.
23. 시료별 재고와 여유/부족/고갈 판정이 Phase 1의 판정 함수 결과와 일치한다.
24. 주문/시료가 전혀 없을 때 "데이터 없음" 형태로 표시된다.

### 3-6 출고 처리 (`docs/PRD.md` 2-1~2-3 결정 반영)
25. `CONFIRMED` 주문 선택 시 `RELEASE`로 전환되고, 물리적 `Stock`이 해당
    주문 수량만큼 `-=` 차감된다(Main에서 물리적 재고가 실제로 줄어드는
    유일한 시점).
26. `CONFIRMED`가 아닌 주문(예: `RESERVED`, `PRODUCING`)을 출고 시도하면 거부되고,
    `Stock`은 변경되지 않는다.
27. 존재하지 않는 OrderId로 출고 시도 시 오류 메시지가 표시된다.
28. `CONFIRMED` 주문이 하나도 없을 때 "출고 대상 없음" 메시지가 표시된다.

## 구현할 클래스/함수 목록

- `Project1/Project1/Controller/SampleController.h/.cpp` (3-1)
- `Project1/Project1/Controller/OrderController.h/.cpp` (3-2, 3-3, 3-6)
- `Project1/Project1/Controller/ProductionLineController.h/.cpp` (3-4, FIFO 큐 보관)
- `Project1/Project1/Controller/MonitoringController.h/.cpp` (3-5)
- `Project1/Project1/View/ISampleView.h`, `IOrderView.h`, `IMonitoringView.h`,
  `IProductionLineView.h` (또는 메뉴 통합 인터페이스 1개로 단순화 — Clean Code
  원칙에 따라 과도한 인터페이스 분리는 지양하고 실제 필요에 맞게 결정)
- 테스트 전용 `FakeView` 계열 (`Project1Tests/View/`)

## 완료 기준 (DoD)

- 위 28개 테스트(및 `docs/PRD.md` 정책 반영에 따라 추가된 7-1/10-1/12-1/19-1/19-2)가
  모두 통과한다.
- Controller 소스에 `std::cin`/`std::cout` 직접 호출이 없다.

## 다음 phase와의 연결점

Phase 4(View)는 이 Controller들이 의존하는 View 인터페이스의 실제 콘솔
구현체를 제공하고, `main()`에서 Repository + View + Controller를 조립한다.
