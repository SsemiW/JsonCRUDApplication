#pragma once
#include "JsonValue.h"
#include <string>
#include <cstdint>

struct Member {
    int64_t     id        = 0;
    std::string name;
    std::string email;
    int64_t     age       = 0;
    std::string createdAt;

    static Member      fromJson(const JsonValue& obj);
    JsonValue          toJson() const;

    static std::string nowIso8601();
};
