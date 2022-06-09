#include "VMWriter.h"
#include <iostream>
#include <unordered_map>

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
            << index
            << "\n";
}

void VMWriter::write_pop(const VirtualMemorySegment& segment, const int index) {
    _vm_out << "pop "
            << segment_to_str(segment)
            << " "
            << index
            << "\n";
}

void VMWriter::write_arithmetic(const ArithmeticLogicOp op_type) {
    switch (op_type) {
        case ArithmeticLogicOp::ADD:
            _vm_out << "add\n";
            break;
        case ArithmeticLogicOp::SUB:
            _vm_out << "sub\n";
            break;
        case ArithmeticLogicOp::NEG:
            _vm_out << "neg\n";
            break;
        case ArithmeticLogicOp::EQ:
            _vm_out << "eq\n";
            break;
        case ArithmeticLogicOp::GT:
            _vm_out << "gt\n";
            break;
        case ArithmeticLogicOp::LT:
            _vm_out << "lt\n";
            break;
        case ArithmeticLogicOp::AND:
            _vm_out << "and\n";
            break;
        case ArithmeticLogicOp::OR:
            _vm_out << "or\n";
            break;
        case ArithmeticLogicOp::NOT:
            _vm_out << "not\n";
            break;
        default:
            throw std::invalid_argument("Unknown arithmetic/logical operation type.");
    }
}

void VMWriter::write_arithmetic(const std::string& op) {
    if (op == "+") write_arithmetic(ArithmeticLogicOp::ADD);
    else if (op == "-") write_arithmetic(ArithmeticLogicOp::SUB);
    else if (op == "&") write_arithmetic(ArithmeticLogicOp::AND);
    else if (op == "|") write_arithmetic(ArithmeticLogicOp::OR);
    else if (op == "<") write_arithmetic(ArithmeticLogicOp::LT);
    else if (op == ">") write_arithmetic(ArithmeticLogicOp::GT);
    else if (op == "=") write_arithmetic(ArithmeticLogicOp::EQ);
    else throw std::invalid_argument(
        "No direct VM mapping for operator '" + op + "'.");
}

void VMWriter::write_label(const std::string& label) {
    _vm_out << "label " << label << "\n";
}

void VMWriter::write_goto(const std::string& label) {
    _vm_out << "goto " << label << "\n";
}

void VMWriter::write_if(const std::string& label) {
    _vm_out << "if-goto " << label << "\n";
}

void VMWriter::write_call(const std::string& function_name, const int num_args) {
    _vm_out << "call " << function_name << " " << num_args << "\n";
}

void VMWriter::write_function(const std::string& function_name, const int num_variables) {
    _vm_out << "function " << function_name << " " << num_variables << "\n";
}

void VMWriter::write_return() {
    _vm_out << "return\n";
}

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