#include "ConsoleUI.h"
#include <iostream>
#include <limits>
#include <stdexcept>
#ifdef _WIN32
#include <windows.h>
#endif

static const std::string LINE(40, '-');

ConsoleUI::ConsoleUI(MemberService& service) : service_(service) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}

void ConsoleUI::run() {
    while (true) {
        showMenu();
        int choice = static_cast<int>(inputInt64("선택"));
        std::cout << "\n";

        try {
            switch (choice) {
                case 1: listAll();         break;
                case 2: searchById();      break;
                case 3: searchByKeyword(); break;
                case 4: createMember();    break;
                case 5: updateMember();    break;
                case 6: deleteMember();    break;
                case 0:
                    std::cout << "종료합니다.\n";
                    return;
                default:
                    std::cout << "잘못된 선택입니다.\n";
            }
        } catch (const std::exception& e) {
            std::cout << "[오류] " << e.what() << "\n";
        }

        std::cout << "\n";
    }
}

void ConsoleUI::showMenu() const {
    std::cout << "========================================\n"
              << "         회원 관리 시스템\n"
              << "========================================\n"
              << "  1. 전체 목록 조회\n"
              << "  2. ID로 회원 검색\n"
              << "  3. 키워드로 회원 검색\n"
              << "  4. 회원 추가\n"
              << "  5. 회원 수정\n"
              << "  6. 회원 삭제\n"
              << "  0. 종료\n"
              << "========================================\n";
}

// ─── Read ────────────────────────────────────────────

void ConsoleUI::listAll() const {
    auto members = service_.findAll();
    if (members.empty()) {
        std::cout << "등록된 회원이 없습니다.\n";
        return;
    }
    std::cout << "총 " << members.size() << "명의 회원이 있습니다.\n\n";
    printList(members);
}

void ConsoleUI::searchById() const {
    int64_t id = inputInt64("검색할 ID");
    auto opt = service_.findById(id);
    if (!opt) {
        std::cout << "ID " << id << " 회원을 찾을 수 없습니다.\n";
        return;
    }
    printMember(*opt);
}

void ConsoleUI::searchByKeyword() const {
    std::string kw = inputLine("검색 키워드 (이름/이메일)");
    auto result = service_.search(kw);
    if (result.empty()) {
        std::cout << "검색 결과가 없습니다.\n";
        return;
    }
    std::cout << result.size() << "건 검색됨.\n\n";
    printList(result);
}

// ─── Create ──────────────────────────────────────────

void ConsoleUI::createMember() {
    std::cout << "[ 회원 추가 ]\n";
    std::string name  = inputLine("이름");
    std::string email = inputLine("이메일");
    int64_t     age   = inputInt64("나이");

    Member m = service_.create(name, email, age);
    std::cout << "\n회원이 추가되었습니다.\n";
    printMember(m);
}

// ─── Update ──────────────────────────────────────────

void ConsoleUI::updateMember() {
    int64_t id = inputInt64("수정할 회원 ID");
    auto opt = service_.findById(id);
    if (!opt) {
        std::cout << "ID " << id << " 회원을 찾을 수 없습니다.\n";
        return;
    }

    std::cout << "\n현재 정보:\n";
    printMember(*opt);

    std::cout << "수정할 항목을 선택하세요.\n"
              << "  1. 이름\n"
              << "  2. 이메일\n"
              << "  3. 나이\n";

    int choice = static_cast<int>(inputInt64("선택"));
    std::string field, value;

    switch (choice) {
        case 1: field = "name";  value = inputLine("새 이름");   break;
        case 2: field = "email"; value = inputLine("새 이메일"); break;
        case 3: field = "age";   value = inputLine("새 나이");   break;
        default:
            std::cout << "잘못된 선택입니다.\n";
            return;
    }

    Member updated = service_.update(id, field, value);
    std::cout << "\n수정이 완료되었습니다.\n";
    printMember(updated);
}

// ─── Delete ──────────────────────────────────────────

void ConsoleUI::deleteMember() {
    int64_t id = inputInt64("삭제할 회원 ID");
    auto opt = service_.findById(id);
    if (!opt) {
        std::cout << "ID " << id << " 회원을 찾을 수 없습니다.\n";
        return;
    }

    std::cout << "\n삭제할 회원 정보:\n";
    printMember(*opt);

    if (!inputConfirm("정말 삭제하시겠습니까? (Y/N)")) {
        std::cout << "삭제가 취소되었습니다.\n";
        return;
    }

    service_.remove(id);
    std::cout << "회원이 삭제되었습니다.\n";
}

// ─── 출력 헬퍼 ───────────────────────────────────────

void ConsoleUI::printMember(const Member& m) const {
    std::cout << LINE << "\n"
              << "  ID       : " << m.id        << "\n"
              << "  이름     : " << m.name      << "\n"
              << "  이메일   : " << m.email     << "\n"
              << "  나이     : " << m.age       << "\n"
              << "  등록일시 : " << m.createdAt << "\n"
              << LINE << "\n";
}

void ConsoleUI::printList(const std::vector<Member>& v) const {
    for (const auto& m : v) {
        std::cout << "  [" << m.id << "] "
                  << m.name  << " | "
                  << m.email << " | "
                  << m.age   << "세 | "
                  << m.createdAt << "\n";
    }
}

// ─── 입력 헬퍼 ───────────────────────────────────────

std::string ConsoleUI::inputLine(const std::string& prompt) const {
    std::cout << prompt << " > ";
    std::string s;
    std::getline(std::cin, s);
    return s;
}

int64_t ConsoleUI::inputInt64(const std::string& prompt) const {
    while (true) {
        std::cout << prompt << " > ";
        std::string s;
        std::getline(std::cin, s);
        try {
            return std::stoll(s);
        } catch (...) {
            std::cout << "숫자를 입력해주세요.\n";
        }
    }
}

bool ConsoleUI::inputConfirm(const std::string& prompt) const {
    std::cout << prompt << " > ";
    std::string s;
    std::getline(std::cin, s);
    return (!s.empty() && (s[0] == 'Y' || s[0] == 'y'));
}
