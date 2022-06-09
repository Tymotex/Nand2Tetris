#include "VMWriter.h"
#include <gtest/gtest.h>
#include <ostream>

class VMWriterTestFixture : public ::testing::Test {
protected:
    std::ostringstream output;
    VMWriter vm_writer;

    VMWriterTestFixture() : vm_writer(output) {
    }
};

TEST_F(VMWriterTestFixture, BasicPushTest) {
    vm_writer.write_push(VirtualMemorySegment::ARGUMENT, 0);
    std::string vm_code = output.str();

    EXPECT_EQ(vm_code, "push argument 0\n");
}

TEST_F(VMWriterTestFixture, BasicPopTest) {
    vm_writer.write_pop(VirtualMemorySegment::ARGUMENT, 0);
    std::string vm_code = output.str();

    EXPECT_EQ(vm_code, "pop argument 0\n");
}

TEST_F(VMWriterTestFixture, ArithmeticLogicOperatorWriteTest) {
    vm_writer.write_arithmetic(ArithmeticLogicOp::ADD);
    vm_writer.write_arithmetic(ArithmeticLogicOp::SUB);
    vm_writer.write_arithmetic(ArithmeticLogicOp::NEG);
    vm_writer.write_arithmetic(ArithmeticLogicOp::EQ);
    vm_writer.write_arithmetic(ArithmeticLogicOp::GT);
    vm_writer.write_arithmetic(ArithmeticLogicOp::LT);
    vm_writer.write_arithmetic(ArithmeticLogicOp::AND);
    vm_writer.write_arithmetic(ArithmeticLogicOp::OR);
    vm_writer.write_arithmetic(ArithmeticLogicOp::NOT);

    std::string vm_code = output.str();
    EXPECT_EQ(vm_code, "add\n"
                       "sub\n"
                       "neg\n"
                       "eq\n"
                       "gt\n"
                       "lt\n"
                       "and\n"
                       "or\n"
                       "not\n");
}

TEST_F(VMWriterTestFixture, WriteLabelTest) {
    vm_writer.write_label("IF_TRUE0");
    std::string vm_code = output.str();

    EXPECT_EQ(vm_code, "label IF_TRUE0\n");
}

TEST_F(VMWriterTestFixture, WriteGotoTest) {
    vm_writer.write_goto("IF_TRUE0");
    std::string vm_code = output.str();

    EXPECT_EQ(vm_code, "goto IF_TRUE0\n");
}

TEST_F(VMWriterTestFixture, WriteIfTest) {
    vm_writer.write_if("IF_TRUE0");
    std::string vm_code = output.str();

    EXPECT_EQ(vm_code, "if-goto IF_TRUE0\n");
}

TEST_F(VMWriterTestFixture, WriteCallTest) {
    vm_writer.write_call("Foo.bar", 2);
    std::string vm_code = output.str();

    EXPECT_EQ(vm_code, "call Foo.bar 2\n");
}

TEST_F(VMWriterTestFixture, WriteFunctionTest) {
    vm_writer.write_function("Main.main", 0);
    std::string vm_code = output.str();

    EXPECT_EQ(vm_code, "function Main.main 0\n");
}

TEST_F(VMWriterTestFixture, WriteReturnTest) {
    vm_writer.write_return();
    std::string vm_code = output.str();

    EXPECT_EQ(vm_code, "return\n");
}

