#include "JsonSerializer.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstdio>

std::string JsonSerializer::serialize(const JsonValue& value, bool pretty, int indentSize) {
    JsonSerializer s(pretty, indentSize);
    return s.serializeValue(value, 0);
}

bool JsonSerializer::saveFile(const JsonValue& value,
                               const std::filesystem::path& filePath,
                               bool pretty,
                               int  indentSize) {
    std::ofstream file(filePath, std::ios::out | std::ios::trunc);
    if (!file.is_open()) return false;
    file << serialize(value, pretty, indentSize);
    return file.good();
}

std::string JsonSerializer::serializeValue(const JsonValue& v, int depth) const {
    switch (v.type()) {
        case JsonValue::Type::Null:    return "null";
        case JsonValue::Type::Bool:    return v.asBool() ? "true" : "false";
        case JsonValue::Type::Integer: return std::to_string(v.asInteger());
        case JsonValue::Type::Double:  return serializeDouble(v.asDouble());
        case JsonValue::Type::String:  return serializeString(v.asString());
        case JsonValue::Type::Array:   return serializeArray(v.asArray(), depth);
        case JsonValue::Type::Object:  return serializeObject(v.asObject(), depth);
    }
    return "null";
}

std::string JsonSerializer::serializeDouble(double v) const {
    std::ostringstream ss;
    ss << std::setprecision(15) << v;
    std::string s = ss.str();
    // 정수처럼 보이면 ".0" 붙임 (JSON 스펙상 부동소수점 구분)
    if (s.find('.') == std::string::npos && s.find('e') == std::string::npos)
        s += ".0";
    return s;
}

std::string JsonSerializer::serializeString(const std::string& v) const {
    std::string out;
    out.reserve(v.size() + 2);
    out += '"';
    for (unsigned char c : v) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            case '\b': out += "\\b";  break;
            case '\f': out += "\\f";  break;
            default:
                if (c < 0x20) {
                    char buf[7];
                    snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else {
                    out += static_cast<char>(c);
                }
        }
    }
    out += '"';
    return out;
}

std::string JsonSerializer::serializeArray(const JsonValue::Array& arr, int depth) const {
    if (arr.empty()) return "[]";

    std::string out = "[";
    const std::string sep = pretty_ ? ",\n" : ",";
    if (pretty_) out += '\n';

    for (size_t i = 0; i < arr.size(); ++i) {
        if (pretty_) out += indent(depth + 1);
        out += serializeValue(arr[i], depth + 1);
        if (i + 1 < arr.size()) out += pretty_ ? "," : ",";
        if (pretty_) out += '\n';
    }

    if (pretty_) out += indent(depth);
    out += ']';
    return out;
}

std::string JsonSerializer::serializeObject(const JsonValue::Object& obj, int depth) const {
    if (obj.empty()) return "{}";

    std::string out = "{";
    if (pretty_) out += '\n';

    size_t i = 0;
    for (const auto& [key, val] : obj) {
        if (pretty_) out += indent(depth + 1);
        out += serializeString(key);
        out += pretty_ ? ": " : ":";
        out += serializeValue(val, depth + 1);
        if (i + 1 < obj.size()) out += ',';
        if (pretty_) out += '\n';
        ++i;
    }

    if (pretty_) out += indent(depth);
    out += '}';
    return out;
}

std::string JsonSerializer::indent(int depth) const {
    return std::string(static_cast<size_t>(depth) * indent_, ' ');
}
