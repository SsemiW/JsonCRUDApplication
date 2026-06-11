#include <gtest/gtest.h>
#include "MemberRepository.h"
#include <filesystem>
#include <atomic>

// 파일 경로가 필요 없는 독립 테스트
TEST(MemberRepositoryMisc, AutoCreateFileWhenMissing) {
    static std::atomic<int> cnt{ 0 };
    auto path = std::filesystem::temp_directory_path() /
                ("mr_autocreate_" + std::to_string(++cnt) + ".json");
    ASSERT_FALSE(std::filesystem::exists(path));
    {
        MemberRepository repo(path);
        EXPECT_TRUE(repo.findAll().empty());
        EXPECT_TRUE(std::filesystem::exists(path));
    }
    std::filesystem::remove(path);
}

TEST(MemberRepositoryMisc, CreatePersistsToFile) {
    static std::atomic<int> cnt{ 0 };
    auto path = std::filesystem::temp_directory_path() /
                ("mr_persist_" + std::to_string(++cnt) + ".json");
    {
        MemberRepository repo(path);
        Member m; m.name = "Persist"; m.email = "p@x.com"; m.age = 30; m.createdAt = "t";
        repo.create(m);
    }
    {
        MemberRepository repo2(path);
        auto all = repo2.findAll();
        ASSERT_EQ(1u, all.size());
        EXPECT_EQ(std::string("Persist"), all[0].name);
    }
    std::filesystem::remove(path);
}

// 픽스처 — 각 테스트마다 임시 파일 생성/삭제
class MemberRepoTest : public ::testing::Test {
protected:
    std::filesystem::path             path_;
    std::unique_ptr<MemberRepository> repo_;

    void SetUp() override {
        static std::atomic<int> cnt{ 0 };
        path_ = std::filesystem::temp_directory_path() /
                ("mr_fixture_" + std::to_string(++cnt) + ".json");
        repo_ = std::make_unique<MemberRepository>(path_);
    }

    void TearDown() override {
        repo_.reset();
        std::filesystem::remove(path_);
    }

    Member make(const std::string& name, const std::string& email, int64_t age) {
        Member m; m.name = name; m.email = email; m.age = age; m.createdAt = "t";
        return repo_->create(m);
    }
};

TEST_F(MemberRepoTest, CreateAutoIncrementId) {
    auto r1 = make("A", "a@x.com", 20);
    auto r2 = make("B", "b@x.com", 21);
    EXPECT_EQ(1LL, r1.id);
    EXPECT_EQ(2LL, r2.id);
}

TEST_F(MemberRepoTest, FindAllReturnsAll) {
    EXPECT_EQ(0u, repo_->findAll().size());
    make("A", "a@x.com", 20);
    make("B", "b@x.com", 21);
    EXPECT_EQ(2u, repo_->findAll().size());
}

TEST_F(MemberRepoTest, FindByIdFound) {
    auto created = make("Find", "f@x.com", 25);
    auto found   = repo_->findById(created.id);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(std::string("Find"), found->name);
    EXPECT_EQ(created.id,          found->id);
}

TEST_F(MemberRepoTest, FindByIdNotFound) {
    EXPECT_FALSE(repo_->findById(999).has_value());
    EXPECT_FALSE(repo_->findById(0).has_value());
}

TEST_F(MemberRepoTest, FindByNamePartialMatch) {
    make("홍길동", "a@x.com", 30);
    make("홍길순", "b@x.com", 25);
    make("김철수", "c@x.com", 20);

    EXPECT_EQ(2u, repo_->findByName("홍").size());
    EXPECT_EQ(0u, repo_->findByName("없는이름").size());
}

TEST_F(MemberRepoTest, UpdateModifiesField) {
    auto created = make("Before", "b@x.com", 20);
    created.name = "After";
    EXPECT_TRUE(repo_->update(created));
    EXPECT_EQ(std::string("After"), repo_->findById(created.id)->name);
}

TEST_F(MemberRepoTest, UpdateReturnsFalseForMissing) {
    Member m; m.id = 999; m.name = "X"; m.email = "x@x.com"; m.age = 20; m.createdAt = "t";
    EXPECT_FALSE(repo_->update(m));
}

TEST_F(MemberRepoTest, RemoveDeletesMember) {
    auto created = make("Del", "d@x.com", 20);
    EXPECT_TRUE(repo_->remove(created.id));
    EXPECT_FALSE(repo_->findById(created.id).has_value());
    EXPECT_EQ(0u, repo_->findAll().size());
}

TEST_F(MemberRepoTest, RemoveReturnsFalseForMissing) {
    EXPECT_FALSE(repo_->remove(999));
}

TEST_F(MemberRepoTest, NextIdNotReused) {
    auto c1 = make("A", "a@x.com", 20);
    ASSERT_TRUE(repo_->remove(c1.id));
    auto c2 = make("B", "b@x.com", 21);
    EXPECT_NE(c1.id, c2.id);
    EXPECT_GT(c2.id, c1.id);
}
