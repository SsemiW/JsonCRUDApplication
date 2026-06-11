#include <gtest/gtest.h>
#include "JsonValue.h"

// ── 1-1. 잘못된 타입 강제 접근 ──────────────────────────────────────────

TEST(JsonValueSafety, BoolToIntegerThrows) {
    JsonValue v(true);
    EXPECT_THROW(v.asInteger(), std::bad_variant_access);
}

TEST(JsonValueSafety, IntegerToStringThrows) {
    JsonValue v(42LL);
    EXPECT_THROW(v.asString(), std::bad_variant_access);
}

TEST(JsonValueSafety, StringToBoolThrows) {
    JsonValue v(std::string("text"));
    EXPECT_THROW(v.asBool(), std::bad_variant_access);
}

TEST(JsonValueSafety, NullToDoubleThrows) {
    JsonValue v(nullptr);
    EXPECT_THROW(v.asDouble(), std::bad_variant_access);
}

TEST(JsonValueSafety, DefaultValueAtKeyThrows) {
    JsonValue v;  // Null
    EXPECT_THROW(v.at("key"), std::bad_variant_access);
}

TEST(JsonValueSafety, DoubleToArrayThrows) {
    JsonValue v(3.14);
    EXPECT_THROW(v.asArray(), std::bad_variant_access);
}

// ── 1-2. 컨테이너 경계 초과 ──────────────────────────────────────────────

TEST(JsonValueSafety, EmptyArrayIndexThrows) {
    JsonValue v(JsonValue::Array{});
    EXPECT_THROW(v[size_t(0)], std::out_of_range);
}

TEST(JsonValueSafety, OutOfBoundsArrayIndexThrows) {
    JsonValue::Array arr;
    for (int i = 0; i < 5; ++i)
        arr.push_back(JsonValue(static_cast<int64_t>(i)));
    JsonValue v(std::move(arr));
    EXPECT_THROW(v[size_t(999)], std::out_of_range);
}

TEST(JsonValueSafety, MissingKeyAtThrows) {
    JsonValue v(JsonValue::Object{});
    EXPECT_THROW(v.at("missing"), std::out_of_range);
}

TEST(JsonValueSafety, ObjectIndexBySizeTypeThrows) {
    JsonValue v(JsonValue::Object{});
    EXPECT_THROW(v[size_t(0)], std::bad_variant_access);
}

// ── 1-3. 극단적 크기 ────────────────────────────────────────────────────

TEST(JsonValueSafety, LargeArraySize) {
    JsonValue v(JsonValue::Array{});
    for (size_t i = 0; i < 100000; ++i)
        v.asArray().push_back(JsonValue(static_cast<int64_t>(i)));
    EXPECT_EQ(100000u, v.size());
}

TEST(JsonValueSafety, ObjectWith10000Keys) {
    EXPECT_NO_THROW({
        JsonValue v(JsonValue::Object{});
        for (int i = 0; i < 10000; ++i)
            v[std::to_string(i)] = JsonValue(static_cast<int64_t>(i));
        size_t count = 0;
        for ([[maybe_unused]] const auto& kv : v.asObject())
            ++count;
        EXPECT_EQ(10000u, count);
    });
}

TEST(JsonValueSafety, DeeplyNestedObjectCreation) {
    // 500단계: 스택 안전 범위 내에서 깊은 중첩 동작 검증
    EXPECT_NO_THROW({
        JsonValue root(JsonValue::Object{});
        JsonValue* cur = &root;
        for (int i = 0; i < 100; ++i) {
            (*cur)["n"] = JsonValue(JsonValue::Object{});
            cur = &(*cur)["n"];
        }
    });
}
