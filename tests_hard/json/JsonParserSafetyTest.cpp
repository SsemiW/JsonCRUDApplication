#include <gtest/gtest.h>
#include "JsonParser.h"
#include <string>
#include <limits>

// ── 2-1. 빈값 / 공백 전용 ────────────────────────────────────────────────

TEST(JsonParserSafety, EmptyStringThrows) {
    EXPECT_THROW(JsonParser::parse(""), std::runtime_error);
}

TEST(JsonParserSafety, WhitespaceOnlyThrows) {
    EXPECT_THROW(JsonParser::parse("   \t\n  "), std::runtime_error);
}

TEST(JsonParserSafety, TrailingGarbageThrows) {
    EXPECT_THROW(JsonParser::parse("null GARBAGE"), std::runtime_error);
}

TEST(JsonParserSafety, TwoRootValuesThrows) {
    EXPECT_THROW(JsonParser::parse("1 2"), std::runtime_error);
}

// ── 2-2. 잘린 JSON (Truncated) ───────────────────────────────────────────

TEST(JsonParserSafety, TruncatedOpenBraceThrows) {
    EXPECT_THROW(JsonParser::parse("{"), std::runtime_error);
}

TEST(JsonParserSafety, TruncatedKeyThrows) {
    EXPECT_THROW(JsonParser::parse("{\"name\""), std::runtime_error);
}

TEST(JsonParserSafety, TruncatedAfterColonThrows) {
    EXPECT_THROW(JsonParser::parse("{\"name\":"), std::runtime_error);
}

TEST(JsonParserSafety, TruncatedArrayThrows) {
    EXPECT_THROW(JsonParser::parse("[1, 2,"), std::runtime_error);
}

TEST(JsonParserSafety, UnterminatedStringThrows) {
    EXPECT_THROW(JsonParser::parse("\"unterminated"), std::runtime_error);
}

TEST(JsonParserSafety, PartialKeywordThrows) {
    EXPECT_THROW(JsonParser::parse("tru"),  std::runtime_error);
    EXPECT_THROW(JsonParser::parse("fals"), std::runtime_error);
    EXPECT_THROW(JsonParser::parse("nul"),  std::runtime_error);
}

// ── 2-3. 숫자 극단값 ────────────────────────────────────────────────────

TEST(JsonParserSafety, Int64Max) {
    EXPECT_NO_THROW(JsonParser::parse("9223372036854775807"));
}

TEST(JsonParserSafety, Int64Min) {
    EXPECT_NO_THROW(JsonParser::parse("-9223372036854775808"));
}

TEST(JsonParserSafety, IntegerOverflowThrows) {
    EXPECT_ANY_THROW(JsonParser::parse("99999999999999999999999"));
}

TEST(JsonParserSafety, DoubleOverflowNocrash) {
    // 파서가 stod 오버플로 시 runtime_error 를 던짐 — 구현이 inf 허용 안 함
    EXPECT_THROW(JsonParser::parse("1e999"), std::runtime_error);
}

TEST(JsonParserSafety, DoubleUnderflow) {
    // 파서가 stod 언더플로 시 runtime_error 를 던짐 — 구현이 0.0 허용 안 함
    EXPECT_THROW(JsonParser::parse("1e-999"), std::runtime_error);
}

TEST(JsonParserSafety, LeadingDecimalPointThrows) {
    EXPECT_THROW(JsonParser::parse(".5"), std::runtime_error);
}

TEST(JsonParserSafety, TrailingDecimalPointNocrash) {
    // 구현에 따라 파싱 성공 또는 실패 — 일관된 동작 기록
    EXPECT_NO_THROW(JsonParser::parse("1."));
}

TEST(JsonParserSafety, DoubleMinusThrows) {
    EXPECT_THROW(JsonParser::parse("--1"), std::runtime_error);
}

TEST(JsonParserSafety, HexLiteralThrows) {
    EXPECT_THROW(JsonParser::parse("0x1F"), std::runtime_error);
}

// ── 2-4. 문자열 이스케이프 악용 ─────────────────────────────────────────

TEST(JsonParserSafety, NullByteInString) {
    EXPECT_NO_THROW(JsonParser::parse("\"\\u0000\""));
}

TEST(JsonParserSafety, LoneSurrogateNocrash) {
    // lone surrogate — 동작 기록
    EXPECT_NO_THROW(JsonParser::parse("\"\\uD800\""));
}

TEST(JsonParserSafety, IncompleteUnicodeEscapeThrows) {
    EXPECT_THROW(JsonParser::parse("\"\\u\""), std::runtime_error);
}

TEST(JsonParserSafety, NonHexUnicodeEscapeThrows) {
    // stoul 실패로 std::invalid_argument 발생 — runtime_error 아님
    EXPECT_ANY_THROW(JsonParser::parse("\"\\uXXXX\""));
}

TEST(JsonParserSafety, UnsupportedEscapeThrows) {
    EXPECT_THROW(JsonParser::parse("\"\\q\""), std::runtime_error);
}

TEST(JsonParserSafety, VeryLongStringNocrash) {
    std::string json = "\"" + std::string(100000, 'a') + "\"";
    EXPECT_NO_THROW(JsonParser::parse(json));
}

TEST(JsonParserSafety, ControlCharInStringNoThrow) {
    // 현재 파서는 제어문자를 그대로 허용 (RFC 7159 위반이지만 크래시 없음)
    std::string json;
    json += '"';
    json += '\x01';
    json += '\x02';
    json += '"';
    EXPECT_NO_THROW(JsonParser::parse(json));
}

// ── 2-5. 구조 파괴 ──────────────────────────────────────────────────────

TEST(JsonParserSafety, ReverseBracesThrows) {
    EXPECT_THROW(JsonParser::parse("}{"), std::runtime_error);
}

TEST(JsonParserSafety, MissingValueThrows) {
    EXPECT_THROW(JsonParser::parse("{\"key\": ,}"), std::runtime_error);
}

TEST(JsonParserSafety, CommaOnlyArrayThrows) {
    EXPECT_THROW(JsonParser::parse("[,]"), std::runtime_error);
}

TEST(JsonParserSafety, EmptyKeyAllowed) {
    // JSON 표준은 빈 키를 허용
    EXPECT_NO_THROW(JsonParser::parse("{\"\":1}"));
}

TEST(JsonParserSafety, DeeplyNestedArrayNocrash) {
    // 깊이 제한(512) 초과 → runtime_error (스택 오버플로 방지)
    std::string json = std::string(1000, '[') + std::string(1000, ']');
    EXPECT_THROW(JsonParser::parse(json), std::runtime_error);
}

TEST(JsonParserSafety, DuplicateKeyLastValueWins) {
    auto v = JsonParser::parse("{\"a\":1,\"a\":2}");
    EXPECT_NO_THROW({
        EXPECT_EQ(2LL, v.at("a").asInteger());
    });
}
