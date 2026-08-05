#ifndef CFSCORE_PINGENTRIES_HPP
#define CFSCORE_PINGENTRIES_HPP

namespace PingEntries {
struct CfsCore_cmdDisp {
    enum { WARN = 3, FATAL = 5 };
};
struct CfsCore_events {
    enum { WARN = 3, FATAL = 5 };
};
struct CfsCore_tlmSend {
    enum { WARN = 3, FATAL = 5 };
};
}  // namespace PingEntries

#endif
