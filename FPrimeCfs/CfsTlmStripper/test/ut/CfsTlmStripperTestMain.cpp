// ======================================================================
// \title  CfsTlmStripperTestMain.cpp
// \brief  cpp file for CfsTlmStripper component test main function
// ======================================================================

#include "CfsTlmStripperTester.hpp"
#include "Fw/Test/UnitTest.hpp"
#include "STest/Random/Random.hpp"

TEST(Nominal, TelemetryStrip) {
    COMMENT("A telemetry packet's cFS secondary header is stripped in place with a rebuilt primary header");
    REQUIREMENT("FPRIMECFS-CFSTLMSTRIPPER-001");
    REQUIREMENT("FPRIMECFS-CFSTLMSTRIPPER-002");
    REQUIREMENT("FPRIMECFS-CFSTLMSTRIPPER-003");
    FPrimeCfs::CfsTlmStripperTester tester;
    tester.testTelemetryStrip();
}

TEST(Nominal, CommandPassthrough) {
    COMMENT("A command packet is forwarded unchanged");
    REQUIREMENT("FPRIMECFS-CFSTLMSTRIPPER-004");
    FPrimeCfs::CfsTlmStripperTester tester;
    tester.testCommandPassthrough();
}

TEST(Nominal, NoSecHdrPassthrough) {
    COMMENT("A telemetry packet without the secondary header flag is forwarded unchanged");
    REQUIREMENT("FPRIMECFS-CFSTLMSTRIPPER-004");
    FPrimeCfs::CfsTlmStripperTester tester;
    tester.testNoSecHdrPassthrough();
}

TEST(OffNominal, MalformedPacket) {
    COMMENT("A buffer too small for a primary header is dropped with a warning and returned");
    REQUIREMENT("FPRIMECFS-CFSTLMSTRIPPER-005");
    FPrimeCfs::CfsTlmStripperTester tester;
    tester.testMalformedPacket();
}

TEST(Nominal, DataReturnPassthrough) {
    COMMENT("dataReturnIn is passed through to dataReturnOut");
    REQUIREMENT("FPRIMECFS-CFSTLMSTRIPPER-006");
    FPrimeCfs::CfsTlmStripperTester tester;
    tester.testDataReturnPassthrough();
}

TEST(Nominal, ComStatusPassthrough) {
    COMMENT("comStatusIn is passed through to comStatusOut");
    REQUIREMENT("FPRIMECFS-CFSTLMSTRIPPER-007");
    FPrimeCfs::CfsTlmStripperTester tester;
    tester.testComStatusPassthrough();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    STest::Random::seed();
    return RUN_ALL_TESTS();
}
