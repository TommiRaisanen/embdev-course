#include <gtest/gtest.h>
#include "TimeParser.h"

// Test suite: TimeParserTest
TEST(TimeParserTest, TestCaseCorrectTime) {

    // Note that this test fails on purpose!!

    // Test with correct time string
    char time_test[] = "141205";
    ASSERT_EQ(time_parse(time_test),12*60+5);

}

TEST(TimeParserTest, ValidTime) {
    char t[] = "000120";
    EXPECT_EQ(time_parse(t), 80);
}

TEST(TimeParserTest, BoundaryValues) {
    char a[] = "235959"; EXPECT_EQ(time_parse(a), 59*60+59);
    char b[] = "000000"; EXPECT_EQ(time_parse(b), 0);
    char c[] = "240000"; EXPECT_LT(time_parse(c), 0);
    char d[] = "006000"; EXPECT_LT(time_parse(d), 0);
    char e[] = "000060"; EXPECT_LT(time_parse(e), 0);
}

// https://google.github.io/googletest/reference/testing.html
// https://google.github.io/googletest/reference/assertions.html