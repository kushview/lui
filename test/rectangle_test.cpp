// Copyright 2026 Kushview, LLC
// SPDX-License-Identifier: ISC

#include <gtest/gtest.h>
#include <lui/rectangle.hpp>

using namespace lui;

TEST(Rectangle, default_constructor) {
    Rectangle<int> r;
    EXPECT_EQ (r.x, 0);
    EXPECT_EQ (r.y, 0);
    EXPECT_EQ (r.width, 0);
    EXPECT_EQ (r.height, 0);
}

TEST(Rectangle, constructor_width_height) {
    Rectangle<int> r (100, 200);
    EXPECT_EQ (r.x, 0);
    EXPECT_EQ (r.y, 0);
    EXPECT_EQ (r.width, 100);
    EXPECT_EQ (r.height, 200);
}

TEST(Rectangle, constructor_full) {
    Rectangle<int> r (10, 20, 100, 200);
    EXPECT_EQ (r.x, 10);
    EXPECT_EQ (r.y, 20);
    EXPECT_EQ (r.width, 100);
    EXPECT_EQ (r.height, 200);
}

TEST(Rectangle, pos) {
    Rectangle<int> r (10, 20, 100, 200);
    auto p = r.pos();
    EXPECT_EQ (p.x, 10);
    EXPECT_EQ (p.y, 20);
}

TEST(Rectangle, empty) {
    Rectangle<int> r1 (0, 0);
    EXPECT_TRUE (r1.empty());

    Rectangle<int> r2 (100, 0);
    EXPECT_TRUE (r2.empty());

    Rectangle<int> r3 (0, 100);
    EXPECT_TRUE (r3.empty());

    Rectangle<int> r4 (100, 100);
    EXPECT_FALSE (r4.empty());

    Rectangle<int> r5 (-10, -20, 100, 100);
    EXPECT_FALSE (r5.empty());
}

TEST(Rectangle, as_conversion) {
    Rectangle<int> ri (10, 20, 100, 200);
    auto rf = ri.as<float>();
    EXPECT_FLOAT_EQ (rf.x, 10.0f);
    EXPECT_FLOAT_EQ (rf.y, 20.0f);
    EXPECT_FLOAT_EQ (rf.width, 100.0f);
    EXPECT_FLOAT_EQ (rf.height, 200.0f);

    Rectangle<float> rf2 (10.5f, 20.5f, 100.5f, 200.5f);
    auto ri2 = rf2.as<int>();
    EXPECT_EQ (ri2.x, 10);
    EXPECT_EQ (ri2.y, 20);
    EXPECT_EQ (ri2.width, 100);
    EXPECT_EQ (ri2.height, 200);
}

TEST(Rectangle, at_xy) {
    Rectangle<int> r (10, 20, 100, 200);
    auto r2 = r.at (50, 60);
    EXPECT_EQ (r2.x, 50);
    EXPECT_EQ (r2.y, 60);
    EXPECT_EQ (r2.width, 100);
    EXPECT_EQ (r2.height, 200);
}

TEST(Rectangle, at_single_value) {
    Rectangle<int> r (10, 20, 100, 200);
    auto r2 = r.at (30);
    EXPECT_EQ (r2.x, 30);
    EXPECT_EQ (r2.y, 30);
    EXPECT_EQ (r2.width, 100);
    EXPECT_EQ (r2.height, 200);
}

TEST(Rectangle, str) {
    Rectangle<int> r (10, 20, 100, 200);
    auto s = r.str();
    EXPECT_EQ (s, "10 20 100 200");
}

TEST(Rectangle, contains_point_xy) {
    Rectangle<int> r (10, 10, 100, 100);
    
    EXPECT_TRUE (r.contains (10, 10));
    EXPECT_TRUE (r.contains (50, 50));
    EXPECT_TRUE (r.contains (109, 109));
    
    EXPECT_FALSE (r.contains (110, 50));
    EXPECT_FALSE (r.contains (50, 110));
    EXPECT_FALSE (r.contains (9, 50));
    EXPECT_FALSE (r.contains (50, 9));
}

TEST(Rectangle, contains_point) {
    Rectangle<int> r (10, 10, 100, 100);
    
    EXPECT_TRUE (r.contains (Point<int> { 10, 10 }));
    EXPECT_TRUE (r.contains (Point<int> { 50, 50 }));
    EXPECT_TRUE (r.contains (Point<int> { 109, 109 }));
    
    EXPECT_FALSE (r.contains (Point<int> { 110, 50 }));
    EXPECT_FALSE (r.contains (Point<int> { 9, 50 }));
}

TEST(Rectangle, contains_rectangle) {
    Rectangle<int> r1 (10, 10, 100, 100);
    Rectangle<int> r2 (20, 20, 50, 50);
    Rectangle<int> r3 (0, 0, 50, 50);
    Rectangle<int> r4 (50, 50, 100, 100);
    
    EXPECT_TRUE (r1.contains (r2));
    EXPECT_FALSE (r1.contains (r3));
    EXPECT_FALSE (r1.contains (r4));
    EXPECT_TRUE (r1.contains (r1));
}

TEST(Rectangle, equality_operators) {
    Rectangle<int> r1 (10, 20, 100, 200);
    Rectangle<int> r2 (10, 20, 100, 200);
    Rectangle<int> r3 (10, 20, 100, 201);
    
    EXPECT_TRUE (r1 == r2);
    EXPECT_FALSE (r1 == r3);
    EXPECT_FALSE (r1 != r2);
    EXPECT_TRUE (r1 != r3);
}

TEST(Rectangle, add_point) {
    Rectangle<int> r (10, 20, 100, 200);
    auto r2 = r + Point<int> { 5, 10 };
    EXPECT_EQ (r2.x, 0);      // Uses 2-arg constructor: x=0
    EXPECT_EQ (r2.y, 0);      // Uses 2-arg constructor: y=0
    EXPECT_EQ (r2.width, 15); // x + delta.x becomes width
    EXPECT_EQ (r2.height, 30); // y + delta.y becomes height
}

TEST(Rectangle, add_assign_point) {
    Rectangle<int> r (10, 20, 100, 200);
    r += Point<int> { 5, 10 };
    EXPECT_EQ (r.x, 15);
    EXPECT_EQ (r.y, 30);
    EXPECT_EQ (r.width, 100);
    EXPECT_EQ (r.height, 200);
}

TEST(Rectangle, subtract_point) {
    Rectangle<int> r (10, 20, 100, 200);
    auto r2 = r - Point<int> { 5, 10 };
    EXPECT_EQ (r2.x, 0);      // Uses 2-arg constructor: x=0
    EXPECT_EQ (r2.y, 0);      // Uses 2-arg constructor: y=0
    EXPECT_EQ (r2.width, 5);  // x - delta.x becomes width
    EXPECT_EQ (r2.height, 10); // y - delta.y becomes height
}

TEST(Rectangle, subtract_assign_point) {
    Rectangle<int> r (10, 20, 100, 200);
    r -= Point<int> { 5, 10 };
    EXPECT_EQ (r.x, 5);
    EXPECT_EQ (r.y, 10);
    EXPECT_EQ (r.width, 100);
    EXPECT_EQ (r.height, 200);
}

TEST(Rectangle, multiply_scalar) {
    Rectangle<int> r (10, 20, 100, 200);
    auto r2 = r * 2;
    EXPECT_EQ (r2.x, 20);
    EXPECT_EQ (r2.y, 40);
    EXPECT_EQ (r2.width, 200);
    EXPECT_EQ (r2.height, 400);
}

TEST(Rectangle, multiply_assign_scalar) {
    Rectangle<int> r (10, 20, 100, 200);
    r *= 2;
    EXPECT_EQ (r.x, 20);
    EXPECT_EQ (r.y, 40);
    EXPECT_EQ (r.width, 200);
    EXPECT_EQ (r.height, 400);
}

TEST(Rectangle, divide_scalar) {
    Rectangle<int> r (20, 40, 200, 400);
    auto r2 = r / 2;
    EXPECT_EQ (r2.x, 10);
    EXPECT_EQ (r2.y, 20);
    EXPECT_EQ (r2.width, 100);
    EXPECT_EQ (r2.height, 200);
}

TEST(Rectangle, divide_assign_scalar) {
    Rectangle<int> r (20, 40, 200, 400);
    r /= 2;
    EXPECT_EQ (r.x, 10);
    EXPECT_EQ (r.y, 20);
    EXPECT_EQ (r.width, 100);
    EXPECT_EQ (r.height, 200);
}

TEST(Rectangle, reduce) {
    Rectangle<int> r (10, 10, 100, 100);
    r.reduce (5, 10);
    EXPECT_EQ (r.x, 15);
    EXPECT_EQ (r.y, 20);
    EXPECT_EQ (r.width, 90);
    EXPECT_EQ (r.height, 80);
}

TEST(Rectangle, reduce_single_value) {
    Rectangle<int> r (10, 10, 100, 100);
    r.reduce (5);
    EXPECT_EQ (r.x, 15);
    EXPECT_EQ (r.y, 15);
    EXPECT_EQ (r.width, 90);
    EXPECT_EQ (r.height, 90);
}

TEST(Rectangle, reduced) {
    Rectangle<int> r (10, 10, 100, 100);
    auto r2 = r.reduced (5, 10);
    EXPECT_EQ (r.x, 10);  // Original unchanged
    EXPECT_EQ (r.y, 10);
    EXPECT_EQ (r2.x, 15);
    EXPECT_EQ (r2.y, 20);
    EXPECT_EQ (r2.width, 90);
    EXPECT_EQ (r2.height, 80);
}

TEST(Rectangle, reduced_single_value) {
    Rectangle<int> r (10, 10, 100, 100);
    auto r2 = r.reduced (5);
    EXPECT_EQ (r2.x, 15);
    EXPECT_EQ (r2.y, 15);
    EXPECT_EQ (r2.width, 90);
    EXPECT_EQ (r2.height, 90);
}

TEST(Rectangle, bigger) {
    Rectangle<int> r (20, 20, 60, 60);
    auto r2 = r.bigger (10);
    EXPECT_EQ (r2.x, 10);
    EXPECT_EQ (r2.y, 10);
    EXPECT_EQ (r2.width, 80);
    EXPECT_EQ (r2.height, 80);
}

TEST(Rectangle, smaller) {
    Rectangle<int> r (10, 10, 100, 100);
    auto r2 = r.smaller (10);
    EXPECT_EQ (r2.x, 20);
    EXPECT_EQ (r2.y, 20);
    EXPECT_EQ (r2.width, 80);
    EXPECT_EQ (r2.height, 80);
}

TEST(Rectangle, smaller_xy) {
    Rectangle<int> r (10, 10, 100, 100);
    auto r2 = r.smaller (5, 10);
    EXPECT_EQ (r2.x, 15);
    EXPECT_EQ (r2.y, 20);
    EXPECT_EQ (r2.width, 90);
    EXPECT_EQ (r2.height, 80);
}

TEST(Rectangle, slice_top) {
    Rectangle<int> r (10, 10, 100, 100);
    auto sliced = r.slice_top (20);
    
    // Sliced portion
    EXPECT_EQ (sliced.x, 10);
    EXPECT_EQ (sliced.y, 10);
    EXPECT_EQ (sliced.width, 100);
    EXPECT_EQ (sliced.height, 20);
    
    // Remaining portion
    EXPECT_EQ (r.x, 10);
    EXPECT_EQ (r.y, 30);
    EXPECT_EQ (r.width, 100);
    EXPECT_EQ (r.height, 80);
}

TEST(Rectangle, slice_left) {
    Rectangle<int> r (10, 10, 100, 100);
    auto sliced = r.slice_left (20);
    
    // Sliced portion
    EXPECT_EQ (sliced.x, 10);
    EXPECT_EQ (sliced.y, 10);
    EXPECT_EQ (sliced.width, 20);
    EXPECT_EQ (sliced.height, 100);
    
    // Remaining portion
    EXPECT_EQ (r.x, 30);
    EXPECT_EQ (r.y, 10);
    EXPECT_EQ (r.width, 80);
    EXPECT_EQ (r.height, 100);
}

TEST(Rectangle, slice_bottom) {
    Rectangle<int> r (10, 10, 100, 100);
    auto sliced = r.slice_bottom (20);
    
    // Sliced portion
    EXPECT_EQ (sliced.x, 10);
    EXPECT_EQ (sliced.y, 90);
    EXPECT_EQ (sliced.width, 100);
    EXPECT_EQ (sliced.height, 20);
    
    // Remaining portion
    EXPECT_EQ (r.x, 10);
    EXPECT_EQ (r.y, 10);
    EXPECT_EQ (r.width, 100);
    EXPECT_EQ (r.height, 80);
}

TEST(Rectangle, slice_right) {
    Rectangle<int> r (10, 10, 100, 100);
    auto sliced = r.slice_right (20);
    
    // Sliced portion
    EXPECT_EQ (sliced.x, 90);
    EXPECT_EQ (sliced.y, 10);
    EXPECT_EQ (sliced.width, 20);
    EXPECT_EQ (sliced.height, 100);
    
    // Remaining portion
    EXPECT_EQ (r.x, 10);
    EXPECT_EQ (r.y, 10);
    EXPECT_EQ (r.width, 80);
    EXPECT_EQ (r.height, 100);
}

TEST(Rectangle, intersects) {
    Rectangle<int> r1 (10, 10, 100, 100);
    Rectangle<int> r2 (50, 50, 100, 100);
    Rectangle<int> r3 (200, 200, 100, 100);
    Rectangle<int> r4 (0, 0, 50, 50);
    
    EXPECT_TRUE (r1.intersects (r2));
    EXPECT_TRUE (r2.intersects (r1));
    EXPECT_FALSE (r1.intersects (r3));
    EXPECT_FALSE (r3.intersects (r1));
    EXPECT_TRUE (r1.intersects (r4));
}

TEST(Rectangle, intersects_edge_cases) {
    Rectangle<int> r1 (0, 0, 10, 10);
    Rectangle<int> r2 (10, 0, 10, 10);  // Adjacent, should not intersect
    Rectangle<int> r3 (9, 0, 10, 10);   // Overlapping by 1 pixel
    
    EXPECT_FALSE (r1.intersects (r2));
    EXPECT_TRUE (r1.intersects (r3));
}

TEST(Rectangle, intersection) {
    Rectangle<int> r1 (10, 10, 100, 100);
    Rectangle<int> r2 (50, 50, 100, 100);
    
    auto i = r1.intersection (r2);
    EXPECT_EQ (i.x, 50);
    EXPECT_EQ (i.y, 50);
    EXPECT_EQ (i.width, 60);
    EXPECT_EQ (i.height, 60);
}

TEST(Rectangle, intersection_no_overlap) {
    Rectangle<int> r1 (0, 0, 10, 10);
    Rectangle<int> r2 (20, 20, 10, 10);
    
    auto i = r1.intersection (r2);
    EXPECT_TRUE (i.empty());
}

TEST(Rectangle, intersection_contained) {
    Rectangle<int> r1 (10, 10, 100, 100);
    Rectangle<int> r2 (30, 30, 20, 20);
    
    auto i = r1.intersection (r2);
    EXPECT_EQ (i.x, 30);
    EXPECT_EQ (i.y, 30);
    EXPECT_EQ (i.width, 20);
    EXPECT_EQ (i.height, 20);
}

TEST(Rectangle, bounds_alias) {
    Bounds b (10, 20, 100, 200);
    EXPECT_EQ (b.x, 10);
    EXPECT_EQ (b.y, 20);
    EXPECT_EQ (b.width, 100);
    EXPECT_EQ (b.height, 200);
}

TEST(Rectangle, floating_point) {
    Rectangle<double> r (10.5, 20.5, 100.5, 200.5);
    EXPECT_DOUBLE_EQ (r.x, 10.5);
    EXPECT_DOUBLE_EQ (r.y, 20.5);
    EXPECT_DOUBLE_EQ (r.width, 100.5);
    EXPECT_DOUBLE_EQ (r.height, 200.5);
    
    auto r2 = r * 2.0;
    EXPECT_DOUBLE_EQ (r2.x, 21.0);
    EXPECT_DOUBLE_EQ (r2.y, 41.0);
    EXPECT_DOUBLE_EQ (r2.width, 201.0);
    EXPECT_DOUBLE_EQ (r2.height, 401.0);
}
