#include "MemberService.h"
#include <stdexcept>
#include <algorithm>

MemberService::MemberService(MemberRepository& repo) : repo_(repo) {}

Member MemberService::create(const std::string& name,
                              const std::string& email,
                              int64_t age) {
    validateName(name);
    validateEmail(email);
    validateAge(age);

    Member m;
    m.name      = name;
    m.email     = email;
    m.age       = age;
    m.createdAt = Member::nowIso8601();
    return repo_.create(std::move(m));
}

Member MemberService::update(int64_t id,
                              const std::string& field,
                              const std::string& value) {
    auto opt = repo_.findById(id);
    if (!opt)
        throw std::runtime_error("ID " + std::to_string(id) + " 회원을 찾을 수 없습니다.");

    Member m = *opt;

    if (field == "name") {
        validateName(value);
        m.name = value;
    } else if (field == "email") {
        validateEmail(value, id);
        m.email = value;
    } else if (field == "age") {
        int64_t age;
        try {
            std::size_t pos;
            age = std::stoll(value, &pos);
            if (pos != value.size())
                throw std::runtime_error("나이는 숫자로 입력해주세요.");
        }
        catch (const std::runtime_error&) { throw; }
        catch (...) { throw std::runtime_error("나이는 숫자로 입력해주세요."); }
        validateAge(age);
        m.age = age;
    } else {
        throw std::runtime_error("수정할 수 없는 필드: " + field);
    }

    repo_.update(m);
    return m;
}

bool MemberService::remove(int64_t id) {
    if (!repo_.findById(id))
        throw std::runtime_error("ID " + std::to_string(id) + " 회원을 찾을 수 없습니다.");
    return repo_.remove(id);
}

std::vector<Member> MemberService::findAll() const {
    return repo_.findAll();
}

std::optional<Member> MemberService::findById(int64_t id) const {
    return repo_.findById(id);
}

std::vector<Member> MemberService::search(const std::string& keyword) const {
    std::vector<Member> result;
    for (const auto& m : repo_.findAll()) {
        if (m.name.find(keyword)  != std::string::npos ||
            m.email.find(keyword) != std::string::npos)
            result.push_back(m);
    }
    return result;
}

void MemberService::validateName(const std::string& name) const {
    if (name.empty())
        throw std::invalid_argument("이름을 입력해주세요.");
}

void MemberService::validateEmail(const std::string& email, int64_t excludeId) const {
    if (email.find('@') == std::string::npos)
        throw std::invalid_argument("올바른 이메일 형식이 아닙니다.");

    for (const auto& m : repo_.findAll()) {
        if (m.email == email && m.id != excludeId)
            throw std::invalid_argument("이미 사용 중인 이메일입니다: " + email);
    }
}

void MemberService::validateAge(int64_t age) const {
    if (age < 1 || age > 150)
        throw std::invalid_argument("나이는 1~150 사이로 입력해주세요.");
}
