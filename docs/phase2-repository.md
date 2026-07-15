# Phase 2 — Repository(영속성) 계층

## 목표

`SampleRepository`/`OrderRepository`를 JSON 파일 기반으로 구현해, 앱을
재시작해도(새 프로세스로 다시 실행해도) 데이터가 유지되게 한다.

## 참고하는 PoC 설계

- `Json/Project1/Project1/SampleRepository.h`, `OrderRepository.h`: 다음
  시그니처를 동일한 형태로 재구현한다.
  ```cpp
  explicit SampleRepository(std::filesystem::path filePath);
  std::vector<Sample> GetAll() const;
  std::optional<Sample> FindById(const std::string& sampleId) const;
  void Add(const Sample& sample);            // 중복 ID -> std::invalid_argument
  bool Update(const Sample& sample);          // 성공 true / 대상 없음 false
  bool Delete(const std::string& sampleId);   // 성공 true / 대상 없음 false
  ```
- `Json/docs/Plan.md`가 확정한 JSON 스키마: camelCase 필드명,
  `{"samples": [...]}`/`{"orders": [...]}` 최상위 래핑, 파일 경로는
  생성자에 주입(하드코딩 금지).
- **손상 데이터 정책은 `Json/`의 fail-soft 정책을 채택한다** (`Main/docs/Plan.md`의
  "PoC 간 확인된 정책 차이" 절에서 결정 및 근거 기록):
  - 파일 없음 → 빈 벡터, 예외 없음
  - 빈 파일(0바이트) → 빈 벡터
  - JSON 문법 오류 → 빈 벡터 반환 + 원본 파일 보존(덮어쓰지 않음)
  - 최상위가 배열이거나 `samples`/`orders` 키 없음 → 빈 벡터
  - 개별 원소 파싱 실패 → 해당 원소만 skip(경고 출력), 나머지는 정상 반영
  - `Add` 중복 ID → `std::invalid_argument` (fail-hard 유지, 사용자 입력 오류이므로)
  - `Update`/`Delete` 대상 없음 → 예외 없이 `false`
- `Dummy/Project1/Project1/JsonIo.h`: `FormatIso8601`/`ParseIso8601` 변환
  방식(ISO 8601, 초 단위) 및 원자적 쓰기(임시 파일 후 rename) 아이디어 참고
  (다만 Json/의 정책이 우선 — Dummy의 `DummyDataIoException`류 fail-hard는
  채택하지 않음).

## 작성할 테스트 목록 (Red 단계)

### 빈 상태
1. 파일이 없을 때 `GetAll()`은 빈 벡터를 반환한다(예외 없음).
2. 파일이 0바이트일 때 `GetAll()`은 빈 벡터를 반환한다.

### Create/Read (같은 인스턴스)
3. `Add` 후 같은 인스턴스의 `GetAll()`에 반영된다.
4. `Add` 후 `FindById`로 조회된다.
5. `Add` 후 파일이 디스크에 생성된다.
6. 중복 ID로 `Add` 시 `std::invalid_argument`가 발생하고 파일이 변경되지 않는다.

### 영속성 재로딩 (핵심)
7. 저장 후 **새 Repository 인스턴스**로 같은 경로를 열었을 때 `GetAll()`이
   저장 전과 동일하다(Sample/Order 각각).
8. 여러 건 저장 후 재로딩해도 순서/내용이 보존된다.

### Update/Delete
9. `Update`로 필드 변경 후 재로딩하면 변경 내용이 유지된다.
10. 존재하지 않는 ID로 `Update` 시 `false` 반환, 파일 미변경.
11. `Delete` 후 재로딩하면 해당 항목이 사라진다.
12. 존재하지 않는 ID로 `Delete` 시 `false` 반환, 파일 미변경.

### 손상 데이터 정책
13. JSON 문법 오류 파일을 읽으면 빈 벡터 반환, 원본 파일이 변경되지 않는다.
14. 최상위가 배열이거나 `samples`/`orders` 키가 없으면 빈 벡터 반환.
15. 배열 안 개별 원소의 필수 필드 누락/타입 불일치 시 해당 원소만 skip되고
    나머지는 정상 반영된다.
16. `Order`의 `status` 문자열이 알려진 5종이 아니면 해당 원소만 skip된다.

## 구현할 클래스/함수 목록

- `Project1/Project1/Repository/SampleRepository.h/.cpp`
- `Project1/Project1/Repository/OrderRepository.h/.cpp`
- `Project1/Project1/Repository/JsonCodec.h/.cpp` (또는 각 Repository 내부) —
  `Sample`/`Order` ↔ `nlohmann::json` 변환, 개별 원소 skip 처리

## 완료 기준 (DoD)

- 위 16개 테스트가 모두 통과한다(Sample/Order 양쪽에 동일하게 적용, 실제
  테스트 파일 수는 두 배가 될 수 있음).
- `Json/`의 `PersistenceReloadTests.cpp`처럼 "새 인스턴스로 재로딩" 테스트가
  Sample/Order 각각에 존재한다.

## 가용재고/OrderId 채번 관련 Repository 설계 결정 (`docs/PRD.md` 참고)

- `docs/PRD.md`에서 확정한 "가용재고 = Stock − 미출고 CONFIRMED 수량 합"
  계산과 "OrderId 일자별 채번"에 Repository의 새 API를 추가하지 않는다.
  `OrderRepository`는 위 시그니처(`GetAll`/`FindById`/`Add`/`Update`/`Delete`)만
  유지하고, 미출고 `CONFIRMED` 수량 합계·기존 `OrderId` 목록 집계는
  Phase 3(Controller)가 `GetAll()` 결과를 순수 함수(`AvailableStockCalculator`/
  `OrderIdAllocator`, Phase 1)에 전달해 계산한다.
- 이렇게 두는 이유: 조회 조건(SampleId+상태, 날짜별 OrderId)이 Main 고유의
  요구사항이라 `Json/`에서 검증된 범용 CRUD 시그니처에 없던 메서드이며, 이런
  질의 로직까지 Repository에 넣으면 영속성 계층에 도메인 판단이 섞여
  Clean Code 원칙(계층 간 책임 분리)에 어긋난다.

## 다음 phase와의 연결점

Phase 3(Controller)은 이 Repository들을 생성자 주입으로 받아 메뉴 로직에서
호출한다. Controller는 Repository의 구체 타입에 의존해도 되지만(Model처럼),
View는 추상 인터페이스에만 의존한다(`MVC/` 패턴).
