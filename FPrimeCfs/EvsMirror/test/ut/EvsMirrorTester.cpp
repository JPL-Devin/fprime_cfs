// ======================================================================
// \title  EvsMirrorTester.cpp
// \brief  cpp file for EvsMirror component test harness implementation class
// ======================================================================

#include "EvsMirrorTester.hpp"
#include "STest/Pick/Pick.hpp"
#include "stubs/CfeStubs.hpp"

namespace FPrimeCfs {

// ----------------------------------------------------------------------
// Construction and destruction
// ----------------------------------------------------------------------

EvsMirrorTester ::EvsMirrorTester()
    : EvsMirrorGTestBase("EvsMirrorTester", EvsMirrorTester::MAX_HISTORY_SIZE), component("EvsMirror") {
    this->initComponents();
    this->connectPorts();
    CfeStub::reset();
}

EvsMirrorTester ::~EvsMirrorTester() {
    this->component.deinit();
}

// ----------------------------------------------------------------------
// Tests
// ----------------------------------------------------------------------

void EvsMirrorTester ::testMirror() {
    CountingLogger logger;
    Fw::Logger::registerLogger(&logger);
    const U32 iterations = STest::Pick::lowerUpper(1, 25);
    for (U32 i = 0; i < iterations; i++) {
        CfeStub::reset();

        const FwEventIdType id = static_cast<FwEventIdType>(STest::Pick::any());
        const Fw::LogSeverity severity = Fw::LogSeverity::WARNING_HI;
        Fw::TextLogString text;
        this->sendTextEvent(id, severity, text);

        ASSERT_EQ(CfeStub::state().sendEventCount, 1u);
        const CfeStub::SendEventCall& call = CfeStub::state().sendEventCalls[0];
        ASSERT_EQ(call.eventId, static_cast<uint16>(id));
        ASSERT_EQ(call.eventType, CFE_EVS_EventType_ERROR);
        ASSERT_STREQ(call.text, text.toChar());
        ASSERT_EQ(logger.messageCount, 0u);
    }
    Fw::Logger::registerLogger(nullptr);
}

void EvsMirrorTester ::testSeverityMapping() {
    const struct {
        Fw::LogSeverity::T severity;
        uint16 eventType;
    } mappings[] = {
        {Fw::LogSeverity::FATAL, CFE_EVS_EventType_CRITICAL},
        {Fw::LogSeverity::WARNING_HI, CFE_EVS_EventType_ERROR},
        {Fw::LogSeverity::WARNING_LO, CFE_EVS_EventType_ERROR},
        {Fw::LogSeverity::COMMAND, CFE_EVS_EventType_INFORMATION},
        {Fw::LogSeverity::ACTIVITY_HI, CFE_EVS_EventType_INFORMATION},
        {Fw::LogSeverity::ACTIVITY_LO, CFE_EVS_EventType_INFORMATION},
        {Fw::LogSeverity::DIAGNOSTIC, CFE_EVS_EventType_DEBUG},
    };
    for (const auto& mapping : mappings) {
        CfeStub::reset();

        const FwEventIdType id = static_cast<FwEventIdType>(STest::Pick::any());
        Fw::TextLogString text;
        this->sendTextEvent(id, Fw::LogSeverity(mapping.severity), text);

        ASSERT_EQ(CfeStub::state().sendEventCount, 1u);
        ASSERT_EQ(CfeStub::state().sendEventCalls[0].eventType, mapping.eventType);
    }
}

void EvsMirrorTester ::testEvsFailure() {
    CountingLogger logger;
    Fw::Logger::registerLogger(&logger);
    CfeStub::state().sendEventStatus = CFE_EVS_APP_NOT_REGISTERED;

    const FwEventIdType id = static_cast<FwEventIdType>(STest::Pick::any());
    Fw::TextLogString text;
    this->sendTextEvent(id, Fw::LogSeverity::ACTIVITY_LO, text);

    ASSERT_EQ(CfeStub::state().sendEventCount, 1u);
    // The EVS failure is reported through Fw::Logger
    ASSERT_EQ(logger.messageCount, 1u);
    Fw::Logger::registerLogger(nullptr);
}

// ----------------------------------------------------------------------
// Helper functions
// ----------------------------------------------------------------------

void EvsMirrorTester ::sendTextEvent(FwEventIdType id, const Fw::LogSeverity& severity, Fw::TextLogString& generatedText) {
    char buffer[32] = {0};
    const U32 length = STest::Pick::lowerUpper(1, sizeof(buffer) - 1);
    for (U32 i = 0; i < length; i++) {
        buffer[i] = static_cast<char>(STest::Pick::lowerUpper('a', 'z'));
    }
    buffer[length] = '\0';
    generatedText = buffer;

    Fw::Time timeTag(STest::Pick::any(), STest::Pick::lowerUpper(0, 999999));
    Fw::TextLogString sentText = generatedText;
    this->invoke_to_TextLogger(0, id, timeTag, severity, sentText);
}

}  // namespace FPrimeCfs
