#include "CompilationEngine.h"
#include <gtest/gtest.h>

class CompilationEngineTestFixture : public ::testing::Test {
protected:
    CompilationEngineTestFixture() {
    }
};

// TODO: Learn gMock. Determine what the best way to approach testing this class
// TODO: would be. Eg. do I mock SymbolTable and VMWriter and check that it gets
// TODO: used correctly?

// TEST_F(CompilationEngineTestFixture, RecordsClass) {
//     EXPECT_EQ(1, 1);
// }

// TODO: test that 'let x = 1' fails when there is no 'var int x;'
