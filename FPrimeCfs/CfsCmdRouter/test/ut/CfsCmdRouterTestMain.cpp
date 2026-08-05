// ======================================================================
// \title  CfsCmdRouterTestMain.cpp
// \brief  cpp file for CfsCmdRouter component test main function
// ======================================================================

#include "CfsCmdRouterTester.hpp"
#include "Fw/Test/UnitTest.hpp"
#include "STest/Random/Random.hpp"

TEST(Nominal, RouteCom) {
    COMMENT("Com-routed function codes are copied without the secondary header and the buffer returned");
    REQUIREMENT("FPRIMECFS-CFSCMDROUTER-001");
    REQUIREMENT("FPRIMECFS-CFSCMDROUTER-003");
    REQUIREMENT("FPRIMECFS-CFSCMDROUTER-004");
    FPrimeCfs::CfsCmdRouterTester tester;
    tester.testRouteCom();
}

TEST(OffNominal, RouteComTooLarge) {
    COMMENT("Oversized com data emits a serialization error and returns the buffer");
    REQUIREMENT("FPRIMECFS-CFSCMDROUTER-009");
    REQUIREMENT("FPRIMECFS-CFSCMDROUTER-010");
    FPrimeCfs::CfsCmdRouterTester tester;
    tester.testRouteComTooLarge();
}

TEST(Nominal, RouteBuffer) {
    COMMENT("Buffer-routed function codes pass the function code and payload; ownership returns via bufferReturnIn");
    REQUIREMENT("FPRIMECFS-CFSCMDROUTER-001");
    REQUIREMENT("FPRIMECFS-CFSCMDROUTER-005");
    REQUIREMENT("FPRIMECFS-CFSCMDROUTER-008");
    FPrimeCfs::CfsCmdRouterTester tester;
    tester.testRouteBuffer();
}

TEST(Nominal, RouteUnknown) {
    COMMENT("Unconfigured function codes route to the unknown output with their context");
    REQUIREMENT("FPRIMECFS-CFSCMDROUTER-006");
    FPrimeCfs::CfsCmdRouterTester tester;
    tester.testRouteUnknown();
}

TEST(OffNominal, BadChecksum) {
    COMMENT("Messages failing the checksum are dropped with a warning and the buffer returned");
    REQUIREMENT("FPRIMECFS-CFSCMDROUTER-002");
    FPrimeCfs::CfsCmdRouterTester tester;
    tester.testBadChecksum();
}

TEST(OffNominal, MissingSecondaryHeader) {
    COMMENT("Messages without a secondary header go to unknown with a warning event");
    REQUIREMENT("FPRIMECFS-CFSCMDROUTER-007");
    FPrimeCfs::CfsCmdRouterTester tester;
    tester.testMissingSecondaryHeader();
}

TEST(OffNominal, ShortSecondaryHeader) {
    COMMENT("Messages whose data is too small for the secondary header go to unknown with a warning");
    REQUIREMENT("FPRIMECFS-CFSCMDROUTER-007");
    FPrimeCfs::CfsCmdRouterTester tester;
    tester.testShortSecondaryHeader();
}

TEST(OffNominal, DisconnectedOutputs) {
    COMMENT("Disconnected route outputs return the buffer rather than assert or leak");
    REQUIREMENT("FPRIMECFS-CFSCMDROUTER-011");
    FPrimeCfs::CfsCmdRouterTester tester(false);
    tester.testDisconnectedOutputs();
}

TEST(OffNominal, BufferReturnUntracked) {
    COMMENT("Untracked buffers returned on bufferReturnIn are forwarded as-is with a default context");
    REQUIREMENT("FPRIMECFS-CFSCMDROUTER-009");
    FPrimeCfs::CfsCmdRouterTester tester;
    tester.testBufferReturnUntracked();
}

TEST(Nominal, CommandResponseNoop) {
    COMMENT("cmdResponseIn is accepted as a no-op");
    REQUIREMENT("FPRIMECFS-CFSCMDROUTER-012");
    FPrimeCfs::CfsCmdRouterTester tester;
    tester.testCommandResponseNoop();
}

TEST(Random, Routing) {
    COMMENT("Randomized routing across all categories returns every buffer");
    REQUIREMENT("FPRIMECFS-CFSCMDROUTER-009");
    FPrimeCfs::CfsCmdRouterTester tester;
    tester.testRandomized();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    STest::Random::seed();
    return RUN_ALL_TESTS();
}
