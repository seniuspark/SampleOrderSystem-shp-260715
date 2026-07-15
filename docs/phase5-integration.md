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

## 다음 phase와의 연결점

이 phase가 Main 통합 개발의 마지막 단계다. 이후 필요 시 버그 수정/기능
추가는 이 문서의 시나리오 7을 리그레션 체크리스트로 재사용한다.
