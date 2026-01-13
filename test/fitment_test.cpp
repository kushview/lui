// Copyright 2024 Kushview, LLC
// SPDX-License-Identifier: ISC

#include <gtest/gtest.h>
#include <lui/fitment.hpp>

using namespace lui;

TEST(Fitment, default_constructor) {
    Fitment fit;
    EXPECT_EQ (fit.flags(), Fitment::CENTERED);
}

TEST(Fitment, flag_constructor) {
    Fitment fit (Fitment::LEFT | Fitment::TOP);
    EXPECT_EQ (fit.flags(), Fitment::LEFT | Fitment::TOP);
}

TEST(Fitment, assignment_operator) {
    Fitment fit;
    fit = Fitment::RIGHT | Fitment::BOTTOM;
    EXPECT_EQ (fit.flags(), Fitment::RIGHT | Fitment::BOTTOM);
}

TEST(Fitment, equality_operators) {
    Fitment fit1 (Fitment::CENTERED);
    Fitment fit2 (Fitment::CENTERED);
    Fitment fit3 (Fitment::LEFT | Fitment::TOP);

    EXPECT_TRUE (fit1 == fit2);
    EXPECT_FALSE (fit1 != fit2);
    EXPECT_FALSE (fit1 == fit3);
    EXPECT_TRUE (fit1 != fit3);
}

TEST(Fitment, apply_stretch) {
    Fitment fit (Fitment::STRETCH);
    
    double x = 10.0, y = 20.0, w = 50.0, h = 30.0;
    double dx = 0.0, dy = 0.0, dw = 100.0, dh = 80.0;
    
    fit.apply (x, y, w, h, dx, dy, dw, dh);
    
    EXPECT_EQ (x, dx);
    EXPECT_EQ (y, dy);
    EXPECT_EQ (w, dw);
    EXPECT_EQ (h, dh);
}

TEST(Fitment, apply_centered) {
    Fitment fit (Fitment::CENTERED);
    
    double x = 0.0, y = 0.0, w = 50.0, h = 30.0;
    double dx = 0.0, dy = 0.0, dw = 100.0, dh = 80.0;
    
    fit.apply (x, y, w, h, dx, dy, dw, dh);
    
    // Scale should be min(100/50, 80/30) = min(2.0, 2.666...) = 2.0
    // New size: w = 50 * 2 = 100, h = 30 * 2 = 60
    EXPECT_DOUBLE_EQ (w, 100.0);
    EXPECT_DOUBLE_EQ (h, 60.0);
    
    // Centered: x = 0 + (100 - 100) / 2 = 0, y = 0 + (80 - 60) / 2 = 10
    EXPECT_DOUBLE_EQ (x, 0.0);
    EXPECT_DOUBLE_EQ (y, 10.0);
}

TEST(Fitment, apply_left_top) {
    Fitment fit (Fitment::LEFT | Fitment::TOP);
    
    double x = 0.0, y = 0.0, w = 50.0, h = 30.0;
    double dx = 10.0, dy = 20.0, dw = 100.0, dh = 80.0;
    
    fit.apply (x, y, w, h, dx, dy, dw, dh);
    
    // Scale should be min(100/50, 80/30) = 2.0
    EXPECT_DOUBLE_EQ (w, 100.0);
    EXPECT_DOUBLE_EQ (h, 60.0);
    
    // Left and Top alignment
    EXPECT_DOUBLE_EQ (x, dx);
    EXPECT_DOUBLE_EQ (y, dy);
}

TEST(Fitment, apply_right_bottom) {
    Fitment fit (Fitment::RIGHT | Fitment::BOTTOM);
    
    double x = 0.0, y = 0.0, w = 50.0, h = 30.0;
    double dx = 10.0, dy = 20.0, dw = 100.0, dh = 80.0;
    
    fit.apply (x, y, w, h, dx, dy, dw, dh);
    
    // Scale should be min(100/50, 80/30) = 2.0
    // New size: w = 100, h = 60
    EXPECT_DOUBLE_EQ (w, 100.0);
    EXPECT_DOUBLE_EQ (h, 60.0);
    
    // Right: x = dx + dw - w = 10 + 100 - 100 = 10
    // Bottom: y = dy + dh - h = 20 + 80 - 60 = 40
    EXPECT_DOUBLE_EQ (x, 10.0);
    EXPECT_DOUBLE_EQ (y, 40.0);
}

TEST(Fitment, apply_fill) {
    Fitment fit (Fitment::FILL | Fitment::CENTERED);
    
    double x = 0.0, y = 0.0, w = 50.0, h = 30.0;
    double dx = 0.0, dy = 0.0, dw = 100.0, dh = 80.0;
    
    fit.apply (x, y, w, h, dx, dy, dw, dh);
    
    // Scale should be max(100/50, 80/30) = max(2.0, 2.666...) = 2.666...
    double expected_scale = 80.0 / 30.0;
    double expected_w = 50.0 * expected_scale;
    double expected_h = 30.0 * expected_scale;
    
    EXPECT_DOUBLE_EQ (w, expected_w);
    EXPECT_DOUBLE_EQ (h, expected_h);
    
    // Centered: x = (100 - expected_w) / 2, y = (80 - expected_h) / 2
    double expected_x = (100.0 - expected_w) / 2.0;
    double expected_y = (80.0 - expected_h) / 2.0;
    
    EXPECT_DOUBLE_EQ (x, expected_x);
    EXPECT_DOUBLE_EQ (y, expected_y);
}

TEST(Fitment, apply_no_grow) {
    Fitment fit (Fitment::CENTERED | Fitment::NO_GROW);
    
    // Source is smaller than destination, should not grow
    double x = 0.0, y = 0.0, w = 50.0, h = 30.0;
    double dx = 0.0, dy = 0.0, dw = 100.0, dh = 80.0;
    
    fit.apply (x, y, w, h, dx, dy, dw, dh);
    
    // Should not scale up, so w and h stay the same
    EXPECT_DOUBLE_EQ (w, 50.0);
    EXPECT_DOUBLE_EQ (h, 30.0);
    
    // Centered in destination
    EXPECT_DOUBLE_EQ (x, 25.0);
    EXPECT_DOUBLE_EQ (y, 25.0);
}

TEST(Fitment, apply_no_shrink) {
    Fitment fit (Fitment::CENTERED | Fitment::NO_SHRINK);
    
    // Source is larger than destination, should not shrink
    double x = 0.0, y = 0.0, w = 200.0, h = 150.0;
    double dx = 0.0, dy = 0.0, dw = 100.0, dh = 80.0;
    
    fit.apply (x, y, w, h, dx, dy, dw, dh);
    
    // Should not scale down, so w and h stay the same
    EXPECT_DOUBLE_EQ (w, 200.0);
    EXPECT_DOUBLE_EQ (h, 150.0);
    
    // Centered in destination (will overflow)
    EXPECT_DOUBLE_EQ (x, -50.0);
    EXPECT_DOUBLE_EQ (y, -35.0);
}

TEST(Fitment, apply_only_grow) {
    Fitment fit (Fitment::CENTERED | Fitment::ONLY_GROW);
    
    // ONLY_GROW is same as NO_SHRINK
    double x = 0.0, y = 0.0, w = 200.0, h = 150.0;
    double dx = 0.0, dy = 0.0, dw = 100.0, dh = 80.0;
    
    fit.apply (x, y, w, h, dx, dy, dw, dh);
    
    EXPECT_DOUBLE_EQ (w, 200.0);
    EXPECT_DOUBLE_EQ (h, 150.0);
}

TEST(Fitment, apply_only_shrink) {
    Fitment fit (Fitment::CENTERED | Fitment::ONLY_SHRINK);
    
    // ONLY_SHRINK is same as NO_GROW
    double x = 0.0, y = 0.0, w = 50.0, h = 30.0;
    double dx = 0.0, dy = 0.0, dw = 100.0, dh = 80.0;
    
    fit.apply (x, y, w, h, dx, dy, dw, dh);
    
    EXPECT_DOUBLE_EQ (w, 50.0);
    EXPECT_DOUBLE_EQ (h, 30.0);
}

TEST(Fitment, apply_no_resize) {
    Fitment fit (Fitment::CENTERED | Fitment::NO_RESIZE);
    
    double x = 0.0, y = 0.0, w = 50.0, h = 30.0;
    double dx = 0.0, dy = 0.0, dw = 100.0, dh = 80.0;
    
    fit.apply (x, y, w, h, dx, dy, dw, dh);
    
    // Should not resize at all
    EXPECT_DOUBLE_EQ (w, 50.0);
    EXPECT_DOUBLE_EQ (h, 30.0);
    
    // But should still center
    EXPECT_DOUBLE_EQ (x, 25.0);
    EXPECT_DOUBLE_EQ (y, 25.0);
}

TEST(Fitment, apply_zero_dimensions) {
    Fitment fit (Fitment::CENTERED);
    
    // Test with zero width
    double x = 10.0, y = 20.0, w = 0.0, h = 30.0;
    double dx = 0.0, dy = 0.0, dw = 100.0, dh = 80.0;
    
    fit.apply (x, y, w, h, dx, dy, dw, dh);
    
    // Should return early, no changes
    EXPECT_DOUBLE_EQ (x, 10.0);
    EXPECT_DOUBLE_EQ (y, 20.0);
    EXPECT_DOUBLE_EQ (w, 0.0);
    EXPECT_DOUBLE_EQ (h, 30.0);
    
    // Test with zero height
    x = 10.0; y = 20.0; w = 50.0; h = 0.0;
    fit.apply (x, y, w, h, dx, dy, dw, dh);
    
    EXPECT_DOUBLE_EQ (x, 10.0);
    EXPECT_DOUBLE_EQ (y, 20.0);
    EXPECT_DOUBLE_EQ (w, 50.0);
    EXPECT_DOUBLE_EQ (h, 0.0);
}

TEST(Fitment, transform_stretch) {
    Fitment fit (Fitment::STRETCH);
    
    Rectangle<double> src (10.0, 20.0, 50.0, 30.0);
    Rectangle<double> dst (0.0, 0.0, 100.0, 80.0);
    
    Transform t = fit.transform (src, dst);
    
    // Should scale x by 100/50 = 2.0, y by 80/30 = 2.666...
    // Translation: -src.x, -src.y, then scale, then translate to dst
    auto p1 = t.map (Point<double> (10.0, 20.0));
    auto p2 = t.map (Point<double> (60.0, 50.0));
    
    // Top-left corner (10, 20) should map to (0, 0)
    EXPECT_NEAR (p1.x, 0.0, 0.001);
    EXPECT_NEAR (p1.y, 0.0, 0.001);
    
    // Bottom-right corner (60, 50) should map to (100, 80)
    EXPECT_NEAR (p2.x, 100.0, 0.001);
    EXPECT_NEAR (p2.y, 80.0, 0.001);
}

TEST(Fitment, transform_centered) {
    Fitment fit (Fitment::CENTERED);
    
    Rectangle<double> src (0.0, 0.0, 50.0, 30.0);
    Rectangle<double> dst (0.0, 0.0, 100.0, 80.0);
    
    Transform t = fit.transform (src, dst);
    
    // Scale should be min(2.0, 2.666) = 2.0
    // Scaled size: 100x60
    // Centered offset: x = 0, y = 10
    
    auto p1 = t.map (Point<double> (0.0, 0.0));
    auto p2 = t.map (Point<double> (50.0, 30.0));
    
    // Top-left should map to (0, 10)
    EXPECT_NEAR (p1.x, 0.0, 0.001);
    EXPECT_NEAR (p1.y, 10.0, 0.001);
    
    // Bottom-right should map to (100, 70)
    EXPECT_NEAR (p2.x, 100.0, 0.001);
    EXPECT_NEAR (p2.y, 70.0, 0.001);
}

TEST(Fitment, transform_left_top) {
    Fitment fit (Fitment::LEFT | Fitment::TOP);
    
    Rectangle<double> src (0.0, 0.0, 50.0, 30.0);
    Rectangle<double> dst (10.0, 20.0, 100.0, 80.0);
    
    Transform t = fit.transform (src, dst);
    
    auto p1 = t.map (Point<double> (0.0, 0.0));
    auto p2 = t.map (Point<double> (50.0, 30.0));
    
    // Top-left should map to dst top-left (10, 20)
    EXPECT_NEAR (p1.x, 10.0, 0.001);
    EXPECT_NEAR (p1.y, 20.0, 0.001);
    
    // With scale 2.0, bottom-right maps to (110, 80)
    EXPECT_NEAR (p2.x, 110.0, 0.001);
    EXPECT_NEAR (p2.y, 80.0, 0.001);
}

TEST(Fitment, transform_right_bottom) {
    Fitment fit (Fitment::RIGHT | Fitment::BOTTOM);
    
    Rectangle<double> src (0.0, 0.0, 50.0, 30.0);
    Rectangle<double> dst (0.0, 0.0, 100.0, 80.0);
    
    Transform t = fit.transform (src, dst);
    
    // Scale 2.0, size becomes 100x60
    // Right-bottom aligned in 100x80: offset = (0, 20)
    
    auto p1 = t.map (Point<double> (0.0, 0.0));
    auto p2 = t.map (Point<double> (50.0, 30.0));
    
    // Top-left should map to (0, 20)
    EXPECT_NEAR (p1.x, 0.0, 0.001);
    EXPECT_NEAR (p1.y, 20.0, 0.001);
    
    // Bottom-right should map to (100, 80)
    EXPECT_NEAR (p2.x, 100.0, 0.001);
    EXPECT_NEAR (p2.y, 80.0, 0.001);
}

TEST(Fitment, transform_fill) {
    Fitment fit (Fitment::FILL | Fitment::CENTERED);
    
    Rectangle<double> src (0.0, 0.0, 50.0, 30.0);
    Rectangle<double> dst (0.0, 0.0, 100.0, 80.0);
    
    Transform t = fit.transform (src, dst);
    
    // Scale should be max(2.0, 2.666) = 2.666
    // Scaled size: 133.33x80
    // Centered: offset = (-16.666, 0)
    
    double scale = 80.0 / 30.0;
    double scaled_width = 50.0 * scale;
    double offset_x = (100.0 - scaled_width) / 2.0;
    
    auto p1 = t.map (Point<double> (0.0, 0.0));
    auto p2 = t.map (Point<double> (50.0, 30.0));
    
    EXPECT_NEAR (p1.x, offset_x, 0.001);
    EXPECT_NEAR (p1.y, 0.0, 0.001);
    
    EXPECT_NEAR (p2.x, offset_x + scaled_width, 0.001);
    EXPECT_NEAR (p2.y, 80.0, 0.001);
}

TEST(Fitment, transform_empty_source) {
    Fitment fit (Fitment::CENTERED);
    
    Rectangle<double> src (0.0, 0.0, 0.0, 0.0);
    Rectangle<double> dst (0.0, 0.0, 100.0, 80.0);
    
    Transform t = fit.transform (src, dst);
    
    // Should return identity transform
    auto p = t.map (Point<double> (10.0, 20.0));
    EXPECT_DOUBLE_EQ (p.x, 10.0);
    EXPECT_DOUBLE_EQ (p.y, 20.0);
}

TEST(Fitment, transform_no_grow) {
    Fitment fit (Fitment::CENTERED | Fitment::NO_GROW);
    
    Rectangle<double> src (0.0, 0.0, 50.0, 30.0);
    Rectangle<double> dst (0.0, 0.0, 100.0, 80.0);
    
    Transform t = fit.transform (src, dst);
    
    // Should not grow, scale = 1.0
    // Centered: offset = (25, 25)
    
    auto p1 = t.map (Point<double> (0.0, 0.0));
    auto p2 = t.map (Point<double> (50.0, 30.0));
    
    EXPECT_NEAR (p1.x, 25.0, 0.001);
    EXPECT_NEAR (p1.y, 25.0, 0.001);
    
    EXPECT_NEAR (p2.x, 75.0, 0.001);
    EXPECT_NEAR (p2.y, 55.0, 0.001);
}

TEST(Fitment, transform_no_shrink) {
    Fitment fit (Fitment::CENTERED | Fitment::NO_SHRINK);
    
    Rectangle<double> src (0.0, 0.0, 200.0, 150.0);
    Rectangle<double> dst (0.0, 0.0, 100.0, 80.0);
    
    Transform t = fit.transform (src, dst);
    
    // Should not shrink, scale = 1.0
    // Centered: offset = (-50, -35)
    
    auto p1 = t.map (Point<double> (0.0, 0.0));
    auto p2 = t.map (Point<double> (200.0, 150.0));
    
    EXPECT_NEAR (p1.x, -50.0, 0.001);
    EXPECT_NEAR (p1.y, -35.0, 0.001);
    
    EXPECT_NEAR (p2.x, 150.0, 0.001);
    EXPECT_NEAR (p2.y, 115.0, 0.001);
}
