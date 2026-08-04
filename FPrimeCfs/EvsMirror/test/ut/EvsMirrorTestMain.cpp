// ======================================================================
// \title  EvsMirrorTestMain.cpp
// \brief  cpp file for EvsMirror component test main function
// ======================================================================

#include "EvsMirrorTester.hpp"
#include "Fw/Test/UnitTest.hpp"
#include "STest/Random/Random.hpp"

TEST(Nominal, LogPassThrough) {
    COMMENT("Events on logIn are forwarded unchanged on logOut");
    REQUIREMENT("FPRIMECFS-EVSMIRROR-001");
    FPrimeCfs::EvsMirrorTester tester;
    tester.testLogPassThrough();
}

TEST(Nominal, TextLogPassThroughAndMirror) {
    COMMENT("Text events are forwarded unchanged and mirrored to EVS");
    REQUIREMENT("FPRIMECFS-EVSMIRROR-002");
    REQUIREMENT("FPRIMECFS-EVSMIRROR-003");
    FPrimeCfs::EvsMirrorTester tester;
    tester.testTextLogPassThroughAndMirror();
}

TEST(Nominal, SeverityMapping) {
    COMMENT("Each F Prime severity maps to the expected EVS event type");
    REQUIREMENT("FPRIMECFS-EVSMIRROR-004");
    FPrimeCfs::EvsMirrorTester tester;
    tester.testSeverityMapping();
}

TEST(OffNominal, EvsFailureStillForwards) {
    COMMENT("An EVS send failure does not prevent forwarding");
    REQUIREMENT("FPRIMECFS-EVSMIRROR-005");
    FPrimeCfs::EvsMirrorTester tester;
    tester.testEvsFailureStillForwards();
}

int main(int argc, char** argv) {
    STest::Random::seed();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
