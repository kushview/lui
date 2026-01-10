// Copyright 2019 Michael Fisher <mfisher@lvtk.org>
// SPDX-License-Identifier: ISC

#include "tests.hpp"
#include "lui/weak_ref.hpp"
#include <memory>

class TestObject {
public:
    TestObject() { weak_status.reset (this); }
    virtual ~TestObject() { weak_status.reset(); }

private:
    LUI_WEAK_REFABLE_WITH_MEMBER (TestObject, weak_status)
};

class SubObject : public TestObject {};

using TestRef = lui::WeakRef<TestObject>;

TEST(WeakRef, basics) {
    auto uptr1   = std::make_unique<TestObject>();
    TestRef ref1 = uptr1.get();
    EXPECT_NE (ref1.lock(), nullptr);
    EXPECT_TRUE (ref1.valid());
    EXPECT_NE (ref1, nullptr);
    uptr1.reset();
    EXPECT_EQ (ref1.lock(), nullptr);
    EXPECT_FALSE (ref1.valid());
    EXPECT_EQ (ref1, nullptr);
}

TEST(WeakRef, subclass) {
    auto uptr1   = std::make_unique<SubObject>();
    TestRef ref1 = uptr1.get();
    EXPECT_NE (ref1.lock(), nullptr);
    EXPECT_TRUE (ref1.valid());
    EXPECT_NE (ref1.as<SubObject>(), nullptr);
    uptr1.reset();
    EXPECT_EQ (ref1.lock(), nullptr);
    EXPECT_FALSE (ref1.valid());
    EXPECT_EQ (ref1.as<SubObject>(), nullptr);
}
