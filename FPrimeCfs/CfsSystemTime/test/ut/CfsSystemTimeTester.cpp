// ======================================================================
// \title  CfsSystemTimeTester.cpp
// \brief  cpp file for CfsSystemTime component test harness implementation class
// ======================================================================

#include "CfsSystemTimeTester.hpp"
#include "stubs/CfeTimeStubs.hpp"

namespace FPrimeCfs {

// ----------------------------------------------------------------------
// Construction and destruction
// ----------------------------------------------------------------------

CfsSystemTimeTester ::CfsSystemTimeTester()
    : CfsSystemTimeGTestBase("CfsSystemTimeTester", CfsSystemTimeTester::MAX_HISTORY_SIZE),
      component("CfsSystemTime") {
    this->initComponents();
    this->connectPorts();
    CfeStub::resetTime();
}

CfsSystemTimeTester ::~CfsSystemTimeTester() {}

// ----------------------------------------------------------------------
// Tests
// ----------------------------------------------------------------------

void CfsSystemTimeTester ::test_get_time() {
    // 0x80000000 subseconds is exactly half a second: 500000 microseconds
    CfeStub::timeState().currentTime.Seconds = 1234567890u;
    CfeStub::timeState().currentTime.Subseconds = 0x80000000u;

    Fw::Time time;
    this->invoke_to_timeGetPort(0, time);
    ASSERT_EQ(CfeStub::timeState().getTimeCount, 1u);
    ASSERT_EQ(time.getSeconds(), 1234567890u);
    ASSERT_EQ(time.getUSeconds(), 500000u);
}

void CfsSystemTimeTester ::test_convert_time() {
    // 250000 microseconds is exactly a quarter second: 0x40000000 subseconds
    const Fw::Time time(1234567890u, 250000u);
    const CfsTime cfsTime = this->invoke_to_cfsTimeConvert(0, time);
    ASSERT_EQ(cfsTime.get_seconds(), 1234567890u);
    ASSERT_EQ(cfsTime.get_subseconds(), 0x40000000u);
}

}  // namespace FPrimeCfs
