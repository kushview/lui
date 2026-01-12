// Copyright 2019 Michael Fisher <mfisher@lvtk.org>
// SPDX-License-Identifier: ISC

#include <gtest/gtest.h>
#include <lui/string.hpp>

#define LOREM_IPSUM \
    R"(Lorem ipsum dolor sit amet, consectetur adipiscing elit. In ut dolor sed lectus condimentum scelerisque ut at ex. Aenean feugiat velit sodales tempus condimentum. Nam sed neque velit. Nulla pretium ut nulla a placerat. Aliquam erat volutpat. Fusce volutpat, urna ut aliquet finibus, nunc mauris porta lacus, ut lacinia dolor sapien ut enim. Vestibulum quis diam mattis, laoreet augue ut, tincidunt magna. Duis semper sit amet leo gravida semper. Lorem ipsum dolor sit amet, consectetur adipiscing elit.)"

using namespace lui;

TEST(String, ctors) {
    using lui::String;
    auto a = String();
    a      = String ("lv2");
    EXPECT_EQ (a, "lv2");
    a = String (String (std::string ("rules")));
    EXPECT_EQ (a, "rules");

    a = String ("test_1");
    EXPECT_EQ (a, "test_1");
    EXPECT_EQ (a, std::string ("test_1"));
    EXPECT_EQ (a, String ("test_1"));
    EXPECT_NE (a, "s df ");
    EXPECT_NE (a, std::string ("seee"));
    EXPECT_NE (a, String ("test_1 "));
}

TEST(String, move_semantics) {
    using lui::String;
    
    // Test move constructor
    std::string tmp = "moved_content";
    String s1 (std::move (tmp));
    EXPECT_EQ (s1, "moved_content");

    // Test move assignment
    String s2;
    s2 = String ("another_move");
    EXPECT_EQ (s2, "another_move");

    // Test move assignment from std::string
    std::string tmp2 = "std_move";
    s2 = std::move (tmp2);
    EXPECT_EQ (s2, "std_move");
}

TEST(String, streams) {
    using lui::String;
    String s1 = "hello";
    EXPECT_EQ (s1, "hello");
    s1 << " " << String ("world");
    EXPECT_EQ (s1, "hello world");

    s1.clear();
    s1 << int (1);
    EXPECT_EQ (s1.str(), "1");

    s1.clear();
    s1 << char ('B');
    EXPECT_EQ (s1.str(), "B");

    s1.clear();
    s1 << int64_t (100);
    EXPECT_EQ (s1.str(), "100");

    s1.clear();
    s1 << double (1.555000);
    EXPECT_EQ (s1.str(), "1.555");

    s1.clear();
    s1 << float (1.444000);
    EXPECT_EQ (s1.str(), "1.444");

    s1.clear();
    s1 << LOREM_IPSUM;
    EXPECT_EQ (s1, LOREM_IPSUM);
}

TEST(String, formatted) {
    using lui::String;
    
    // Test formatted method
    auto s1 = String::formatted ("Number: {}, Float: {:.2f}", 42, 3.14159);
    EXPECT_EQ (s1, "Number: 42, Float: 3.14");

    // Test append_formatted
    String s2 = "Start: ";
    s2.append_formatted ("{} + {} = {}", 1, 2, 3);
    EXPECT_EQ (s2, "Start: 1 + 2 = 3");
}

TEST(String, utf8_basics) {
    using lui::String;
    
    String ascii = "hello";
    EXPECT_EQ (ascii.charCount(), 5);
    EXPECT_EQ (ascii.length(), 5);
    EXPECT_TRUE (ascii.valid_utf8());

    // UTF-8: é (C3 A9), ñ (C3 B1), ü (C3 BC)
    String utf8 = "café";  // é is 2 bytes in UTF-8
    EXPECT_EQ (utf8.charCount(), 4);  // 4 characters
    EXPECT_EQ (utf8.length(), 5);  // 5 bytes
    EXPECT_TRUE (utf8.valid_utf8());

    String emoji = "Hello🎉";
    EXPECT_TRUE (emoji.valid_utf8());
    EXPECT_GT (emoji.length(), emoji.charCount());
}

TEST(String, substring) {
    using lui::String;
    
    String s = "hello world";
    String sub = s.substring (0, 5);
    EXPECT_EQ (sub, "hello");

    String sub2 = s.substring (6, 5);
    EXPECT_EQ (sub2, "world");

    String empty = s.substring (100, 5);
    EXPECT_TRUE (empty.empty());
}

TEST(String, contains) {
    using lui::String;
    
    String s = "hello world";
    EXPECT_TRUE (s.contains ("world"));
    EXPECT_TRUE (s.contains ("hello"));
    EXPECT_TRUE (s.contains ("o w"));
    EXPECT_FALSE (s.contains ("xyz"));
    EXPECT_FALSE (s.contains ("World"));  // case sensitive
}

TEST(String, startswith_endswith) {
    using lui::String;
    
    String s = "hello world";
    EXPECT_TRUE (s.starts_with ("hello"));
    EXPECT_TRUE (s.starts_with ("hello world"));
    EXPECT_FALSE (s.starts_with ("world"));
    EXPECT_FALSE (s.starts_with ("Hello"));

    EXPECT_TRUE (s.ends_with ("world"));
    EXPECT_TRUE (s.ends_with ("hello world"));
    EXPECT_FALSE (s.ends_with ("hello"));
    EXPECT_FALSE (s.ends_with ("World"));
}

TEST(String, trim) {
    using lui::String;
    
    String s1 = "  hello world  ";
    String trimmed = s1.trim();
    EXPECT_EQ (trimmed, "hello world");

    String s2 = "\t\n  content  \n\t";
    String trimmed2 = s2.trim();
    EXPECT_EQ (trimmed2, "content");

    String s3 = "no_whitespace";
    String trimmed3 = s3.trim();
    EXPECT_EQ (trimmed3, "no_whitespace");

    String s4 = "   ";
    String trimmed4 = s4.trim();
    EXPECT_TRUE (trimmed4.empty());
}

TEST(String, replace) {
    using lui::String;
    
    String s = "hello hello world";
    String result = s;
    result.replace ("hello", "hi");
    EXPECT_EQ (result, "hi hi world");

    String s2 = "aaa";
    String result2 = s2;
    result2.replace ("a", "b");
    EXPECT_EQ (result2, "bbb");

    String s3 = "no match";
    String result3 = s3;
    result3.replace ("xyz", "abc");
    EXPECT_EQ (result3, "no match");
}

TEST(String, case_conversion) {
    using lui::String;
    
    String lower = "Hello World";
    String lower_result = lower.to_lower();
    EXPECT_EQ (lower_result, "hello world");

    String upper = "Hello World";
    String upper_result = upper.to_upper();
    EXPECT_EQ (upper_result, "HELLO WORLD");

    String mixed = "MiXeD CaSe";
    EXPECT_EQ (mixed.to_lower(), "mixed case");
    EXPECT_EQ (mixed.to_upper(), "MIXED CASE");
}
