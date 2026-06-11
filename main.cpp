#include <iostream>
#include <cassert>
#include "JsonParser.h"
#include "JsonSerializer.h"

// ─────────────────────────────────────────────────────────────────────────────
// 헬퍼: 구분선 출력
// ─────────────────────────────────────────────────────────────────────────────
static void section(const char* title) {
    std::cout << "\n========================================\n"
              << "  " << title << "\n"
              << "========================================\n";
}

int main() {
    // ─────────────────────────────────────────────
    // 1. JSON 문자열 파싱
    // ─────────────────────────────────────────────
    section("1. JSON 문자열 파싱");

    const std::string raw = R"({
        "name"   : "홍길동",
        "age"    : 30,
        "score"  : 98.5,
        "active" : true,
        "address": null,
        "tags"   : ["C++", "JSON", "PoC"],
        "contact": {
            "email": "hong@example.com",
            "phone": "010-1234-5678"
        }
    })";

    JsonValue root = JsonParser::parse(raw);

    std::cout << "name   : " << root.at("name").asString()    << "\n";
    std::cout << "age    : " << root.at("age").asInteger()    << "\n";
    std::cout << "score  : " << root.at("score").asDouble()   << "\n";
    std::cout << "active : " << std::boolalpha
                             << root.at("active").asBool()    << "\n";
    std::cout << "address: " << (root.at("address").isNull() ? "(null)" : "?") << "\n";

    // ─────────────────────────────────────────────
    // 2. 배열 순회
    // ─────────────────────────────────────────────
    section("2. 배열 순회");

    const auto& tags = root.at("tags").asArray();
    for (size_t i = 0; i < tags.size(); ++i)
        std::cout << "tags[" << i << "] = " << tags[i].asString() << "\n";

    // ─────────────────────────────────────────────
    // 3. 중첩 객체 접근
    // ─────────────────────────────────────────────
    section("3. 중첩 객체 접근");

    const auto& contact = root.at("contact");
    std::cout << "email: " << contact.at("email").asString() << "\n";
    std::cout << "phone: " << contact.at("phone").asString() << "\n";

    // ─────────────────────────────────────────────
    // 4. JsonValue 직접 빌드
    // ─────────────────────────────────────────────
    section("4. JsonValue 직접 빌드");

    JsonValue::Object person;
    person["id"]      = JsonValue(42LL);
    person["name"]    = JsonValue("김철수");
    person["height"]  = JsonValue(175.3);
    person["married"] = JsonValue(false);
    person["memo"]    = JsonValue(nullptr);

    JsonValue::Array hobbies;
    hobbies.push_back(JsonValue("독서"));
    hobbies.push_back(JsonValue("등산"));
    hobbies.push_back(JsonValue("코딩"));
    person["hobbies"] = JsonValue(std::move(hobbies));

    JsonValue built(std::move(person));

    // ─────────────────────────────────────────────
    // 5. 직렬화 (Compact)
    // ─────────────────────────────────────────────
    section("5. 직렬화 - Compact");

    std::string compact = JsonSerializer::serialize(built, /*pretty=*/false);
    std::cout << compact << "\n";

    // ─────────────────────────────────────────────
    // 6. 직렬화 (Pretty print)
    // ─────────────────────────────────────────────
    section("6. 직렬화 - Pretty Print");

    std::string pretty = JsonSerializer::serialize(built, /*pretty=*/true, /*indent=*/4);
    std::cout << pretty << "\n";

    // ─────────────────────────────────────────────
    // 7. 파일 저장
    // ─────────────────────────────────────────────
    section("7. 파일 저장");

    const std::filesystem::path outPath = "output.json";
    bool saved = JsonSerializer::saveFile(built, outPath, true, 4);
    std::cout << "저장 결과: " << (saved ? "성공" : "실패") << " -> " << outPath << "\n";

    // ─────────────────────────────────────────────
    // 8. 파일에서 다시 읽기 & 검증
    // ─────────────────────────────────────────────
    section("8. 파일 로드 & 검증");

    JsonValue loaded = JsonParser::parseFile(outPath);
    assert(loaded.at("id").asInteger()   == 42);
    assert(loaded.at("name").asString()  == "김철수");
    assert(loaded.at("height").asDouble() == 175.3);
    assert(loaded.at("married").asBool() == false);
    assert(loaded.at("memo").isNull());
    assert(loaded.at("hobbies")[1].asString() == "등산");

    std::cout << "파일 로드 후 검증 통과!\n";
    std::cout << "hobbies[0]: " << loaded.at("hobbies")[0].asString() << "\n";
    std::cout << "hobbies[1]: " << loaded.at("hobbies")[1].asString() << "\n";
    std::cout << "hobbies[2]: " << loaded.at("hobbies")[2].asString() << "\n";

    // ─────────────────────────────────────────────
    // 9. 에러 처리
    // ─────────────────────────────────────────────
    section("9. 에러 처리");

    try {
        JsonParser::parse("{ invalid }");
    } catch (const std::exception& e) {
        std::cout << "[예외 정상 발생] " << e.what() << "\n";
    }

    try {
        JsonParser::parseFile("nonexistent.json");
    } catch (const std::exception& e) {
        std::cout << "[예외 정상 발생] " << e.what() << "\n";
    }

    section("완료");
    std::cout << "모든 PoC 시나리오 성공!\n";

    return 0;
}
