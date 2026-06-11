#pragma once
#include "MemberRepository.h"
#include <string>
#include <vector>
#include <optional>

class MemberService {
public:
    explicit MemberService(MemberRepository& repo);

    Member              create(const std::string& name,
                               const std::string& email,
                               int64_t age);

    Member              update(int64_t id,
                               const std::string& field,
                               const std::string& value);

    bool                remove(int64_t id);

    std::vector<Member> findAll()                          const;
    std::optional<Member> findById(int64_t id)             const;
    std::vector<Member> search(const std::string& keyword) const;

private:
    void validateName (const std::string& name)                          const;
    void validateEmail(const std::string& email, int64_t excludeId = -1) const;
    void validateAge  (int64_t age)                                      const;

    MemberRepository& repo_;
};
