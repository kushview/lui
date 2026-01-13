// Copyright 2026 Kushview, LLC
// SPDX-License-Identifier: ISC

#include <gtest/gtest.h>
#include <lui/color.hpp>

using namespace lui;

TEST (Color, uint32_constructor_format) {
    // format is 0xAARRGGBB
    // Red: alpha=FF, red=FF, green=00, blue=00
    Color red (0xffff0000);
    EXPECT_EQ (red.alpha(), 0xff);
    EXPECT_EQ (red.red(), 0xff);
    EXPECT_EQ (red.green(), 0x00);
    EXPECT_EQ (red.blue(), 0x00);

    // Green: alpha=FF, red=00, green=FF, blue=00
    Color green (0xff00ff00);
    EXPECT_EQ (green.alpha(), 0xff);
    EXPECT_EQ (green.red(), 0x00);
    EXPECT_EQ (green.green(), 0xff);
    EXPECT_EQ (green.blue(), 0x00);

    // Blue: alpha=FF, red=00, green=00, blue=FF
    Color blue (0xff0000ff);
    EXPECT_EQ (blue.alpha(), 0xff);
    EXPECT_EQ (blue.red(), 0x00);
    EXPECT_EQ (blue.green(), 0x00);
    EXPECT_EQ (blue.blue(), 0xff);

    // Semi-transparent white: alpha=80, red=FF, green=FF, blue=FF
    Color white_semi (0x80ffffff);
    EXPECT_EQ (white_semi.alpha(), 0x80);
    EXPECT_EQ (white_semi.red(), 0xff);
    EXPECT_EQ (white_semi.green(), 0xff);
    EXPECT_EQ (white_semi.blue(), 0xff);
}

TEST (Color, rgba_constructor) {
    Color c (255, 128, 64, 32);
    EXPECT_EQ (c.red(), 255);
    EXPECT_EQ (c.green(), 128);
    EXPECT_EQ (c.blue(), 64);
    EXPECT_EQ (c.alpha(), 32);
}

TEST (Color, rgb_constructor) {
    Color c (255, 128, 64);
    EXPECT_EQ (c.red(), 255);
    EXPECT_EQ (c.green(), 128);
    EXPECT_EQ (c.blue(), 64);
    EXPECT_EQ (c.alpha(), 255); // Should be fully opaque
}

TEST (Color, float_constructor) {
    Color c (1.0f, 0.5f, 0.25f, 0.125f);
    EXPECT_EQ (c.red(), 255);
    EXPECT_EQ (c.green(), 127); // 0.5 * 255 = 127.5, rounds to 127
    EXPECT_EQ (c.blue(), 63);   // 0.25 * 255 = 63.75, rounds to 63
    EXPECT_EQ (c.alpha(), 31);  // 0.125 * 255 = 31.875, rounds to 31
}

TEST (Color, float_accessors) {
    Color c (255, 128, 64, 32);
    EXPECT_FLOAT_EQ (c.fred(), 1.0f);
    EXPECT_FLOAT_EQ (c.fgreen(), 128.0f / 255.0f);
    EXPECT_FLOAT_EQ (c.fblue(), 64.0f / 255.0f);
    EXPECT_FLOAT_EQ (c.falpha(), 32.0f / 255.0f);
}

TEST (Color, brighter) {
    Color c (100, 50, 25, 255);
    Color brighter_color = c.brighter();
    
    // Brighter should increase RGB values
    EXPECT_GT (brighter_color.red(), c.red());
    EXPECT_GT (brighter_color.green(), c.green());
    EXPECT_GT (brighter_color.blue(), c.blue());
    EXPECT_EQ (brighter_color.alpha(), c.alpha()); // Alpha unchanged
}

TEST (Color, darker) {
    Color c (200, 150, 100, 255);
    Color darker_color = c.darker();
    
    // Darker should decrease RGB values
    EXPECT_LT (darker_color.red(), c.red());
    EXPECT_LT (darker_color.green(), c.green());
    EXPECT_LT (darker_color.blue(), c.blue());
    EXPECT_EQ (darker_color.alpha(), c.alpha()); // Alpha unchanged
}

TEST (Color, with_alpha_float) {
    Color c (255, 128, 64, 255);
    Color semi = c.with_alpha (0.5f);
    
    EXPECT_EQ (semi.red(), 255);
    EXPECT_EQ (semi.green(), 128);
    EXPECT_EQ (semi.blue(), 64);
    EXPECT_EQ (semi.alpha(), 127); // 0.5 * 255
}

TEST (Color, with_alpha_int) {
    Color c (255, 128, 64, 255);
    Color semi = c.with_alpha (128);
    
    EXPECT_EQ (semi.red(), 255);
    EXPECT_EQ (semi.green(), 128);
    EXPECT_EQ (semi.blue(), 64);
    EXPECT_EQ (semi.alpha(), 128);
}

TEST (Color, equality_operators) {
    Color c1 (255, 128, 64, 32);
    Color c2 (255, 128, 64, 32);
    Color c3 (255, 128, 64, 33);
    
    EXPECT_EQ (c1, c2);
    EXPECT_NE (c1, c3);
}

TEST (Color, copy_constructor_and_assignment) {
    Color c1 (255, 128, 64, 32);
    Color c2 (c1);
    Color c3 = c1;
    
    EXPECT_EQ (c1, c2);
    EXPECT_EQ (c1, c3);
    EXPECT_EQ (c2.red(), 255);
    EXPECT_EQ (c2.green(), 128);
    EXPECT_EQ (c2.blue(), 64);
    EXPECT_EQ (c2.alpha(), 32);
}

TEST (Color, default_constructor) {
    Color c;
    // Default should have alpha channel set
    EXPECT_EQ (c.alpha(), 0xff);
}
