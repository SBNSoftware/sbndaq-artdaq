#ifndef _NEVISTPC_XMITMODULE_H
#define _NEVISTPC_XMITMODULE_H 1

#include "ControllerModule.h"

namespace nevistpc {

  enum XMITErrorFlag_t {
    kAlignment,
    kRecTimeOut,
    kNUOptConn,
    kSNOptConn,
    kNUBusy,
    kSNBusy,
    kNULock,
    kSNLock,
    kTokenDPALock,
    kTokenRecLock,
    kPLLLock,
    kXMIT_ERROR_TYPE_MAX
  };

  class XMITModuleStatus {
    
  public:
    
  XMITModuleStatus(uint8_t crate_num=0) 
    : config_crate(crate_num),
      align1(false), align2(false),
      timeout1(false), timeout2(false),
      optical_nu1(false), optical_nu2(false),
      optical_sn1(false), optical_sn2(false),
      busy_nu(false), busy_sn(false),
      opt_lock_nu(false),
      opt_lock_sn(false),
      dpa_lock(false),
      receiver_lock(false),
      pll_lock(false),
      buffer_crate(crate_num),
      frame_ctr(0),
      event_ctr(0),
      nu_read_ctr(0),
      sn_read_ctr(0),
      error_flag_v(kXMIT_ERROR_TYPE_MAX,false),
      badstat(false)
   {}

    // config status
    int16_t config_crate;
    bool align1, align2;
    bool timeout1, timeout2;
    bool optical_nu1, optical_nu2;
    bool optical_sn1, optical_sn2;
    bool busy_nu, busy_sn;
    bool opt_lock_nu, opt_lock_sn;
    bool dpa_lock;
    bool receiver_lock;
    bool pll_lock;

    // counter
    int16_t  buffer_crate;
    int32_t frame_ctr;
    int32_t event_ctr;
    int32_t nu_read_ctr;
    int32_t sn_read_ctr;
    
    std::vector<bool> error_flag_v;
    
    bool isValid() const;
    void report() const;
    void describe(const XMITErrorFlag_t type, std::ostringstream &strm) const;
    void set_badstat() { badstat=true; }
    bool get_badstat() const { return badstat; }
    
  private:
    bool badstat;

  };

  class XMITModule: public UsesControllerModule {

  public:
    //Chip id=2
    enum chip_2:
    operation_id_t {
      SET_CONFIG_MODE = 0
    };
    typedef chip_2 max3000_FPGA;

    //Chip id=3
    enum chip_3:
    operation_id_t {
        XMIT_MODCOUNT = 1,
        XMIT_ENABLE_NU_EVENTS,
        XMIT_ENABLE_SN_EVENTS,
        XMIT_NU_FIBER_TEST,
        XMIT_SN_FIBER_TEST,
        XMIT_TESTDATA = 10,
        XMIT_RDSTATUS = 20,
        XMIT_RDCOUNTERS = 21,
        XMIT_LINK_RESET,
        XMIT_RESET_OPT_TRANSCEIVER,
        XMIT_DPA_FIFO_RESET,
        XMIT_DPA_WORD_ALIGN,
	XMIT_LINK_PLL_RESET = 26,
    };
    typedef chip_3 xmit_chip;

    enum device:
    chip_address_t {
        MAX3000_FPGA = 2,
        XMIT_CHIP = 3,
        MAX3000_FPGA_PROGRAM = 7
    };

    enum class dma_completion_status: uint8_t {
      timeout = 0,
        complete
    };

  public:
    explicit XMITModule ( uint8_t slot_number );
    ~XMITModule();

    void useController ( ControllerModuleSPtr controller );

    void setMax3000Config();
    void setFEMModuleCount ( data_payload_t const& size );
    void enableNUChanEvents ( data_payload_bool_t const& flag );
    void enableSNChanEvents ( data_payload_bool_t const& flag );
    void enableSNChanTest ( data_payload_bool_t const& flag );
    void enableNUChanTest ( data_payload_bool_t const& flag );
    void uploadTestDataPacket ( data_payload_t const& packet );
    void loadStatus();
    void readStatus();
    void reportStatus();
    XMITModuleStatus const& getStatus() const { return _status; }

    void resetLink();
    void resetOpticalTransceiver();
    void resetDPA();
    void alignDPA();
    void resetPLLLink();
    void programMax3000FPGAFirmware ( std::string const& firmwareName );
    
    XMITModule() = delete;
    XMITModule ( XMITModule const& ) = delete;
    XMITModule& operator= ( XMITModule const& ) = delete;
    XMITModule ( XMITModule && ) = delete;
    XMITModule& operator= ( XMITModule && ) = delete;

  protected:
    enum {
      TERROR    = TLVL_ERROR,
      TWARNING  = TLVL_WARNING,
      TINFO     = TLVL_INFO,
      TDEBUG    = TLVL_DEBUG
    };
    
  private:
    uint8_t _slot_number; 
    XMITModuleStatus _status;
    bool _nu_stream_enabled;
    bool _sn_stream_enabled;
  };

  typedef std::unique_ptr<XMITModule> XMITModuleUPtr;
  typedef std::shared_ptr<XMITModule> XMITModuleSPtr;
  
} // end of namespace nevistpc


#endif //_NEVISTPC_XMITMODULE








