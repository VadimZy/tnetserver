//
// Created by vadimzy on 6/1/25.
//
#include <gtest/gtest.h>

#include "../../util/util.h"

TEST(buff_split, pos) {
    char buf[100] = "11111111111111\n|11111111111222222|";

    auto sp = util::BuffSplitter(buf, strlen(buf));
    ASSERT_TRUE(sp.has_split());
    ASSERT_TRUE(sp.left() == "11111111111111");
    ASSERT_TRUE(sp.right() == "|11111111111222222|");

    char buf1[100] = "qqqqqqqqqqqqqqq";
    sp = util::BuffSplitter(buf1, strlen(buf1));
    ASSERT_FALSE(sp.has_split());
    ASSERT_TRUE(sp.left() == "qqqqqqqqqqqqqqq");
    ASSERT_TRUE(sp.right().empty());

    char buf2[100] = "qqqqqqqqqqqqqqq\n";
    sp = util::BuffSplitter(buf2, strlen(buf2));
    ASSERT_TRUE(sp.has_split());
    ASSERT_TRUE(sp.left() == "qqqqqqqqqqqqqqq");
    ASSERT_TRUE(sp.right().empty());

    char buf3[100] = "\nqqqqqqqqqqqqqqq";
    sp = util::BuffSplitter(buf3, strlen(buf3));
    ASSERT_TRUE(sp.has_split());
    ASSERT_TRUE(sp.left().empty() );
    ASSERT_TRUE(sp.right()== "qqqqqqqqqqqqqqq");
}