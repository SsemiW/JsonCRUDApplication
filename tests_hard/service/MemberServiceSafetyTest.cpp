#include <gtest/gtest.h>
#include "MemberRepository.h"
#include "MemberService.h"
#include <filesystem>
#include <atomic>
#include <climits>

class MemberSvcSafetyTest : public ::testing::Test {
protected:
    std::filesystem::path             path_;
    std::unique_ptr<MemberRepository> repo_;
    std::unique_ptr<MemberService>    svc_;

    void SetUp() override {
        static std::atomic<int> cnt{ 0 };
        path_ = std::filesystem::temp_directory_path() /
                ("svc_safety_" + std::to_string(++cnt) + ".json");
        repo_ = std::make_unique<MemberRepository>(path_);
        svc_  = std::make_unique<MemberService>(*repo_);
    }

    void TearDown() override {
        svc_.reset();
        repo_.reset();
        std::filesystem::remove(path_);
    }
};

// ── 6-1. 이름 극단값 ────────────────────────────────────────────────────

TEST_F(MemberSvcSafetyTest, SingleSpaceNameAllowed) {
    // validateName: empty() 만 검사 — 공백은 통과
    EXPECT_NO_THROW(svc_->create(" ", "a@b.com", 20));
}

TEST_F(MemberSvcSafetyTest, TabNewlineNameAllowed) {
    EXPECT_NO_THROW(svc_->create("\t\n", "b@b.com", 20));
}

TEST_F(MemberSvcSafetyTest, LongNameAllowed) {
    EXPECT_NO_THROW(svc_->create(std::string(10000, 'A'), "c@b.com", 20));
}

TEST_F(MemberSvcSafetyTest, EmojiNameAllowed) {
    EXPECT_NO_THROW(svc_->create("홍길동\xF0\x9F\x98\x80", "d@b.com", 20));
}

TEST_F(MemberSvcSafetyTest, SqlInjectionNameStored) {
    std::string sql = "'; DROP TABLE members; --";
    auto m = svc_->create(sql, "e@b.com", 20);
    EXPECT_EQ(sql, svc_->findById(m.id)->name);
}

TEST_F(MemberSvcSafetyTest, HtmlTagNameStored) {
    std::string html = "<script>alert(1)</script>";
    auto m = svc_->create(html, "f@b.com", 20);
    EXPECT_EQ(html, svc_->findById(m.id)->name);
}

TEST_F(MemberSvcSafetyTest, PathTraversalNameStored) {
    std::string path = "../../../etc/passwd";
    auto m = svc_->create(path, "g@b.com", 20);
    EXPECT_EQ(path, svc_->findById(m.id)->name);
}

// ── 6-2. 이메일 극단값 ──────────────────────────────────────────────────

TEST_F(MemberSvcSafetyTest, AtOnlyEmailAllowed) {
    // '@' 포함 조건만 있음
    EXPECT_NO_THROW(svc_->create("A", "@", 20));
}

TEST_F(MemberSvcSafetyTest, MultipleAtEmailAllowed) {
    EXPECT_NO_THROW(svc_->create("A", "@@@@", 21));
}

TEST_F(MemberSvcSafetyTest, DoubleAtEmailAllowed) {
    EXPECT_NO_THROW(svc_->create("A", "a@b@c", 22));
}

TEST_F(MemberSvcSafetyTest, LongEmailAllowed) {
    std::string longEmail = std::string(5000, 'a') + "@" + std::string(4999, 'b');
    EXPECT_NO_THROW(svc_->create("A", longEmail, 23));
}

TEST_F(MemberSvcSafetyTest, SpaceInEmailAllowed) {
    EXPECT_NO_THROW(svc_->create("A", "a b@c.com", 24));
}

TEST_F(MemberSvcSafetyTest, CaseDifferentEmailTreatedAsSeparate) {
    // 대소문자 이메일은 별개로 처리
    EXPECT_NO_THROW({
        svc_->create("A", "A@B.COM", 20);
        svc_->create("B", "a@b.com", 21);
        EXPECT_EQ(2u, svc_->findAll().size());
    });
}

// ── 6-3. 나이 극단값 ────────────────────────────────────────────────────

TEST_F(MemberSvcSafetyTest, AgeZeroThrows) {
    EXPECT_THROW(svc_->create("A", "a@b.com", 0), std::invalid_argument);
}

TEST_F(MemberSvcSafetyTest, AgeNegOneThrows) {
    EXPECT_THROW(svc_->create("A", "b@b.com", -1), std::invalid_argument);
}

TEST_F(MemberSvcSafetyTest, AgeInt64MinThrows) {
    EXPECT_THROW(svc_->create("A", "c@b.com", INT64_MIN), std::invalid_argument);
}

TEST_F(MemberSvcSafetyTest, Age151Throws) {
    EXPECT_THROW(svc_->create("A", "d@b.com", 151), std::invalid_argument);
}

TEST_F(MemberSvcSafetyTest, AgeInt64MaxThrows) {
    EXPECT_THROW(svc_->create("A", "e@b.com", INT64_MAX), std::invalid_argument);
}

TEST_F(MemberSvcSafetyTest, AgeBoundaryAllowed) {
    EXPECT_NO_THROW(svc_->create("A", "f@b.com",   1));
    EXPECT_NO_THROW(svc_->create("B", "g@b.com", 150));
}

// ── 6-4. Update 극단값 ──────────────────────────────────────────────────

TEST_F(MemberSvcSafetyTest, UpdateAgeFractionalStringThrows) {
    auto m = svc_->create("A", "a@b.com", 20);
    EXPECT_THROW(svc_->update(m.id, "age", "1.5"), std::runtime_error);
}

TEST_F(MemberSvcSafetyTest, UpdateAgeOverflowStringThrows) {
    auto m = svc_->create("A", "a@b.com", 20);
    EXPECT_THROW(svc_->update(m.id, "age", "99999999999999999999"), std::runtime_error);
}

TEST_F(MemberSvcSafetyTest, UpdateAgeEmptyStringThrows) {
    auto m = svc_->create("A", "a@b.com", 20);
    EXPECT_THROW(svc_->update(m.id, "age", ""), std::runtime_error);
}

TEST_F(MemberSvcSafetyTest, UpdateAgeSpacedOutOfRangeThrows) {
    // pos != size() 로 먼저 걸림 → runtime_error
    auto m = svc_->create("A", "a@b.com", 20);
    EXPECT_THROW(svc_->update(m.id, "age", "  151  "), std::runtime_error);
}

TEST_F(MemberSvcSafetyTest, UpdateUnknownFieldThrows) {
    auto m = svc_->create("A", "a@b.com", 20);
    EXPECT_THROW(svc_->update(m.id, "nickname", "B"), std::runtime_error);
}

TEST_F(MemberSvcSafetyTest, UpdateDeletedIdThrows) {
    auto m = svc_->create("A", "a@b.com", 20);
    svc_->remove(m.id);
    EXPECT_THROW(svc_->update(m.id, "name", "B"), std::runtime_error);
}

TEST_F(MemberSvcSafetyTest, UpdateEmailSelfAllowed) {
    auto m = svc_->create("A", "self@b.com", 20);
    EXPECT_NO_THROW(svc_->update(m.id, "email", "self@b.com"));
}

TEST_F(MemberSvcSafetyTest, UpdateEmailDuplicateOtherThrows) {
    svc_->create("A", "other@b.com", 20);
    auto m = svc_->create("B", "b@b.com", 21);
    EXPECT_THROW(svc_->update(m.id, "email", "other@b.com"), std::invalid_argument);
}

// ── 6-5. 연쇄 파괴 시나리오 ─────────────────────────────────────────────

TEST_F(MemberSvcSafetyTest, CreateDeleteAllThenCreateGetsNextId) {
    for (int i = 0; i < 10; ++i)
        svc_->create("M" + std::to_string(i),
                     "m" + std::to_string(i) + "@b.com", 20);
    for (const auto& m : svc_->findAll())
        svc_->remove(m.id);
    auto newMember = svc_->create("New", "new@b.com", 30);
    EXPECT_EQ(11LL, newMember.id);
}

TEST_F(MemberSvcSafetyTest, SameEmail1000TimesOnlyOneStored) {
    svc_->create("First", "dup@b.com", 20);
    for (int i = 0; i < 999; ++i) {
        try { svc_->create("X" + std::to_string(i), "dup@b.com", 20); }
        catch (const std::invalid_argument&) {}
    }
    EXPECT_EQ(1u, svc_->findAll().size());
}

TEST_F(MemberSvcSafetyTest, Update1000TimesNamePreservesLast) {
    auto m = svc_->create("Init", "a@b.com", 20);
    std::string lastValue;
    for (int i = 0; i < 1000; ++i) {
        lastValue = "Name" + std::to_string(i);
        svc_->update(m.id, "name", lastValue);
    }
    EXPECT_EQ(lastValue, svc_->findById(m.id)->name);
}

TEST_F(MemberSvcSafetyTest, SearchEmptyKeywordNocrash) {
    svc_->create("A", "a@b.com", 20);
    svc_->create("B", "b@b.com", 21);
    EXPECT_NO_THROW({
        auto result = svc_->search("");
        EXPECT_EQ(2u, result.size());
    });
}

TEST_F(MemberSvcSafetyTest, SearchAtSymbolMatchesAllEmails) {
    svc_->create("A", "a@b.com", 20);
    svc_->create("B", "b@b.com", 21);
    svc_->create("C", "c@b.com", 22);
    auto result = svc_->search("@");
    EXPECT_EQ(3u, result.size());
}

TEST_F(MemberSvcSafetyTest, Remove1000MissingIdNocrash) {
    EXPECT_NO_THROW({
        for (int i = 0; i < 1000; ++i) {
            try { svc_->remove(99999 + i); }
            catch (const std::runtime_error&) {}
        }
    });
}
