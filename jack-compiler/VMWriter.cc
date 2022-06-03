#include "VMWriter.h"
#include <ostream>

VMWriter::VMWriter(std::ostream& stream)
        : _vm_out(stream) {
}

VMWriter::~VMWriter() {
    // Instances of `std::ofstream` need not be closed explicitly.
}

void VMWriter::write_push(const VirtualMemorySegment& segment, const int index) {

}

void VMWriter::write_pop(const VirtualMemorySegment& segment, const int index) {

}

void VMWriter::write_arithmetic(const std::string& operator_symbol) {

}

void VMWriter::write_label(const std::string& label) {

}

void VMWriter::write_goto(const std::string& label) {

}

void VMWriter::write_if(const std::string& label) {

}

void VMWriter::write_call(const std::string& function_name, const int num_args) {

}

void VMWriter::write_function(const std::string& function_name, const int num_variables) {

}

void VMWriter::write_return() {

}
