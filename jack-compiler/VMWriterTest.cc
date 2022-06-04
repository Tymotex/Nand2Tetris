#include "VMWriter.h"
#include <gtest/gtest.h>
#include <ostream>

class VMWriterTestFixture : public ::testing::Test {
protected:
    std::ostringstream s;
    VMWriter vm_writer;

    VMWriterTestFixture() : vm_writer(s) {
    }
};

TEST_F(VMWriterTestFixture, BasicPushTest) {
    vm_writer.write_push(VirtualMemorySegment::ARGUMENT, 0);
    std::string vm_code = s.str();

    EXPECT_EQ(vm_code, "push argument 0");
}

TEST_F(VMWriterTestFixture, BasicPopTest) {
    vm_writer.write_pop(VirtualMemorySegment::ARGUMENT, 0);
    std::string vm_code = s.str();

    EXPECT_EQ(vm_code, "pop argument 0");
}
