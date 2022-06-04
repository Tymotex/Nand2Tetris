#include "LexicalAnalyser.h"
#include <gtest/gtest.h>

class LexicalAnalyserTestFixture : public ::testing::Test {
protected:
    LexicalAnalyserTestFixture() {
    }
};

TEST_F(LexicalAnalyserTestFixture, SanityCheck) {
    EXPECT_EQ(1, 1);
}
