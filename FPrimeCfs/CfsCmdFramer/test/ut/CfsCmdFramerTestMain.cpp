// ======================================================================
// \title  CfsCmdFramerTestMain.cpp
// \brief  cpp file for CfsCmdFramer component test main function
// ======================================================================

#include "CfsCmdFramerTester.hpp"
#include "Fw/Test/UnitTest.hpp"
#include "STest/Random/Random.hpp"

TEST(Nominal, Framing) {
    COMMENT("Wrapping prepends the cFS command secondary header and sets the context flag");
    REQUIREMENT("FPRIMECFS-CFSCMDFRAMER-001");
    FPrimeCfs::CfsCmdFramerTester tester;
    tester.testNominalFraming();
}

TEST(Nominal, Checksum) {
    COMMENT("The checksum makes the predicted complete command packet XOR to zero");
    REQUIREMENT("FPRIMECFS-CFSCMDFRAMER-002");
    FPrimeCfs::CfsCmdFramerTester tester;
    tester.testChecksum();
}

TEST(OffNominal, AllocationFailure) {
    COMMENT("An undersized allocation drops the packet and returns the data");
    REQUIREMENT("FPRIMECFS-CFSCMDFRAMER-003");
    FPrimeCfs::CfsCmdFramerTester tester;
    tester.testAllocationFailure();
}

TEST(Nominal, ComStatusPassthrough) {
    COMMENT("comStatusIn is passed through to comStatusOut");
    REQUIREMENT("FPRIMECFS-CFSCMDFRAMER-004");
    FPrimeCfs::CfsCmdFramerTester tester;
    tester.testComStatusPassthrough();
}

TEST(Nominal, DataReturnPassthrough) {
    COMMENT("dataReturnIn deallocates the wrapped buffer");
    REQUIREMENT("FPRIMECFS-CFSCMDFRAMER-005");
    FPrimeCfs::CfsCmdFramerTester tester;
    tester.testDataReturnPassthrough();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    STest::Random::seed();
    return RUN_ALL_TESTS();
}
