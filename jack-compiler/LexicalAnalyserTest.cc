#include <gtest/gtest.h>

// Demonstrate some basic assertions.
TEST(LexicalAnalyserTest, BasicAssertions) {
    EXPECT_STRNE("hello", "world");
    EXPECT_EQ(7 * 6, 42);
}