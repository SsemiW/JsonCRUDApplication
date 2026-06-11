# 테스트 구현 계획 — Regression Unit Test

## 목적

리팩토링 및 기능 추가 이후에도 기존 동작이 그대로 유지되는지 검증한다.  
각 레이어(JSON 라이브러리 → 모델 → 저장소 → 서비스)를 독립적으로 테스트하여  
의도치 않은 회귀 버그를 조기에 감지한다.

---

## 테스트 프레임워크

**Google Test 1.11.0** (NuGet 패키지 `gmock.1.11.0`) 사용.  
`gtest-all.cc` / `gmock-all.cc` 를 직접 컴파일하는 소스 통합 방식.

### 주요 매크로

```cpp
// 테스트 정의
TEST(TestSuite, TestName)               // 독립 테스트
TEST_F(FixtureClass, TestName)          // 픽스처 테스트 (SetUp / TearDown)

// 단언 — EXPECT_* : 실패해도 계속 / ASSERT_* : 실패 시 즉시 중단
EXPECT_TRUE(expr)   /  ASSERT_TRUE(expr)
EXPECT_FALSE(expr)  /  ASSERT_FALSE(expr)
EXPECT_EQ(a, b)     /  ASSERT_EQ(a, b)
EXPECT_NE(a, b)
EXPECT_GT(a, b)
EXPECT_DOUBLE_EQ(a, b)

// 예외 검증
EXPECT_THROW(expr, ExceptionType)       // 특정 예외 타입 검증
EXPECT_ANY_THROW(expr)                  // 예외 발생 여부만 검증
EXPECT_NO_THROW(expr)
```

**실행 결과 형식**
```
[==========] Running 57 tests from 7 test suites.
[ RUN      ] JsonParserTest.ParseNull
[       OK ] JsonParserTest.ParseNull (0 ms)
[ RUN      ] MemberSvcTest.CreateRejectsDuplicateEmail
[       OK ] MemberSvcTest.CreateRejectsDuplicateEmail (1 ms)
...
[  PASSED  ] 57 tests.
```

---

## 파일 구조

```
tests/
├── json/
│   ├── JsonValueTest.cpp        - JsonValue 타입 시스템
│   ├── JsonParserTest.cpp       - JSON 파싱
│   └── JsonSerializerTest.cpp   - JSON 직렬화
├── model/
│   └── MemberTest.cpp           - Member 변환 (fromJson / toJson)
├── repository/
│   └── MemberRepositoryTest.cpp - 파일 CRUD (TEST_F 픽스처)
└── service/
    └── MemberServiceTest.cpp    - 비즈니스 로직 및 유효성 검사 (TEST_F 픽스처)
```

---

## 테스트 케이스 명세

### 1. JsonValue

| # | GTest 이름 | 검증 내용 |
|---|---|---|
| 1 | `BasicTypeCreation` | Null / Bool / Integer / Double / String 각 타입 생성 및 `isXxx()` 확인 |
| 2 | `ArrayCreationAndAccess` | `operator[](size_t)`, `size()`, `empty()` |
| 3 | `ObjectCreationAndAccess` | `operator[](string)`, `at()`, `contains()` |
| 4 | `TypeMismatchThrows` | 잘못된 타입으로 `asXxx()` 호출 시 `std::bad_variant_access` 발생 |
| 5 | `AtKeyNotFoundThrows` | 존재하지 않는 키 접근 시 `std::out_of_range` 발생 |

---

### 2. JsonParser

| # | GTest 이름 | 검증 내용 |
|---|---|---|
| 1 | `ParseNull` | `"null"` → `JsonValue::Type::Null` |
| 2 | `ParseBoolean` | `"true"` / `"false"` → `bool` |
| 3 | `ParseInteger` | `"42"`, `"-7"` → `int64_t` |
| 4 | `ParseDouble` | `"3.14"`, `"-1.5e2"` → `double` |
| 5 | `ParseString` | 일반 문자열, 이스케이프(`\n` `\"` `\\`) 처리 |
| 6 | `ParseArray` | 빈 배열, 혼합 타입 배열 |
| 7 | `ParseNestedObject` | 객체 안 객체, 배열 안 객체 |
| 8 | `IgnoreWhitespace` | 줄바꿈·탭·공백이 포함된 JSON 정상 파싱 |
| 9 | `InvalidJsonThrows` | `"{ invalid }"` → `std::runtime_error` |
| 10 | `FileNotFoundThrows` | 존재하지 않는 경로 → `std::runtime_error` |

---

### 3. JsonSerializer

| # | GTest 이름 | 검증 내용 |
|---|---|---|
| 1 | `SerializeNull` | `"null"` 출력 |
| 2 | `SerializeBool` | `"true"` / `"false"` 출력 |
| 3 | `SerializeNumber` | 정수, 실수 포맷 |
| 4 | `StringEscape` | `"`, `\`, `\n`, `\t` 등 특수문자 이스케이프 |
| 5 | `EmptyContainers` | `"[]"`, `"{}"` 출력 |
| 6 | `CompactOutput` | 공백 없는 한 줄 출력 |
| 7 | `PrettyOutput` | 들여쓰기 포함, indent 크기 반영 |
| 8 | `RoundTrip` | 파싱 → 직렬화 → 재파싱 왕복, 원본 값과 동일 |
| 9 | `SaveFile` | 지정 경로에 파일 생성 및 내용 일치 |

---

### 4. Member

| # | GTest 이름 | 검증 내용 |
|---|---|---|
| 1 | `ToJsonFieldsMatch` | id / name / email / age / createdAt 키 및 값 |
| 2 | `FromJsonFieldsMatch` | JsonValue → Member 각 필드 정상 복원 |
| 3 | `RoundTrip` | `fromJson → toJson` 변환 전후 값 동일 |
| 4 | `NowIso8601Format` | `YYYY-MM-DDTHH:MM:SS` 형식 검증 |

---

### 5. MemberRepository

> 픽스처 클래스 `MemberRepoTest` (`::testing::Test` 상속)  
> `SetUp` — 임시 파일 경로 생성 + `MemberRepository` 초기화  
> `TearDown` — 인스턴스 소멸 + 임시 파일 삭제

| # | GTest 이름 | 검증 내용 |
|---|---|---|
| 1 | `MemberRepositoryMisc.AutoCreateFileWhenMissing` | 경로에 파일이 없어도 초기화 성공 |
| 2 | `MemberRepositoryMisc.CreatePersistsToFile` | 저장 후 새 인스턴스로 재로드 시 동일 데이터 |
| 3 | `CreateAutoIncrementId` | 첫 번째 회원 id=1, 두 번째 id=2 |
| 4 | `FindAllReturnsAll` | 추가한 수만큼 반환 |
| 5 | `FindByIdFound` | 정확한 회원 반환 |
| 6 | `FindByIdNotFound` | `std::nullopt` 반환 |
| 7 | `FindByNamePartialMatch` | 이름 일부로 검색 시 포함 결과 반환 |
| 8 | `UpdateModifiesField` | 수정 후 findById로 변경 확인 |
| 9 | `UpdateReturnsFalseForMissing` | `false` 반환 |
| 10 | `RemoveDeletesMember` | 삭제 후 findById → `std::nullopt` |
| 11 | `RemoveReturnsFalseForMissing` | `false` 반환 |
| 12 | `NextIdNotReused` | 삭제 후 새 추가 시 nextId가 재사용되지 않음 |

---

### 6. MemberService

> 픽스처 클래스 `MemberSvcTest` (`::testing::Test` 상속)  
> `SetUp` — 임시 파일 + `MemberRepository` + `MemberService` 초기화  
> `TearDown` — 인스턴스 소멸 + 임시 파일 삭제

| # | GTest 이름 | 검증 내용 |
|---|---|---|
| 1 | `CreateSuccess` | 유효한 입력 → Member 반환, id 자동 부여 |
| 2 | `CreateRejectsEmptyName` | `""` → `std::invalid_argument` |
| 3 | `CreateRejectsInvalidEmail` | `@` 미포함 → `std::invalid_argument` |
| 4 | `CreateRejectsDuplicateEmail` | 동일 이메일 2회 등록 → `std::invalid_argument` |
| 5 | `CreateRejectsOutOfRangeAge` | `0`, `151` → `std::invalid_argument` |
| 6 | `CreateAcceptsBoundaryAge` | `1`, `150` → 정상 등록 |
| 7 | `UpdateName` | 변경 후 findById로 확인 |
| 8 | `UpdateEmail` | 변경 후 중복 검사 기준도 갱신 |
| 9 | `UpdateAge` | 숫자 문자열 파싱 및 범위 검사 |
| 10 | `UpdateAgeRejectsNonNumeric` | `"abc"` → `std::runtime_error` |
| 11 | `UpdateRejectsMissingId` | `std::runtime_error` |
| 12 | `UpdateEmailSelfAllowed` | 동일 email로 자신 수정 가능 |
| 13 | `DeleteSuccess` | `true` 반환 |
| 14 | `DeleteRejectsMissingId` | `std::runtime_error` |
| 15 | `SearchByName` | 부분 일치 결과 반환 |
| 16 | `SearchByEmail` | 부분 일치 결과 반환 |
| 17 | `SearchNoMatch` | 빈 벡터 반환 |

---

## vcxproj 구성 방침

| Configuration | 진입점 | 동작 |
|---|---|---|
| **Debug** | `src/main.cpp` (`#ifdef _DEBUG` 분기) | GTest 실행 |
| **Release** | `src/main.cpp` (`#else` 분기) | CRUD 앱 실행 |
| **Test** | `src/main.cpp` | GTest 실행 (Debug와 동일) |

- 테스트 파일(`tests/**/*.cpp`)은 Release에서 `ExcludedFromBuild = true`
- `src/main.cpp` 의 `#ifdef _DEBUG` 안에서 `::testing::InitGoogleTest` 호출 후 `RUN_ALL_TESTS()`
- gmock.targets 가 `lib/native/include/` 를 모든 설정에 자동 추가
