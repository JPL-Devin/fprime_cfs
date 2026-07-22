// ======================================================================
// \title  SchAppDriverTestMain.cpp
// \author mstarch
// \brief  cpp file for SchAppDriver component test main function
// ======================================================================

#include "Fw/Test/UnitTest.hpp"
#include "STest/Random/Random.hpp"
#include "SchAppDriverTester.hpp"

TEST(Nominal, SingleTick) {
    COMMENT("A single scheduler message produces a single tick and buffer return");
    REQUIREMENT("REQ-SchAppDriver-001");
    REQUIREMENT("REQ-SchAppDriver-002");
    REQUIREMENT("REQ-SchAppDriver-003");
    FPrimeCfs::SchAppDriverTester tester;
    tester.testSingleTick();
}

TEST(Nominal, MultipleTicks) {
    COMMENT("Each of a series of scheduler messages produces one tick and buffer return");
    REQUIREMENT("REQ-SchAppDriver-002");
    FPrimeCfs::SchAppDriverTester tester;
    tester.testMultipleTicks();
}

TEST(OffNominal, UnexpectedFunctionCode) {
    COMMENT("A message with an unexpected function code produces a warning event and no tick");
    REQUIREMENT("REQ-SchAppDriver-004");
    FPrimeCfs::SchAppDriverTester tester;
    tester.testUnexpectedFunctionCode();
}

int main(int argc, char** argv) {
    STest::Random::seed();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
