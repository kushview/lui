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
    EXPECT_EQ (s1.str(), "1.555000");

    s1.clear();
    s1 << float (1.444000);
    EXPECT_EQ (s1.str(), "1.444000");

    s1.clear();
    s1 << LOREM_IPSUM;
    EXPECT_EQ (s1, LOREM_IPSUM);
}
