#include <gtest/gtest.h>
#include "MemberRepository.h"
#include "MemberService.h"
#include <filesystem>
#include <atomic>

class MemberSvcTest : public ::testing::Test {
protected:
    std::filesystem::path             path_;
    std::unique_ptr<MemberRepository> repo_;
    std::unique_ptr<MemberService>    svc_;

    void SetUp() override {
        static std::atomic<int> cnt{ 0 };
        path_ = std::filesystem::temp_directory_path() /
                ("svc_fixture_" + std::to_string(++cnt) + ".json");
        repo_ = std::make_unique<MemberRepository>(path_);
        svc_  = std::make_unique<MemberService>(*repo_);
    }

    void TearDown() override {
        svc_.reset();
        repo_.reset();
        std::filesystem::remove(path_);
    }
};

TEST_F(MemberSvcTest, CreateSuccess) {
    auto m = svc_->create("홍길동", "hong@ex.com", 30);
    EXPECT_GT(m.id, 0LL);
    EXPECT_EQ(std::string("홍길동"),      m.name);
    EXPECT_EQ(std::string("hong@ex.com"), m.email);
    EXPECT_EQ(30LL,                       m.age);
    EXPECT_FALSE(m.createdAt.empty());
}

TEST_F(MemberSvcTest, CreateRejectsEmptyName) {
    EXPECT_THROW(svc_->create("", "a@b.com", 20), std::invalid_argument);
}

TEST_F(MemberSvcTest, CreateRejectsInvalidEmail) {
    EXPECT_THROW(svc_->create("홍길동", "invalid-email", 20), std::invalid_argument);
    EXPECT_THROW(svc_->create("홍길동", "noemail",       20), std::invalid_argument);
}

TEST_F(MemberSvcTest, CreateRejectsDuplicateEmail) {
    svc_->create("홍길동", "dup@ex.com", 20);
    EXPECT_THROW(svc_->create("김철수", "dup@ex.com", 25), std::invalid_argument);
}

TEST_F(MemberSvcTest, CreateRejectsOutOfRangeAge) {
    EXPECT_THROW(svc_->create("A", "a@b.com",   0), std::invalid_argument);
    EXPECT_THROW(svc_->create("B", "b@b.com", 151), std::invalid_argument);
    EXPECT_THROW(svc_->create("C", "c@b.com",  -1), std::invalid_argument);
}

TEST_F(MemberSvcTest, CreateAcceptsBoundaryAge) {
    EXPECT_EQ(1LL,   svc_->create("A", "a@b.com",   1).age);
    EXPECT_EQ(150LL, svc_->create("B", "b@b.com", 150).age);
}

TEST_F(MemberSvcTest, UpdateName) {
    auto m = svc_->create("Before", "x@b.com", 20);
    auto updated = svc_->update(m.id, "name", "After");
    EXPECT_EQ(std::string("After"), updated.name);
    EXPECT_EQ(std::string("After"), svc_->findById(m.id)->name);
}

TEST_F(MemberSvcTest, UpdateEmail) {
    auto m = svc_->create("Test", "old@b.com", 20);
    auto updated = svc_->update(m.id, "email", "new@b.com");
    EXPECT_EQ(std::string("new@b.com"), updated.email);
    EXPECT_EQ(std::string("new@b.com"), svc_->findById(m.id)->email);
}

TEST_F(MemberSvcTest, UpdateAge) {
    auto m = svc_->create("Test", "t@b.com", 20);
    EXPECT_EQ(35LL, svc_->update(m.id, "age", "35").age);
}

TEST_F(MemberSvcTest, UpdateAgeRejectsNonNumeric) {
    auto m = svc_->create("Test", "t@b.com", 20);
    EXPECT_THROW(svc_->update(m.id, "age", "abc"), std::runtime_error);
    EXPECT_THROW(svc_->update(m.id, "age", ""),    std::runtime_error);
}

TEST_F(MemberSvcTest, UpdateRejectsMissingId) {
    EXPECT_THROW(svc_->update(999, "name", "X"), std::runtime_error);
}

TEST_F(MemberSvcTest, UpdateEmailSelfAllowed) {
    auto m = svc_->create("Test", "self@b.com", 20);
    EXPECT_EQ(std::string("self@b.com"), svc_->update(m.id, "email", "self@b.com").email);
}

TEST_F(MemberSvcTest, DeleteSuccess) {
    auto m = svc_->create("Del", "d@b.com", 20);
    EXPECT_TRUE(svc_->remove(m.id));
    EXPECT_FALSE(svc_->findById(m.id).has_value());
}

TEST_F(MemberSvcTest, DeleteRejectsMissingId) {
    EXPECT_THROW(svc_->remove(999), std::runtime_error);
}

TEST_F(MemberSvcTest, SearchByName) {
    svc_->create("홍길동", "a@b.com", 20);
    svc_->create("홍길순", "b@b.com", 21);
    svc_->create("김철수", "c@b.com", 22);
    EXPECT_EQ(2u, svc_->search("홍").size());
}

TEST_F(MemberSvcTest, SearchByEmail) {
    svc_->create("A", "test1@domain.com", 20);
    svc_->create("B", "test2@domain.com", 21);
    svc_->create("C", "other@other.com",  22);
    EXPECT_EQ(2u, svc_->search("domain.com").size());
}

TEST_F(MemberSvcTest, SearchNoMatch) {
    svc_->create("A", "a@b.com", 20);
    EXPECT_TRUE(svc_->search("zzz_no_match").empty());
}
