#include "LexicalAnalyser.h"
#include <gtest/gtest.h>
#include <sstream>

class LexicalAnalyserTestFixture : public ::testing::Test {
protected:
    std::ostringstream sink;

    LexicalAnalyserTestFixture() {
    }
};

// Verifies that a minimal valid 'Hello World' Jack program can be tokenised
// correctly. It makes use of a bunch of language constructs like classes,
// static methods, statements, expressions, etc.
TEST_F(LexicalAnalyserTestFixture, HelloWorldTokenisationTest) {
    std::istringstream source_code(
        "class Main {"
        "    function void main() {"
        "        do Output.printString(\"Hello World\");"
        "        return;"
        "    }"
        "}"
    );
    std::vector<std::string> expected_tokens = {
        "class", "Main", "{",
            "function", "void", "main", "(", ")", "{", 
                "do", "Output", ".", "printString", "(", "Hello World", ")", ";",
                "return", ";",
            "}",
        "}"
    };

    LexicalAnalyser lexical_analyser(source_code, sink);
    for (int i = 0; lexical_analyser.try_advance(); ++i) {
        EXPECT_EQ(lexical_analyser.get_token(), expected_tokens[i]);
    }
}

// Advancing on an empty stream should not result in an exception being thrown.
TEST_F(LexicalAnalyserTestFixture, EmptyStreamTest) {
    std::istringstream empty("");
    LexicalAnalyser lexical_analyser(empty, sink);
    EXPECT_FALSE(lexical_analyser.try_advance());
    EXPECT_FALSE(lexical_analyser.try_advance());
    EXPECT_FALSE(lexical_analyser.try_advance());
}
