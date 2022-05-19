#include "VMParser.h"
#include <fstream>
#include <iostream>
#include <regex>
#include <unordered_set>

std::unordered_set<std::string> VMParser::_arithmetic_logic_operators = {
    "add", "sub", "and", "or", "eq", "gt", "lt", "neg", "not"
};

VMParser::VMParser(const std::string& vm_source_file_path) 
    : _vm_in(std::ifstream(vm_source_file_path)),
      _curr_line(0) {
}

bool VMParser::has_more_lines() {
    return !_vm_in.eof();
}

void VMParser::advance() {
    if (!std::getline(_vm_in, _curr_instruction)) {
        _instruction_type = VMOperationType::INVALID;
        _arg1 = "";
        _arg2 = -1;
        return;
    }
    ++_curr_line;
    if (!parse()) advance();
    else show_instruction_debug_info();
}

VMOperationType VMParser::instruction_type() {
    return _instruction_type;
}

std::string VMParser::curr_function_name() {
    return _curr_function_name;
}

std::string VMParser::arg1() {
    return _arg1;
}

int VMParser::arg2() {
    return _arg2;
}

std::string VMParser::get_curr_instruction() {
    return _curr_instruction;
}

void VMParser::preprocess() {
    // Strips all leading whitespace.
    const std::string WHITESPACE = " \n\r\t\f\v";

    int start_index = _curr_instruction.find_first_not_of(WHITESPACE);
    if (start_index != std::string::npos) _curr_instruction = _curr_instruction.substr(start_index);
    
    // Strip inline comments.
    int comment_start_index = _curr_instruction.find_first_of("/");
    if (comment_start_index != std::string::npos) _curr_instruction = _curr_instruction.substr(0, comment_start_index);

    // Strip all trailing whitespace.
    int last_index = _curr_instruction.find_last_not_of(WHITESPACE);
    if (last_index != std::string::npos) _curr_instruction = _curr_instruction.substr(0, last_index + 1);
}

bool VMParser::parse() {
    preprocess();
    if (_curr_instruction.empty()) return false;
    
    // First, we determine what command type and therefore what subsequent
    // arguments to expect.
    int run_len = 0; 
    while (run_len < _curr_instruction.size()) {
        const char& c = _curr_instruction[run_len];
        if (c == ' ') break;
        ++run_len;
    }
    std::string instruction = _curr_instruction.substr(0, run_len);

    // Next, we pull out the expected arguments and populate/clear _arg1 and
    // _arg2.
    // TODO: extract out regex and smatch.
    if (instruction == "push") {
        // Expect the segment name, followed by the index.
        std::regex push_pop_pattern(R"(^(push|pop) +([a-zA-Z]+) +([0-9]+))");
        std::smatch matches;
        if (!std::regex_search(_curr_instruction, matches, push_pop_pattern)) {
            std::cerr << "Syntax Error: push/pop commands expect 2 args.\n";
            return false;
        }
        _arg1 = matches[2];
        _arg2 = stoi(matches[3]);
        _instruction_type = VMOperationType::C_PUSH;
    } else if (instruction == "pop") {
        // Expect the segment name, followed by the index.
        std::regex push_pop_pattern(R"(^(push|pop) +([a-zA-Z]+) +([0-9]+))");
        std::smatch matches;
        if (!std::regex_search(_curr_instruction, matches, push_pop_pattern)) {
            std::cerr << "Syntax Error: push/pop commands expect 2 args.\n";
            return false;
        }
        _arg1 = matches[2];
        _arg2 = stoi(matches[3]);
        _instruction_type = VMOperationType::C_POP;
    } else if (_arithmetic_logic_operators.find(instruction) != _arithmetic_logic_operators.end()) {
        // If additional arguments were supplied, then the instruction is invalid.
        std::regex arithmetic_logic_pattern(R"(^[a-zA-Z]+)");
        std::smatch matches;
        if (!std::regex_search(_curr_instruction, matches, arithmetic_logic_pattern)) {
            std::cerr << "Syntax Error: arithmetic/logical instructions expect no arguments.\n";
            return false;
        }
        // Clear _arg1 and _arg2.
        _arg1 = "";
        _arg2 = -1;
        _instruction_type = VMOperationType::C_ARITHMETIC;
    } else if (instruction == "label") {
        std::regex label_pattern(R"(^label +(.*))");   // TODO: should make this regex pattern check \S, not .
        std::smatch matches;
        if (!std::regex_search(_curr_instruction, matches, label_pattern)) {
            std::cerr << "Syntax Error: label expects 1 argument.\n";
            return false;
        }
        _arg1 = matches[1];
        _arg2 = -1;
        _instruction_type = VMOperationType::C_LABEL;
    } else if (instruction == "goto") {
        std::regex goto_pattern(R"(^goto +(.*))");
        std::smatch matches;
        if (!std::regex_search(_curr_instruction, matches, goto_pattern)) {
            std::cerr << "Syntax Error: goto expects 1 argument.\n";
            return false;
        }
        _arg1 = matches[1];
        _arg2 = -1;
        _instruction_type = VMOperationType::C_GOTO;
    } else if (instruction == "if-goto") {
        std::regex if_pattern(R"(^if-goto +(.*))");
        std::smatch matches;
        if (!std::regex_search(_curr_instruction, matches, if_pattern)) {
            std::cerr << "Syntax Error: if-goto expects 1 argument.\n";
            return false;
        }
        _arg1 = matches[1];
        _arg2 = -1;
        _instruction_type = VMOperationType::C_IF;
    } else if (instruction == "function") {
        std::regex function_pattern(R"(^function +(.*) +([0-9]+))");
        std::smatch matches;
        if (!std::regex_search(_curr_instruction, matches, function_pattern)) {
            std::cerr << "Syntax Error: invalid function declaration.\n";
            return false;
        }
        _curr_function_name = matches[1];
        _arg1 = matches[1];
        _arg2 = stoi(matches[2]);
        _instruction_type = VMOperationType::C_FUNCTION;
    } else if (instruction == "call") {
        std::regex call_pattern(R"(^call +(.*) +([0-9]+))");
        std::smatch matches;
        if (!std::regex_search(_curr_instruction, matches, call_pattern)) {
            std::cerr << "Syntax Error: invalid function invocation.\n";
            return false;
        }
        _arg1 = matches[1];
        _arg1 = stoi(matches[2]);
        _instruction_type = VMOperationType::C_CALL;
    } else if (instruction == "return") {
        std::regex call_pattern(R"(^return)");
        std::smatch matches;
        if (!std::regex_search(_curr_instruction, matches, call_pattern)) {
            std::cerr << "Syntax Error: invalid return statement.\n";
            return false;
        }
        // Clear arguments.
        _arg1 = "";
        _arg1 = -1;
        _instruction_type = VMOperationType::C_RETURN;
    } else {
        // Clear _arg1 and _arg2.
        _arg1 = "";
        _arg2 = -1;
        _instruction_type = VMOperationType::INVALID;
        return false;
    }
    return true;
}

void VMParser::show_instruction_debug_info() {
    std::cout << "Line " << _curr_line << ") " << _curr_instruction << "\n";
    switch (_instruction_type) {
        case VMOperationType::C_ARITHMETIC:
            std::cout << "\tInstruction: arithmetic" << "\n";
            break;
        case VMOperationType::C_PUSH:
            std::cout << "\tInstruction: push" << "\n";
            break;
        case VMOperationType::C_POP:
            std::cout << "\tInstruction: pop" << "\n";
            break;
        case VMOperationType::INVALID:
            std::cout << "\tInstruction: INVALID";
            break;
        case VMOperationType::C_LABEL:
            std::cout << "\tInstruction: label\n";
            break;
        case VMOperationType::C_GOTO:
            std::cout << "\tInstruction: goto\n";
            break;
        case VMOperationType::C_IF:
            std::cout << "\tInstruction: if-goto\n";
            break;
        case VMOperationType::C_FUNCTION:
            std::cout << "\tInstruction: function\n";
            break;
        case VMOperationType::C_CALL:
            std::cout << "\tInstruction: call\n";
            break;
        case VMOperationType::C_RETURN:
            std::cout << "\tInstruction: return\n";
            break;
        default:
            break;
    }
    std::cout << "\tArg1: " << _arg1 << "\n";
    std::cout << "\tArg2: " << _arg2 << "\n";
    if (!_curr_function_name.empty()) std::cout << "\tCurrent Function: " << _curr_function_name << "\n";
}
