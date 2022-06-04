#include "CompilationEngine.h"
#include <gtest/gtest.h>

class CompilationEngineTestFixture : public ::testing::Test {
protected:
    CompilationEngineTestFixture() {
    }
};

TEST_F(CompilationEngineTestFixture, SanityCheck) {
    EXPECT_EQ(1, 1);
}
