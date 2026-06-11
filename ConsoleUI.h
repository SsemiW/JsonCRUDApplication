#pragma once
#include "MemberService.h"

class ConsoleUI {
public:
    explicit ConsoleUI(MemberService& service);
    void run();

private:
    void showMenu()        const;
    void listAll()         const;
    void searchById()      const;
    void searchByKeyword() const;
    void createMember();
    void updateMember();
    void deleteMember();

    void printMember(const Member& m)              const;
    void printList  (const std::vector<Member>& v) const;

    std::string inputLine  (const std::string& prompt) const;
    int64_t     inputInt64 (const std::string& prompt) const;
    bool        inputConfirm(const std::string& prompt) const;

    MemberService& service_;
};
