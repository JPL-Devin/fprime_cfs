// ======================================================================
// \title  CfsTlmDeframer.hpp
// \brief  hpp file for CfsTlmDeframer component implementation class
// ======================================================================

#ifndef FPrimeCfs_CfsTlmDeframer_HPP
#define FPrimeCfs_CfsTlmDeframer_HPP

#include "FPrimeCfs/CfsTlmDeframer/CfsTlmDeframerComponentAc.hpp"

namespace FPrimeCfs {

//! Size in bytes of the CCSDS space packet primary header
constexpr FwSizeType CFS_TLM_DEFRAMER_SPACE_PACKET_HEADER_SIZE = 6;
//! Mask of the packet type bit (1 = command) within the space packet stream identifier
constexpr U32 CFS_TLM_DEFRAMER_SPACE_PACKET_TYPE_MASK = 0x1000;
//! Mask of the secondary header flag within the space packet stream identifier
constexpr U32 CFS_TLM_DEFRAMER_SPACE_PACKET_SEC_HDR_MASK = 0x0800;
//! Byte offset of the big-endian 16-bit packet data length field within the space packet primary header
constexpr FwSizeType CFS_TLM_DEFRAMER_SPACE_PACKET_LENGTH_OFFSET = 4;
//! Size in bytes of the cFS telemetry secondary header (big-endian 4-byte seconds, 2-byte subseconds)
constexpr FwSizeType CFS_TLM_DEFRAMER_TLM_SEC_HDR_SIZE = 6;

class CfsTlmDeframer final : public CfsTlmDeframerComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct CfsTlmDeframer object
    CfsTlmDeframer(const char* const compName  //!< The component name
    );

    //! Destroy CfsTlmDeframer object
    ~CfsTlmDeframer();

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for dataIn
    //!
    //! Port to receive a complete space packet whose cFS telemetry secondary header is removed
    void dataIn_handler(FwIndexType portNum,  //!< The port number
                        Fw::Buffer& data,
                        const ComCfg::FrameContext& context) override;

    //! Handler implementation for dataReturnIn
    //!
    //! Buffer coming back from the downstream component; returned to the upstream sender
    void dataReturnIn_handler(FwIndexType portNum,  //!< The port number
                              Fw::Buffer& data,
                              const ComCfg::FrameContext& context) override;
};

}  // namespace FPrimeCfs

#endif
