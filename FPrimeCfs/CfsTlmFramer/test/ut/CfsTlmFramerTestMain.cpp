// ======================================================================
// \title  CfsTlmFramerTestMain.cpp
// \brief  cpp file for CfsTlmFramer component test main function
// ======================================================================

#include "CfsTlmFramerTester.hpp"
#include "Fw/Test/UnitTest.hpp"
#include "STest/Random/Random.hpp"

TEST(Nominal, FrameTelemetryPacket) {
    COMMENT("A telemetry packet is framed with the cFS telemetry secondary header");
    REQUIREMENT("FPRIMECFS-CFSTLMFRAMER-001");
    REQUIREMENT("FPRIMECFS-CFSTLMFRAMER-002");
    FPrimeCfs::CfsTlmFramerTester tester;
    tester.testFrameTelemetryPacket();
}

TEST(Nominal, PassthroughCommandPacket) {
    COMMENT("A command packet passes through unmodified");
    REQUIREMENT("FPRIMECFS-CFSTLMFRAMER-003");
    FPrimeCfs::CfsTlmFramerTester tester;
    tester.testPassthroughCommandPacket();
}

TEST(Nominal, PassthroughSecHdrTelemetry) {
    COMMENT("A telemetry packet already carrying a secondary header passes through unmodified");
    REQUIREMENT("FPRIMECFS-CFSTLMFRAMER-003");
    FPrimeCfs::CfsTlmFramerTester tester;
    tester.testPassthroughSecHdrTelemetry();
}

TEST(Nominal, MultiplePackets) {
    COMMENT("A multi-packet buffer frames only the telemetry packets lacking a secondary header");
    REQUIREMENT("FPRIMECFS-CFSTLMFRAMER-004");
    FPrimeCfs::CfsTlmFramerTester tester;
    tester.testMultiplePackets();
}

TEST(OffNominal, MalformedBuffer) {
    COMMENT("A malformed buffer is forwarded verbatim");
    REQUIREMENT("FPRIMECFS-CFSTLMFRAMER-005");
    FPrimeCfs::CfsTlmFramerTester tester;
    tester.testMalformedBuffer();
}

TEST(OffNominal, AllocationFailure) {
    COMMENT("An allocation failure drops the buffer, returns it, and keeps com status flowing");
    REQUIREMENT("FPRIMECFS-CFSTLMFRAMER-006");
    FPrimeCfs::CfsTlmFramerTester tester;
    tester.testAllocationFailure();
}

TEST(Nominal, DataReturnDeallocate) {
    COMMENT("A buffer returned on dataReturnIn is deallocated");
    REQUIREMENT("FPRIMECFS-CFSTLMFRAMER-007");
    FPrimeCfs::CfsTlmFramerTester tester;
    tester.testDataReturnDeallocate();
}

TEST(Nominal, ComStatusPassthrough) {
    COMMENT("Com status passes through the component");
    REQUIREMENT("FPRIMECFS-CFSTLMFRAMER-008");
    FPrimeCfs::CfsTlmFramerTester tester;
    tester.testComStatusPassthrough();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    STest::Random::seed();
    return RUN_ALL_TESTS();
}
