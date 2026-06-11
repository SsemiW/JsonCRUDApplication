# JsonDataParsing — CRUD 콘솔 애플리케이션

## 프로젝트 목적

`feature/json-library` 브랜치의 JSON PoC 라이브러리를 기반으로,  
**회원 정보를 JSON 파일로 관리하는 CRUD 콘솔 애플리케이션**을 개발


---

## PoC 라이브러리 구조 (feature/json-library 참고)

새 코드 작성 시 아래 클래스를 그대로 재사용한다.

### `JsonValue` (`JsonValue.h`)
`std::variant` 기반 JSON 타입 시스템.

```
Null / Bool / Integer(int64_t) / Double / String / Array / Object
```

| API | 설명 |
|---|---|
| `isNull()`, `isBool()`, … | 타입 확인 |
| `asBool()`, `asInteger()`, `asDouble()`, `asString()` | 값 추출 |
| `asArray()`, `asObject()` | 컨테이너 참조 |
| `operator[](size_t)` | 배열 인덱스 접근 |
| `operator[](string)` | 객체 키 접근 (없으면 삽입) |
| `at(string)` | 객체 키 접근 (없으면 예외) |
| `contains(string)`, `size()`, `empty()` | 유틸리티 |

### `JsonParser` (`JsonParser.h/.cpp`)
재귀 하강 파서. 정적 메서드만 사용.

```cpp
JsonValue JsonParser::parse(const std::string& jsonStr);
JsonValue JsonParser::parseFile(const std::filesystem::path& filePath);
```

### `JsonSerializer` (`JsonSerializer.h/.cpp`)
직렬화 및 파일 저장. 정적 메서드만 사용.

```cpp
std::string JsonSerializer::serialize(const JsonValue&, bool pretty, int indent);
bool        JsonSerializer::saveFile (const JsonValue&, const std::filesystem::path&, bool pretty, int indent);
```

---

## 개발 목표: 회원 CRUD 콘솔 앱

### 회원 데이터 필드 (미정 - 추가 및 삭제하기 쉽도록 구현)

| 필드 | 타입 | 설명 |
|---|---|---|
| `id` | Integer | 고유 식별자 (자동 증가) |
| `name` | String | 이름 |
| `email` | String | 이메일 |
| `age` | Integer | 나이 |
| `createdAt` | String | 등록일시 (ISO 8601) |

### JSON 파일 포맷 (members.json)

```json
{
    "members": [
        {
            "id": 1,
            "name": "홍길동",
            "email": "hong@example.com",
            "age": 30,
            "createdAt": "2026-06-11T12:00:00"
        }
    ],
    "nextId": 2
}
```

### CRUD 기능

| 기능 | 설명 |
|---|---|
| **Create** | 새로운 데이터를 입력 받아 JSON 파일에 저장 |
| **Read** | 전체 목록 보기 및 특정 ID/키값으로 검색 |
| **Update** | 기존 데이터를 선택하여 특정 필드 수정 |
| **Delete** | 특정 데이터를 안전하게 삭제 |

### 예상 파일 구조

```
JsonDataParsing/
├── JsonValue.h              # PoC 재사용
├── JsonParser.h/.cpp        # PoC 재사용
├── JsonSerializer.h/.cpp    # PoC 재사용
├── Member.h                 # 회원 구조체 + JsonValue 변환
├── MemberRepository.h/.cpp  # JSON 파일 CRUD 로직
├── MemberService.h/.cpp     # 비즈니스 로직 (유효성 검사 등)
├── ConsoleUI.h/.cpp         # 콘솔 메뉴 입출력
└── main.cpp                 # 진입점
```

---

## 코딩 규칙

- 소스 파일 인코딩: **UTF-8 BOM** 필수 (MSVC 인코딩 경고 방지)
- 새 파일 추가 시 `JsonDataParsing.vcxproj` / `.vcxproj.filters` 에 함께 등록
- 외부 라이브러리 추가 금지 — 표준 라이브러리 + PoC 라이브러리만 사용
- 데이터 파일 경로: 실행 파일 위치 기준 상대 경로 `members.json`
