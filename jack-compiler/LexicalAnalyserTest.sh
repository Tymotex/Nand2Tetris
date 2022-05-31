#!/bin/sh
# Tests that the LexicalAnalyser produces the correct token stream by comparing
# the XML it produces against the expected token streams in the *T.expected.xml
# files in the test-files/ directory.

# TODO: for each subdirectory of test-files, run the compiler, then run diff
#       against T.xml and T.expected.xml file pairs. Report result as pass/fail.

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

echo "Running Jack Compiler on all test files..."
for each_project in $(find test-files -maxdepth 1 -mindepth 1); do
    ./build/JackCompiler "$each_project"
done

test_round=0
for each_file in $(find test-files | grep "\.jack$"); do
    echo "═════ Test $test_round ═════"

    file_directory=$(dirname $(readlink -f $each_file))
    file_basename=$(basename $each_file | cut -d. -f1) 

    output_file=${file_directory}/${file_basename}T.xml
    expected_file=${file_directory}/${file_basename}T.expected.xml

    echo "Out: $output_file"
    echo "Exp: $expected_file"

    diff --strip-trailing-cr $output_file $expected_file && printf "${GREEN}Passed.${NC}\n" || printf "${RED}FAILED${NC}\n"
    echo

    test_round=$(expr $test_round + 1)
done
