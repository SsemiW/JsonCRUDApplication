# 테스트 구현 계획 — Regression Unit Test

## 목적

리팩토링 및 기능 추가 이후에도 기존 동작이 그대로 유지되는지 검증한다.  
각 레이어(JSON 라이브러리 → 모델 → 저장소 → 서비스)를 독립적으로 테스트하여  
의도치 않은 회귀 버그를 조기에 감지한다.

---

## 테스트 프레임워크

외부 라이브러리 없이 표준 라이브러리만 사용하는 **경량 커스텀 프레임워크**를 직접 구현한다.

### 구조 (`tests/framework/TestRunner.h`)

```cpp
// 단언 매크로
ASSERT_TRUE(expr)
ASSERT_FALSE(expr)
ASSERT_EQ(expected, actual)
ASSERT_NE(expected, actual)
ASSERT_THROWS(expr)          // std::exception 발생 여부
ASSERT_THROWS_MSG(expr, msg) // 예외 메시지 포함 여부

// 테스트 등록 및 실행
TEST_CASE("테스트명", []{ ... })
RUN_ALL_TESTS()              // 결과 집계 후 콘솔 출력
```

**실행 결과 형식**
```
[PASS] JsonParser - null 파싱
[PASS] JsonParser - 중첩 객체 파싱
[FAIL] MemberService - 중복 이메일 거부  →  expected exception not thrown
...
========================================
총 32건 | 통과 31 | 실패 1
```

---

## 파일 구조

```
tests/
├── framework/
│   └── TestRunner.h             - 커스텀 테스트 프레임워크
├── json/
│   ├── JsonValueTest.cpp        - JsonValue 타입 시스템
│   ├── JsonParserTest.cpp       - JSON 파싱
│   └── JsonSerializerTest.cpp   - JSON 직렬화
├── model/
│   └── MemberTest.cpp           - Member 변환 (fromJson / toJson)
├── repository/
│   └── MemberRepositoryTest.cpp - 파일 CRUD
├── service/
│   └── MemberServiceTest.cpp    - 비즈니스 로직 및 유효성 검사
└── main_test.cpp                - 테스트 진입점
```

---

## 테스트 케이스 명세

### 1. JsonValue

| # | 테스트명 | 검증 내용 |
|---|---|---|
| 1 | 기본 타입 생성 | Null / Bool / Integer / Double / String 각 타입 생성 및 `isXxx()` 확인 |
| 2 | 배열 생성 및 접근 | `operator[](size_t)`, `size()`, `empty()` |
| 3 | 객체 생성 및 접근 | `operator[](string)`, `at()`, `contains()` |
| 4 | 타입 불일치 예외 | 잘못된 타입으로 `asXxx()` 호출 시 `std::bad_variant_access` 발생 |
| 5 | `at()` 키 없음 예외 | 존재하지 않는 키 접근 시 `std::out_of_range` 발생 |

---

### 2. JsonParser

| # | 테스트명 | 검증 내용 |
|---|---|---|
| 1 | null 파싱 | `"null"` → `JsonValue::Type::Null` |
| 2 | boolean 파싱 | `"true"` / `"false"` → `bool` |
| 3 | 정수 파싱 | `"42"`, `"-7"` → `int64_t` |
| 4 | 실수 파싱 | `"3.14"`, `"-1.5e2"` → `double` |
| 5 | 문자열 파싱 | 일반 문자열, 이스케이프(`\n` `\"` `\\`) 처리 |
| 6 | 배열 파싱 | 빈 배열, 혼합 타입 배열 |
| 7 | 중첩 객체 파싱 | 객체 안 객체, 배열 안 객체 |
| 8 | 공백 무시 | 줄바꿈·탭·공백이 포함된 JSON 정상 파싱 |
| 9 | 잘못된 JSON 예외 | `"{ invalid }"` → `std::runtime_error` |
| 10 | 파일 없음 예외 | 존재하지 않는 경로 → `std::runtime_error` |

---

### 3. JsonSerializer

| # | 테스트명 | 검증 내용 |
|---|---|---|
| 1 | null 직렬화 | `"null"` 출력 |
| 2 | boolean 직렬화 | `"true"` / `"false"` 출력 |
| 3 | 숫자 직렬화 | 정수, 실수 포맷 |
| 4 | 문자열 이스케이프 | `"`, `\`, `\n` 등 특수문자 이스케이프 |
| 5 | 빈 배열/객체 | `"[]"`, `"{}"` 출력 |
| 6 | Compact 직렬화 | 공백 없는 한 줄 출력 |
| 7 | Pretty 직렬화 | 들여쓰기 포함, indent 크기 반영 |
| 8 | 파싱 → 직렬화 → 재파싱 왕복 | 원본 값과 동일 (Round-trip) |
| 9 | 파일 저장 | 지정 경로에 파일 생성 및 내용 일치 |

---

### 4. Member

| # | 테스트명 | 검증 내용 |
|---|---|---|
| 1 | `toJson()` 필드 일치 | id / name / email / age / createdAt 키 및 값 |
| 2 | `fromJson()` 필드 일치 | JsonValue → Member 각 필드 정상 복원 |
| 3 | `fromJson → toJson` 왕복 | 변환 전후 값 동일 |
| 4 | `nowIso8601()` 포맷 | `YYYY-MM-DDTHH:MM:SS` 형식 검증 |

---

### 5. MemberRepository

> 각 테스트는 임시 파일(`test_members_XXXX.json`)을 생성하고 테스트 후 삭제한다.

| # | 테스트명 | 검증 내용 |
|---|---|---|
| 1 | 파일 없을 때 자동 생성 | 경로에 파일이 없어도 초기화 성공 |
| 2 | Create — ID 자동 증가 | 첫 번째 회원 id=1, 두 번째 id=2 |
| 3 | Create — 파일 영속성 | 저장 후 새 Repository 인스턴스로 재로드 시 동일 데이터 |
| 4 | findAll — 전체 조회 | 추가한 수만큼 반환 |
| 5 | findById — 존재하는 ID | 정확한 회원 반환 |
| 6 | findById — 없는 ID | `std::nullopt` 반환 |
| 7 | findByName — 부분 일치 | 이름 일부로 검색 시 포함 결과 반환 |
| 8 | Update — 필드 수정 | 수정 후 findById로 변경 확인 |
| 9 | Update — 없는 ID | `false` 반환 |
| 10 | Delete — 정상 삭제 | 삭제 후 findById → `std::nullopt` |
| 11 | Delete — 없는 ID | `false` 반환 |
| 12 | nextId 연속성 | 삭제 후 새 추가 시 nextId가 재사용되지 않음 |

---

### 6. MemberService

| # | 테스트명 | 검증 내용 |
|---|---|---|
| 1 | 정상 생성 | 유효한 입력 → Member 반환, id 자동 부여 |
| 2 | 이름 빈 값 거부 | `""` → `std::invalid_argument` |
| 3 | 이메일 형식 오류 거부 | `@` 미포함 → `std::invalid_argument` |
| 4 | 이메일 중복 거부 | 동일 이메일 2회 등록 → `std::invalid_argument` |
| 5 | 나이 범위 초과 거부 | `0`, `151` → `std::invalid_argument` |
| 6 | 나이 경계값 허용 | `1`, `150` → 정상 등록 |
| 7 | Update — name 수정 | 변경 후 findById 로 확인 |
| 8 | Update — email 수정 | 변경 후 중복 검사 기준도 갱신 |
| 9 | Update — age 수정 | 숫자 문자열 파싱 및 범위 검사 |
| 10 | Update — age 비숫자 입력 거부 | `"abc"` → `std::runtime_error` |
| 11 | Update — 없는 ID 거부 | `std::runtime_error` |
| 12 | Update — email 수정 시 자기 자신 중복 허용 | 동일 email로 자신 수정 가능 |
| 13 | Delete — 정상 삭제 | `true` 반환 |
| 14 | Delete — 없는 ID 거부 | `std::runtime_error` |
| 15 | search — 이름 키워드 | 부분 일치 결과 반환 |
| 16 | search — 이메일 키워드 | 부분 일치 결과 반환 |
| 17 | search — 매칭 없음 | 빈 벡터 반환 |

---

## 구현 순서

```
STEP 1  tests/framework/TestRunner.h   ← 커스텀 프레임워크
   ↓
STEP 2  JsonValueTest / JsonParserTest / JsonSerializerTest
   ↓
STEP 3  MemberTest
   ↓
STEP 4  MemberRepositoryTest          ← 임시 파일 픽스처 포함
   ↓
STEP 5  MemberServiceTest
   ↓
STEP 6  main_test.cpp + vcxproj 등록
```

---

## vcxproj 구성 방침

- 기존 `Debug / Release` Configuration에서는 `src/main.cpp` 를 진입점으로 사용
- `Test` Configuration을 추가하고 `tests/main_test.cpp` 를 진입점으로 교체
- `src/main.cpp` 는 `Test` Configuration에서 컴파일 대상에서 제외
