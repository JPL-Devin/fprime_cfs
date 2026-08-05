// ======================================================================
// \title  CfsTlmStripper.hpp
// \brief  hpp file for CfsTlmStripper component implementation class
// ======================================================================

#ifndef FPrimeCfs_CfsTlmStripper_HPP
#define FPrimeCfs_CfsTlmStripper_HPP

#include "FPrimeCfs/CfsTlmStripper/CfsTlmStripperComponentAc.hpp"

namespace FPrimeCfs {

//! Size in bytes of the CCSDS space packet primary header
constexpr FwSizeType CFS_TLM_STRIPPER_PRI_HDR_SIZE = 6;

//! Size in bytes of the cFS telemetry secondary header (4-byte seconds, 2-byte subseconds)
constexpr FwSizeType CFS_TLM_STRIPPER_SEC_HDR_SIZE = 6;

//! Secondary header flag mask within the high byte of the stream identifier
constexpr U8 CFS_TLM_STRIPPER_SEC_HDR_FLAG = 0x08;

//! Packet type (command) bit mask within the high byte of the stream identifier
constexpr U8 CFS_TLM_STRIPPER_TYPE_FLAG = 0x10;

//! Byte offset of the packet data length field within the primary header
constexpr FwSizeType CFS_TLM_STRIPPER_LENGTH_OFFSET = 4;

class CfsTlmStripper final : public CfsTlmStripperComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct CfsTlmStripper object
    CfsTlmStripper(const char* const compName  //!< The component name
    );

    //! Destroy CfsTlmStripper object
    ~CfsTlmStripper();

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for dataIn
    void dataIn_handler(FwIndexType portNum,
                        Fw::Buffer& data,
                        const ComCfg::FrameContext& context) override;

    //! Handler implementation for dataReturnIn
    void dataReturnIn_handler(FwIndexType portNum,
                              Fw::Buffer& frameBuffer,
                              const ComCfg::FrameContext& context) override;

    //! Handler implementation for comStatusIn
    void comStatusIn_handler(FwIndexType portNum, Fw::Success& condition) override;
};

}  // namespace FPrimeCfs

#endif
