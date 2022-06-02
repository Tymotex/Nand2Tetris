#!/bin/sh
# Tests that the LexicalAnalyser produces the correct token stream by comparing
# the XML it produces against the expected token streams in the *T.expected.xml
# files in the test-files/ directory.
# Also tests that CompilationEngine produces the correct XML parse tree.

# ANSI colours.
RED='\033[0;31m'
GREEN='\033[0;32m'
RESET='\033[0m'

echo "Building and running Jack Compiler on all test files..."
cmake -S . -B build || exit 1
cmake --build build || exit 1
for each_project in $(find test-files -maxdepth 1 -mindepth 1); do
    if ! ./build/JackCompiler "$each_project" > /dev/null; then
        printf "${RED}Jack Compiler failed...${RESET}\n"
        exit 1
    fi
done

echo "══════════ LexicalAnalyser XML Tests ══════════"
test_round=0
for each_file in $(find test-files | grep "\.jack$"); do
    echo "\t═════ LexicalAnalyser Test $test_round ═════"

    file_directory=$(dirname $(readlink -f $each_file))
    file_basename=$(basename $each_file | cut -d. -f1) 

    output_file=${file_directory}/${file_basename}T.xml
    expected_file=${file_directory}/${file_basename}T.expected.xml

    if diff --strip-trailing-cr -s $output_file $expected_file > /dev/null; then
        printf "\t${GREEN}Passed.${RESET}\n"
    else
        echo "\tOut: $output_file"
        echo "\tExp: $expected_file"
        printf "\t${RED}FAILED${RESET}\n"
    fi
    echo
    test_round=$(expr $test_round + 1)
done

echo "══════════ CompilationEngine XML Tests ══════════"
test_round=0
for each_file in $(find test-files | grep "\.jack$"); do
    echo "\t═════ CompilationEngine Test $test_round ═════"

    file_directory=$(dirname $(readlink -f $each_file))
    file_basename=$(basename $each_file | cut -d. -f1) 

    output_file=${file_directory}/${file_basename}.xml
    expected_file=${file_directory}/${file_basename}.expected.xml

    if diff --strip-trailing-cr -s $output_file $expected_file > /dev/null; then
        printf "\t${GREEN}Passed.${RESET}\n"
    else
        echo "\tOut: $output_file"
        echo "\tExp: $expected_file"
        printf "\t${RED}FAILED${RESET}\n"
    fi
    echo
    test_round=$(expr $test_round + 1)
done
