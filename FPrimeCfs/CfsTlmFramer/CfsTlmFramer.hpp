// ======================================================================
// \title  CfsTlmFramer.hpp
// \brief  hpp file for CfsTlmFramer component implementation class
// ======================================================================

#ifndef FPrimeCfs_CfsTlmFramer_HPP
#define FPrimeCfs_CfsTlmFramer_HPP

#include "FPrimeCfs/CfsTlmFramer/CfsTlmFramerComponentAc.hpp"

namespace FPrimeCfs {

//! Size in bytes of the CCSDS space packet primary header
constexpr FwSizeType CFS_TLM_FRAMER_SPACE_PACKET_HEADER_SIZE = 6;
//! Mask of the packet type bit (1 = command) within the space packet stream identifier
constexpr U32 CFS_TLM_FRAMER_SPACE_PACKET_TYPE_MASK = 0x1000;
//! Mask of the secondary header flag within the space packet stream identifier
constexpr U32 CFS_TLM_FRAMER_SPACE_PACKET_SEC_HDR_MASK = 0x0800;
//! Byte offset of the big-endian 16-bit packet data length field within the space packet primary header
constexpr FwSizeType CFS_TLM_FRAMER_SPACE_PACKET_LENGTH_OFFSET = 4;
//! Size in bytes of the cFS telemetry secondary header (big-endian 4-byte seconds, 2-byte subseconds)
constexpr FwSizeType CFS_TLM_FRAMER_TLM_SEC_HDR_SIZE = 6;
//! Number of microseconds in one second, used to convert to 1/65536-second subseconds
constexpr U32 CFS_TLM_FRAMER_USECS_PER_SECOND = 1000000;

class CfsTlmFramer final : public CfsTlmFramerComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct CfsTlmFramer object
    CfsTlmFramer(const char* const compName  //!< The component name
    );

    //! Destroy CfsTlmFramer object
    ~CfsTlmFramer();

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for dataIn
    //!
    //! Port to receive one or more complete space packets to frame as cFS telemetry packets
    void dataIn_handler(FwIndexType portNum,  //!< The port number
                        Fw::Buffer& data,
                        const ComCfg::FrameContext& context) override;

    //! Handler implementation for dataReturnIn
    //!
    //! Buffer coming back from the downstream component; deallocated back to the buffer pool
    void dataReturnIn_handler(FwIndexType portNum,  //!< The port number
                              Fw::Buffer& data,
                              const ComCfg::FrameContext& context) override;

    //! Handler implementation for comStatusIn
    //!
    //! Com status from the downstream component, passed through to the upstream component
    void comStatusIn_handler(FwIndexType portNum,  //!< The port number
                             Fw::Success& condition) override;

    // ----------------------------------------------------------------------
    // Helpers
    // ----------------------------------------------------------------------

    //! Write the 6-byte cFS telemetry secondary header (current time) at the supplied location
    void writeTelemetrySecondaryHeader(U8* location);
};

}  // namespace FPrimeCfs

#endif
