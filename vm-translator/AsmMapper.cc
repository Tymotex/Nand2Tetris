#include "AsmMapper.h"
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>

std::unordered_map<std::string, int> AsmMapper::_predef_segment_base_addresses = {
    {"static", 16},
    {"temp", 5},
};

std::unordered_map<std::string, std::string> AsmMapper::_init_segment_addr_registers = {
    {"local", "LCL"},
    {"argument", "ARG"},
    {"this", "THIS"},
    {"that", "THAT"},
};

std::string AsmMapper::_constant_segment = "constant";
std::string AsmMapper::_pointer_segment = "pointer";

std::unordered_map<std::string, const char> AsmMapper::_arithmetic_logical_binary_op = {
    {"add", '+'},
    {"sub", '-'},
    {"and", '&'},
    {"or", '|'},
};

std::unordered_map<std::string, const char> AsmMapper::_arithmetic_logical_unary_op = {
    {"not", '!'},
    {"neg", '-'}
};

std::unordered_map<std::string, std::string> AsmMapper::_comparison_op = {
    {"eq", "JEQ"},
    {"gt", "JGT"},
    {"lt", "JLT"}
};

AsmMapper::AsmMapper(const std::string& asm_output_file_path)
    : _asm_out(std::ofstream(asm_output_file_path)),
      _label_count(0) {
}

/**
 * Assumes that the given command is valid.
 * 
 * Binary operators (add, sub, eq, etc.):
 * 1. First decrement sp by performing sp--
 *      @SP
 *      M = M - 1
 * 2. Now RAM[sp] == b. Save this to D
 *      A = M
 *      D = M
 * 3. RAM[sp - 1] == a. Write a op b into where a was.
 *      A = A - 1 
 *      M = M op D    // A op B
 * 
 * Unary operators (not, etc.):
 * 1. Write RAM[sp-1] = op RAM[sp-1]
 *      @SP
 *      A = M - 1
 *      M = op M
 * 
 * Comparison operators:
 * 1. First decrement sp by performing sp--
 *      @SP
 *      M = M - 1   // --sp
 *      A = M
 *      D = M       // D contains second operand 'b'.
 *      A = A - 1   // Look up RAM[sp - 2], which is where first operand `a` is.
 *      D = M - D   // a - b
 *      M = -1       // Write 'true' to where 'a' is.
 *      @COMP_i     
 *      D;JEQ       // if a - b == 0, then leave the result as true. We change JEQ to JLT or JGT depending on the comparison operator.
 *      M = 0
 *  (COMP_i) 
 *      ...
 */
void AsmMapper::write_arithmetic(const std::string& command) {
    _asm_out << "// " << command << "\n";

    if (_arithmetic_logical_binary_op.find(command) != _arithmetic_logical_binary_op.end()) {
        // Binary arithmetic/logic operation.
        const char op_character = _arithmetic_logical_binary_op[command];
        _asm_out << "\t@SP\n"          // TODO: dupe1 start
                 << "\tM = M - 1\n"
                 << "\tA = M\n"
                 << "\tD = M\n"
                 << "\tA = A - 1\n"    // TODO: dupe1 end
                 << "\tM = M " << op_character << " D\n";
    } else if (_arithmetic_logical_unary_op.find(command) != _arithmetic_logical_unary_op.end()) {
        // Unary arithmetic/logic operation.
        const char op_character = _arithmetic_logical_unary_op[command];
        _asm_out << "\t@SP\n" 
                 << "\tA = M - 1\n"
                 << "\tM = " << op_character << "M\n";
    } else if (_comparison_op.find(command) != _comparison_op.end()) {
        // Comparison operation.
        const std::string jump_instr = _comparison_op[command];
        _asm_out << "\t@SP\n"         // TODO: dupe1 start
                 << "\tM = M - 1\n"
                 << "\tA = M\n"
                 << "\tD = M\n"
                 << "\tA = A - 1\n"   // TODO: dupe1 end
                 << "\tD = M - D\n"
                 << "\tM = -1\n"
                 << "\t@COMP_" << _label_count << "\n"
                 << "\tD;" << jump_instr << "\n"
                 << "\t@SP\n"
                 << "\tA = M - 1\n"
                 << "\tM = 0\n"
                 << "(COMP_" << _label_count++ << ")\n";
    } else {
        std::cerr << "Syntax Error: unknown arithmetic command '" << command << "', len: " << command.size() << "\n";
    }
}

/**
 * Push operation:
 * 1. Read from `segment[index]` and make the value available in D.
 *      @segmentReg
 *      D = M
 *      @offset     // Or we can directly just do @base_addr + offset if the segment has a predefined address like static or temp.
 *      A = A + D   // Address of segment + offset
 *      D = M
 * 2. Write D to the stack.
 *      @SP 
 *      A = M
 *      M = D
 *      @SP
 *      M = M + 1
 * 
 * For the `constant` segment,
 * 1. Read the value into D.
 *      @constant
 *      D = A
 * 2. Set the value at RAM[++sp].
 *      @SP
 *      M = M + 1
 *      A = M
 *      M = D
 */
void AsmMapper::write_push(const std::string& command,
        const std::string& segment, const int& index) {
    _asm_out << "// " << command << "\n";

    if (_predef_segment_base_addresses.find(segment) != _predef_segment_base_addresses.end()) {
        const int segment_base_addr = _predef_segment_base_addresses[segment];
        _asm_out << "\t@" << segment_base_addr + index << "\n"
                 << "\tD = M\n"         // TODO: Duplicated 2
                 << "\t@SP\n"
                 << "\tA = M\n"
                 << "\tM = D\n"
                 << "\t@SP\n"
                 << "\tM = M + 1\n";
    } else if (_init_segment_addr_registers.find(segment) != _init_segment_addr_registers.end()) {
        const std::string segment_register = _init_segment_addr_registers[segment];
        _asm_out << "\t@" << segment_register << "\n"
                 << "\tD = M\n"
                 << "\t@" << index << "\n"
                 << "\tA = A + D\n"
                 << "\tD = M\n"         // TODO: Duplicated 2
                 << "\t@SP\n"
                 << "\tA = M\n"
                 << "\tM = D\n"
                 << "\t@SP\n"
                 << "\tM = M + 1\n";
    } else if (segment == _constant_segment) {
        _asm_out << "\t@" << index << "\n"
                 << "\tD = A\n"
                 << "\t@SP\n"
                 << "\tM = M + 1\n"
                 << "\tA = M - 1\n"
                 << "\tM = D\n";
    } else if (segment == _pointer_segment) {
        // Retrieve value directly from the `this` or `that` register.
        const std::string this_or_that = (index == 0) ? _init_segment_addr_registers["this"] : _init_segment_addr_registers["that"];
        _asm_out << "\t@" << this_or_that << "\n"
                 << "\tD = M\n"
                 << "\t@SP\n"            // TODO: extract out 'push to stack'
                 << "\tM = M + 1\n"
                 << "\tA = M - 1\n"
                 << "\tM = D\n";
    } else {
        std::cerr << "Syntax Error: unknown segment '" << segment << "'\n";
    }
}

/**
 * Pop operation:
 * 1. Save address of `segment[index]` to a tmp register.
 *      @segmentReg
 *      D = M
 *      @offset    // Or we can directly just do @base_addr + offset if the segment has a predefined address like static or temp.
 *      D = A + D  // D contains address of segment[index]
 *      @R13
 *      M = D
 * 2. Load RAM[sp--].into D.
 *      @SP 
 *      M = M - 1
 *      A = M
 *      D = M 
 * 3. Write D into the saved memory address at `segment[index]`.
 *      @R13 
 *      A = M
 *      M = D
 */
void AsmMapper::write_pop(const std::string& command,
        const std::string& segment,
        const int& index) {
    _asm_out << "// " << command << "\n";
    if (_predef_segment_base_addresses.find(segment) != _predef_segment_base_addresses.end()) {
        const int segment_base_addr = _predef_segment_base_addresses[segment];
        _asm_out << "\t@" << segment_base_addr + index << "\n"
                 << "\tD = A\n"   
                 << "\t@R13\n"    // TODO: duplicated, 4
                 << "\tM = D\n"
                 << "\t@SP\n"
                 << "\tM = M - 1\n"
                 << "\tA = M\n"
                 << "\tD = M\n"
                 << "\t@R13\n"
                 << "\tA = M\n"
                 << "\tM = D\n";
    } else if (_init_segment_addr_registers.find(segment) != _init_segment_addr_registers.end()) {
        const std::string segment_register = _init_segment_addr_registers[segment];
        _asm_out << "\t@" << segment_register << "\n"
                 << "\tD = M\n"
                 << "\t@" << index << "\n"
                 << "\tD = A + D\n"   // TODO: duplicated, 4
                 << "\t@R13\n"
                 << "\tM = D\n"
                 << "\t@SP\n"
                 << "\tM = M - 1\n"
                 << "\tA = M\n"
                 << "\tD = M\n"
                 << "\t@R13\n"
                 << "\tA = M\n"
                 << "\tM = D\n";
    } else if (segment == _pointer_segment) {
        const std::string this_or_that = (index == 0) ? _init_segment_addr_registers["this"] : _init_segment_addr_registers["that"];
        _asm_out << "\t@SP\n"
                 << "\tM = M - 1\n"
                 << "\tA = M\n"
                 << "\tD = M\n"
                 << "\t@" << this_or_that << "\n"
                 << "\tM = D\n";
    } else {
        std::cerr << "Syntax Error: unknown segment '" << segment << "'\n";
    }
}

void AsmMapper::write_inf_loop() {
    _asm_out << "// Final infinite loop.\n";
    _asm_out << "(END_INF)\n"
             << "\t@END_INF\n"
             << "\t0;JEQ\n";
}

void AsmMapper::close() {
    _asm_out.close();
}
