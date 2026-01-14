// Copyright 2026 Kushview, LLC
// SPDX-License-Identifier: ISC

#include <gtest/gtest.h>
#include <lui/point.hpp>
#include <cmath>

using namespace lui;

// ============================================================================
// Default Constructor Tests
// ============================================================================

TEST(Point, default_constructor_int) {
    Point<int> p;
    EXPECT_EQ(p.x, 0);
    EXPECT_EQ(p.y, 0);
}

TEST(Point, default_constructor_float) {
    Point<float> p;
    EXPECT_EQ(p.x, 0.0f);
    EXPECT_EQ(p.y, 0.0f);
}

TEST(Point, default_constructor_double) {
    Point<double> p;
    EXPECT_EQ(p.x, 0.0);
    EXPECT_EQ(p.y, 0.0);
}

// ============================================================================
// Templated Constructor Tests - Same Type
// ============================================================================

TEST(Point, constructor_int_int) {
    Point<int> p(10, 20);
    EXPECT_EQ(p.x, 10);
    EXPECT_EQ(p.y, 20);
}

TEST(Point, constructor_float_float) {
    Point<float> p(1.5f, 2.5f);
    EXPECT_FLOAT_EQ(p.x, 1.5f);
    EXPECT_FLOAT_EQ(p.y, 2.5f);
}

TEST(Point, constructor_double_double) {
    Point<double> p(3.14, 2.71);
    EXPECT_DOUBLE_EQ(p.x, 3.14);
    EXPECT_DOUBLE_EQ(p.y, 2.71);
}

// ============================================================================
// Templated Constructor Tests - Mixed Types
// ============================================================================

TEST(Point, constructor_int_from_float) {
    Point<int> p(1.9f, 2.1f);
    EXPECT_EQ(p.x, 1);  // truncated
    EXPECT_EQ(p.y, 2);  // truncated
}

TEST(Point, constructor_float_from_int) {
    Point<float> p(10, 20);
    EXPECT_FLOAT_EQ(p.x, 10.0f);
    EXPECT_FLOAT_EQ(p.y, 20.0f);
}

TEST(Point, constructor_double_from_int_float) {
    Point<double> p(5, 3.14f);
    EXPECT_DOUBLE_EQ(p.x, 5.0);
    // Note: float 3.14f has limited precision, so we compare against the float value
    EXPECT_DOUBLE_EQ(p.y, static_cast<double>(3.14f));
}

TEST(Point, constructor_int_from_double) {
    Point<int> p(10.5, 20.5);
    EXPECT_EQ(p.x, 10);
    EXPECT_EQ(p.y, 20);
}

// ============================================================================
// Type Conversion Tests
// ============================================================================

TEST(Point, as_int_to_float) {
    Point<int> pi(5, 10);
    Point<float> pf = pi.as<float>();
    EXPECT_FLOAT_EQ(pf.x, 5.0f);
    EXPECT_FLOAT_EQ(pf.y, 10.0f);
}

TEST(Point, as_float_to_int) {
    Point<float> pf(3.7f, 8.2f);
    Point<int> pi = pf.as<int>();
    EXPECT_EQ(pi.x, 3);
    EXPECT_EQ(pi.y, 8);
}

TEST(Point, as_double_to_float) {
    Point<double> pd(1.5, 2.5);
    Point<float> pf = pd.as<float>();
    EXPECT_FLOAT_EQ(pf.x, 1.5f);
    EXPECT_FLOAT_EQ(pf.y, 2.5f);
}

// ============================================================================
// Addition Operator Tests
// ============================================================================

TEST(Point, addition_int) {
    Point<int> p1(10, 20);
    Point<int> p2(5, 3);
    Point<int> result = p1 + p2;
    EXPECT_EQ(result.x, 15);
    EXPECT_EQ(result.y, 23);
}

TEST(Point, addition_float) {
    Point<float> p1(1.5f, 2.5f);
    Point<float> p2(0.5f, 1.5f);
    Point<float> result = p1 + p2;
    EXPECT_FLOAT_EQ(result.x, 2.0f);
    EXPECT_FLOAT_EQ(result.y, 4.0f);
}

TEST(Point, addition_double) {
    Point<double> p1(1.1, 2.2);
    Point<double> p2(3.3, 4.4);
    Point<double> result = p1 + p2;
    EXPECT_DOUBLE_EQ(result.x, 4.4);
    EXPECT_DOUBLE_EQ(result.y, 6.6);
}

TEST(Point, addition_negative) {
    Point<int> p1(10, 20);
    Point<int> p2(-5, -3);
    Point<int> result = p1 + p2;
    EXPECT_EQ(result.x, 5);
    EXPECT_EQ(result.y, 17);
}

// ============================================================================
// Addition Assignment Operator Tests
// ============================================================================

TEST(Point, add_assign_int) {
    Point<int> p1(10, 20);
    Point<int> p2(5, 3);
    p1 += p2;
    EXPECT_EQ(p1.x, 15);
    EXPECT_EQ(p1.y, 23);
}

TEST(Point, add_assign_float) {
    Point<float> p1(1.5f, 2.5f);
    Point<float> p2(0.5f, 1.5f);
    p1 += p2;
    EXPECT_FLOAT_EQ(p1.x, 2.0f);
    EXPECT_FLOAT_EQ(p1.y, 4.0f);
}

TEST(Point, add_assign_returns_reference) {
    Point<int> p1(10, 20);
    Point<int> p2(5, 3);
    Point<int>& ref = (p1 += p2);
    EXPECT_EQ(&ref, &p1);
}

// ============================================================================
// Subtraction Operator Tests
// ============================================================================

TEST(Point, subtraction_int) {
    Point<int> p1(10, 20);
    Point<int> p2(5, 3);
    Point<int> result = p1 - p2;
    EXPECT_EQ(result.x, 5);
    EXPECT_EQ(result.y, 17);
}

TEST(Point, subtraction_float) {
    Point<float> p1(2.0f, 4.0f);
    Point<float> p2(0.5f, 1.5f);
    Point<float> result = p1 - p2;
    EXPECT_FLOAT_EQ(result.x, 1.5f);
    EXPECT_FLOAT_EQ(result.y, 2.5f);
}

TEST(Point, subtraction_negative) {
    Point<int> p1(10, 20);
    Point<int> p2(-5, -3);
    Point<int> result = p1 - p2;
    EXPECT_EQ(result.x, 15);
    EXPECT_EQ(result.y, 23);
}

// ============================================================================
// Subtraction Assignment Operator Tests
// ============================================================================

TEST(Point, subtract_assign_int) {
    Point<int> p1(10, 20);
    Point<int> p2(5, 3);
    p1 -= p2;
    EXPECT_EQ(p1.x, 5);
    EXPECT_EQ(p1.y, 17);
}

TEST(Point, subtract_assign_float) {
    Point<float> p1(2.0f, 4.0f);
    Point<float> p2(0.5f, 1.5f);
    p1 -= p2;
    EXPECT_FLOAT_EQ(p1.x, 1.5f);
    EXPECT_FLOAT_EQ(p1.y, 2.5f);
}

TEST(Point, subtract_assign_returns_reference) {
    Point<int> p1(10, 20);
    Point<int> p2(5, 3);
    Point<int>& ref = (p1 -= p2);
    EXPECT_EQ(&ref, &p1);
}

// ============================================================================
// Multiplication Operator Tests
// ============================================================================

TEST(Point, multiplication_int_int) {
    Point<int> p(10, 20);
    Point<int> result = p * 2;
    EXPECT_EQ(result.x, 20);
    EXPECT_EQ(result.y, 40);
}

TEST(Point, multiplication_int_float) {
    Point<int> p(10, 20);
    auto result = p * 1.5f;
    EXPECT_FLOAT_EQ(result.x, 15.0f);
    EXPECT_FLOAT_EQ(result.y, 30.0f);
}

TEST(Point, multiplication_float_int) {
    Point<float> p(2.5f, 3.5f);
    Point<float> result = p * 2;
    EXPECT_FLOAT_EQ(result.x, 5.0f);
    EXPECT_FLOAT_EQ(result.y, 7.0f);
}

TEST(Point, multiplication_float_float) {
    Point<float> p(2.0f, 3.0f);
    Point<float> result = p * 1.5f;
    EXPECT_FLOAT_EQ(result.x, 3.0f);
    EXPECT_FLOAT_EQ(result.y, 4.5f);
}

TEST(Point, multiplication_zero) {
    Point<int> p(10, 20);
    Point<int> result = p * 0;
    EXPECT_EQ(result.x, 0);
    EXPECT_EQ(result.y, 0);
}

TEST(Point, multiplication_negative) {
    Point<int> p(10, 20);
    Point<int> result = p * -1;
    EXPECT_EQ(result.x, -10);
    EXPECT_EQ(result.y, -20);
}

// ============================================================================
// Division Operator Tests
// ============================================================================

TEST(Point, division_int_int) {
    Point<int> p(20, 40);
    Point<int> result = p / 2;
    EXPECT_EQ(result.x, 10);
    EXPECT_EQ(result.y, 20);
}

TEST(Point, division_int_float) {
    Point<int> p(20, 40);
    auto result = p / 2.0f;
    EXPECT_FLOAT_EQ(result.x, 10.0f);
    EXPECT_FLOAT_EQ(result.y, 20.0f);
}

TEST(Point, division_float_float) {
    Point<float> p(10.0f, 20.0f);
    Point<float> result = p / 2.0f;
    EXPECT_FLOAT_EQ(result.x, 5.0f);
    EXPECT_FLOAT_EQ(result.y, 10.0f);
}

TEST(Point, division_truncation) {
    Point<int> p(21, 39);
    Point<int> result = p / 2;
    EXPECT_EQ(result.x, 10);
    EXPECT_EQ(result.y, 19);
}

// ============================================================================
// String Conversion Tests
// ============================================================================

TEST(Point, str_int) {
    Point<int> p(10, 20);
    std::string result = p.str();
    EXPECT_EQ(result, "10 20");
}

TEST(Point, str_float) {
    Point<float> p(1.5f, 2.5f);
    std::string result = p.str();
    // String conversion may have varying precision, just check it's not empty
    EXPECT_FALSE(result.empty());
    EXPECT_NE(result.find("1.5"), std::string::npos);
    EXPECT_NE(result.find("2.5"), std::string::npos);
}

TEST(Point, str_negative) {
    Point<int> p(-10, -20);
    std::string result = p.str();
    EXPECT_EQ(result, "-10 -20");
}

TEST(Point, str_zero) {
    Point<int> p(0, 0);
    std::string result = p.str();
    EXPECT_EQ(result, "0 0");
}

// ============================================================================
// Edge Cases and Special Values
// ============================================================================

TEST(Point, large_values_int) {
    Point<int> p(2147483647, 2147483647);  // max int32
    EXPECT_EQ(p.x, 2147483647);
    EXPECT_EQ(p.y, 2147483647);
}

TEST(Point, negative_values_int) {
    Point<int> p(-100, -200);
    EXPECT_EQ(p.x, -100);
    EXPECT_EQ(p.y, -200);
}

TEST(Point, very_small_float) {
    Point<float> p(1e-6f, 1e-6f);
    EXPECT_FLOAT_EQ(p.x, 1e-6f);
    EXPECT_FLOAT_EQ(p.y, 1e-6f);
}

TEST(Point, very_large_float) {
    Point<float> p(1e6f, 1e6f);
    EXPECT_FLOAT_EQ(p.x, 1e6f);
    EXPECT_FLOAT_EQ(p.y, 1e6f);
}

// ============================================================================
// Copy and Move Semantics
// ============================================================================

TEST(Point, copy_constructor) {
    Point<int> p1(10, 20);
    Point<int> p2 = p1;
    EXPECT_EQ(p2.x, 10);
    EXPECT_EQ(p2.y, 20);
}

TEST(Point, copy_assignment) {
    Point<int> p1(10, 20);
    Point<int> p2(5, 5);
    p2 = p1;
    EXPECT_EQ(p2.x, 10);
    EXPECT_EQ(p2.y, 20);
}

TEST(Point, move_semantics) {
    Point<int> p1(10, 20);
    Point<int> p2 = std::move(p1);
    EXPECT_EQ(p2.x, 10);
    EXPECT_EQ(p2.y, 20);
}

// ============================================================================
// Chaining Operations
// ============================================================================

TEST(Point, chained_addition) {
    Point<int> p1(10, 20);
    Point<int> p2(5, 3);
    Point<int> p3(2, 7);
    Point<int> result = p1 + p2 + p3;
    EXPECT_EQ(result.x, 17);
    EXPECT_EQ(result.y, 30);
}

TEST(Point, combined_operations) {
    Point<int> p1(10, 20);
    Point<int> p2(2, 4);
    Point<int> result = (p1 + p2) * 2;
    EXPECT_EQ(result.x, 24);
    EXPECT_EQ(result.y, 48);
}

TEST(Point, mixed_arithmetic) {
    Point<int> p1(10, 20);
    Point<int> p2(5, 10);
    Point<int> result = (p1 - p2) * 3;
    EXPECT_EQ(result.x, 15);
    EXPECT_EQ(result.y, 30);
}

// ============================================================================
// Type Safety (at compile time, verified through proper compilation)
// ============================================================================

// These tests verify that the correct types compile
TEST(Point, integral_types_compile) {
    // All integral types should compile
    Point<char> pc(1, 2);
    Point<short> ps(1, 2);
    Point<int> pi(1, 2);
    Point<long> pl(1, 2);
    Point<long long> pll(1, 2);
    Point<unsigned int> pui(1, 2);
    
    EXPECT_TRUE(true);  // Just verify compilation
}

TEST(Point, floating_point_types_compile) {
    // All floating-point types should compile
    Point<float> pf(1.0f, 2.0f);
    Point<double> pd(1.0, 2.0);
    
    EXPECT_TRUE(true);  // Just verify compilation
}

// ============================================================================
// Verify Original Values Unchanged (const correctness)
// ============================================================================

TEST(Point, const_correctness) {
    const Point<int> p(10, 20);
    EXPECT_EQ(p.x, 10);
    EXPECT_EQ(p.y, 20);
    
    // Verify that operations return new Point without modifying original
    auto as_float = p.as<float>();
    EXPECT_EQ(p.x, 10);
    EXPECT_EQ(p.y, 20);
}

TEST(Point, operations_dont_modify_operands) {
    Point<int> p1(10, 20);
    Point<int> p2(5, 3);
    
    auto sum = p1 + p2;
    EXPECT_EQ(p1.x, 10);
    EXPECT_EQ(p1.y, 20);
    EXPECT_EQ(p2.x, 5);
    EXPECT_EQ(p2.y, 3);
}

// ============================================================================
// Distance-related calculations
// ============================================================================

TEST(Point, distance_formula) {
    Point<int> p1(0, 0);
    Point<int> p2(3, 4);
    Point<int> diff = p2 - p1;
    // Distance should be 5 (3-4-5 triangle)
    double dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
    EXPECT_DOUBLE_EQ(dist, 5.0);
}

TEST(Point, midpoint_calculation) {
    Point<double> p1(0.0, 0.0);
    Point<double> p2(10.0, 20.0);
    Point<double> midpoint = (p1 + p2) / 2.0;
    EXPECT_DOUBLE_EQ(midpoint.x, 5.0);
    EXPECT_DOUBLE_EQ(midpoint.y, 10.0);
}
