#include <gtest/gtest.h>
#include "MemberRepository.h"
#include <filesystem>
#include <fstream>
#include <atomic>
#include <climits>

namespace {

void writeFile(const std::filesystem::path& p, const std::string& content) {
    std::ofstream f(p);
    f << content;
}

}  // namespace

class MemberRepoSafetyTest : public ::testing::Test {
protected:
    std::filesystem::path path_;

    void SetUp() override {
        static std::atomic<int> cnt{ 0 };
        path_ = std::filesystem::temp_directory_path() /
                ("repo_safety_" + std::to_string(++cnt) + ".json");
    }

    void TearDown() override {
        std::error_code ec;
        std::filesystem::remove(path_, ec);
    }

    Member makeMember(const std::string& name, const std::string& email, int64_t age) {
        Member m; m.name = name; m.email = email; m.age = age; m.createdAt = "t";
        return m;
    }
};

// ── 5-1. 비정상 파일 상태 ───────────────────────────────────────────────

TEST_F(MemberRepoSafetyTest, EmptyFileInitializesEmpty) {
    writeFile(path_, "");
    MemberRepository repo(path_);
    EXPECT_TRUE(repo.findAll().empty());
}

TEST_F(MemberRepoSafetyTest, CorruptedJsonInitializesEmpty) {
    writeFile(path_, "BROKEN");
    MemberRepository repo(path_);
    EXPECT_TRUE(repo.findAll().empty());
}

TEST_F(MemberRepoSafetyTest, MissingMembersKeyInitializesEmpty) {
    writeFile(path_, R"({"nextId":1})");
    MemberRepository repo(path_);
    EXPECT_TRUE(repo.findAll().empty());
}

TEST_F(MemberRepoSafetyTest, MissingNextIdKeyCreateSucceeds) {
    writeFile(path_, R"({"members":[]})");
    MemberRepository repo(path_);
    EXPECT_NO_THROW(repo.create(makeMember("A", "a@b.com", 20)));
}

TEST_F(MemberRepoSafetyTest, NegativeNextIdCreateSucceeds) {
    writeFile(path_, R"({"nextId":-999,"members":[]})");
    MemberRepository repo(path_);
    EXPECT_NO_THROW({
        auto m = repo.create(makeMember("A", "a@b.com", 20));
        EXPECT_EQ(-999LL, m.id);
    });
}

TEST_F(MemberRepoSafetyTest, Int64MaxNextIdNocrash) {
    writeFile(path_,
        R"({"nextId":9223372036854775807,"members":[]})");
    MemberRepository repo(path_);
    EXPECT_NO_THROW(repo.create(makeMember("A", "a@b.com", 20)));
}

TEST_F(MemberRepoSafetyTest, NullElementInMembersArrayNocrash) {
    writeFile(path_, R"({"nextId":2,"members":[null]})");
    EXPECT_NO_THROW(MemberRepository repo(path_));
}

TEST_F(MemberRepoSafetyTest, DuplicateIdsInFileLoadsAll) {
    writeFile(path_,
        R"({"nextId":3,"members":[)"
        R"({"id":1,"name":"A","email":"a@b.com","age":20,"createdAt":"t"},)"
        R"({"id":1,"name":"B","email":"b@b.com","age":21,"createdAt":"t"})"
        R"(]})");
    EXPECT_NO_THROW({
        MemberRepository repo(path_);
        EXPECT_EQ(2u, repo.findAll().size());
    });
}

TEST_F(MemberRepoSafetyTest, MissingFieldMemberNocrash) {
    // 필드 누락 항목 — load 예외 catch 로 전체 목록 비워질 수 있음
    writeFile(path_,
        R"({"nextId":2,"members":[{"id":1,"name":"A"}]})");
    EXPECT_NO_THROW(MemberRepository repo(path_));
}

// ── 5-2. 경로 극단값 ────────────────────────────────────────────────────

TEST(MemberRepoSafetyMisc, NonExistentDirectoryNocrash) {
    auto path = std::filesystem::temp_directory_path()
                / "no_such_dir_safety_xyz"
                / "data.json";
    EXPECT_NO_THROW(MemberRepository repo(path));
}

TEST(MemberRepoSafetyMisc, EmptyPathNocrash) {
    // 빈 경로: save() 실패하지만 예외 없이 초기화됨
    EXPECT_NO_THROW(MemberRepository repo(std::filesystem::path("")));
}

TEST(MemberRepoSafetyMisc, VeryLongPathNocrash) {
    // 260자 초과 경로: save() 실패하지만 예외 없이 초기화됨
    std::string longDir(240, 'x');
    auto path = std::filesystem::temp_directory_path() / longDir / "data.json";
    EXPECT_NO_THROW(MemberRepository repo(path));
}

// ── 5-3. 대용량 / 연속 조작 ─────────────────────────────────────────────

TEST_F(MemberRepoSafetyTest, Create1000Members) {
    MemberRepository repo(path_);
    for (int i = 0; i < 1000; ++i)
        repo.create(makeMember("M" + std::to_string(i),
                               "m" + std::to_string(i) + "@b.com", 20));
    EXPECT_EQ(1000u, repo.findAll().size());
}

TEST_F(MemberRepoSafetyTest, Create1000ThenDeleteAll) {
    MemberRepository repo(path_);
    for (int i = 0; i < 1000; ++i)
        repo.create(makeMember("M" + std::to_string(i),
                               "m" + std::to_string(i) + "@b.com", 20));
    for (const auto& m : repo.findAll())
        repo.remove(m.id);
    EXPECT_TRUE(repo.findAll().empty());
}

TEST_F(MemberRepoSafetyTest, Create1000ThenReload) {
    {
        MemberRepository repo(path_);
        for (int i = 0; i < 1000; ++i)
            repo.create(makeMember("M" + std::to_string(i),
                                   "m" + std::to_string(i) + "@b.com", 20));
    }
    MemberRepository repo2(path_);
    EXPECT_EQ(1000u, repo2.findAll().size());
}

TEST_F(MemberRepoSafetyTest, Create500DeleteEvenIdsReload) {
    {
        MemberRepository repo(path_);
        for (int i = 0; i < 500; ++i)
            repo.create(makeMember("M" + std::to_string(i),
                                   "m" + std::to_string(i) + "@b.com", 20));
        for (const auto& m : repo.findAll())
            if (m.id % 2 == 0) repo.remove(m.id);
    }
    MemberRepository repo2(path_);
    EXPECT_EQ(250u, repo2.findAll().size());
}

TEST_F(MemberRepoSafetyTest, DeleteAllThenCreateGetsNextId) {
    MemberRepository repo(path_);
    for (int i = 0; i < 1000; ++i)
        repo.create(makeMember("M" + std::to_string(i),
                               "m" + std::to_string(i) + "@b.com", 20));
    for (const auto& m : repo.findAll())
        repo.remove(m.id);
    auto newMember = repo.create(makeMember("New", "new@b.com", 30));
    EXPECT_EQ(1001LL, newMember.id);
}
