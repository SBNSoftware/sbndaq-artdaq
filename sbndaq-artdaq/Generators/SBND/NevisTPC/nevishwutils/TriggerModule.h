#ifndef _NEVISTPC_TRIGGERMODULE_H
#define _NEVISTPC_TRIGGERMODULE_H  1

#include "ControllerModule.h"

namespace nevistpc
{
  class TriggerModuleStatus {
    
  public:
    
  TriggerModuleStatus( uint8_t mod_num=0 ) 
    : module(mod_num),    
      frame_ctr(0), 
      sample_ctr_tpc(0),
      sample_ctr_trigger(0),
      trigger_ctr(0),
      pmt_trigger_bit_ctr(8,0),
      gps_frame(0),
      gps_sample(0),
      gps_sample_div(0),
      badstat(false)
	{}
    
    ~TriggerModuleStatus() {}
    
    int16_t module;
    int32_t frame_ctr;
    int32_t sample_ctr_tpc;
    int32_t sample_ctr_trigger;
    int32_t trigger_ctr;
    std::vector<uint32_t> pmt_trigger_bit_ctr;
    int32_t gps_frame;
    int32_t gps_sample;
    int32_t gps_sample_div;
    void report() const;

    void set_badstat() { badstat=true; }
    bool get_badstat() const { return badstat; }
    
  private:
    bool badstat;
    
  };

  class TriggerModuleConfig
  {
  public:    
  TriggerModuleConfig():deadtime(0), framesize(0),
      trigmask0(0),trigmask1(0),trigmask8(0),
      prescale0(0),prescale1(0),prescale8(0),prescale9(0),
      g1start(0),g1end(0),g1width(0),g1delay(0),
      g2start(0),g2end(0),g2width(0),g2delay(0),
      timeinfo_file_path("")
	{}

    void readConfiguration( fhicl::ParameterSet const & ps );
    std::string debugInfo () const noexcept;
    
    data_payload_t deadtime;
    data_payload_t framesize;
    data_payload_t trigmask0;
    data_payload_t trigmask1;
    data_payload_t trigmask8;
    data_payload_t prescale0;
    data_payload_t prescale1;
    data_payload_t prescale8;
    data_payload_t prescale9;
    data_payload_t g1start;
    data_payload_t g1end;
    data_payload_t g1width;
    data_payload_t g1delay;
    data_payload_t g2start;
    data_payload_t g2end;
    data_payload_t g2width;
    data_payload_t g2delay;
    
    std::string timeinfo_file_path;

  protected:
    enum {
      TERROR    = TLVL_ERROR,
      TWARNING  = TLVL_WARNING,
      TINFO     = TLVL_INFO,
      TDEBUG    = TLVL_DEBUG
    };
    
  };
 
  typedef std::unique_ptr<TriggerModuleConfig> TriggerModuleConfigUPtr;
  typedef std::shared_ptr<TriggerModuleConfig> TriggerModuleConfigSPtr;

  class TriggerModuleGPSStamp {
    
  public:
    
  TriggerModuleGPSStamp( uint32_t new_frame, uint16_t new_sample, uint16_t new_div ) 
    : gps_frame(new_frame),
      gps_sample(new_sample),
      gps_sample_div(new_div)
      {}
    
    ~TriggerModuleGPSStamp() {}
    
    uint32_t gps_frame; // Frame number when GPS pulse was received
    uint16_t gps_sample; // 2 MHz sample when GPS pulse was received
    uint16_t gps_sample_div; // 16 MHz sample when GPS pulse was received
  };
  
  class TriggerModule : public UsesControllerModule
  {
  public:
    //Chip id=1
    enum chip_0:
    operation_id_t {
      RUN_OFF = 0,
      RUN_ON__SYNC_ON, //"mb_trig_run" in Nevis lingo
      FRAME_LENGTH,
      DEADTIME_SIZE,
      ACTIVE_SIZE,
      GATE1_DELAY,
      GATE2_DELAY,
      ENABLE_TRIGGER,
      CALIB_DELAY,

      PRESCALE0 = 10,
      PRESCALE1,
      PRESCALE2,
      PRESCALE3,
      PRESCALE4,      
      PRESCALE5,
      PRESCALE6,
      PRESCALE7,
      PRESCALE8,
      PRESCALE9,
        
      MASK0 = 20,
      MASK1,
      MASK2,
      MASK3,
      MASK4,
      MASK5,
      MASK6,
      MASK7,
      MASK8,

      READ_PARAM=30,
      SEND_PC_TRIGGER,
      READ_STATUS,
      COUNTER_RESET,
      SEND_CALIB_PULSE,
      //GPS_TB_FRAME_SAMPLE,
        
      GATE1_ACTIVE_START=36,
      GATE1_ACTIVE_END,
      GATE2_ACTIVE_START,
      GATE2_ACTIVE_END,

      GATE1_WIDTH=45,
      GATE2_WIDTH,

      OUT_PULSE1_DELAY=50,
      OUT_PULSE1_WIDTH,
      OUT_PULSE2_DELAY,
      OUT_PULSE2_WIDTH,
      OUT_PULSE3_DELAY,
      OUT_PULSE3_WIDTH,

      OUT_PULSE_DELAY=58,
      OUT_PULSE_MODE_SELECT,
      OUT_PULSE1_FIRE,
      OUT_PULSE2_FIRE,
      OUT_PULSE3_FIRE,
      ENABLE_FRAME_TRIGGER,
    };
    typedef chip_0 trigger;
    
    enum chip_3:
    operation_id_t {
      GPS_TB_FRAME_SAMPLE=35,
    };    
    typedef chip_3 GPS;

    enum device:
    chip_address_t
    {
      trg = 0,
      gps = 3
    };
    
public:
    explicit TriggerModule( uint8_t slot_number ) ;
    ~TriggerModule();

    void useController( ControllerModuleSPtr crateController );

    // void registerGangliaMetricsImpl();
    void configureTrigger(fhicl::ParameterSet const & ps); // uboonedaq runConfig()

    void runOnSyncOn();
    void runOnSyncOff();
    void runOff();
    void disableTriggers(bool write_time_info=true);
    void enableTriggers(bool write_time_info=true);
    void setFrameLength(data_payload_t const &length);
    void setDeadtimeSize(data_payload_t const &size);
    void setTrigOutput(data_payload_t const &out);
    void setMask0(data_payload_t const &mask);
    void setMask1(data_payload_t const &mask);
    void setMask8(data_payload_t const &mask);
    void setPrescale0(data_payload_t const &prescale);
    void setPrescale1(data_payload_t const &prescale);
    void setPrescale8(data_payload_t const &prescale);
    void setPrescale9(data_payload_t const &prescale);
    void readStatus();
    void loadStatus();
    TriggerModuleStatus const& getStatus() const { return _status; }
    void setGate1ActiveStart(data_payload_t const &size);
    void setGate1ActiveEnd(data_payload_t const &size);
    void setGate1Width(data_payload_t const &size);
    void setGate1Delay(data_payload_t const &size);
    void setGate2ActiveStart(data_payload_t const &size);
    void setGate2ActiveEnd(data_payload_t const &size);
    void setGate2Width(data_payload_t const &size);
    void setGate2Delay(data_payload_t const &size);
    void setCalibDelay(data_payload_t const &size);
    void sendOutPCtrigger();
    void sendOutCalibTrigger();
    TriggerModuleGPSStamp getLastGPSClockRegister();

    TriggerModule() = delete;
    TriggerModule(TriggerModule const &) = delete;
    TriggerModule &operator= (TriggerModule const &) = delete;
    TriggerModule(TriggerModule &&) = delete;
    TriggerModule &operator= (TriggerModule &&) = delete;

    // Write down the time stamp in a text file
    void writeTriggerTimeInfo( std::string );

  protected:
    enum {
      TERROR    = TLVL_ERROR,
      TWARNING  = TLVL_WARNING,
      TINFO     = TLVL_INFO,
      TDEBUG    = TLVL_DEBUG
    };
    
  private:
    uint8_t _slot_number; 
    TriggerModuleStatus _status;
    std::string _timeinfo_file_path;
};

typedef std::unique_ptr<TriggerModule> TriggerModuleUPtr;
typedef std::shared_ptr<TriggerModule> TriggerModuleSPtr;

} // end of namespace hwutils

#endif //_NEVISTPC_TRIGGERMODULE_H








