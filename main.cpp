#include <iostream>
#include "MemberRepository.h"
#include "MemberService.h"
#include "ConsoleUI.h"

int main() {
    try {
        MemberRepository repo("members.json");
        MemberService    service(repo);
        ConsoleUI        ui(service);
        ui.run();
    } catch (const std::exception& e) {
        std::cerr << "치명적 오류: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
