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

AsmMapper::AsmMapper(const std::string& asm_output_file_path, 
        const std::string& translation_unit_name) {
    start_new_translation_unit(asm_output_file_path, translation_unit_name);
}

void AsmMapper::start_new_translation_unit(const std::string& asm_output_file_path,
        const std::string& translation_unit_name) {
    // Close previous output file stream.
    if (_asm_out.is_open()) _asm_out.close();

    // Initialise new .asm output stream.
    _asm_out = std::ofstream(asm_output_file_path);
    _trans_unit_name = translation_unit_name;
    _label_count = 0;
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
 *      M = -1      // Write 'true' to where 'a' is.
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
        pop_from_stack();             // D contains first operand.
        _asm_out << "\tA = A - 1\n"   // M contains second operand.
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
        pop_from_stack();             // D contains first operand.
        _asm_out << "\tA = A - 1\n"   // M contains second operand.
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

void AsmMapper::push_to_stack() {
    _asm_out << "\t@SP   // Pushing to stack.\n"
             << "\tM = M + 1\n"
             << "\tA = M - 1\n"
             << "\tM = D // Done pushing.\n";
}

template <typename T>
void AsmMapper::push_to_stack(const T& value, const bool& use_register_a) {
    // Set the D register to the given value. If an integer was given, then we
    // load D with that integer. If a symbol (string) was given, then we load
    // D with the contents at that symbol's memory address (ie. the M register).
    _asm_out << "\t@" << value << "\n"
             << "\tD = " << (use_register_a ? "A" : "M") << "\n";
    push_to_stack();
}

void AsmMapper::pop_from_stack() {
    _asm_out << "\t@SP   // Popping from stack.\n"
             << "\tM = M - 1\n"
             << "\tA = M\n"
             << "\tD = M // Done popping.\n";
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
                 << "\tD = M\n";
        push_to_stack();
    } else if (_init_segment_addr_registers.find(segment) != _init_segment_addr_registers.end()) {
        const std::string segment_register = _init_segment_addr_registers[segment];
        _asm_out << "\t@" << segment_register << "\n"
                 << "\tD = M\n"
                 << "\t@" << index << "\n"
                 << "\tA = A + D\n"
                 << "\tD = M\n";
        push_to_stack();
    } else if (segment == _constant_segment) {
        _asm_out << "\t@" << index << "\n"
                 << "\tD = A\n";
        push_to_stack();
    } else if (segment == _pointer_segment) {
        // Retrieve value directly from the `this` or `that` register.
        const std::string this_or_that = (index == 0) ? _init_segment_addr_registers["this"] : _init_segment_addr_registers["that"];
        _asm_out << "\t@" << this_or_that << "\n"
                 << "\tD = M\n";
        push_to_stack();
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
                 << "\t@R13\n"    
                 << "\tM = D\n";
        pop_from_stack();
        _asm_out << "\t@R13\n"
                 << "\tA = M\n"
                 << "\tM = D\n";
    } else if (_init_segment_addr_registers.find(segment) != _init_segment_addr_registers.end()) {
        const std::string segment_register = _init_segment_addr_registers[segment];
        _asm_out << "\t@" << segment_register << "\n"
                 << "\tD = M\n"
                 << "\t@" << index << "\n"
                 << "\tD = A + D\n"   
                 << "\t@R13\n"   
                 << "\tM = D\n";
        pop_from_stack();
        _asm_out << "\t@R13\n"
                 << "\tA = M\n"
                 << "\tM = D\n";
    } else if (segment == _pointer_segment) {
        const std::string this_or_that = (index == 0) ? _init_segment_addr_registers["this"] : _init_segment_addr_registers["that"];
        pop_from_stack();
        _asm_out << "\t@" << this_or_that << "\n"
                 << "\tM = D\n";
    } else {
        std::cerr << "Syntax Error: unknown segment '" << segment << "'\n";
    }
}

void AsmMapper::write_label(const std::string& command, const std::string& label, const std::string& function_name) {
    _asm_out << "// " << command << "\n";
    if (function_name.empty()) {
        const std::string label_name = _trans_unit_name + "." + label;
        _asm_out << "(" << label_name << ")\n";
    } else {
        const std::string label_name = _trans_unit_name + "." + function_name + "$" + label;
        _asm_out << "(" << label_name << ")\n";
    }
}

void AsmMapper::write_goto(const std::string& command, const std::string& label, const std::string& function_name) {
    _asm_out << "// " << command << "\n";
    const std::string dest_name = get_dest_name(label, function_name);
    _asm_out << "\t@"  << dest_name << "\n" 
                << "\t0;JMP\n";
}

void AsmMapper::write_if(const std::string& command, const std::string& label, const std::string& function_name) {
    _asm_out << "// " << command << "\n";
    const std::string dest_name = get_dest_name(label, function_name);
    pop_from_stack();
    _asm_out << "\t@" << dest_name << " // Conditional jump.\n"
             << "\tD;JNE\n";
}

void AsmMapper::write_function(const std::string& command, const std::string& function_name, const int& num_local_vars) {
    _asm_out << "// " << command << "\n";

    // Create label and push to the stack `num_local_vars` local variables whose
    // values are 0.
    _asm_out << "(" << function_name << ")  // Function declaration.\n";
    for (int i = 0; i < num_local_vars; ++i)
        push_to_stack(0, true);
}

void AsmMapper::write_call(const std::string& command, const std::string& target_function_name, const int& num_args, const int& return_counter) {
    _asm_out << "// " << command << "\n";
    
    const std::string return_label = _trans_unit_name + "." + target_function_name + "$ret." + std::to_string(return_counter);

    // Save return address that the callee returns to.
    push_to_stack(return_label, true);

    // Saves caller's state.
    push_to_stack("LCL", false);
    push_to_stack("ARG", false);
    push_to_stack("THIS", false);
    push_to_stack("THAT", false);

    // Point ARG to the base address where the callee can expect its `num_args`
    // arguments.
    // Ie. we set ARG = (SP - 5) - num_args.
    push_to_stack("SP", false);
    push_to_stack(5, true);
    write_arithmetic("sub");
    push_to_stack(num_args, true);
    write_arithmetic("sub");

    pop_from_stack();
    _asm_out << "\t@ARG\n"
             << "\tM = D\n";

    // Point LCL to the base address where the callee can expect its local
    // variables to start from. 
    _asm_out << "\t@SP\n"
             << "\tD = M\n"
             << "\t@LCL\n"
             << "\tM = D\n";

    // Jump to callee.
    _asm_out << "\t@"  << target_function_name << "\n" 
             << "\t0;JMP\n";
    
    // Inject the return label.
    _asm_out << "(" << return_label << ")\n";
}

void AsmMapper::write_return(const std::string& command, const std::string& function_name) {
    _asm_out << "// " << command << "\n";

    // Save R13 = LCL (ie. int frame = LCL address).
    _asm_out << "\t@LCL // frame = LCL\n"
             << "\tD = M\n"
             << "\t@R13\n"
             << "\tM = D\n";

    // Set R14 = R13 - 5 (ie. int ret = *frame - 5).
    push_to_stack("R13", false);
    push_to_stack(5, true);
    write_arithmetic("sub");
    pop_from_stack();

    _asm_out << "\t@R14 // ret = *(frame - 5)\n"
             << "\tA = D\n"
             << "\tM = D\n";
             
    // Set ARG = pop_from_stack(). This is what places the return value to where
    // the caller expects it. From their point of view, they 'traded' what they 
    // put as function arguments before the call into the return value after the
    // call.
    pop_from_stack();        // We expect the top of the stack to contain the return value.
    _asm_out << "\t@ARG\n"
             << "\tA = M\n"
             << "\tM = D\n"       // Wrote to return value to *ARG.
             << "\tD = A + 1\n";  // D = ARG + 1. We want SP = D in the next lines.
    
    // Set SP = ARG + 1, which is right after where the caller sees the return
    // value.
    _asm_out << "\t@SP\n"
             << "\tM = D\n";
    
    // Restore THAT, THIS, ARG, LCL.
    // TODO: lots of duplication.
    _asm_out << "\t// Restoring THAT, THIS, ARG, LCL.\n";
    _asm_out << "\t@R13\n"       // R13 contains the memory address of where the LCL segment is.
             << "\tA = M - 1\n"  // Now M == RAM[frame - 1], so M contains the memory address of the THAT segment.
             << "\tD = M\n"      // D contains what's stored at LCL - 1, ie. THAT's previous address.
             << "\t@THAT\n"
             << "\tM = D\n";     // THAT has the memory address stored at LCL - 1.
    _asm_out << "\t@R13\n"
             << "\tA = M - 1\n"
             << "\tA = A - 1\n"  // Now M == RAM[frame - 2], so M contains the memory address of the THIS segment.
             << "\tD = M\n"      // D contains what's stored at LCL - 2, ie. THAT's previous address.
             << "\t@THIS\n"
             << "\tM = D\n";     // THIS has the memory address stored at LCL - 1.
    _asm_out << "\t@R13\n"
             << "\tA = M - 1\n"
             << "\tA = A - 1\n"
             << "\tA = A - 1\n"
             << "\tD = M\n"      // D contains what's stored at LCL - 3, ie. ARG's previous address.
             << "\t@ARG\n"
             << "\tM = D\n";     // ARG has the memory address stored at LCL - 1.
    _asm_out << "\t@R13\n"
             << "\tA = M - 1\n"
             << "\tA = A - 1\n"
             << "\tA = A - 1\n"
             << "\tA = A - 1\n"
             << "\tD = M\n"      // D contains what's stored at LCL - 4, ie. LCL's previous address.
             << "\t@LCL\n"
             << "\tM = D\n";     // LCL has the memory address stored at LCL - 1.
    
    // Jump to the return label.
    _asm_out << "\t@R14\n"
             << "\tA = M\n"
             << "\t0;JMP\n";
}

void AsmMapper::write_inf_loop() {
    _asm_out << "// ===== Final infinite loop =====\n";
    _asm_out << "(END_INF)\n"
             << "\t@END_INF\n"
             << "\t0;JEQ\n"
             << "// Done.";
}

void AsmMapper::write_bootstrap_init() {
    _asm_out << "// ===== Boostrap Start =====\n"
             << "@256  // Initialise stack pointer to base of stack.\n"
             << "D = A\n"
             << "@SP\n"
             << "M = D // Done initialising stack pointer.\n"
             << "// ===== Boostrap End =====\n";
}

void AsmMapper::close() {
    _asm_out.close();
}

std::string AsmMapper::get_dest_name(const std::string& label, const std::string& function_name) {
    if (label.empty()) {
        std::cerr << "Syntax Error: empty label.\n";
        return label;
    }
    return _trans_unit_name + "." + (function_name.empty() ? "" : function_name + "$") + label;
}
