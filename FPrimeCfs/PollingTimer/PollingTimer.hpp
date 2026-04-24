// ======================================================================
// 	itle  PollingTimer.hpp
// \author mstarch
// \brief  hpp file for PollingTimer component implementation class
// ======================================================================

#ifndef Svc_PollingTimer_HPP
#define Svc_PollingTimer_HPP

#include "FPrimeCfs/PollingTimer/PollingTimerComponentAc.hpp"
#include <chrono>

namespace Svc {

class PollingTimer final : public PollingTimerComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct PollingTimer object
    PollingTimer(const char* const compName  //!< The component name
    );

    //! Destroy PollingTimer object
    ~PollingTimer();

    //! Start interval timer 
    void startTimer(const Fw::TimeInterval& interval  //!< Interval time
    );

    //! Trigger a cycle if the timer interval has expired
    void cycle();

    //! Stop the timer
    void stop();

  private:
    using Clock = std::chrono::steady_clock;

    //! Configured poll interval
    Clock::duration m_interval;

    //! Next scheduled tick time
    Clock::time_point m_nextCycleTime;

    //! Whether the timer is currently active
    bool m_enabled;
};

}  // namespace Svc

#endif
