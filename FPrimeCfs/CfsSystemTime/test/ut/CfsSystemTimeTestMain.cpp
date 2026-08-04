// ======================================================================
// \title  CfsSystemTimeTestMain.cpp
// \brief  cpp file for CfsSystemTime component test main function
// ======================================================================

#include "CfsSystemTimeTester.hpp"

TEST(Nominal, TestGetTime) {
    FPrimeCfs::CfsSystemTimeTester tester;
    tester.test_get_time();
}

TEST(Nominal, TestConvertTime) {
    FPrimeCfs::CfsSystemTimeTester tester;
    tester.test_convert_time();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
