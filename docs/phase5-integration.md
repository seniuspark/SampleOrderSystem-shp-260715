# Phase 5 — Dummy 데이터 통합 & 최종 검증

## 목표

`Dummy/`에서 검증한 더미 데이터 생성 로직을 재구현해 초기 데모 데이터를
채우고, 전체 시나리오(주문 접수 → 승인/거절 → 생산 → 출고)를 수동으로
End-to-End 검증한다. 아울러 계층 경계를 자동 테스트로 고정하고 Clean Code
관점의 최종 리팩터링을 수행한다.

## 참고하는 PoC 설계

- `Dummy/Project1/Project1/SampleGenerator.h/.cpp`,
  `OrderGenerator.h/.cpp`: 시드가 주입된 `std::mt19937`만으로 도메인 제약을
  만족하는 값을 만드는 순수 함수 설계(수율 0 초과 1 이하, 생산시간 양수,
  SampleId 참조 무결성, 상태별 `createdAt` 그럴듯한 값).
- `Dummy/Project1/Project1/IdAllocator.h/.cpp`: `S-###`/`ORD-YYYYMMDD-####`
  형식의 기존 최댓값+1 채번 전략(파싱 실패 항목은 무시).
- `Dummy/Project1/Project1/DummyDataAppender.h/.cpp`: 생성+채번+Repository
  반영을 조합하는 통합 로직, `clearExisting` 옵션.
- `MVC/Project1/Project1Tests/BoundaryTests/LayerBoundaryTest.cpp`: 소스
  코드에 특정 패턴(콘솔 I/O 호출, 재고/수율 비교 로직 등)이 없는지 문자열
  검사로 자동 고정하는 계층 경계 테스트 패턴.

## 작성할 테스트 목록 (Red 단계)

### 더미 데이터 생성/시딩
1. 시드 기반 시료 생성이 항상 도메인 제약(수율/생산시간/재고)을 만족한다
   (여러 시드에 대한 property-based 스타일 반복 검증).
2. 시드 기반 주문 생성이 항상 존재하는 SampleId를 참조한다.
3. 초기 데모 데이터 시딩 실행 후 Repository에 지정한 개수만큼 Sample/Order가
   추가된다(기존 데이터 보존, append).

### 계층 경계
4. Model/Controller 소스 파일에 `std::cin`/`std::cout`/`printf` 등 콘솔 I/O
   호출 문자열이 없다.
5. View 소스 파일에 재고/수율 비교, 생산 큐 계산 등 도메인 판단 로직 문자열
   패턴이 없다.
6. Repository 소스 파일 외 다른 계층에 직접적인 파일 I/O(`std::ofstream` 등)
   호출이 없다.

### 전체 시나리오 (수동 검증, 기록)
7. 초기 데모 데이터로 앱을 실행해, 다음 시나리오가 문서에 기록된 절차대로
   기대한 결과를 낸다(`docs/PRD.md` 정책 반영): 시료 등록 확인(재고 0으로
   시작) → 신규 주문 접수(OrderId 일자별 채번 확인) → 가용재고 충분 케이스
   승인(즉시 CONFIRMED, 물리적 Stock은 불변) → 가용재고 부족 케이스 승인
   (PRODUCING + 생산 큐 등록) → 생산라인 메뉴 재진입으로 시간 경과 자동
   완료 확인(PRODUCING→CONFIRMED, 물리적 Stock에 실생산량 반영) → 모니터링에서
   상태별 집계/재고 판정 확인 → 출고 처리(CONFIRMED→RELEASE, 이 시점에만
   물리적 Stock 차감) → 거절 케이스(REJECTED가 모니터링에서 제외됨) 확인.

## 구현할 클래스/함수 목록

- `Project1/Project1/Seed/DemoDataSeeder.h/.cpp` — 초기 데모 데이터 생성/반영
  (Dummy의 생성기/채번 로직을 재구현)
- `Project1Tests/BoundaryTests/LayerBoundaryTest.cpp` — 계층 경계 자동 검증

## 완료 기준 (DoD)

- 위 자동 테스트(1~6)가 모두 통과한다.
- 시나리오 7의 각 단계가 수동 실행으로 확인되고 결과가 이 문서 또는
  `Main/docs/Plan.md`에 기록된다.
- Clean Code 원칙(`../CLAUDE.md`의 "Clean Code 원칙" 절)에 따른 최종
  리팩터링(매직 넘버 제거, 중복 제거, 이름 정리 등)이 완료된다.
- 빌드 경고/오류 0건, 전체 테스트 그린 상태로 Main 개발을 마무리한다.

## 시나리오 7 수동 검증 기록 (2026-07-15)

`Project1.exe`를 직접 실행해(빌드 산출물 `x64/Debug/Project1.exe`) 아래 절차를
확인했다. 데모 시드(시료 5개, 주문 8개, seed=20260416)가 깔린 상태에서
시작했다.

1. **시료 등록(재고 0으로 시작)**: `S-DEMO`를 새로 등록 → 목록 조회 결과
   `Stock=0`으로 시작함을 확인.
2. **신규 주문 접수(OrderId 일자별 채번)**: `S-DEMO`에 대해 주문 접수 →
   기존 8건(seed) 다음 순번인 `ORD-20260715-0009`가 채번됨을 확인.
3. **가용재고 충분 케이스 승인**: 여유 재고가 있던 시드 시료 `S-002`
   (Stock=26)에 새 주문(수량 3) 접수 후 승인 → 즉시 `CONFIRMED`로 전환되고,
   승인 전후 `S-002`의 물리적 Stock이 26으로 그대로 유지됨을 모니터링
   화면으로 확인(가용재고만 내부적으로 예약되고 물리적 차감은 없음).
4. **가용재고 부족 케이스 승인**: `S-DEMO`(Stock=0) 주문(수량 3) 승인 →
   `PRODUCING`으로 전환되고 생산 큐에 등록됨(생산라인 대기: 1)을 확인.
5. **생산라인 재진입 자동 완료 + 재시작 지속성**: 생산 큐는 프로세스
   메모리에만 있었기 때문에, 검증 도중 앱을 재시작하면 대기 중이던 항목이
   사라지고 해당 주문이 `PRODUCING`에 영원히 멈추는 **실제 버그를 발견**했다
   (재시작해도 데이터가 유지되어야 한다는 요구사항 위반). 원인은 생산 큐
   메타데이터(부족분/실생산량/총생산시간/시작시각)가 어디에도 영속화되지
   않았기 때문. `Order`에 `ProductionShortage`/`ProductionActualQuantity`/
   `ProductionTotalTimeMinutes`/`ProductionStartedAt`(모두 `std::optional`)를
   추가해 `PRODUCING` 전환 시점에 함께 저장하고, `ProductionLineController`
   생성자에서 Repository의 PRODUCING 주문들을 시작시각(FIFO) 순으로 큐에
   복원하도록 수정했다(`RebuildQueueFromPersistedOrders`). 수정 후 재검증한
   결과: 프로세스를 재시작해도 생산라인 대기 수가 정확히 유지되고, 총생산
   시간이 지난 뒤 재진입하면 `PRODUCING → CONFIRMED` 전환과 `Stock` 반영
   (0 → 3, 실생산량만큼)이 재시작 여부와 무관하게 정상 동작함을 확인.
6. **모니터링 확인**: 상태별 건수(RESERVED/CONFIRMED/PRODUCING/RELEASE)와
   시료별 재고 판정(여유/부족/고갈)이 각 단계 전환에 맞춰 갱신됨을 확인.
7. **출고 처리**: 3번에서 `CONFIRMED`된 `S-002` 주문을 출고 → `RELEASE`로
   전환되고, 이 시점에만 물리적 Stock이 26 → 23(주문수량 3 차감)으로
   줄어듦을 확인(승인 시점에는 차감되지 않았던 것과 대비).
8. **거절 케이스**: 별도 주문을 접수 후 거절 → `REJECTED`로 전환되고,
   모니터링의 상태별 건수 집계에서 제외됨(RESERVED 건수만 감소)을 확인.

결과: 시나리오 7의 모든 단계가 기대한 대로 동작하며, 검증 과정에서 발견된
생산 큐 재시작 지속성 버그는 수정 완료했다(기존 130개 테스트 전부 회귀 없이
Green 유지, 신규 자동화 테스트는 추가하지 않고 수동 시나리오로 확인 —
필요 시 이 기록을 리그레션 체크리스트로 재사용한다).

## 다음 phase와의 연결점

이 phase가 Main 통합 개발의 마지막 단계다. 이후 필요 시 버그 수정/기능
추가는 이 문서의 시나리오 7을 리그레션 체크리스트로 재사용한다.
