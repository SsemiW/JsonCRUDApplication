#pragma once
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <stdexcept>

class JsonValue {
public:
    using Null    = std::monostate;
    using Bool    = bool;
    using Integer = int64_t;
    using Double  = double;
    using String  = std::string;
    using Array   = std::vector<JsonValue>;
    using Object  = std::map<std::string, JsonValue>;

    enum class Type { Null, Bool, Integer, Double, String, Array, Object };

    JsonValue()                      : data_(Null{}) {}
    JsonValue(std::nullptr_t)        : data_(Null{}) {}
    JsonValue(bool v)                : data_(v) {}
    JsonValue(int v)                 : data_(static_cast<Integer>(v)) {}
    JsonValue(int64_t v)             : data_(v) {}
    JsonValue(double v)              : data_(v) {}
    JsonValue(const char* v)         : data_(String(v)) {}
    JsonValue(std::string v)         : data_(std::move(v)) {}
    JsonValue(Array v)               : data_(std::move(v)) {}
    JsonValue(Object v)              : data_(std::move(v)) {}

    Type type() const {
        return std::visit([](auto&& v) -> Type {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, Null>)    return Type::Null;
            if constexpr (std::is_same_v<T, Bool>)    return Type::Bool;
            if constexpr (std::is_same_v<T, Integer>) return Type::Integer;
            if constexpr (std::is_same_v<T, Double>)  return Type::Double;
            if constexpr (std::is_same_v<T, String>)  return Type::String;
            if constexpr (std::is_same_v<T, Array>)   return Type::Array;
            return Type::Object;
        }, data_);
    }

    bool isNull()    const { return type() == Type::Null; }
    bool isBool()    const { return type() == Type::Bool; }
    bool isInteger() const { return type() == Type::Integer; }
    bool isDouble()  const { return type() == Type::Double; }
    bool isNumber()  const { return isInteger() || isDouble(); }
    bool isString()  const { return type() == Type::String; }
    bool isArray()   const { return type() == Type::Array; }
    bool isObject()  const { return type() == Type::Object; }

    Bool    asBool()    const { return std::get<Bool>(data_); }
    Integer asInteger() const {
        if (isInteger()) return std::get<Integer>(data_);
        return static_cast<Integer>(std::get<Double>(data_));
    }
    Double  asDouble()  const {
        if (isDouble())  return std::get<Double>(data_);
        return static_cast<Double>(std::get<Integer>(data_));
    }
    const String& asString() const { return std::get<String>(data_); }
    Array&        asArray()        { return std::get<Array>(data_); }
    const Array&  asArray()  const { return std::get<Array>(data_); }
    Object&       asObject()       { return std::get<Object>(data_); }
    const Object& asObject() const { return std::get<Object>(data_); }

    // 배열 인덱스 접근
    JsonValue&       operator[](size_t idx)       { return std::get<Array>(data_)[idx]; }
    const JsonValue& operator[](size_t idx) const { return std::get<Array>(data_)[idx]; }

    // 객체 키 접근 (없으면 삽입)
    JsonValue& operator[](const std::string& key) { return std::get<Object>(data_)[key]; }

    // 객체 키 접근 (없으면 예외)
    const JsonValue& at(const std::string& key) const { return std::get<Object>(data_).at(key); }

    bool contains(const std::string& key) const {
        if (!isObject()) return false;
        return std::get<Object>(data_).count(key) > 0;
    }

    size_t size() const {
        if (isArray())  return std::get<Array>(data_).size();
        if (isObject()) return std::get<Object>(data_).size();
        return 0;
    }

    bool empty() const { return size() == 0; }

private:
    std::variant<Null, Bool, Integer, Double, String, Array, Object> data_;
};
