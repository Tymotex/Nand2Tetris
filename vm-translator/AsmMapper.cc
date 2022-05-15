#include "AsmMapper.h"
#include <fstream>
#include <unordered_map>
#include <string>

std::unordered_map<std::string, int> AsmMapper::_segment_base_addresses = {
    {"static", 16},
    {"local", 2}
};

AsmMapper::AsmMapper(const std::string& asm_output_file_path)
    : _asm_out(std::ofstream(asm_output_file_path)) {
}

/**
 * 1. First decrement sp by performing sp--
 *      @sp
 *      M = M - 1
 * 2. Now RAM[sp] == b. Save this to D
 *      A = M
 *      D = M
 * 3. RAM[sp - 1] == a. Write a op b into where a was.
 *      A = A - 1 
 *      M = M op D    // A op B
 */
void AsmMapper::write_arithmetic(const std::string& command) {
    _asm_out << "// " << command << "\n";
    
}

/**
 * Push operation:
 * 1. Read from `segment[index]` and make the value available in D.
 *      @segmentReg
 *      D = M
 *      @offset 
 *      A = A + D   // Address of segment + offset
 *      D = M
 * 2. Write D to the stack.
 *      @sp 
 *      A = M
 *      M = D
 *      @sp
 *      M = M + 1
 * 
 * Pop operation:
 * 1. Save address of `segment[index]` to a tmp register.
 *      @segmentReg
 *      D = M
 *      @offset
 *      D = A + D  // D contains address of segment[index]
 *      @R13
 *      M = D
 * 2. Load RAM[sp--].into D.
 *      @sp 
 *      M = M - 1
 *      A = M
 *      D = M 
 * 3. Write D into the saved memory address at `segment[index]`.
 *      @R13 
 *      A = M
 *      M = D
 */
void AsmMapper::write_push_pop(const std::string& command,
        const std::string& segment,
        const int& index) {
    _asm_out << "// " << command << "\n";

}

void AsmMapper::close() {
    _asm_out.close();
}
