#include "VMWriter.h"
#include <iostream>

VMWriter::VMWriter(std::ostream& stream)
        : _vm_out(stream) {
}

VMWriter::~VMWriter() {
    // Instances of `std::ofstream` need not be closed explicitly.
}

void VMWriter::write_push(const VirtualMemorySegment& segment, const int index) {
    _vm_out << "push "
            << segment_to_str(segment)
            << " "
            << index;
}

void VMWriter::write_pop(const VirtualMemorySegment& segment, const int index) {
    _vm_out << "pop "
            << segment_to_str(segment)
            << " "
            << index;
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

// TODO: Make this into a map
std::string VMWriter::segment_to_str(const VirtualMemorySegment& segment) {
    switch (segment) {
        case VirtualMemorySegment::ARGUMENT:
            return "argument";
        case VirtualMemorySegment::CONSTANT:
            return "constant";
        case VirtualMemorySegment::LOCAL:
            return "local";
        case VirtualMemorySegment::STATIC:
            return "static";
        case VirtualMemorySegment::THIS:
            return "this";
        case VirtualMemorySegment::THAT:
            return "that";
        case VirtualMemorySegment::POINTER:
            return "pointer";
        case VirtualMemorySegment::TEMP:
            return "temp";
        default:
            throw std::invalid_argument("Invalid segment.");
    }
}