# Phase 1 — Model 계층

## 목표

`Sample`/`Order` 엔티티, `OrderStatus` enum, 생산 큐 계산 순수 함수(부족분/
실생산량/총생산시간), 재고 판정 순수 함수, 시간 소스 추상화(주입 가능한 clock)를
구현한다. 이 계층은 콘솔 I/O나 파일 I/O를 전혀 갖지 않는 순수 도메인 로직이다.

`docs/PRD.md`에서 확정한 재고 차감 시점/승인 판정 기준/생산완료 트리거/
`OrderId` 채번 정책에 따라, 가용재고 계산·생산완료 자동 판정·`OrderId`
채번도 이 phase의 순수 함수로 포함한다(아래 "가용재고 계산", "생산완료
자동 판정", "OrderId 채번" 절 참고).

## 참고하는 PoC 설계

- `MVC/Project1/Project1/Model/Sample.h`: 시료 엔티티를 콘솔 I/O 없는 순수
  데이터 클래스로 두는 설계(`SampleId`/`Name`/`AvgProductionTime`/`Yield`/`Stock`).
- `Json/Project1/Project1/Sample.h`, `Order.h`: 필드 구성과 타입(`avgProductionTime`
  double, `yield` double, `stock` int, `quantity` int), `OrderStatus`
  5종(`RESERVED`/`REJECTED`/`PRODUCING`/`CONFIRMED`/`RELEASE`)과
  `ToString`/`ParseOrderStatus` 변환 방식(알 수 없는 문자열은 예외).
- `Monitor/Project1/Project1/StockStatusJudge.h/.cpp`: 재고 판정 경계값 정책
  — `stock == 0` → DEPLETED(최우선), `stock > 0 && stock < demand` → SHORTAGE,
  `stock >= demand`(동률 포함, demand==0 포함) → SUFFICIENT. 음수 입력은 0으로
  clamp.
- `Dummy/Project1/Project1/DummySample.h`, `DummyOrder.h`: 도메인 제약(수율
  0 초과 1 이하, 평균생산시간 양수, 재고 0 이상, 주문수량 1 이상)을 엔티티
  생성 시점의 불변식으로 반영.
- 상위 `../../CLAUDE.md`: 부족분 = 주문수량 - 재고, 실생산량 = ceil(부족분/수율),
  총생산시간 = 평균생산시간 * 실생산량.

## 작성할 테스트 목록 (Red 단계)

### Sample
1. 유효한 값으로 `Sample` 생성 시 필드가 그대로 보관된다.
2. 수율이 0 이하이거나 1 초과면 생성이 거부된다(또는 불변식 검증 함수가 false 반환).
3. 평균생산시간이 0 이하면 생성이 거부된다.
4. 재고가 음수면 생성이 거부된다.

### Order / OrderStatus
5. `OrderStatus` 5종 문자열 ↔ enum 변환이 왕복(round-trip)한다.
6. 알 수 없는 상태 문자열 파싱 시 예외가 발생한다.
7. 주문 수량이 1 미만이면 생성이 거부된다.
8. 생성 직후 주문 상태는 항상 `RESERVED`다.

### 생산 큐 계산 순수 함수
9. 재고가 주문 수량 이상이면 부족분은 0이다.
10. 부족분이 수율로 나누어 떨어지지 않을 때 실생산량이 올림(ceil) 처리된다
    (예: 부족분 10, 수율 0.9 → ceil(10/0.9)=12).
11. 부족분이 0이면 실생산량도 0이고 총생산시간도 0이다.
12. 총생산시간 = 평균생산시간 * 실생산량이 정확히 계산된다.
13. 수율이 1.0(경계값)일 때 실생산량 = 부족분과 같다.

### 재고 판정 순수 함수
14. `stock == 0`이면 demand와 무관하게 DEPLETED.
15. `stock > 0 && stock < demand`면 SHORTAGE.
16. `stock == demand`(둘 다 양수)면 SUFFICIENT(부족 아님).
17. `demand == 0`이면 SUFFICIENT.
18. 음수 stock/demand 입력은 0으로 clamp되어 처리된다.

### 시간 소스 추상화
19. 주입한 가짜 clock이 반환하는 고정 시각이 그대로 사용된다(생성 시각 등에
    실제 `system_clock::now()`가 직접 호출되지 않음을 검증).

### 가용재고 계산 순수 함수 (`docs/PRD.md` 2-1~2-3 결정 반영)
20. `가용재고 = Stock - 미출고 CONFIRMED 수량 합`이 정확히 계산된다.
21. 미출고 `CONFIRMED` 수량 합이 0이면 가용재고는 `Stock`과 같다.
22. 미출고 `CONFIRMED` 수량 합이 `Stock`보다 크면(이론상 발생하면 안 되지만)
    가용재고는 0으로 clamp된다(음수 가용재고 방지).
23. 생산완료로 `Stock`이 실생산량만큼 늘어난 직후, 초과분(실생산량 − 그
    주문의 수량)이 가용재고 증가분으로 그대로 나타난다(별도 특례 로직 없이
    위 공식만으로 성립함을 검증).

### 생산완료 자동 판정 순수 함수 (`docs/PRD.md` 2-4 결정 반영)
24. 큐 선두 항목의 생산 시작 시각 + 총생산시간이 현재 시각(주입된 clock) 이하면
    "완료됨"으로 판정된다.
25. 현재 시각이 완료 예정 시각 이전이면 "완료 안 됨"으로 판정된다.
26. 완료 예정 시각과 현재 시각이 정확히 같으면 "완료됨"으로 판정된다(경계값).

### OrderId 채번 순수 함수 (`docs/PRD.md` 2-7 결정 반영)
27. 기존 주문이 없을 때 오늘 날짜로 `ORD-YYYYMMDD-0001`이 채번된다.
28. 같은 날짜에 기존 최댓값 순번이 있으면 그 값 + 1이 채번된다.
29. 채번 대상 날짜가 바뀌면(자정 경과) 순번이 `0001`부터 다시 시작한다.
30. 기존 `OrderId` 중 형식이 맞지 않아 파싱에 실패하는 항목은 채번 계산에서
    무시된다.

## 구현할 클래스/함수 목록

- `Project1/Project1/Model/Sample.h` — `Sample` 엔티티
- `Project1/Project1/Model/Order.h` — `Order` 엔티티, `OrderStatus` enum,
  `ToString`/`ParseOrderStatus`
- `Project1/Project1/Model/ProductionQueueCalculator.h/.cpp` — 부족분/실생산량/
  총생산시간 순수 함수
- `Project1/Project1/Model/StockStatus.h/.cpp` — 재고 판정 enum + 순수 함수
- `Project1/Project1/Model/AvailableStockCalculator.h/.cpp` — 가용재고 계산
  순수 함수(`Stock`, 미출고 `CONFIRMED` 수량 합을 인자로 받음 — Order 목록
  조회/필터링은 Controller 책임, Model은 합계값만 받아 계산)
- `Project1/Project1/Model/ProductionCompletionJudge.h/.cpp` — 생산완료
  자동 판정 순수 함수(생산 시작 시각, 총생산시간, 현재 시각을 인자로 받음)
- `Project1/Project1/Model/OrderIdAllocator.h/.cpp` — 일자별 순번 리셋
  `OrderId` 채번 순수 함수(오늘 날짜, 기존 `OrderId` 목록을 인자로 받음)
- `Project1/Project1/Model/IClock.h` — 시간 소스 추상 인터페이스, `SystemClock`
  구현체, 테스트용 `FixedClock`(테스트 프로젝트 쪽에 위치)

## 완료 기준 (DoD)

- 위 30개 테스트가 모두 통과한다.
- Model 계층 소스에 `std::cin`/`std::cout`/파일 I/O 호출이 없다(추후 Phase 5의
  계층 경계 테스트로 재확인 예정이지만, 이 phase에서도 코드 리뷰로 확인).

## 다음 phase와의 연결점

Phase 2(Repository)는 이 phase의 `Sample`/`Order`/`OrderStatus`를 JSON으로
직렬화/역직렬화하는 대상으로 그대로 사용한다. `AvailableStockCalculator`/
`ProductionCompletionJudge`/`OrderIdAllocator`가 필요로 하는 집계값(미출고
`CONFIRMED` 수량 합, 기존 `OrderId` 목록 등)은 Phase 3(Controller)에서
Repository 조회 결과로부터 계산해 전달한다.
