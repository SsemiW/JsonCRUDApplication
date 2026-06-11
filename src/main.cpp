#include <iostream>
#include <cstdio>
#ifdef _WIN32
#include <windows.h>
#endif

#ifdef _DEBUG
#include <gtest/gtest.h>
#else
#include "MemberRepository.h"
#include "MemberService.h"
#include "ConsoleUI.h"
#endif

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stdin,  nullptr, _IONBF, 0);
    std::cout << std::unitbuf;
#endif

#ifdef _DEBUG
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
#else
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
#endif
}
