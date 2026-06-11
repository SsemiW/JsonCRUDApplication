#include <gtest/gtest.h>
#include "Member.h"
#include <climits>

// ── 4-1. fromJson 파괴 시도 ──────────────────────────────────────────────

TEST(MemberSafety, FromJsonMissingNameThrows) {
    JsonValue::Object obj;
    obj["id"]        = JsonValue(1LL);
    obj["email"]     = JsonValue(std::string("a@b.com"));
    obj["age"]       = JsonValue(20LL);
    obj["createdAt"] = JsonValue(std::string("t"));
    EXPECT_THROW(Member::fromJson(JsonValue(obj)), std::out_of_range);
}

TEST(MemberSafety, FromJsonAgeAsStringThrows) {
    JsonValue::Object obj;
    obj["id"]        = JsonValue(1LL);
    obj["name"]      = JsonValue(std::string("A"));
    obj["email"]     = JsonValue(std::string("a@b.com"));
    obj["age"]       = JsonValue(std::string("twenty"));  // String이어야 함 → bad_variant_access
    obj["createdAt"] = JsonValue(std::string("t"));
    EXPECT_THROW(Member::fromJson(JsonValue(obj)), std::bad_variant_access);
}

TEST(MemberSafety, FromJsonIdAsNullThrows) {
    JsonValue::Object obj;
    obj["id"]        = JsonValue(nullptr);  // Null → bad_variant_access
    obj["name"]      = JsonValue(std::string("A"));
    obj["email"]     = JsonValue(std::string("a@b.com"));
    obj["age"]       = JsonValue(20LL);
    obj["createdAt"] = JsonValue(std::string("t"));
    EXPECT_THROW(Member::fromJson(JsonValue(obj)), std::bad_variant_access);
}

TEST(MemberSafety, FromJsonEmptyObjectThrows) {
    EXPECT_ANY_THROW(Member::fromJson(JsonValue(JsonValue::Object{})));
}

// ── 4-2. 데이터 보존 (라운드트립) ───────────────────────────────────────

TEST(MemberSafety, LongNameRoundTrip) {
    std::string longName(10000, '가');
    Member m;
    m.id        = 1;
    m.name      = longName;
    m.email     = "a@b.com";
    m.age       = 20;
    m.createdAt = "t";

    auto roundtrip = Member::fromJson(m.toJson());
    EXPECT_EQ(longName, roundtrip.name);
}

TEST(MemberSafety, XssEmailRoundTrip) {
    std::string xss = "<script>alert(1)</script>@xss.com";
    Member m;
    m.id = 1; m.name = "A"; m.email = xss; m.age = 20; m.createdAt = "t";

    auto roundtrip = Member::fromJson(m.toJson());
    EXPECT_EQ(xss, roundtrip.email);
}

TEST(MemberSafety, SpecialCharsInNameRoundTrip) {
    std::string special = "\"back\\slash\"\nnewline";
    Member m;
    m.id = 1; m.name = special; m.email = "a@b.com"; m.age = 20; m.createdAt = "t";

    auto roundtrip = Member::fromJson(m.toJson());
    EXPECT_EQ(special, roundtrip.name);
}

TEST(MemberSafety, Int64MaxIdRoundTrip) {
    Member m;
    m.id = INT64_MAX; m.name = "A"; m.email = "a@b.com"; m.age = 20; m.createdAt = "t";

    auto roundtrip = Member::fromJson(m.toJson());
    EXPECT_EQ(INT64_MAX, roundtrip.id);
}

TEST(MemberSafety, NegativeIdRoundTrip) {
    Member m;
    m.id = -1LL; m.name = "A"; m.email = "a@b.com"; m.age = 20; m.createdAt = "t";

    auto roundtrip = Member::fromJson(m.toJson());
    EXPECT_EQ(-1LL, roundtrip.id);
}

TEST(MemberSafety, ZeroAgeRoundTrip) {
    Member m;
    m.id = 1; m.name = "A"; m.email = "a@b.com"; m.age = 0; m.createdAt = "t";

    auto roundtrip = Member::fromJson(m.toJson());
    EXPECT_EQ(0LL, roundtrip.age);
}
