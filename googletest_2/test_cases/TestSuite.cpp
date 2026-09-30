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
    char b[] = "000000"; EXPECT_EQ(time_parse(b), TIME_ZERO_ERROR);
    char c[] = "240000"; EXPECT_LT(time_parse(c), 0);
    char d[] = "006000"; EXPECT_LT(time_parse(d), 0);
    char e[] = "000060"; EXPECT_LT(time_parse(e), 0);
}

TEST(TimeParserTest, NullCheck) {
    char ok[] = "000105";
    EXPECT_EQ(time_parse(ok), 65);            
    EXPECT_EQ(time_parse(NULL), TIME_NULL_ERROR);  
}

TEST(TimeParserTest, LengthCheck) {
    char ok[] = "000105";
    char tooShort[] = "0105";
    char tooLong[] = "00001055";
    EXPECT_EQ(time_parse(ok), 65);
    EXPECT_EQ(time_parse(tooShort), TIME_LEN_ERROR);
    EXPECT_EQ(time_parse(tooLong), TIME_LEN_ERROR);
}

TEST(TimeParserTest, DigitCheck) {
    char ok[] = "000105";
    char bad[] = "00A105";
    EXPECT_EQ(time_parse(ok), 65);
    EXPECT_EQ(time_parse(bad), TIME_VALUE_ERROR);
}

TEST(TimeParserTest, ZeroSecondsCheck) {
    char ok[] = "000001";
    char bad[] = "000000";
    EXPECT_EQ(time_parse(ok), 1);
    EXPECT_EQ(time_parse(bad), TIME_ZERO_ERROR);
}

// https://google.github.io/googletest/reference/testing.html
// https://google.github.io/googletest/reference/assertions.html