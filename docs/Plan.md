# Main 통합 개발 계획

이 문서는 개발 과정에서 `MVC`/`Json`/`Monitor`/`Dummy` 4개 PoC에서 검증한 설계를
사람이 참고해 재구현하는 방식으로, `Main`의 반도체 시료 생산주문관리 콘솔
시스템을 phase별로 통합 개발하기 위한 전체 계획이다. 도메인 모델과 이
디렉토리의 목표/메뉴 명세는 `../CLAUDE.md`(이 저장소 자체의 `CLAUDE.md`)를
따른다.

**원칙**: 각 PoC의 코드(.cpp/.h)는 그대로 복사하지 않는다. PoC의 README/
`docs/Plan.md`/`docs/phase*.md`에서 확정된 설계(스키마, API 시그니처, 정책,
클래스 책임)만 참고하여 Main의 요구사항에 맞게 사람이 다시 구현한다. TDD
(Red → Green → Refactor)로 진행하며, 기능 단위로 작게 커밋한다.

## Phase 구성 및 순서

Model → Repository → Controller → View 순으로 아래로 갈수록 상위 계층에
의존한다. 각 phase는 이전 phase가 초록불(테스트 전부 통과)인 상태에서만 시작한다.

| Phase | 이름 | 의존 | 세부 문서 |
|---|---|---|---|
| 0 | 테스트 하네스 구성 | 없음 | [phase0-harness.md](phase0-harness.md) |
| 1 | Model 계층 | Phase 0 | [phase1-model.md](phase1-model.md) |
| 2 | Repository(영속성) 계층 | Phase 1 | [phase2-repository.md](phase2-repository.md) |
| 3 | Controller 계층 | Phase 2 | [phase3-controller.md](phase3-controller.md) |
| 4 | View 계층 + E2E 연결 | Phase 3 | [phase4-view.md](phase4-view.md) |
| 5 | Dummy 데이터 통합 & 최종 검증 | Phase 4 | [phase5-integration.md](phase5-integration.md) |

### Phase 0 — 테스트 하네스 구성
- 목표: gtest(NuGet)를 Main의 실행 파일 프로젝트와 공유하는 테스트 프로젝트로 구성.
- 참고: `MVC/`의 `Project1Tests` vcxproj 구성 방식(실행 파일 소스를
  `AdditionalIncludeDirectories`/`ClCompile` 상대경로로 공유), `Json/`·`Dummy/`의
  `nlohmann.json` NuGet 패키지 구성.
- DoD: 더미 sanity 테스트 1개가 통과한다.

### Phase 1 — Model 계층
- 목표: `Sample`/`Order` 엔티티, `OrderStatus` enum, 생산 큐 계산 순수 함수
  (부족분/실생산량/총생산시간), 시간 소스 추상화(clock 주입).
- 참고:
  - `MVC/`의 `Sample` 값 객체 설계(콘솔 I/O 없는 순수 데이터 클래스), 계층
    분리 원칙.
  - `Json/`의 `Order`/`OrderStatus` 필드 구성과 `ParseOrderStatus`/`ToString`
    변환 방식(문자열 5종 검증).
  - `Monitor/`의 `JudgeStockStatus` 경계값 정책(재고 0 → 고갈, 재고==수요 →
    여유)을 Model 계층의 순수 함수로 재구현.
  - `Dummy/`의 도메인 제약(수율 0 초과 1 이하, 생산시간 양수, 수량 1 이상)을
    엔티티 불변식으로 반영.
- DoD: 생산 큐 계산(부족분/실생산량 ceil/총생산시간) 및 재고 판정 함수가
  경계값까지 테스트로 고정된다.

### Phase 2 — Repository(영속성) 계층
- 목표: `SampleRepository`/`OrderRepository`를 JSON 파일 기반으로 재구현.
- 참고: `Json/`이 확정한 스키마(camelCase, `{"samples":[...]}`/`{"orders":[...]}`
  래핑, 파일 경로 주입)와 CRUD API 시그니처(`GetAll`/`FindById`/`Add`/`Update`/
  `Delete`), 손상 데이터 정책 전체.
- **정책 결정(아래 "PoC 간 정책 차이" 절 참고)**: Main은 `Json/`의 **fail-soft**
  정책을 채택한다.
- DoD: 파일 없음/빈 파일/JSON 문법 오류/스키마 불일치/개별 원소 손상 각각에
  대한 정책이 테스트로 고정되고, 재시작(새 Repository 인스턴스) 후에도 데이터가
  유지됨이 검증된다.
- **재시작 안전성(PRD.md 2-9)**: `Add`/`Update`/`Delete`는 항상 호출 즉시
  파일에 저장한다(지연/배치 저장 금지). 프로그램이 정상 종료 절차 없이
  강제 종료되어도, 그 시점까지 처리된 변경 사항은 파일에 반영되어 있어야
  한다.

### Phase 3 — Controller 계층
- 목표: 6개 메뉴(시료관리/시료주문/승인·거절/모니터링/생산라인/출고)를
  FakeView 기반으로 TDD.
- 참고:
  - `MVC/`의 `ISampleView`/`FakeSampleView`/`SampleController` 의존성 역전
    패턴(Controller는 추상 View에만 의존, 콘솔 I/O 없음).
  - `Json/`의 Repository API를 Controller가 어떻게 호출하는지(Add 시
    invalid_argument, Update/Delete 시 bool 반환 처리).
  - `Monitor/`의 `CountOrdersByStatus`/`BuildStockLevels` 조립 로직(모니터링
    메뉴에 재구현).
  - 세부 서브페이즈(3-1 시료관리, 3-2 시료주문, 3-3 승인/거절+생산큐 등록,
    3-4 생산라인 FIFO, 3-5 모니터링, 3-6 출고)로 나눠 진행.
- DoD: 6개 메뉴 각각의 정상/예외 분기가 FakeView로 전부 테스트된다.

### Phase 4 — View 계층 + E2E 연결
- 목표: 실제 콘솔 View 구현 + `main()` 조립 + 메인 메뉴(요약 정보 포함) 동작.
- 참고: `MVC/`의 `ConsoleSampleView`(istream/ostream 주입) 및 `main.cpp`
  조립 방식, `Monitor/`의 `ConsoleRenderer`(순수 문자열 렌더링 후 출력 분리) 패턴.
- DoD: 6개 메뉴가 실제 콘솔 입출력으로 End-to-End 동작.

### Phase 5 — Dummy 데이터 통합 & 최종 검증
- 목표: 초기 데모 데이터로 전체 시나리오(주문 접수 → 승인/거절 → 생산 → 출고)
  수동 검증, 계층 경계 자동 테스트, 리팩터링.
- 참고: `Dummy/`의 생성 로직(순수 함수 설계, ID 채번 전략, append 정책) 재구현,
  `MVC/`의 `LayerBoundaryTest` 패턴(콘솔 I/O가 Model/Controller에 없는지 자동 검증).
- **재시작 안전성(PRD.md 2-9)**: 생산 큐처럼 프로세스 메모리에만 있는 상태도
  포함해, 강제 종료 후 재시작 시 데이터가 유지되고 정상 흐름이 막히지
  않는지 시나리오 7 수동 검증에 포함한다.
- DoD: 데모 시나리오 수동 검증 완료, 계층 경계 테스트 통과, Clean Code 리팩터링 완료.

## PoC 간 확인된 정책 차이와 Main의 결정

| 항목 | Json (fail-soft) | Monitor/Dummy (fail-hard 경향) |
|---|---|---|
| 파일 없음 | 빈 벡터 반환 | Monitor: `std::runtime_error` |
| JSON 문법 오류 | 빈 벡터 반환, 원본 보존(덮어쓰지 않음) | Monitor: 상태 문자열 오류 시 `runtime_error`; Dummy: `DummyDataIoException` |
| 개별 원소 손상 | 해당 원소만 skip, 경고 출력, 나머지는 정상 반영 | (Monitor/Dummy는 이 세분화된 정책 없음, 전체 실패로 처리) |

**최종 결정: Main은 `Json/`의 fail-soft 정책을 채택한다.**

근거:
1. Main은 담당자가 반복 사용하는 운영 콘솔 앱이다. 파일 일부가 손상되었다고
   앱 전체가 예외로 죽으면(fail-hard), 담당자가 복구할 수 없는 상태에서 전체
   업무가 멈춘다. 나머지 정상 데이터만이라도 보여주고 계속 동작하는 편이
   실무적으로 안전하다.
2. `Monitor`는 읽기 전용 보조 도구, `Dummy`는 개발/테스트 지원 CLI로, 실패를
   드러내 개발자가 즉시 인지하게 하는 것이 더 유용한 맥락이었다. Main은
   최종 사용자가 쓰는 시스템이라 맥락이 다르다.
3. `Json/`의 정책(손상 파일 미덮어쓰기 + skip 로깅)은 이미 gtest로 고정되어
   재구현 리스크가 낮다.

다만 Repository의 `Add`(중복 ID)는 `Json/`과 동일하게 예외(`std::invalid_argument`)로
유지한다 — 이는 손상 데이터 처리가 아니라 명백한 프로그래밍 오류/사용자 입력
오류이므로 fail-soft 정책과 무관하게 즉시 드러내는 것이 맞다.

## 재고/생산/채번 정책 결정 (PRD.md 참고)

4개 PoC 어디에도 없던 신규 정책(재고 차감 시점, 승인 판정 기준, 생산완료
트리거 방식, `OrderId` 채번 전략 등)은 `docs/PRD.md`의 "정책 결정 현황" 절에서
논의·확정했다. 핵심만 요약하면:

- 물리적 `Stock`은 생산완료 시 `+= 실생산량`(기존 규칙 유지), 출고 시
  `-= 주문수량`(신규 결정)로만 증감한다. 승인(재고충분 → `CONFIRMED`)
  시점에는 `Stock`을 바꾸지 않고 "예약(미출고 `CONFIRMED` 수량 합)"으로만
  선점 처리한다.
- 승인 시 재고 충분/부족 판정은 `가용재고 = Stock − 미출고 CONFIRMED 수량 합`
  기준(물리적 `Stock` 직접 비교 아님).
- 생산완료는 `IClock` 기준 경과시간 자동 판정(수동 트리거 없음).
- 시료 등록 시 `Stock` 초기값은 항상 0, 수율은 등록 시 고정값(갱신 없음).
- `OrderId`는 일자별 순번 리셋(`YYYYMMDD` + 그 날짜 내 최댓값+1).

세부 근거와 논의 과정은 `docs/PRD.md`를 따른다.

## 참고 문서 링크

- `../CLAUDE.md` — Main의 목표/메뉴 명세/평가 기준/도메인 모델(이 저장소 자체 문서)
- `PRD.md` — 재고/생산/채번 등 4개 PoC 어디에도 없던 신규 정책의 결정 근거
- `phase0-harness.md` ~ `phase5-integration.md` — phase별 세부 테스트 목록과
  설계 결정 사항

이 계획을 세우던 개발 초기(PoC 단계)에는 `MVC`/`Json`/`Monitor`/`Dummy` 4개
PoC 저장소의 README/`docs/Plan.md`를 사람이 직접 읽고 설계(계층 분리, JSON
스키마, 집계/판정 로직, 더미 생성/ID 채번 로직)를 참고했다. 그 결과 확정된
내용은 이미 위 문서들(`../CLAUDE.md`, `PRD.md`, 각 phase 문서)에 반영되어
있으므로, Main 저장소만으로 개발 이력과 결정 사항을 모두 확인할 수 있다.
