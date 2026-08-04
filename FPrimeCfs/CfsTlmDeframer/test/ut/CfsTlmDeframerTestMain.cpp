// ======================================================================
// \title  CfsTlmDeframerTestMain.cpp
// \brief  cpp file for CfsTlmDeframer component test main function
// ======================================================================

#include "CfsTlmDeframerTester.hpp"
#include "Fw/Test/UnitTest.hpp"
#include "STest/Random/Random.hpp"

TEST(Nominal, StripTelemetrySecHdr) {
    COMMENT("A telemetry packet with a secondary header is stripped in place");
    REQUIREMENT("FPRIMECFS-CFSTLMDEFRAMER-001");
    REQUIREMENT("FPRIMECFS-CFSTLMDEFRAMER-002");
    FPrimeCfs::CfsTlmDeframerTester tester;
    tester.testStripTelemetrySecHdr();
}

TEST(Nominal, PassthroughBareTelemetry) {
    COMMENT("A telemetry packet without a secondary header passes through unmodified");
    REQUIREMENT("FPRIMECFS-CFSTLMDEFRAMER-003");
    FPrimeCfs::CfsTlmDeframerTester tester;
    tester.testPassthroughBareTelemetry();
}

TEST(Nominal, PassthroughCommandPacket) {
    COMMENT("A command packet passes through unmodified");
    REQUIREMENT("FPRIMECFS-CFSTLMDEFRAMER-003");
    FPrimeCfs::CfsTlmDeframerTester tester;
    tester.testPassthroughCommandPacket();
}

TEST(OffNominal, PassthroughShortPacket) {
    COMMENT("A packet too short to strip passes through unmodified");
    REQUIREMENT("FPRIMECFS-CFSTLMDEFRAMER-004");
    FPrimeCfs::CfsTlmDeframerTester tester;
    tester.testPassthroughShortPacket();
}

TEST(Nominal, DataReturnPassthrough) {
    COMMENT("A buffer returned on dataReturnIn is returned upstream");
    REQUIREMENT("FPRIMECFS-CFSTLMDEFRAMER-005");
    FPrimeCfs::CfsTlmDeframerTester tester;
    tester.testDataReturnPassthrough();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    STest::Random::seed();
    return RUN_ALL_TESTS();
}
