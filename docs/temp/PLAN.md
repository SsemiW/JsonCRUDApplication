## 아키텍처

```
[ main.cpp ]
     │
     ▼
[ ConsoleUI ]          ← 사용자 입력/출력, 메뉴 루프
     │
     ▼
[ MemberService ]      ← 비즈니스 로직, 유효성 검사
     │
     ▼
[ MemberRepository ]   ← JSON 파일 읽기/쓰기 (CRUD)
     │
     ▼
[ JsonParser / JsonSerializer / JsonValue ]   ← PoC 재사용
     │
     ▼
[ members.json ]       ← 영속 데이터
```

---

## 데이터 명세

### 회원 필드

| 필드 | JSON 키 | C++ 타입 | 설명 |
|---|---|---|---|
| ID | `id` | `int64_t` | 자동 증가, 변경 불가 |
| 이름 | `name` | `std::string` | 필수 |
| 이메일 | `email` | `std::string` | 필수, 중복 불가 |
| 나이 | `age` | `int64_t` | 필수, 1~150 |
| 등록일시 | `createdAt` | `std::string` | 자동 생성 (ISO 8601) |

### members.json 구조

```json
{
    "nextId": 2,
    "members": [
        {
            "id": 1,
            "name": "홍길동",
            "email": "hong@example.com",
            "age": 30,
            "createdAt": "2026-06-11T12:00:00"
        }
    ]
}
```

---

## 파일 구조

```
JsonDataParsing/
├── JsonValue.h              (PoC 재사용)
├── JsonParser.h/.cpp        (PoC 재사용)
├── JsonSerializer.h/.cpp    (PoC 재사용)
│
├── Member.h                 (회원 구조체 + JsonValue 변환)
├── MemberRepository.h/.cpp  (JSON 파일 CRUD)
├── MemberService.h/.cpp     (유효성 검사, 비즈니스 로직)
├── ConsoleUI.h/.cpp         (메뉴, 입출력)
└── main.cpp                 (진입점)
```

---

## 구현 단계

### STEP 1 — Member 모델 (`Member.h`)

- 회원 데이터를 담는 구조체 정의
- `JsonValue` ↔ `Member` 변환 함수

```cpp
struct Member {
    int64_t     id;
    std::string name;
    std::string email;
    int64_t     age;
    std::string createdAt;

    static Member    fromJson(const JsonValue& obj);
    JsonValue        toJson()  const;
};
```

---

### STEP 2 — MemberRepository (`MemberRepository.h/.cpp`)

JSON 파일 읽기/쓰기를 담당. 비즈니스 로직 없이 데이터 접근만 처리.

```cpp
class MemberRepository {
public:
    explicit MemberRepository(const std::filesystem::path& filePath);

    std::vector<Member> findAll()                        const;
    std::optional<Member> findById(int64_t id)           const;
    std::vector<Member> findByName(const std::string& name) const;

    Member  create(Member member);       // nextId 자동 발급 후 저장
    bool    update(const Member& member);
    bool    remove(int64_t id);

private:
    void load();
    void save() const;

    std::filesystem::path       filePath_;
    std::vector<Member>         members_;
    int64_t                     nextId_ = 1;
};
```

**파일 없을 때**: 빈 `members.json` 자동 생성  
**저장 방식**: 변경 시마다 전체 파일을 pretty-print로 덮어씀

---

### STEP 3 — MemberService (`MemberService.h/.cpp`)

유효성 검사 및 비즈니스 규칙을 담당. Repository를 래핑.

```cpp
class MemberService {
public:
    explicit MemberService(MemberRepository& repo);

    // 반환: 성공 시 생성된 Member, 실패 시 오류 메시지
    std::expected<Member, std::string>      create(const std::string& name,
                                                   const std::string& email,
                                                   int64_t age);
    std::expected<Member, std::string>      update(int64_t id,
                                                   const std::string& field,
                                                   const std::string& value);
    std::expected<bool, std::string>        remove(int64_t id);
    std::vector<Member>                     findAll() const;
    std::optional<Member>                   findById(int64_t id) const;
    std::vector<Member>                     search(const std::string& keyword) const;

private:
    bool isValidEmail(const std::string& email) const;
    bool isDuplicateEmail(const std::string& email, int64_t excludeId = -1) const;

    MemberRepository& repo_;
};
```

**유효성 검사 항목**

| 항목 | 규칙 |
|---|---|
| 이름 | 1자 이상 |
| 이메일 | `@` 포함, 중복 불가 |
| 나이 | 1 ~ 150 |
| Update 필드 | `name` / `email` / `age` 만 수정 허용 |

---

### STEP 4 — ConsoleUI (`ConsoleUI.h/.cpp`)

콘솔 메뉴 루프 및 입출력 담당.

```
========================================
  회원 관리 시스템
========================================
  1. 전체 목록 조회
  2. ID로 회원 검색
  3. 키워드로 회원 검색
  4. 회원 추가
  5. 회원 수정
  6. 회원 삭제
  0. 종료
========================================
선택 > 
```

**각 메뉴 흐름**

| 메뉴 | 흐름 |
|---|---|
| 전체 목록 | 테이블 형태로 출력 |
| ID 검색 | ID 입력 → 단건 출력 |
| 키워드 검색 | 문자열 입력 → 이름/이메일 포함 검색 → 목록 출력 |
| 회원 추가 | 이름/이메일/나이 순 입력 → 유효성 검사 → 저장 → 결과 출력 |
| 회원 수정 | ID 입력 → 수정할 필드 선택 → 새 값 입력 → 저장 |
| 회원 삭제 | ID 입력 → 대상 정보 출력 → 확인(Y/N) → 삭제 |

---

### STEP 5 — main.cpp

```cpp
int main() {
    MemberRepository repo("members.json");
    MemberService    service(repo);
    ConsoleUI        ui(service);
    ui.run();
    return 0;
}
```

---

## 구현 순서 (의존성 순)

```
STEP 1  Member.h
   ↓
STEP 2  MemberRepository
   ↓
STEP 3  MemberService
   ↓
STEP 4  ConsoleUI
   ↓
STEP 5  main.cpp 연결
```

---

## 예외 처리 방침

- `JsonParser::parseFile` 실패 → 빈 데이터로 초기화 (파일 없음 허용)
- `JsonSerializer::saveFile` 실패 → 오류 메시지 출력, 프로그램 계속
- 잘못된 입력 → Service에서 `std::expected`로 반환, UI에서 메시지 출력
- 예기치 않은 예외 → `main()`의 `try/catch`에서 최종 처리
