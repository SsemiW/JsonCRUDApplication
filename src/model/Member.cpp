#include "Member.h"
#include <ctime>
#include <cstdio>

Member Member::fromJson(const JsonValue& obj) {
    Member m;
    m.id        = obj.at("id").asInteger();
    m.name      = obj.at("name").asString();
    m.email     = obj.at("email").asString();
    m.age       = obj.at("age").asInteger();
    m.createdAt = obj.at("createdAt").asString();
    return m;
}

JsonValue Member::toJson() const {
    JsonValue::Object obj;
    obj["id"]        = JsonValue(id);
    obj["name"]      = JsonValue(name);
    obj["email"]     = JsonValue(email);
    obj["age"]       = JsonValue(age);
    obj["createdAt"] = JsonValue(createdAt);
    return JsonValue(std::move(obj));
}

std::string Member::nowIso8601() {
    std::time_t t = std::time(nullptr);
    std::tm tm{};
    localtime_s(&tm, &t);
    char buf[20];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &tm);
    return buf;
}
