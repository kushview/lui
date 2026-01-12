// Copyright 2026 Michael Fisher <mfisher@lvtk.org>
// SPDX-License-Identifier: ISC

// Known issues in path.hpp:
// - Copy assignment uses std::copy without resize (undefined behavior)
// - Iterator comparison operators not const (breaks standard usage)
// - Iterator requires increment before dereference (non-standard)

#include <gtest/gtest.h>
#include <lui/path.hpp>

using namespace lui;

TEST(Path, default_constructor) {
    Path p;
    EXPECT_TRUE (p.data().empty());
}

TEST(Path, move_to) {
    Path p;
    p.move_to (10.f, 20.f);
    
    EXPECT_FALSE (p.data().empty());
    auto it = p.begin();
    ++it;  // Need to increment to load first item
    EXPECT_EQ ((*it).type, PathOp::MOVE);
    EXPECT_FLOAT_EQ ((*it).x1, 10.f);
    EXPECT_FLOAT_EQ ((*it).y1, 20.f);
}

TEST(Path, line_to) {
    Path p;
    p.move_to (10.f, 20.f);
    p.line_to (30.f, 40.f);
    
    auto it = p.begin();
    ++it;
    EXPECT_EQ ((*it).type, PathOp::MOVE);
    
    ++it;
    EXPECT_EQ ((*it).type, PathOp::LINE);
    EXPECT_FLOAT_EQ ((*it).x1, 30.f);
    EXPECT_FLOAT_EQ ((*it).y1, 40.f);
}

TEST(Path, line_to_empty_path_adds_move) {
    Path p;
    p.line_to (30.f, 40.f);
    
    auto it = p.begin();
    ++it;
    EXPECT_EQ ((*it).type, PathOp::MOVE);
    EXPECT_FLOAT_EQ ((*it).x1, 0.f);
    EXPECT_FLOAT_EQ ((*it).y1, 0.f);
}

TEST(Path, quad_to) {
    Path p;
    p.move_to (10.f, 20.f);
    p.quad_to (30.f, 40.f, 50.f, 60.f);
    
    auto it = p.begin();
    ++it;
    ++it;
    EXPECT_EQ ((*it).type, PathOp::QUADRATIC);
    EXPECT_FLOAT_EQ ((*it).x1, 30.f);
    EXPECT_FLOAT_EQ ((*it).y1, 40.f);
    EXPECT_FLOAT_EQ ((*it).x2, 50.f);
    EXPECT_FLOAT_EQ ((*it).y2, 60.f);
}

TEST(Path, cubic_to) {
    Path p;
    p.move_to (10.f, 20.f);
    p.cubic_to (30.f, 40.f, 50.f, 60.f, 70.f, 80.f);
    
    auto it = p.begin();
    ++it;
    ++it;
    EXPECT_EQ ((*it).type, PathOp::CUBIC);
    EXPECT_FLOAT_EQ ((*it).x1, 30.f);
    EXPECT_FLOAT_EQ ((*it).y1, 40.f);
    EXPECT_FLOAT_EQ ((*it).x2, 50.f);
    EXPECT_FLOAT_EQ ((*it).y2, 60.f);
    EXPECT_FLOAT_EQ ((*it).x3, 70.f);
    EXPECT_FLOAT_EQ ((*it).y3, 80.f);
}

TEST(Path, close_path) {
    Path p;
    p.move_to (10.f, 20.f);
    p.line_to (30.f, 40.f);
    p.close_path();
    
    auto it = p.begin();
    ++it;
    ++it;
    ++it;
    EXPECT_EQ ((*it).type, PathOp::CLOSE);
}

TEST(Path, range_based_for) {
    Path p;
    p.move_to (0.f, 0.f);
    p.line_to (10.f, 10.f);
    p.close_path();
    
    int count = 0;
    for (const auto& item : p) {
        count++;
        EXPECT_TRUE (item.type == PathOp::MOVE || 
                    item.type == PathOp::LINE || 
                    item.type == PathOp::CLOSE);
    }
    EXPECT_EQ (count, 3);
}

TEST(Path, clear) {
    Path p;
    p.move_to (10.f, 20.f);
    EXPECT_FALSE (p.data().empty());
    
    p.clear();
    EXPECT_TRUE (p.data().empty());
}

TEST(Path, begin_path) {
    Path p;
    p.move_to (10.f, 20.f);
    p.begin_path();
    EXPECT_TRUE (p.data().empty());
}

TEST(Path, reserve) {
    Path p;
    auto initial_size = p.data().size();
    p.reserve (100);
    EXPECT_GE (p.data().capacity(), initial_size + 100);
}

TEST(Path, add_ellipse) {
    Path p;
    p.add_ellipse (10.f, 20.f, 100.f, 50.f);
    
    int move_count = 0;
    int cubic_count = 0;
    
    for (const auto& item : p) {
        if (item.type == PathOp::MOVE) move_count++;
        if (item.type == PathOp::CUBIC) cubic_count++;
    }
    
    EXPECT_GT (move_count, 0);
    EXPECT_EQ (cubic_count, 4);
}

TEST(Path, add_ellipse_rectangle) {
    Path p;
    Rectangle<float> r (10.f, 20.f, 100.f, 50.f);
    p.add_ellipse (r);
    
    int count = 0;
    for (const auto& item : p) {
        count++;
    }
    EXPECT_GT (count, 0);
}

TEST(PathOp, enum_values) {
    EXPECT_EQ (static_cast<int> (PathOp::MOVE), 100000);
    EXPECT_EQ (static_cast<int> (PathOp::LINE), 100001);
    EXPECT_EQ (static_cast<int> (PathOp::QUADRATIC), 100002);
    EXPECT_EQ (static_cast<int> (PathOp::CUBIC), 100003);
    EXPECT_EQ (static_cast<int> (PathOp::CLOSE), 100004);
}

TEST(PathItem, default_values) {
    PathItem item;
    EXPECT_EQ (item.type, PathOp::MOVE);
    EXPECT_FLOAT_EQ (item.x1, 0.f);
    EXPECT_FLOAT_EQ (item.y1, 0.f);
    EXPECT_FLOAT_EQ (item.x2, 0.f);
    EXPECT_FLOAT_EQ (item.y2, 0.f);
    EXPECT_FLOAT_EQ (item.x3, 0.f);
    EXPECT_FLOAT_EQ (item.y3, 0.f);
}

TEST(Graphics, rounded_rect_all_corners) {
    Path p;
    graphics::rounded_rect (p, 0.f, 0.f, 100.f, 100.f, 10.f, 10.f,
                           true, true, true, true);
    
    bool has_cubic = false;
    
    for (const auto& item : p) {
        if (item.type == PathOp::CUBIC) has_cubic = true;
    }
    
    EXPECT_TRUE (has_cubic);
}

TEST(Graphics, rounded_rect_no_corners) {
    Path p;
    graphics::rounded_rect (p, 0.f, 0.f, 100.f, 100.f, 10.f, 10.f,
                           false, false, false, false);
    
    bool has_cubic = false;
    
    for (const auto& item : p) {
        if (item.type == PathOp::CUBIC) has_cubic = true;
    }
    
    EXPECT_FALSE (has_cubic);
}

TEST(Graphics, rounded_rect_simple) {
    Path p;
    graphics::rounded_rect (p, 10.f, 20.f, 100.f, 50.f, 5.f);
    
    int cubic_count = 0;
    for (const auto& item : p) {
        if (item.type == PathOp::CUBIC)
            cubic_count++;
    }
    
    EXPECT_EQ (cubic_count, 4);
}
