#include <gtest/gtest.h>
#include "JsonSerializer.h"
#include "JsonParser.h"
#include <filesystem>
#include <fstream>
#include <limits>
#include <cmath>

// ── 3-1. 특수 문자 직렬화 ────────────────────────────────────────────────

TEST(JsonSerializerSafety, ControlCharEscaped) {
    std::string raw;
    raw += '\x01'; raw += '\x02'; raw += '\x1F';
    std::string result = JsonSerializer::serialize(JsonValue(raw), false, 0);
    EXPECT_NE(std::string::npos, result.find("\\u00"));
}

TEST(JsonSerializerSafety, NullByteInString) {
    std::string raw;
    raw += '\0';
    EXPECT_NO_THROW(JsonSerializer::serialize(JsonValue(raw), false, 0));
}

TEST(JsonSerializerSafety, QuoteAndBackslashRoundTrip) {
    std::string raw = "\"back\\slash\"";
    EXPECT_NO_THROW({
        std::string serialized = JsonSerializer::serialize(JsonValue(raw), false, 0);
        auto reparsed = JsonParser::parse(serialized);
        EXPECT_EQ(raw, reparsed.asString());
    });
}

TEST(JsonSerializerSafety, NanNocrash) {
    JsonValue v(std::numeric_limits<double>::quiet_NaN());
    EXPECT_NO_THROW(JsonSerializer::serialize(v, false, 0));
}

TEST(JsonSerializerSafety, PosInfinityNocrash) {
    JsonValue v(std::numeric_limits<double>::infinity());
    EXPECT_NO_THROW(JsonSerializer::serialize(v, false, 0));
}

TEST(JsonSerializerSafety, NegInfinityNocrash) {
    JsonValue v(-std::numeric_limits<double>::infinity());
    EXPECT_NO_THROW(JsonSerializer::serialize(v, false, 0));
}

TEST(JsonSerializerSafety, LargeArrayNocrash) {
    JsonValue::Array arr;
    for (int64_t i = 0; i < 100000; ++i)
        arr.push_back(JsonValue(i));
    EXPECT_NO_THROW(JsonSerializer::serialize(JsonValue(std::move(arr)), false, 0));
}

TEST(JsonSerializerSafety, VeryLongStringRoundTrip) {
    std::string original(100000, 'x');
    std::string serialized = JsonSerializer::serialize(JsonValue(original), false, 0);
    auto reparsed = JsonParser::parse(serialized);
    EXPECT_EQ(original, reparsed.asString());
}

TEST(JsonSerializerSafety, PrettyRoundTrip) {
    JsonValue::Object obj;
    obj["k"] = JsonValue(std::string("v"));
    obj["n"] = JsonValue(42LL);
    JsonValue v1(obj);
    std::string pretty = JsonSerializer::serialize(v1, true, 4);
    auto v2 = JsonParser::parse(pretty);
    EXPECT_EQ(v1.at("k").asString(), v2.at("k").asString());
    EXPECT_EQ(v1.at("n").asInteger(), v2.at("n").asInteger());
}

// ── 3-2. 파일 저장 실패 ─────────────────────────────────────────────────

TEST(JsonSerializerSafety, SaveToNonExistentDirectory) {
    auto path = std::filesystem::temp_directory_path()
                / "nonexistent_safety_dir_xyz"
                / "file.json";
    EXPECT_FALSE(JsonSerializer::saveFile(
        JsonValue(JsonValue::Object{}), path, false, 0));
}

TEST(JsonSerializerSafety, SaveToReadOnlyFile) {
    auto path = std::filesystem::temp_directory_path() / "readonly_safety_test.json";
    { std::ofstream f(path); f << "{}"; }

    namespace fs = std::filesystem;
    fs::permissions(path,
        fs::perms::owner_read | fs::perms::group_read | fs::perms::others_read,
        fs::perm_options::replace);

    bool ok = JsonSerializer::saveFile(
        JsonValue(JsonValue::Object{}), path, false, 0);
    EXPECT_FALSE(ok);

    fs::permissions(path, fs::perms::all, fs::perm_options::add);
    fs::remove(path);
}
