#include "JsonParser.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

JsonValue JsonParser::parse(const std::string& jsonStr) {
    JsonParser parser(jsonStr);
    parser.skipWhitespace();
    JsonValue result = parser.parseValue();
    parser.skipWhitespace();
    if (!parser.isEnd())
        throw std::runtime_error("JSON 파싱 오류: 값 이후 예상치 못한 문자가 있습니다.");
    return result;
}

JsonValue JsonParser::parseFile(const std::filesystem::path& filePath) {
    std::ifstream file(filePath, std::ios::in);
    if (!file.is_open())
        throw std::runtime_error("파일을 열 수 없습니다: " + filePath.string());
    std::ostringstream ss;
    ss << file.rdbuf();
    return parse(ss.str());
}

JsonValue JsonParser::parseValue() {
    if (depth_ >= 512)
        throw std::runtime_error("JSON 중첩 깊이 초과 (최대 512)");
    struct DepthGuard { int& d; ~DepthGuard() { --d; } } guard{ depth_ };
    ++depth_;

    skipWhitespace();
    if (isEnd())
        throw std::runtime_error("JSON 파싱 오류: 입력이 예상보다 일찍 끝났습니다.");

    char c = peek();
    if (c == 'n')                   return parseNull();
    if (c == 't' || c == 'f')       return parseBool();
    if (c == '"')                   return parseString();
    if (c == '[')                   return parseArray();
    if (c == '{')                   return parseObject();
    if (c == '-' || std::isdigit(static_cast<unsigned char>(c)))
                                    return parseNumber();

    throw std::runtime_error(std::string("JSON 파싱 오류: 예상치 못한 문자 '") + c + "'");
}

JsonValue JsonParser::parseNull() {
    if (input_.substr(pos_, 4) != "null")
        throw std::runtime_error("JSON 파싱 오류: 잘못된 null 리터럴");
    pos_ += 4;
    return JsonValue(nullptr);
}

JsonValue JsonParser::parseBool() {
    if (input_.substr(pos_, 4) == "true")  { pos_ += 4; return JsonValue(true);  }
    if (input_.substr(pos_, 5) == "false") { pos_ += 5; return JsonValue(false); }
    throw std::runtime_error("JSON 파싱 오류: 잘못된 boolean 리터럴");
}

JsonValue JsonParser::parseNumber() {
    size_t start  = pos_;
    bool isFloat  = false;

    if (peek() == '-') consume();

    if (isEnd() || !std::isdigit(static_cast<unsigned char>(peek())))
        throw std::runtime_error("JSON 파싱 오류: 잘못된 숫자 형식");

    while (!isEnd() && std::isdigit(static_cast<unsigned char>(peek()))) consume();

    if (!isEnd() && peek() == '.') {
        isFloat = true;
        consume();
        while (!isEnd() && std::isdigit(static_cast<unsigned char>(peek()))) consume();
    }

    if (!isEnd() && (peek() == 'e' || peek() == 'E')) {
        isFloat = true;
        consume();
        if (!isEnd() && (peek() == '+' || peek() == '-')) consume();
        while (!isEnd() && std::isdigit(static_cast<unsigned char>(peek()))) consume();
    }

    std::string numStr = input_.substr(start, pos_ - start);
    try {
        if (isFloat) return JsonValue(std::stod(numStr));
        return JsonValue(static_cast<int64_t>(std::stoll(numStr)));
    } catch (...) {
        throw std::runtime_error("JSON 파싱 오류: 숫자 변환 실패 '" + numStr + "'");
    }
}

JsonValue JsonParser::parseString() {
    expect('"');
    std::string result;

    while (!isEnd() && peek() != '"') {
        char c = consume();
        if (c != '\\') { result += c; continue; }

        if (isEnd()) throw std::runtime_error("JSON 파싱 오류: 문자열 이스케이프 중 입력 종료");
        char esc = consume();
        switch (esc) {
            case '"':  result += '"';  break;
            case '\\': result += '\\'; break;
            case '/':  result += '/';  break;
            case 'n':  result += '\n'; break;
            case 'r':  result += '\r'; break;
            case 't':  result += '\t'; break;
            case 'b':  result += '\b'; break;
            case 'f':  result += '\f'; break;
            case 'u': {
                if (pos_ + 4 > input_.size())
                    throw std::runtime_error("JSON 파싱 오류: \\u 이스케이프 시퀀스가 너무 짧습니다.");
                std::string hex = input_.substr(pos_, 4);
                pos_ += 4;
                uint32_t cp = static_cast<uint32_t>(std::stoul(hex, nullptr, 16));
                if (cp < 0x80) {
                    result += static_cast<char>(cp);
                } else if (cp < 0x800) {
                    result += static_cast<char>(0xC0 | (cp >> 6));
                    result += static_cast<char>(0x80 | (cp & 0x3F));
                } else {
                    result += static_cast<char>(0xE0 | (cp >> 12));
                    result += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                    result += static_cast<char>(0x80 | (cp & 0x3F));
                }
                break;
            }
            default:
                throw std::runtime_error(std::string("JSON 파싱 오류: 알 수 없는 이스케이프 '\\") + esc + "'");
        }
    }

    expect('"');
    return JsonValue(result);
}

JsonValue JsonParser::parseArray() {
    expect('[');
    JsonValue::Array arr;
    skipWhitespace();

    if (!isEnd() && peek() == ']') { consume(); return JsonValue(std::move(arr)); }

    while (true) {
        arr.push_back(parseValue());
        skipWhitespace();
        if (isEnd()) throw std::runtime_error("JSON 파싱 오류: 배열이 닫히지 않았습니다.");
        if (peek() == ']') { consume(); break; }
        expect(',');
    }
    return JsonValue(std::move(arr));
}

JsonValue JsonParser::parseObject() {
    expect('{');
    JsonValue::Object obj;
    skipWhitespace();

    if (!isEnd() && peek() == '}') { consume(); return JsonValue(std::move(obj)); }

    while (true) {
        skipWhitespace();
        auto key = parseString();
        skipWhitespace();
        expect(':');
        obj[key.asString()] = parseValue();
        skipWhitespace();
        if (isEnd()) throw std::runtime_error("JSON 파싱 오류: 객체가 닫히지 않았습니다.");
        if (peek() == '}') { consume(); break; }
        expect(',');
    }
    return JsonValue(std::move(obj));
}

void JsonParser::skipWhitespace() {
    while (!isEnd() && std::isspace(static_cast<unsigned char>(peek()))) ++pos_;
}

char JsonParser::peek()    const { return input_[pos_]; }
char JsonParser::consume()       { return input_[pos_++]; }
bool JsonParser::isEnd()   const { return pos_ >= input_.size(); }

void JsonParser::expect(char c) {
    if (isEnd())
        throw std::runtime_error(std::string("JSON 파싱 오류: '") + c + "' 예상, 입력 종료");
    if (peek() != c)
        throw std::runtime_error(std::string("JSON 파싱 오류: '") + c + "' 예상, '" + peek() + "' 발견");
    consume();
}
