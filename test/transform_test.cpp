// Copyright 2024 Michael Fisher <mfisher@lvtk.org>
// SPDX-License-Identifier: ISC

#include <gtest/gtest.h>
#include <lui/transform.hpp>
#include <cmath>

using namespace lui;

constexpr double PI = 3.14159265358979323846;

TEST(Transform, default_constructor) {
    Transform t;
    
    // Should be identity matrix
    EXPECT_DOUBLE_EQ (t.m00, 1.0);
    EXPECT_DOUBLE_EQ (t.m01, 0.0);
    EXPECT_DOUBLE_EQ (t.m02, 0.0);
    EXPECT_DOUBLE_EQ (t.m10, 0.0);
    EXPECT_DOUBLE_EQ (t.m11, 1.0);
    EXPECT_DOUBLE_EQ (t.m12, 0.0);
}

TEST(Transform, parameterized_constructor) {
    Transform t (2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
    
    EXPECT_DOUBLE_EQ (t.m00, 2.0);
    EXPECT_DOUBLE_EQ (t.m01, 3.0);
    EXPECT_DOUBLE_EQ (t.m02, 4.0);
    EXPECT_DOUBLE_EQ (t.m10, 5.0);
    EXPECT_DOUBLE_EQ (t.m11, 6.0);
    EXPECT_DOUBLE_EQ (t.m12, 7.0);
}

TEST(Transform, equality_operators) {
    Transform t1 (1.0, 2.0, 3.0, 4.0, 5.0, 6.0);
    Transform t2 (1.0, 2.0, 3.0, 4.0, 5.0, 6.0);
    Transform t3 (1.0, 2.0, 3.0, 4.0, 5.0, 7.0); // Different
    
    EXPECT_TRUE (t1 == t2);
    EXPECT_FALSE (t1 != t2);
    EXPECT_FALSE (t1 == t3);
    EXPECT_TRUE (t1 != t3);
}

TEST(Transform, translation_static) {
    Transform t = Transform::translation (10.0, 20.0);
    
    EXPECT_DOUBLE_EQ (t.m00, 1.0);
    EXPECT_DOUBLE_EQ (t.m01, 0.0);
    EXPECT_DOUBLE_EQ (t.m02, 10.0);
    EXPECT_DOUBLE_EQ (t.m10, 0.0);
    EXPECT_DOUBLE_EQ (t.m11, 1.0);
    EXPECT_DOUBLE_EQ (t.m12, 20.0);
}

TEST(Transform, translation_applies_to_point) {
    Transform t = Transform::translation (10.0, 20.0);
    Point<double> p {5.0, 7.0};
    
    auto result = t.map (p);
    
    EXPECT_DOUBLE_EQ (result.x, 15.0);
    EXPECT_DOUBLE_EQ (result.y, 27.0);
}

TEST(Transform, translated_method) {
    Transform t1 (2.0, 0.0, 5.0, 0.0, 2.0, 10.0);
    Transform t2 = t1.translated (3.0, 4.0);
    
    // Translation should just add to m02 and m12
    EXPECT_DOUBLE_EQ (t2.m00, 2.0);
    EXPECT_DOUBLE_EQ (t2.m01, 0.0);
    EXPECT_DOUBLE_EQ (t2.m02, 8.0);  // 5.0 + 3.0
    EXPECT_DOUBLE_EQ (t2.m10, 0.0);
    EXPECT_DOUBLE_EQ (t2.m11, 2.0);
    EXPECT_DOUBLE_EQ (t2.m12, 14.0); // 10.0 + 4.0
    
    // Original should be unchanged
    EXPECT_DOUBLE_EQ (t1.m02, 5.0);
    EXPECT_DOUBLE_EQ (t1.m12, 10.0);
}

TEST(Transform, rotation_static_90_degrees) {
    Transform t = Transform::rotation (PI / 2.0); // 90 degrees
    
    // For 90 degree rotation:
    // cos(90°) = 0, sin(90°) = 1
    // Matrix should be [0, -1, 0]
    //                  [1,  0, 0]
    EXPECT_NEAR (t.m00, 0.0, 0.0001);
    EXPECT_NEAR (t.m01, -1.0, 0.0001);
    EXPECT_DOUBLE_EQ (t.m02, 0.0);
    EXPECT_NEAR (t.m10, 1.0, 0.0001);
    EXPECT_NEAR (t.m11, 0.0, 0.0001);
    EXPECT_DOUBLE_EQ (t.m12, 0.0);
}

TEST(Transform, rotation_static_180_degrees) {
    Transform t = Transform::rotation (PI); // 180 degrees
    
    // For 180 degree rotation:
    // cos(180°) = -1, sin(180°) = 0
    // Matrix should be [-1,  0, 0]
    //                  [ 0, -1, 0]
    EXPECT_NEAR (t.m00, -1.0, 0.0001);
    EXPECT_NEAR (t.m01, 0.0, 0.0001);
    EXPECT_DOUBLE_EQ (t.m02, 0.0);
    EXPECT_NEAR (t.m10, 0.0, 0.0001);
    EXPECT_NEAR (t.m11, -1.0, 0.0001);
    EXPECT_DOUBLE_EQ (t.m12, 0.0);
}

TEST(Transform, rotation_applies_to_point_90_degrees) {
    Transform t = Transform::rotation (PI / 2.0);
    Point<double> p {1.0, 0.0};
    
    auto result = t.map (p);
    
    // (1, 0) rotated 90° counterclockwise becomes (0, 1)
    EXPECT_NEAR (result.x, 0.0, 0.0001);
    EXPECT_NEAR (result.y, 1.0, 0.0001);
}

TEST(Transform, rotation_applies_to_point_45_degrees) {
    Transform t = Transform::rotation (PI / 4.0); // 45 degrees
    Point<double> p {1.0, 0.0};
    
    auto result = t.map (p);
    
    // (1, 0) rotated 45° should be (cos(45°), sin(45°))
    double expected = std::sqrt (2.0) / 2.0;
    EXPECT_NEAR (result.x, expected, 0.0001);
    EXPECT_NEAR (result.y, expected, 0.0001);
}

TEST(Transform, scaled_uniform) {
    Transform t;
    Transform scaled = t.scaled (2.5);
    
    // Uniform scaling from identity
    EXPECT_DOUBLE_EQ (scaled.m00, 2.5);
    EXPECT_DOUBLE_EQ (scaled.m01, 0.0);
    EXPECT_DOUBLE_EQ (scaled.m02, 0.0);
    EXPECT_DOUBLE_EQ (scaled.m10, 0.0);
    EXPECT_DOUBLE_EQ (scaled.m11, 2.5);
    EXPECT_DOUBLE_EQ (scaled.m12, 0.0);
}

TEST(Transform, scaled_non_uniform) {
    Transform t;
    Transform scaled = t.scaled (2.0, 3.0);
    
    // Non-uniform scaling from identity
    EXPECT_DOUBLE_EQ (scaled.m00, 2.0);
    EXPECT_DOUBLE_EQ (scaled.m01, 0.0);
    EXPECT_DOUBLE_EQ (scaled.m02, 0.0);
    EXPECT_DOUBLE_EQ (scaled.m10, 0.0);
    EXPECT_DOUBLE_EQ (scaled.m11, 3.0);
    EXPECT_DOUBLE_EQ (scaled.m12, 0.0);
}

TEST(Transform, scaled_applies_to_point) {
    Transform t = Transform().scaled (2.0, 3.0);
    Point<double> p {5.0, 7.0};
    
    auto result = t.map (p);
    
    EXPECT_DOUBLE_EQ (result.x, 10.0);
    EXPECT_DOUBLE_EQ (result.y, 21.0);
}

TEST(Transform, scaled_on_existing_transform) {
    Transform t (2.0, 0.0, 10.0, 0.0, 2.0, 20.0);
    Transform scaled = t.scaled (3.0);
    
    // Scaling should multiply all components by the scale factor
    EXPECT_DOUBLE_EQ (scaled.m00, 6.0);
    EXPECT_DOUBLE_EQ (scaled.m01, 0.0);
    EXPECT_DOUBLE_EQ (scaled.m02, 30.0);
    EXPECT_DOUBLE_EQ (scaled.m10, 0.0);
    EXPECT_DOUBLE_EQ (scaled.m11, 6.0);
    EXPECT_DOUBLE_EQ (scaled.m12, 60.0);
}

TEST(Transform, map_identity) {
    Transform t; // Identity
    Point<double> p {10.0, 20.0};
    
    auto result = t.map (p);
    
    EXPECT_DOUBLE_EQ (result.x, 10.0);
    EXPECT_DOUBLE_EQ (result.y, 20.0);
}

TEST(Transform, map_with_integer_point) {
    Transform t = Transform::translation (5.5, 10.5);
    Point<int> p {10, 20};
    
    auto result = t.map (p);
    
    // Should cast properly
    EXPECT_EQ (result.x, 15); // 10 + 5.5 = 15.5 -> 15
    EXPECT_EQ (result.y, 30); // 20 + 10.5 = 30.5 -> 30
}

TEST(Transform, map_with_float_point) {
    Transform t = Transform::translation (1.5, 2.5);
    Point<float> p {3.5f, 4.5f};
    
    auto result = t.map (p);
    
    EXPECT_NEAR (result.x, 5.0f, 0.0001f);
    EXPECT_NEAR (result.y, 7.0f, 0.0001f);
}

TEST(Transform, combined_transformations_translate_then_scale) {
    // Start with translation
    Transform t = Transform::translation (10.0, 20.0);
    // Then scale
    t = t.scaled (2.0);
    
    Point<double> p {5.0, 7.0};
    auto result = t.map (p);
    
    // Scale applies to everything including translation offset
    // x = 2.0 * 5.0 + 20.0 = 30.0
    // y = 2.0 * 7.0 + 40.0 = 54.0
    EXPECT_DOUBLE_EQ (result.x, 30.0);
    EXPECT_DOUBLE_EQ (result.y, 54.0);
}

TEST(Transform, combined_transformations_scale_then_translate) {
    // Start with scale
    Transform t = Transform().scaled (2.0);
    // Then translate
    t = t.translated (10.0, 20.0);
    
    Point<double> p {5.0, 7.0};
    auto result = t.map (p);
    
    // Scale first, then translate
    // x = 2.0 * 5.0 + 10.0 = 20.0
    // y = 2.0 * 7.0 + 20.0 = 34.0
    EXPECT_DOUBLE_EQ (result.x, 20.0);
    EXPECT_DOUBLE_EQ (result.y, 34.0);
}

TEST(Transform, complex_affine_transformation) {
    // Create a custom affine transformation matrix
    Transform t (2.0, 0.5, 10.0, 0.3, 3.0, 20.0);
    
    Point<double> p {4.0, 6.0};
    auto result = t.map (p);
    
    // x' = 2.0 * 4.0 + 0.5 * 6.0 + 10.0 = 8.0 + 3.0 + 10.0 = 21.0
    // y' = 0.3 * 4.0 + 3.0 * 6.0 + 20.0 = 1.2 + 18.0 + 20.0 = 39.2
    EXPECT_DOUBLE_EQ (result.x, 21.0);
    EXPECT_DOUBLE_EQ (result.y, 39.2);
}

TEST(Transform, origin_remains_at_origin_with_rotation_and_scale) {
    Transform t = Transform::rotation (PI / 4.0).scaled (2.0);
    Point<double> origin {0.0, 0.0};
    
    auto result = t.map (origin);
    
    // Origin should stay at origin (no translation component)
    EXPECT_NEAR (result.x, 0.0, 0.0001);
    EXPECT_NEAR (result.y, 0.0, 0.0001);
}

TEST(Transform, chained_translations) {
    Transform t = Transform::translation (5.0, 10.0)
                      .translated (3.0, 7.0)
                      .translated (2.0, 3.0);
    
    // Total translation should be (10, 20)
    EXPECT_DOUBLE_EQ (t.m02, 10.0);
    EXPECT_DOUBLE_EQ (t.m12, 20.0);
}

TEST(Transform, negative_scaling) {
    Transform t = Transform().scaled (-1.0, 1.0); // Mirror on Y axis
    
    Point<double> p {5.0, 7.0};
    auto result = t.map (p);
    
    EXPECT_DOUBLE_EQ (result.x, -5.0);
    EXPECT_DOUBLE_EQ (result.y, 7.0);
}

TEST(Transform, zero_point) {
    Transform t = Transform::translation (10.0, 20.0).scaled (2.0);
    Point<double> p {0.0, 0.0};
    
    auto result = t.map (p);
    
    EXPECT_DOUBLE_EQ (result.x, 20.0);
    EXPECT_DOUBLE_EQ (result.y, 40.0);
}
