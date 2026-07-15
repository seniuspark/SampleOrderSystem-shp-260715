# SampleOrderSystem-shp-260715

반도체 시료 생산주문관리 시스템의 **최종 통합 시스템**. `MVC`/`Json`/`Monitor`/
`Dummy` 4개 PoC에서 검증된 설계(계층 분리, JSON 영속성, 모니터링 집계, 더미
데이터 생성)를 사람이 다시 구현하는 방식으로 통합한다. 전체 배경/도메인 모델은
상위 저장소의 `../CLAUDE.md`, 이 저장소의 목표/메뉴 명세는 [`CLAUDE.md`](CLAUDE.md),
phase별 진행 계획은 [`docs/Plan.md`](docs/Plan.md)를 참고한다.

## 디렉토리 구조

```
Main/
├── CLAUDE.md                # 목표/메뉴 명세/Clean Code 원칙
├── docs/
│   ├── Plan.md               # Phase 0~5 전체 계획
│   ├── PRD.md                # 정책 결정 문서(재고/생산/채번 등, 아래 요약 참고)
│   ├── phase0-harness.md ~ phase5-integration.md
└── Project1/                 # Visual Studio 솔루션 스캐폴딩(Phase 0 착수 전)
    ├── Project1.slnx
    └── Project1/Project1.vcxproj
```

## 현재 진행 상태

`Project1.slnx`/`Project1.vcxproj` 스캐폴딩만 존재하며, 아직 Phase 0(테스트
하네스 구성)을 시작하기 전 단계다. 구현에 들어가기 전, 4개 PoC 어디에도 없던
정책(재고 차감 시점, 승인 판정 기준, 생산완료 트리거, ID 채번 등)을 먼저
논의해 `docs/PRD.md`에 결정 사항으로 고정했다. 아래는 그 요약이다.

## 확정된 핵심 정책 (`docs/PRD.md` 상세 근거 참고)

| 항목 | 결정 |
|---|---|
| 물리적 `Stock` 증감 시점 | 증가는 **생산완료** 시(`+= 실생산량`, 상위 CLAUDE.md 원 규칙 유지), 감소는 **출고**(`RELEASE`) 시(`-= 주문수량`, 이번에 신규 결정) |
| 승인(`CONFIRMED`) 시점 재고 처리 | 물리적 `Stock`은 변경하지 않고, 내부적으로 "예약(선점)"만 처리해 다른 주문의 중복 확보를 막음 |
| 승인 시 재고 충분/부족 판정 기준 | **가용재고 = `Stock` − 미출고 `CONFIRMED` 주문 수량 합** (물리적 `Stock` 직접 비교 아님) |
| 모니터링 재고 판정 `demand` 정의 | `RESERVED + CONFIRMED + PRODUCING` 수량 합 (기존 Monitor PoC 정책 그대로 유지) |
| 생산 초과분(실생산량 − 부족분) 처리 | 생산완료 즉시 가용재고에 자동 반영(별도 특례 로직 불필요 — 위 가용재고 공식만으로 성립) |
| 생산완료(`PRODUCING -> CONFIRMED`) 트리거 | 수동 메뉴 액션 없음. **시간 경과 기반 자동 완료**(생산라인 메뉴 진입 시마다 `IClock` 기준으로 판정) |
| 시료 등록 시 재고(`Stock`) 초기값 | 항상 **0** (등록 입력 폼에 재고 필드 없음) |
| 생산 큐 아이템 단위 | 주문(Order) **1:1** (여러 주문을 하나의 생산 배치로 병합하지 않음) |
| 수율(Yield) 갱신 여부 | 등록 시 입력받은 값을 **고정값**으로 취급, 이후 실측 갱신 없음 |
| `OrderId` 채번 전략 | **일자별 순번 리셋** (`ORD-YYYYMMDD-####`, 같은 날짜 안에서 기존 최댓값 + 1, 자정 경과 시 `0001`부터 재시작) |
| Repository 손상 데이터 처리 | `Json/` PoC의 **fail-soft** 정책 채택(파일 없음/빈 파일/문법 오류/스키마 불일치 → 빈 값 또는 원소 skip, 예외 없음). 단 `Add` 중복 ID는 fail-hard(`std::invalid_argument`) 유지 |

각 정책의 논의 배경, 대안 비교, 파생 이슈(예: 초과분이 가용재고 공식만으로
자동 반영되는 이유)는 [`docs/PRD.md`](docs/PRD.md)에 상세히 기록되어 있다.

## 다음 단계

`docs/Plan.md`의 Phase 0(테스트 하네스 구성)부터 TDD(Red → Green → Refactor)로
착수한다. Phase별 세부 테스트 목록은 `docs/phase0-harness.md` ~
`docs/phase5-integration.md`를 참고한다.
