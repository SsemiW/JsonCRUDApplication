#include <gtest/gtest.h>
#include "Member.h"

TEST(MemberTest, ToJsonFieldsMatch) {
    Member m;
    m.id        = 1;
    m.name      = "홍길동";
    m.email     = "hong@example.com";
    m.age       = 30;
    m.createdAt = "2026-06-11T12:00:00";

    auto json = m.toJson();
    ASSERT_TRUE(json.isObject());
    EXPECT_EQ(1LL,                                json.at("id").asInteger());
    EXPECT_EQ(std::string("홍길동"),               json.at("name").asString());
    EXPECT_EQ(std::string("hong@example.com"),    json.at("email").asString());
    EXPECT_EQ(30LL,                               json.at("age").asInteger());
    EXPECT_EQ(std::string("2026-06-11T12:00:00"), json.at("createdAt").asString());
    EXPECT_TRUE(json.contains("id"));
    EXPECT_TRUE(json.contains("name"));
    EXPECT_TRUE(json.contains("email"));
    EXPECT_TRUE(json.contains("age"));
    EXPECT_TRUE(json.contains("createdAt"));
}

TEST(MemberTest, FromJsonFieldsMatch) {
    JsonValue::Object obj;
    obj["id"]        = JsonValue(2LL);
    obj["name"]      = JsonValue(std::string("김철수"));
    obj["email"]     = JsonValue(std::string("kim@example.com"));
    obj["age"]       = JsonValue(25LL);
    obj["createdAt"] = JsonValue(std::string("2026-06-11T09:00:00"));

    auto m = Member::fromJson(JsonValue(obj));
    EXPECT_EQ(2LL,                               m.id);
    EXPECT_EQ(std::string("김철수"),              m.name);
    EXPECT_EQ(std::string("kim@example.com"),    m.email);
    EXPECT_EQ(25LL,                              m.age);
    EXPECT_EQ(std::string("2026-06-11T09:00:00"), m.createdAt);
}

TEST(MemberTest, RoundTrip) {
    JsonValue::Object obj;
    obj["id"]        = JsonValue(3LL);
    obj["name"]      = JsonValue(std::string("이영희"));
    obj["email"]     = JsonValue(std::string("lee@example.com"));
    obj["age"]       = JsonValue(40LL);
    obj["createdAt"] = JsonValue(std::string("2026-01-01T00:00:00"));

    auto m    = Member::fromJson(JsonValue(obj));
    auto back = m.toJson();

    EXPECT_EQ(3LL,                               back.at("id").asInteger());
    EXPECT_EQ(std::string("이영희"),              back.at("name").asString());
    EXPECT_EQ(std::string("lee@example.com"),    back.at("email").asString());
    EXPECT_EQ(40LL,                              back.at("age").asInteger());
    EXPECT_EQ(std::string("2026-01-01T00:00:00"), back.at("createdAt").asString());
}

TEST(MemberTest, NowIso8601Format) {
    auto s = Member::nowIso8601();
    ASSERT_EQ(19u, s.size());
    EXPECT_EQ('T', s[10]);
    EXPECT_EQ('-', s[4]);
    EXPECT_EQ('-', s[7]);
    EXPECT_EQ(':', s[13]);
    EXPECT_EQ(':', s[16]);
    EXPECT_TRUE(s[0] >= '0' && s[0] <= '9');
    EXPECT_TRUE(s[3] >= '0' && s[3] <= '9');
}
