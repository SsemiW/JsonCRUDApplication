#include <gtest/gtest.h>
#include "JsonSerializer.h"
#include "JsonParser.h"
#include <filesystem>

TEST(JsonSerializerTest, SerializeNull) {
    EXPECT_EQ(std::string("null"), JsonSerializer::serialize(JsonValue()));
    EXPECT_EQ(std::string("null"), JsonSerializer::serialize(JsonValue(nullptr)));
}

TEST(JsonSerializerTest, SerializeBool) {
    EXPECT_EQ(std::string("true"),  JsonSerializer::serialize(JsonValue(true)));
    EXPECT_EQ(std::string("false"), JsonSerializer::serialize(JsonValue(false)));
}

TEST(JsonSerializerTest, SerializeNumber) {
    EXPECT_EQ(std::string("42"),  JsonSerializer::serialize(JsonValue(42LL)));
    EXPECT_EQ(std::string("-7"),  JsonSerializer::serialize(JsonValue(-7LL)));
    EXPECT_EQ(std::string("0"),   JsonSerializer::serialize(JsonValue(0LL)));

    auto sd = JsonSerializer::serialize(JsonValue(3.14));
    EXPECT_NE(std::string::npos, sd.find("3.14"));
}

TEST(JsonSerializerTest, StringEscape) {
    EXPECT_NE(std::string::npos,
        JsonSerializer::serialize(JsonValue(std::string("say \"hi\""))).find("\\\""));

    EXPECT_NE(std::string::npos,
        JsonSerializer::serialize(JsonValue(std::string("line1\nline2"))).find("\\n"));

    EXPECT_NE(std::string::npos,
        JsonSerializer::serialize(JsonValue(std::string("back\\slash"))).find("\\\\"));

    EXPECT_NE(std::string::npos,
        JsonSerializer::serialize(JsonValue(std::string("tab\there"))).find("\\t"));
}

TEST(JsonSerializerTest, EmptyContainers) {
    EXPECT_EQ(std::string("[]"), JsonSerializer::serialize(JsonValue(JsonValue::Array{})));
    EXPECT_EQ(std::string("{}"), JsonSerializer::serialize(JsonValue(JsonValue::Object{})));
}

TEST(JsonSerializerTest, CompactOutput) {
    auto v = JsonParser::parse(R"({"a":1,"b":2})");
    auto s = JsonSerializer::serialize(v, false);
    EXPECT_EQ(std::string::npos, s.find('\n'));
    EXPECT_EQ(std::string::npos, s.find("  "));
}

TEST(JsonSerializerTest, PrettyOutput) {
    JsonValue::Object obj;
    obj["key"] = JsonValue(1LL);
    auto s = JsonSerializer::serialize(JsonValue(obj), true, 4);
    EXPECT_NE(std::string::npos, s.find('\n'));
    EXPECT_NE(std::string::npos, s.find("    "));
}

TEST(JsonSerializerTest, RoundTrip) {
    const std::string json = R"({"name":"홍길동","age":30,"active":true})";
    auto v1 = JsonParser::parse(json);
    auto v2 = JsonParser::parse(JsonSerializer::serialize(v1, false));

    EXPECT_EQ(v1.at("name").asString(),  v2.at("name").asString());
    EXPECT_EQ(v1.at("age").asInteger(),  v2.at("age").asInteger());
    EXPECT_EQ(v1.at("active").asBool(),  v2.at("active").asBool());
}

TEST(JsonSerializerTest, SaveFile) {
    auto path = std::filesystem::temp_directory_path() / "test_serializer_save.json";
    JsonValue::Object obj;
    obj["test"] = JsonValue(std::string("value"));

    EXPECT_TRUE(JsonSerializer::saveFile(JsonValue(obj), path, true, 4));
    ASSERT_TRUE(std::filesystem::exists(path));
    EXPECT_EQ(std::string("value"), JsonParser::parseFile(path).at("test").asString());

    std::filesystem::remove(path);
}
