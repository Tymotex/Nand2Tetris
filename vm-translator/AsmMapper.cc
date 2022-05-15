#include "AsmMapper.h"
#include <fstream>

AsmMapper::AsmMapper(const std::string& asm_output_file_path)
    : _asm_out(std::ofstream(asm_output_file_path)) {
}

void AsmMapper::write_arithmetic(const std::string& command) {

}

void AsmMapper::write_push_pop(const std::string& command,
        const std::string& segment,
        const int& index) {

}

void AsmMapper::close() {
    _asm_out.close();
}
