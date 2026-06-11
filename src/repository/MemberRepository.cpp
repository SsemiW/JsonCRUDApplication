#include "MemberRepository.h"
#include "JsonParser.h"
#include "JsonSerializer.h"
#include <algorithm>

MemberRepository::MemberRepository(const std::filesystem::path& filePath)
    : filePath_(filePath) {
    load();
}

std::vector<Member> MemberRepository::findAll() const {
    return members_;
}

std::optional<Member> MemberRepository::findById(int64_t id) const {
    for (const auto& m : members_)
        if (m.id == id) return m;
    return std::nullopt;
}

std::vector<Member> MemberRepository::findByName(const std::string& name) const {
    std::vector<Member> result;
    for (const auto& m : members_)
        if (m.name.find(name) != std::string::npos)
            result.push_back(m);
    return result;
}

Member MemberRepository::create(Member member) {
    member.id = nextId_++;
    members_.push_back(member);
    save();
    return member;
}

bool MemberRepository::update(const Member& member) {
    for (auto& m : members_) {
        if (m.id == member.id) {
            m = member;
            save();
            return true;
        }
    }
    return false;
}

bool MemberRepository::remove(int64_t id) {
    auto it = std::remove_if(members_.begin(), members_.end(),
        [id](const Member& m) { return m.id == id; });
    if (it == members_.end()) return false;
    members_.erase(it, members_.end());
    save();
    return true;
}

void MemberRepository::load() {
    if (!std::filesystem::exists(filePath_)) {
        save();
        return;
    }
    try {
        JsonValue root = JsonParser::parseFile(filePath_);
        nextId_ = root.at("nextId").asInteger();
        for (const auto& obj : root.at("members").asArray())
            members_.push_back(Member::fromJson(obj));
    } catch (...) {
        members_.clear();
        nextId_ = 1;
    }
}

void MemberRepository::save() const {
    JsonValue::Array arr;
    for (const auto& m : members_)
        arr.push_back(m.toJson());

    JsonValue::Object root;
    root["nextId"]  = JsonValue(nextId_);
    root["members"] = JsonValue(std::move(arr));

    JsonSerializer::saveFile(JsonValue(std::move(root)), filePath_, true, 4);
}
