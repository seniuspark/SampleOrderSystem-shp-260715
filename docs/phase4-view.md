# Phase 4 — View 계층 + E2E 연결

## 목표

Phase 3에서 정의한 View 인터페이스의 실제 콘솔 구현체를 작성하고, `main.cpp`에서
Repository + View + Controller를 조립해 메인 메뉴(전체 요약 정보 포함)부터
6개 메뉴 전체가 실제 콘솔 입출력으로 End-to-End 동작하게 한다.

## 참고하는 PoC 설계

- `MVC/Project1/Project1/View/ConsoleSampleView.h/.cpp`: `std::istream&`/
  `std::ostream&`를 생성자로 주입받는 방식(기본값 `std::cin`/`std::cout`)으로
  테스트 가능성을 확보하는 패턴.
- `MVC/Project1/Project1/main.cpp`: Repository/View/Controller를 조립하고
  `controller.Run()`을 호출하는 최소 진입점 구조.
- `Monitor/Project1/Project1/ConsoleRenderer.h/.cpp`: `std::cout`을 직접
  호출하지 않는 순수 문자열 렌더링 함수와, 그 결과를 출력만 하는 `Main.cpp`의
  역할 분리(렌더링 로직과 콘솔 I/O 분리).
- `../CLAUDE.md`의 메인 메뉴 명세: 메인 메뉴에 전체 요약 정보(등록 시료 수,
  총 재고, 전체 주문 수, 생산라인 대기 등)를 함께 보여줘야 한다는 요구사항.
  PDF 예시 UI는 참고용일 뿐 화면 구성은 자유.

## 작성할 테스트 목록 (Red 단계)

1. `ConsoleView` 구현체가 istream/ostream을 주입받아 메뉴 선택 입력을 올바르게
   파싱한다(잘못된 입력 시 재입력 유도 또는 오류 처리 방식을 테스트로 고정).
2. 시료 등록 입력 폼(ID/이름/생산시간/수율)이 올바르게 파싱되어 Controller에 전달된다.
3. 시료 목록/검색 결과가 사람이 읽을 수 있는 형식으로 출력된다(재고 포함).
4. 주문 접수 입력 폼(SampleId/고객명/수량)이 올바르게 파싱된다.
5. 승인/거절 대상 선택 입력이 올바르게 파싱되고, 결과 메시지가 출력된다.
6. 모니터링 화면에 상태별 건수와 시료별 재고 상태(여유/부족/고갈)가 함께 출력된다.
7. 생산라인 화면에 현재 생산 중 항목과 FIFO 대기열이 함께 출력된다.
8. 출고 처리 입력/결과가 올바르게 파싱/출력된다.
9. 메인 메뉴 진입 시 요약 정보(등록 시료 수/총 재고/전체 주문 수/생산 대기 수)가
   함께 표시된다.
10. `0` 입력 시 프로그램이 정상 종료된다.
11. (E2E) `main()`을 통해 실행 파일이 "시료 등록 → 주문 접수 → 승인(재고
    충분/부족 각 1건) → 생산 완료 → 출고" 시나리오를 실제 콘솔 입력으로
    끝까지 처리한다(자동화된 통합 테스트 또는 수동 스모크 테스트로 수행 —
    Phase 5에서 본격적으로 다룸).

## 구현할 클래스/함수 목록

- `Project1/Project1/View/ConsoleView.h/.cpp` (또는 메뉴별로 분리)
- `Project1/Project1/View/MenuOption.h` — 메뉴 번호 enum class(매직 넘버 제거)
- `Project1/Project1/main.cpp` — Repository/Controller/View 조립, 메인 루프
- `Project1/Project1/View/SummaryRenderer.h/.cpp` — 메인 메뉴 요약 정보 렌더링
  (순수 문자열 생성, `Monitor/ConsoleRenderer` 패턴 참고)

## 완료 기준 (DoD)

- 위 11개 테스트(가능한 범위에서 자동화, E2E는 수동 스모크 테스트 기록으로
  대체 가능)가 통과/확인된다.
- 실행 파일을 직접 실행해 6개 메뉴 모두가 콘솔에서 동작함을 확인한다.

## 다음 phase와의 연결점

Phase 5는 이 실행 파일에 `Dummy/`에서 검증한 로직으로 생성한 초기 데이터를
채워 전체 시나리오를 수동 검증하고, 계층 경계 자동 테스트와 최종 리팩터링을
수행한다.
