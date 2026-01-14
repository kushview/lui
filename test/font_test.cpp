// Copyright 2019 Kushview, LLC
// SPDX-License-Identifier: ISC

#include "tests.hpp"
#include <lui/font.hpp>

TEST(Font, styles) {
    using Font = lui::Font;

    auto f = Font();
    EXPECT_EQ (f.normal(), true);
    EXPECT_EQ (f.bold(), false);
    EXPECT_EQ (f.italic(), false);
    EXPECT_EQ (f.underline(), false);

    f = f.with_style (Font::BOLD);
    EXPECT_EQ (f.normal(), false);
    EXPECT_EQ (f.bold(), true);
    EXPECT_EQ (f.italic(), false);
    EXPECT_EQ (f.underline(), false);

    f = f.with_style (f.flags() | Font::UNDERLINE);
    EXPECT_EQ (f.normal(), false);
    EXPECT_EQ (f.bold(), true);
    EXPECT_EQ (f.italic(), false);
    EXPECT_EQ (f.underline(), true);

    f = f.with_style (f.flags() | Font::ITALIC);
    EXPECT_EQ (f.normal(), false);
    EXPECT_EQ (f.bold(), true);
    EXPECT_EQ (f.italic(), true);
    EXPECT_EQ (f.underline(), true);

    auto f2          = Font (f.height(), Font::BOLD | Font::ITALIC | Font::UNDERLINE);
    auto fonts_equal = f == f2;
    EXPECT_EQ (fonts_equal, true);

    f2                   = f2.with_height (f2.height() - 1.f);
    auto fonts_not_equal = f != f2;
    EXPECT_EQ (fonts_not_equal, true);
}
