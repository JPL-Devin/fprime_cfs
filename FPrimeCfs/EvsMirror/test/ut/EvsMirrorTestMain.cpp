// ======================================================================
// \title  EvsMirrorTestMain.cpp
// \brief  cpp file for EvsMirror component test main function
// ======================================================================

#include "EvsMirrorTester.hpp"
#include "Fw/Test/UnitTest.hpp"
#include "STest/Random/Random.hpp"

TEST(Nominal, Mirror) {
    COMMENT("Text events are mirrored to EVS with the formatted text and truncated ID");
    REQUIREMENT("FPRIMECFS-EVSMIRROR-001");
    FPrimeCfs::EvsMirrorTester tester;
    tester.testMirror();
}

TEST(Nominal, SeverityMapping) {
    COMMENT("Each F Prime severity maps to the expected EVS event type");
    REQUIREMENT("FPRIMECFS-EVSMIRROR-002");
    FPrimeCfs::EvsMirrorTester tester;
    tester.testSeverityMapping();
}

TEST(OffNominal, EvsFailure) {
    COMMENT("An EVS send failure is tolerated without asserting");
    REQUIREMENT("FPRIMECFS-EVSMIRROR-003");
    FPrimeCfs::EvsMirrorTester tester;
    tester.testEvsFailure();
}

int main(int argc, char** argv) {
    STest::Random::seed();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
