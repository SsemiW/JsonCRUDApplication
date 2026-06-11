#pragma once
#include "JsonValue.h"
#include <string>
#include <filesystem>

class JsonParser {
public:
    static JsonValue parse(const std::string& jsonStr);
    static JsonValue parseFile(const std::filesystem::path& filePath);

private:
    explicit JsonParser(const std::string& input) : input_(input), pos_(0), depth_(0) {}

    JsonValue parseValue();
    JsonValue parseNull();
    JsonValue parseBool();
    JsonValue parseNumber();
    JsonValue parseString();
    JsonValue parseArray();
    JsonValue parseObject();

    void skipWhitespace();
    char peek()  const;
    char consume();
    void expect(char c);
    bool isEnd() const;

    const std::string& input_;
    size_t pos_;
    int    depth_;
};
