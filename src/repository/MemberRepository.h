#pragma once
#include "Member.h"
#include <vector>
#include <optional>
#include <filesystem>

class MemberRepository {
public:
    explicit MemberRepository(const std::filesystem::path& filePath);

    std::vector<Member>   findAll()                           const;
    std::optional<Member> findById(int64_t id)                const;
    std::vector<Member>   findByName(const std::string& name) const;

    Member create(Member member);
    bool   update(const Member& member);
    bool   remove(int64_t id);

private:
    void load();
    void save() const;

    std::filesystem::path filePath_;
    std::vector<Member>   members_;
    int64_t               nextId_ = 1;
};
