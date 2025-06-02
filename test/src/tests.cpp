//
// Created by vadimzy on 6/1/25.
//
#include <gtest/gtest.h>

#include "../../util/util.h"
#include "../../src/client/DigestGenerators.h"

TEST(buff_split, hash) {
    char buf[100] = "11111111111111\n|11111111111222222|";

    auto sp = util::BuffSplitter({buf, strlen(buf)});
    ASSERT_TRUE(sp.has_split());
    ASSERT_TRUE(sp.left() == "11111111111111");
    ASSERT_TRUE(sp.right() == "|11111111111222222|");

    char buf1[100] = "qqqqqqqqqqqqqqq";
    sp = util::BuffSplitter({buf1, strlen(buf1)});
    ASSERT_FALSE(sp.has_split());
    ASSERT_TRUE(sp.left() == "qqqqqqqqqqqqqqq");
    ASSERT_TRUE(sp.right().empty());

    char buf2[100] = "qqqqqqqqqqqqqqq\n";
    sp = util::BuffSplitter({buf2, strlen(buf2)});
    ASSERT_TRUE(sp.has_split());
    ASSERT_TRUE(sp.left() == "qqqqqqqqqqqqqqq");
    ASSERT_TRUE(sp.right().empty());

    char buf3[100] = "\nqqqqqqqqqqqqqqq";
    sp = util::BuffSplitter({buf3, strlen(buf3)});
    ASSERT_TRUE(sp.has_split());
    ASSERT_TRUE(sp.left().empty());
    ASSERT_TRUE(sp.right()== "qqqqqqqqqqqqqqq");
}

TEST(buff_split, hash_stream) {
    MD5Digest md5;
    StreamMD5Digest sd('\n');

    std::vector<std::string> result;

    auto pFun = [&](std::string s) {
        result.emplace_back(s);
        return 0;
    };

    sd.append(pFun, "11111111111111\n|11111111111222222|");
    ASSERT_TRUE(result.size() == 1);

    md5.update("11111111111111");
    ASSERT_EQ(md5.to_hex_string(), result[0]);

    sd.reset();
    result.clear();
    sd.append(pFun, "11111111111111\n|11111111111222222|\n");
    ASSERT_TRUE(result.size() == 2);
    md5.reset();
    md5.update("11111111111111");
    ASSERT_EQ(md5.to_hex_string(), result[0]);

    md5.reset();
    md5.update("|11111111111222222|");
    ASSERT_EQ(md5.to_hex_string(), result[1]);

    // append one \n
    sd.reset();
    result.clear();
    sd.append(pFun, "11111111111111\n|11111111111222222|");
    sd.append(pFun,"\n");
    ASSERT_TRUE(result.size() == 2);
    md5.reset();
    md5.update("11111111111111");
    ASSERT_EQ(md5.to_hex_string(), result[0]);

    md5.reset();
    md5.update("|11111111111222222|");
    ASSERT_EQ(md5.to_hex_string(), result[1]);


    // \n is the first char
    sd.reset();
    result.clear();
    sd.append(pFun, "\n11111111111111\n|11111111111222222|\n");
    ASSERT_TRUE(result.size() == 3);
    md5.reset();
    md5.update("11111111111111");
    ASSERT_EQ(md5.to_hex_string(), result[1]);

    md5.reset();
    md5.update("|11111111111222222|");
    ASSERT_EQ(md5.to_hex_string(), result[2]);



}
