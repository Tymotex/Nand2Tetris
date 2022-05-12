#include <gtest/gtest.h>
#include "Parser.h"

const std::string TEST_SRC = "test-files";

TEST(ParserTest, DeterminesIsCInstruction) { 
    HackAsmParser parser(TEST_SRC + "/sample.asm");
    ASSERT_TRUE(parser.advance());
    ASSERT_EQ(parser.instruction_type(), InstructionType::C_INSTRUCTION);
}
