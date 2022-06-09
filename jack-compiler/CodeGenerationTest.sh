#!/bin/sh
# Tests that the LexicalAnalyser produces the correct token stream by comparing
# the XML it produces against the expected token streams in the *T.expected.xml
# files in the code-gen-test-files/ AND syntax-test-files/ directory.
# Also tests that CompilationEngine produces the correct XML parse tree.

# ANSI colours.
RED='\033[0;31m'
GREEN='\033[0;32m'
RESET='\033[0m'

echo "Building and running Jack Compiler on all test files..."
cmake -S . -B build || exit 1
cmake --build build || exit 1
for each_project in $(find code-gen-test-files syntax-test-files -maxdepth 1 -mindepth 1); do
    if ! ./build/JackCompiler "$each_project" > /dev/null; then
        printf "${RED}Jack Compiler failed on project: '${each_project}'...${RESET}\n"
        exit 1
    fi
done

echo "══════════ Compilation Tests ══════════"
test_round=0
for each_file in $(find code-gen-test-files syntax-test-files | grep "\.jack$"); do
    echo "\t═════ Code Gen Test $test_round ═════"

    file_directory=$(dirname $(readlink -f $each_file))
    file_basename=$(basename $each_file | cut -d. -f1) 

    output_file=${file_directory}/${file_basename}.vm
    expected_file=${file_directory}/${file_basename}.vm.expected

    # In the expected file, we need to first strip out all labels and references
    # to labels so that diff does not report different label names as a test
    # failure.
    transformed_output_file="${output_file}.normalised"
    transformed_expected_file="${expected_file}.normalised"
    cat $output_file | 
        sed 's/^label .*$/label/g' |
        sed 's/^goto .*$/goto/g' |
        sed 's/^if-goto .*$/if-goto/g' > $transformed_output_file
    cat $expected_file | 
        sed 's/^label .*$/label/g' |
        sed 's/^goto .*$/goto/g' |
        sed 's/^if-goto .*$/if-goto/g' > $transformed_expected_file

    if diff --strip-trailing-cr -s $transformed_output_file $transformed_expected_file > /dev/null; then
        printf "\t${GREEN}Passed.${RESET}\n"
    else
        echo "\tOut: $transformed_output_file"
        echo "\tExp: $transformed_expected_file"
        printf "\t${RED}FAILED${RESET}\n"
    fi
    echo
    test_round=$(expr $test_round + 1)
done
