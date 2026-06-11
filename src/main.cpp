#include <iostream>
#include <cstdio>
#ifdef _WIN32
#include <windows.h>
#endif
#include "MemberRepository.h"
#include "MemberService.h"
#include "ConsoleUI.h"

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stdin,  nullptr, _IONBF, 0);
    std::cout << std::unitbuf;
#endif

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
