// Copyright 2026 Michael Fisher <mfisher@lvtk.org>
// SPDX-License-Identifier: ISC

#include <gtest/gtest.h>
#include <lui/range.hpp>

using namespace lui;

TEST(Range, default_constructor) {
    Range<int> r;
    EXPECT_EQ (r.min, 0);
    EXPECT_EQ (r.max, 0);
    EXPECT_TRUE (r.empty());
}

TEST(Range, constructor_with_values) {
    Range<int> r (10, 100);
    EXPECT_EQ (r.min, 10);
    EXPECT_EQ (r.max, 100);
    EXPECT_FALSE (r.empty());
}

TEST(Range, empty) {
    Range<int> r1 (0, 0);
    EXPECT_TRUE (r1.empty());

    Range<int> r2 (5, 5);
    EXPECT_TRUE (r2.empty());

    Range<int> r3 (0, 10);
    EXPECT_FALSE (r3.empty());
}

TEST(Range, diff) {
    Range<int> r1 (0, 100);
    EXPECT_EQ (r1.diff(), 100);

    Range<int> r2 (50, 150);
    EXPECT_EQ (r2.diff(), 100);

    Range<double> r3 (0.5, 2.5);
    EXPECT_DOUBLE_EQ (r3.diff(), 2.0);
}

TEST(Range, ratio) {
    Range<int> r (0, 100);
    EXPECT_DOUBLE_EQ (r.ratio (0), 0.0);
    EXPECT_DOUBLE_EQ (r.ratio (50), 0.5);
    EXPECT_DOUBLE_EQ (r.ratio (100), 1.0);
    EXPECT_DOUBLE_EQ (r.ratio (25), 0.25);
    EXPECT_DOUBLE_EQ (r.ratio (75), 0.75);
}

TEST(Range, ratio_offset_range) {
    Range<int> r (50, 150);
    EXPECT_DOUBLE_EQ (r.ratio (50), 0.0);
    EXPECT_DOUBLE_EQ (r.ratio (100), 0.5);
    EXPECT_DOUBLE_EQ (r.ratio (150), 1.0);
}

TEST(Range, ratio_floating_point) {
    Range<double> r (0.0, 1.0);
    EXPECT_DOUBLE_EQ (r.ratio (0.0), 0.0);
    EXPECT_DOUBLE_EQ (r.ratio (0.25), 0.25);
    EXPECT_DOUBLE_EQ (r.ratio (0.5), 0.5);
    EXPECT_DOUBLE_EQ (r.ratio (0.75), 0.75);
    EXPECT_DOUBLE_EQ (r.ratio (1.0), 1.0);
}

TEST(Range, convert) {
    Range<int> r1 (0, 100);
    Range<int> r2 (0, 10);

    // Convert value from r2's range to r1's range
    EXPECT_EQ (r1.convert (r2, 0), 0);
    EXPECT_EQ (r1.convert (r2, 5), 50);
    EXPECT_EQ (r1.convert (r2, 10), 100);

    // Convert value from r1's range to r2's range
    EXPECT_EQ (r2.convert (r1, 0), 0);
    EXPECT_EQ (r2.convert (r1, 50), 5);
    EXPECT_EQ (r2.convert (r1, 100), 10);
}

TEST(Range, convert_offset_ranges) {
    Range<int> r1 (50, 150);  // 100 units wide
    Range<int> r2 (0, 10);    // 10 units wide

    // Convert from r2 to r1
    EXPECT_EQ (r1.convert (r2, 0), 50);
    EXPECT_EQ (r1.convert (r2, 5), 100);
    EXPECT_EQ (r1.convert (r2, 10), 150);

    // Convert from r1 to r2
    EXPECT_EQ (r2.convert (r1, 50), 0);
    EXPECT_EQ (r2.convert (r1, 100), 5);
    EXPECT_EQ (r2.convert (r1, 150), 10);
}

TEST(Range, convert_floating_point) {
    Range<double> r1 (0.0, 1.0);
    Range<double> r2 (0.0, 100.0);

    EXPECT_DOUBLE_EQ (r1.convert (r2, 0.0), 0.0);
    EXPECT_DOUBLE_EQ (r1.convert (r2, 50.0), 0.5);
    EXPECT_DOUBLE_EQ (r1.convert (r2, 100.0), 1.0);

    EXPECT_DOUBLE_EQ (r2.convert (r1, 0.0), 0.0);
    EXPECT_DOUBLE_EQ (r2.convert (r1, 0.5), 50.0);
    EXPECT_DOUBLE_EQ (r2.convert (r1, 1.0), 100.0);
}

TEST(Range, equality_operators) {
    Range<int> r1 (0, 100);
    Range<int> r2 (0, 100);
    Range<int> r3 (0, 50);

    EXPECT_TRUE (r1 == r2);
    EXPECT_FALSE (r1 == r3);
    EXPECT_FALSE (r1 != r2);
    EXPECT_TRUE (r1 != r3);
}

TEST(Range, different_types) {
    Range<float> rf (0.0f, 100.0f);
    EXPECT_FLOAT_EQ (rf.diff(), 100.0f);
    EXPECT_FLOAT_EQ (static_cast<float> (rf.ratio (50.0f)), 0.5f);

    Range<long> rl (0L, 1000L);
    EXPECT_EQ (rl.diff(), 1000L);
    EXPECT_DOUBLE_EQ (rl.ratio (500L), 0.5);
}
