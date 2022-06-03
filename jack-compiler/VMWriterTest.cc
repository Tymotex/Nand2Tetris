#include "VMWriter.h"
#include <gtest/gtest.h>
#include <ostream>

TEST(VMWriterTest, BasicAssertions) {
    std::ostringstream s;
    VMWriter vm_writer(s);
}
