#ifndef _NEVISTPC_NEVISTPCFEM_H
#define _NEVISTPC_NEVISTPCFEM_H 1

#include "ControllerModule.h"

namespace nevistpc {

  enum FEMErrorFlag_t {
    kModAddressError,
    kNUMemNotReady,
    kSNMemNotReady,
    kPLLLockError,
    kNUPreBuffError,
    kSNPreBuffError,
    kDrainError,
    kRightPLLLockError,
    kLeftPLLLockError,
    kRightDPALockError,
    kLeftDPALockError,
    kRightADCAlign,
    kLeftADCAlign,
    kNUDataDrainError,
    kSNDataDrainError,
    kFEM_ERROR_TYPE_MAX
  };

  class TPCFEMStatus {

  public:

  TPCFEMStatus(uint8_t mod_num=0)
    : config_module(mod_num),
      nu_buffer_module(mod_num),
      sn_buffer_module(mod_num),
      error_flag_v(kFEM_ERROR_TYPE_MAX,false),
      adc_dpa_lock_right(false), adc_dpa_lock_left(false),
      adc_pll_lock_right(false), adc_pll_lock_left(false),
      buffer_nu(false), buffer_sn(false),
      mem_nu(false), mem_sn(false),
      adc_align_cmd_right(false), adc_align_cmd_left(false),
      adc_align_done_right(false), adc_align_done_left(false),
      nu_data_empty(false),
      nu_header_empty(false),
      nu_block_id(0), nu_nwords(0),
      old_nu_block_id(0), old_nu_nwords(0),
      sn_block_id(0), sn_nwords(0),
      old_sn_block_id(0), old_sn_nwords(0),
      badstat(false)
	{}

    int16_t config_module;
    int16_t nu_buffer_module;
    int16_t sn_buffer_module;

    std::vector<bool> error_flag_v;

    bool pll_lock,
      adc_dpa_lock_right, adc_dpa_lock_left,
      adc_pll_lock_right, adc_pll_lock_left,
      buffer_nu, buffer_sn,
      mem_nu, mem_sn,
      adc_align_cmd_right, adc_align_cmd_left,
      adc_align_done_right, adc_align_done_left,
      nu_data_empty, nu_header_empty;
    int32_t nu_block_id, nu_nwords;
    int32_t old_nu_block_id, old_nu_nwords;
    int32_t sn_block_id, sn_nwords;
    int32_t old_sn_block_id, old_sn_nwords;


    bool isValid() const;
    void report() const;
    void describe(const FEMErrorFlag_t type, std::ostringstream& strm) const;
    void set_badstat() { badstat=true; }
    bool get_badstat() const { return badstat; }

  private:
    bool badstat;
  };

  class NevisTPCFEM: public UsesControllerModule {

  public:

    //Chip id=1  arria_fpga
    enum  chip_1:
    operation_id_t
    {
      POWER_ON = 1,
      POWER_OFF
    };
    typedef chip_1 arria_fpga;

    //Chip id=2 stratix_fpga_cfg
    enum  chip_2:
    operation_id_t
    {
	CONFIGURE_ON = 0,
	CONFIGURE_OFF
    };
    typedef chip_2 stratix_fpga_cfg;

    enum  chip_3:
      operation_id_t{
	ADC_RECEIVER_DATA_ALIGNMENT = 1,
	NU_CHAN_COMPRESSION = 2,
	SN_CHAN_COMPRESSION,
	BLOCK_SIZE,
	DRIFT_TIME_SIZE,
	MODULE_NUMBER = 6,
	NU_CHAN_ID,
	SN_CHAN_ID,
	PREBUFFER_SIZE,
	TEST_MODE = 10,
	TM1_SAMPLE,
	TM1_FRAME,
	TM1_CHANNEL,
	TM1_PULSE_VALUE,
	TM1_BASELINE_VALUE,
	FEB_TEST_RAM_DATA = 16,
	NU_DATA_SLOW_READBACK = 17,
	SN_DATA_SLOW_READBACK,
	READ_CONFIG_STATUS    = 20,
	NU_HEADER_READ_PULSE  = 21,
	NU_DATA_READ_PULSE = 22,
	SN_HEADER_READ_PULSE  = 23,
	SN_DATA_READ_PULSE = 24,
  	TM1_READ_PULSE = 30,
	DRAM_RESET = 31,
	DRAM_USER_LOGIC_RESET = 32,
	ADC_RESET_PULSE = 33,
	READ_NU_BUFFER_STATUS = 34,
	READ_SN_BUFFER_STATUS = 35,
	NU_MAX_BUFFER = 40,
	SN_MAX_BUFFER,
	LINK_PORT_HOLD_ENABLE,
	INPUT_DATA_SELECTION = 44,
	LOAD_THRESHOLD = 100,
	LOAD_THRESHOLD_MEAN = 164,
	LOAD_THRESHOLD_VARIANCE,
  	BIPOLAR = 167,
	LOAD_PRESAMPLE,
	LOAD_POSTSAMPLE,
	CHANNEL_THRESHOLD,
	LOAD_FAKE_DATA_ADDRESS = 240,
	LOAD_FAKE_DATA_ADC = 241,
	WRITE_FAKE_DATA = 242,
	LOAD_BASELINE_CHANNEL = 245,
	LOAD_BASELINE = 246,
	WRITE_BASELINE = 247

	  };
    typedef chip_3 stratix_fpga;


    //Chip id=4 token_passing_control
    enum  chip_4:
    operation_id_t
    {
	LAST_MODULE_OFF=0,
	LAST_MODULE_ON,
	TP_RECEIVER_RESET,
	PULSE_TP_RECEIVER,
	PLL_LINK_RESET=5,
	OP8=8
    };
    typedef chip_4 token_passing_control;

    //Chip id=5 adc
    enum  chip_5:
    operation_id_t
    {
	DATA_SEND1 = 0

    };
    typedef chip_5 adc;

    enum  chip_7:
    operation_id_t
    {
	PROGRAM_FIRMWARE = 0
    };
    typedef chip_7 stratix_fpga_program;

    enum  device:
    chip_address_t
    {
	ARRIA_FPGA = 1,
	STRATIX_FPGA_CFG,
	STRATIX_FPGA,
	TOKEN_PASSING_CONTROL,
	ADC_CONTROL_BLOCK,
	STRATIX_FPGA_PROGRAM = 7
    };

  public:
    explicit NevisTPCFEM( uint8_t slot_number );
    ~NevisTPCFEM();

    void useController ( ControllerModuleSPtr controller );

    void powerOnArriaFPGA();
    void powerOffArriaFPGA();
    void configOnStratixFPGA();
    void configOffStratixFPGA();
    void setLastModuleInTokenPassingOn();
    void setLastModuleInTokenPassingOff();
    void resetTokenPassingReceiver();
    void pulseTokenPassingAllignmentCircuit();
    void resetPLLLink();
    void programStratixFPGAFirmware ( std::string const& firmwareName );
    void selectSPIAddress(data_payload_t const &adc_chan);

    void loadSlowMonitorInfo();
    void readSlowMonitorInfo();

    void readStatus();
    void loadStatus();
    void readCounters(){}
    void loadCounters(){}

    TPCFEMStatus const& getStatus()  const { return _status; }

    void disableNUChanCompression(data_payload_bool_t const &flag);
    void disableSNChanCompression(data_payload_bool_t const &flag);
    void setBlockSize(data_payload_t const &size);
    void setDriftTimeSize(data_payload_t const &size);
    void setModuleNumber(data_payload_t const &number);
    void setNUChannelID(data_payload_t const &id);
    void setSNChannelID(data_payload_t const &id);
    void setPrebufferSize(data_payload_t const &size);
    void setTestMode(data_payload_t const &mode);
    void tm1_testSample(data_payload_t const &mode);
    void tm1_testFrame(data_payload_t const &mode);
    void tm1_testChannel(data_payload_t const &mode);
    void tm1_testPulseValue(data_payload_t const &mode);
    void tm1_testBaselineValue(data_payload_t const &mode);
    void tm2_testRamData(data_payload_t const &mode);
    void enableNUDataSlowReadback(data_payload_bool_t const &flag);
    void enableSNDataSlowReadback(data_payload_bool_t const &flag);
    void setNUMaxBufferSize(data_payload_t const &size);
    void setSNMaxBufferSize(data_payload_t const &size);
    void enableLinkPortHold(data_payload_bool_t const &flag);
    void readEnablePulseNUEventHeader(data_payload_bool_t const &flag);
    void readEnablePulseSNEventHeader(data_payload_bool_t const &flag);
    void readEnablePulseNUEventData(data_payload_bool_t const &flag);
    void readEnablePulseSNEventData(data_payload_bool_t const &flag);
    void readPulseForTestMode1(data_payload_bool_t const &flag);
    void resetDRAM(data_payload_bool_t const &flag);
    void resetDRAMUserLogic();
    void prebufferStatusNU();
    void prebufferStatusSN();
    void setLoadThreshold(data_payload_t const &chan, data_payload_t const &size);
    void setLoadThresholdMean(data_payload_t const &size);
    void setLoadThresholdVariance(data_payload_t const &size);
    void setLoadPresample(data_payload_t const &size);
    void setLoadPostsample(data_payload_t const &size);
    void setChannelThreshold(data_payload_bool_t const &flag);
    void setFEMBipolar(data_payload_t const &size);
    void setLoadBaseline(data_payload_t const &chan, data_payload_t const &size);
    void enableFEMFakeData(bool const &flag);
    void loadFEMFakeData(std::string const &pattern);

    void runDefaultNUConfig(data_payload_t const&febChanID);
    void fem_setup(fhicl::ParameterSet const& configParams);

    uint module_number(){
      return _slot_number;
    }

    NevisTPCFEM() = delete;
    NevisTPCFEM ( NevisTPCFEM const& ) = delete;
    NevisTPCFEM& operator= ( NevisTPCFEM const& ) = delete;
    NevisTPCFEM ( NevisTPCFEM && ) = delete;
    NevisTPCFEM& operator= ( NevisTPCFEM && ) = delete;

  private:
    uint8_t _slot_number;
    TPCFEMStatus _status;
  };

  typedef std::unique_ptr<NevisTPCFEM> NevisTPCFEMUPtr;
  typedef std::shared_ptr<NevisTPCFEM> NevisTPCFEMSPtr;

} // end of namespace nevistpc

#endif //_NEVISTPC_NEVISTPCFEM_H
