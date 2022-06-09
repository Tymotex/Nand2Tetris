#!/bin/sh
# Tests that the LexicalAnalyser produces the correct token stream by comparing
# the XML it produces against the expected token streams in the *T.expected.xml
# files in the code-gen-test-files/ directory.
# Also tests that CompilationEngine produces the correct XML parse tree.

# ANSI colours.
RED='\033[0;31m'
GREEN='\033[0;32m'
RESET='\033[0m'

echo "Building and running Jack Compiler on all test files..."
cmake -S . -B build || exit 1
cmake --build build || exit 1
for each_project in $(find code-gen-test-files -maxdepth 1 -mindepth 1); do
    if ! ./build/JackCompiler "$each_project" > /dev/null; then
        printf "${RED}Jack Compiler failed on project: '${each_project}'...${RESET}\n"
        exit 1
    fi
done
