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

void EvsMirrorTester ::testLogPassThrough() {
    const FwEventIdType id = static_cast<FwEventIdType>(STest::Pick::any());
    Fw::Time timeTag(STest::Pick::any(), STest::Pick::lowerUpper(0, 999999));
    const Fw::LogSeverity severity = Fw::LogSeverity::ACTIVITY_HI;
    Fw::LogBuffer args;
    ASSERT_EQ(args.serializeFrom(static_cast<U32>(STest::Pick::any())), Fw::FW_SERIALIZE_OK);

    Fw::Time sentTime = timeTag;
    Fw::LogBuffer sentArgs = args;
    this->invoke_to_logIn(0, id, sentTime, severity, sentArgs);

    ASSERT_from_logOut_SIZE(1);
    const FromPortEntry_logOut& entry = this->fromPortHistory_logOut->at(0);
    ASSERT_EQ(entry.id, id);
    ASSERT_EQ(entry.timeTag, timeTag);
    ASSERT_EQ(entry.severity, severity);
    ASSERT_EQ(entry.args, args);
    // With text logging enabled, binary events are not mirrored (the text form is)
    ASSERT_EQ(CfeStub::state().sendEventCount, 0u);
}

void EvsMirrorTester ::testTextLogPassThroughAndMirror() {
    const U32 iterations = STest::Pick::lowerUpper(1, 25);
    for (U32 i = 0; i < iterations; i++) {
        this->clearHistory();
        CfeStub::reset();

        const FwEventIdType id = static_cast<FwEventIdType>(STest::Pick::any());
        const Fw::LogSeverity severity = Fw::LogSeverity::WARNING_HI;
        Fw::TextLogString text;
        this->sendTextEvent(id, severity, text);

        ASSERT_from_textLogOut_SIZE(1);
        const FromPortEntry_textLogOut& entry = this->fromPortHistory_textLogOut->at(0);
        ASSERT_EQ(entry.id, id);
        ASSERT_EQ(entry.severity, severity);
        ASSERT_STREQ(entry.text.toChar(), text.toChar());

        ASSERT_EQ(CfeStub::state().sendEventCount, 1u);
        const CfeStub::SendEventCall& call = CfeStub::state().sendEventCalls[0];
        ASSERT_EQ(call.eventId, static_cast<uint16>(id));
        ASSERT_EQ(call.eventType, CFE_EVS_EventType_ERROR);
        ASSERT_STREQ(call.text, text.toChar());
    }
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
        this->clearHistory();
        CfeStub::reset();

        const FwEventIdType id = static_cast<FwEventIdType>(STest::Pick::any());
        Fw::TextLogString text;
        this->sendTextEvent(id, Fw::LogSeverity(mapping.severity), text);

        ASSERT_EQ(CfeStub::state().sendEventCount, 1u);
        ASSERT_EQ(CfeStub::state().sendEventCalls[0].eventType, mapping.eventType);
    }
}

void EvsMirrorTester ::testEvsFailureStillForwards() {
    CfeStub::state().sendEventStatus = CFE_EVS_APP_NOT_REGISTERED;

    const FwEventIdType id = static_cast<FwEventIdType>(STest::Pick::any());
    Fw::TextLogString text;
    this->sendTextEvent(id, Fw::LogSeverity::ACTIVITY_LO, text);

    ASSERT_EQ(CfeStub::state().sendEventCount, 1u);
    ASSERT_from_textLogOut_SIZE(1);
    ASSERT_STREQ(this->fromPortHistory_textLogOut->at(0).text.toChar(), text.toChar());
}

// ----------------------------------------------------------------------
// Helper functions
// ----------------------------------------------------------------------

void EvsMirrorTester ::sendTextEvent(FwEventIdType id, const Fw::LogSeverity& severity, Fw::TextLogString& text) {
    char buffer[32];
    const U32 length = STest::Pick::lowerUpper(1, sizeof(buffer) - 1);
    for (U32 i = 0; i < length; i++) {
        buffer[i] = static_cast<char>(STest::Pick::lowerUpper('a', 'z'));
    }
    buffer[length] = '\0';
    text = buffer;

    Fw::Time timeTag(STest::Pick::any(), STest::Pick::lowerUpper(0, 999999));
    Fw::TextLogString sentText = text;
    this->invoke_to_textLogIn(0, id, timeTag, severity, sentText);
}

}  // namespace FPrimeCfs
