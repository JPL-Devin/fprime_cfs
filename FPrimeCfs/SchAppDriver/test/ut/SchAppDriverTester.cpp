// ======================================================================
// \title  SchAppDriverTester.cpp
// \author mstarch
// \brief  cpp file for SchAppDriver component test harness implementation class
// ======================================================================

#include "SchAppDriverTester.hpp"
#include "STest/Pick/Pick.hpp"

namespace FPrimeCfs {

// ----------------------------------------------------------------------
// Construction and destruction
// ----------------------------------------------------------------------

SchAppDriverTester ::SchAppDriverTester()
    : SchAppDriverGTestBase("SchAppDriverTester", SchAppDriverTester::MAX_HISTORY_SIZE),
      component("SchAppDriver"),
      m_messageData() {
    this->initComponents();
    this->connectPorts();
}

SchAppDriverTester ::~SchAppDriverTester() {
    this->component.deinit();
}

// ----------------------------------------------------------------------
// Tests
// ----------------------------------------------------------------------

void SchAppDriverTester ::testSingleTick() {
    Fw::Buffer buffer;
    ComCfg::FrameContext context;
    this->sendSchMessage(buffer, context);
    this->assertSingleTickAndReturn(buffer, context);
}

void SchAppDriverTester ::testMultipleTicks() {
    const U32 iterations = STest::Pick::lowerUpper(2, 25);
    for (U32 i = 0; i < iterations; i++) {
        Fw::Buffer buffer;
        ComCfg::FrameContext context;
        this->sendSchMessage(buffer, context);
        this->assertSingleTickAndReturn(buffer, context);
    }
}

// ----------------------------------------------------------------------
// Helper functions
// ----------------------------------------------------------------------

void SchAppDriverTester ::sendSchMessage(Fw::Buffer& buffer, ComCfg::FrameContext& context) {
    this->clearHistory();
    const FwSizeType size = STest::Pick::lowerUpper(1, MAX_MESSAGE_SIZE);
    for (FwSizeType i = 0; i < size; i++) {
        this->m_messageData[i] = static_cast<U8>(STest::Pick::lowerUpper(0, 0xFF));
    }
    buffer.setData(this->m_messageData);
    buffer.setSize(size);
    context.set_apid(static_cast<ComCfg::Apid::T>(STest::Pick::lowerUpper(0, 0x7FF)));
    this->invoke_to_dataIn(0, buffer, context);
}

void SchAppDriverTester ::assertSingleTickAndReturn(const Fw::Buffer& buffer, const ComCfg::FrameContext& context) {
    ASSERT_from_CycleOut_SIZE(1);
    ASSERT_from_dataReturnOut_SIZE(1);
    const FromPortEntry_dataReturnOut& entry = this->fromPortHistory_dataReturnOut->at(0);
    ASSERT_EQ(entry.data.getData(), buffer.getData());
    ASSERT_EQ(entry.data.getSize(), buffer.getSize());
    ASSERT_EQ(entry.context, context);
}

}  // namespace FPrimeCfs
