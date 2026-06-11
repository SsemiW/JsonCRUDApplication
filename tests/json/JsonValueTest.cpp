#include <gtest/gtest.h>
#include "JsonValue.h"

TEST(JsonValueTest, BasicTypeCreation) {
    EXPECT_TRUE(JsonValue().isNull());
    EXPECT_EQ(JsonValue::Type::Null, JsonValue().type());

    EXPECT_TRUE(JsonValue(true).isBool());
    EXPECT_EQ(true, JsonValue(true).asBool());

    EXPECT_TRUE(JsonValue(42LL).isInteger());
    EXPECT_EQ(42LL, JsonValue(42LL).asInteger());

    EXPECT_TRUE(JsonValue(3.14).isDouble());
    EXPECT_DOUBLE_EQ(3.14, JsonValue(3.14).asDouble());

    EXPECT_TRUE(JsonValue(std::string("hello")).isString());
    EXPECT_EQ(std::string("hello"), JsonValue(std::string("hello")).asString());
}

TEST(JsonValueTest, ArrayCreationAndAccess) {
    JsonValue::Array arr;
    arr.push_back(JsonValue(1LL));
    arr.push_back(JsonValue(2LL));
    arr.push_back(JsonValue(3LL));
    JsonValue v(arr);

    EXPECT_TRUE(v.isArray());
    EXPECT_EQ(3u, v.size());
    EXPECT_FALSE(v.empty());
    EXPECT_EQ(1LL, v[0].asInteger());
    EXPECT_EQ(2LL, v[1].asInteger());
    EXPECT_EQ(3LL, v[2].asInteger());

    EXPECT_TRUE(JsonValue(JsonValue::Array{}).empty());
    EXPECT_EQ(0u, JsonValue(JsonValue::Array{}).size());
}

TEST(JsonValueTest, ObjectCreationAndAccess) {
    JsonValue::Object obj;
    obj["name"] = JsonValue(std::string("홍길동"));
    obj["age"]  = JsonValue(30LL);
    JsonValue v(obj);

    EXPECT_TRUE(v.isObject());
    EXPECT_EQ(2u, v.size());
    EXPECT_TRUE(v.contains("name"));
    EXPECT_FALSE(v.contains("email"));
    EXPECT_EQ(std::string("홍길동"), v.at("name").asString());
    EXPECT_EQ(30LL, v.at("age").asInteger());
}

TEST(JsonValueTest, TypeMismatchThrows) {
    JsonValue b(true);
    EXPECT_THROW(b.asInteger(), std::bad_variant_access);
    EXPECT_THROW(b.asDouble(),  std::bad_variant_access);
    EXPECT_THROW(b.asString(),  std::bad_variant_access);
    EXPECT_THROW(b.asArray(),   std::bad_variant_access);
    EXPECT_THROW(b.asObject(),  std::bad_variant_access);

    JsonValue i(42LL);
    EXPECT_THROW(i.asBool(),   std::bad_variant_access);
    EXPECT_THROW(i.asString(), std::bad_variant_access);
    EXPECT_THROW(i.asArray(),  std::bad_variant_access);
    EXPECT_THROW(i.asObject(), std::bad_variant_access);

    JsonValue s(std::string("text"));
    EXPECT_THROW(s.asBool(),    std::bad_variant_access);
    EXPECT_THROW(s.asInteger(), std::bad_variant_access);
    EXPECT_THROW(s.asDouble(),  std::bad_variant_access);
    EXPECT_THROW(s.asArray(),   std::bad_variant_access);
    EXPECT_THROW(s.asObject(),  std::bad_variant_access);
}

TEST(JsonValueTest, AtKeyNotFoundThrows) {
    JsonValue::Object obj;
    obj["key"] = JsonValue(1LL);
    JsonValue v(obj);

    EXPECT_THROW(v.at("nonexistent"), std::out_of_range);
    EXPECT_THROW(v.at(""),            std::out_of_range);
}
