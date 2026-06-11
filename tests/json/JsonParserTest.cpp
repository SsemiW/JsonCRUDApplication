#include <gtest/gtest.h>
#include "JsonParser.h"

TEST(JsonParserTest, ParseNull) {
    auto v = JsonParser::parse("null");
    EXPECT_TRUE(v.isNull());
    EXPECT_EQ(JsonValue::Type::Null, v.type());
}

TEST(JsonParserTest, ParseBoolean) {
    auto t = JsonParser::parse("true");
    EXPECT_TRUE(t.isBool());
    EXPECT_EQ(true, t.asBool());

    auto f = JsonParser::parse("false");
    EXPECT_TRUE(f.isBool());
    EXPECT_EQ(false, f.asBool());
}

TEST(JsonParserTest, ParseInteger) {
    EXPECT_EQ(42LL,  JsonParser::parse("42").asInteger());
    EXPECT_EQ(-7LL,  JsonParser::parse("-7").asInteger());
    EXPECT_EQ(0LL,   JsonParser::parse("0").asInteger());
    EXPECT_TRUE(JsonParser::parse("42").isInteger());
}

TEST(JsonParserTest, ParseDouble) {
    auto v1 = JsonParser::parse("3.14");
    EXPECT_TRUE(v1.isDouble());
    EXPECT_DOUBLE_EQ(3.14, v1.asDouble());

    EXPECT_DOUBLE_EQ(-150.0, JsonParser::parse("-1.5e2").asDouble());
    EXPECT_DOUBLE_EQ(1.0,    JsonParser::parse("1.0").asDouble());
}

TEST(JsonParserTest, ParseString) {
    EXPECT_EQ(std::string("hello"),       JsonParser::parse("\"hello\"").asString());
    EXPECT_EQ(std::string("line1\nline2"), JsonParser::parse("\"line1\\nline2\"").asString());
    EXPECT_EQ(std::string("say \"hi\""),  JsonParser::parse("\"say \\\"hi\\\"\"").asString());
    EXPECT_EQ(std::string("back\\slash"), JsonParser::parse("\"back\\\\slash\"").asString());
    EXPECT_EQ(std::string(""),            JsonParser::parse("\"\"").asString());
}

TEST(JsonParserTest, ParseArray) {
    auto empty = JsonParser::parse("[]");
    EXPECT_TRUE(empty.isArray());
    EXPECT_EQ(0u, empty.size());

    auto arr = JsonParser::parse("[1, \"two\", true, null]");
    ASSERT_EQ(4u, arr.size());
    EXPECT_EQ(1LL,              arr[0].asInteger());
    EXPECT_EQ(std::string("two"), arr[1].asString());
    EXPECT_EQ(true,             arr[2].asBool());
    EXPECT_TRUE(arr[3].isNull());
}

TEST(JsonParserTest, ParseNestedObject) {
    auto v = JsonParser::parse(R"({"a": {"b": {"c": 42}}})");
    ASSERT_TRUE(v.isObject());
    EXPECT_EQ(42LL, v.at("a").at("b").at("c").asInteger());

    auto arr_obj = JsonParser::parse(R"([{"id":1},{"id":2}])");
    ASSERT_EQ(2u, arr_obj.size());
    EXPECT_EQ(1LL, arr_obj[0].at("id").asInteger());
    EXPECT_EQ(2LL, arr_obj[1].at("id").asInteger());
}

TEST(JsonParserTest, IgnoreWhitespace) {
    auto v = JsonParser::parse("  {\n  \"key\"  :  42  \n}  ");
    EXPECT_EQ(42LL, v.at("key").asInteger());

    auto arr = JsonParser::parse("  [  1  ,  2  ,  3  ]  ");
    EXPECT_EQ(3u, arr.size());
}

TEST(JsonParserTest, InvalidJsonThrows) {
    EXPECT_THROW(JsonParser::parse("{ invalid }"), std::runtime_error);
    EXPECT_THROW(JsonParser::parse("{"),            std::runtime_error);
    EXPECT_THROW(JsonParser::parse(""),             std::runtime_error);
    EXPECT_THROW(JsonParser::parse("[1,2,"),        std::runtime_error);
    EXPECT_THROW(JsonParser::parse("\"unterminated"), std::runtime_error);
}

TEST(JsonParserTest, FileNotFoundThrows) {
    EXPECT_THROW(JsonParser::parseFile("__nonexistent_xyz_12345__.json"), std::runtime_error);
}
