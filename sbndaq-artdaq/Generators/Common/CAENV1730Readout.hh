//
//  sbndaq-artdaq/Generators/Common/CAENV1730Readout.hh
//

#ifndef sbndaq_artdaq_Generators_CAENV1730Readout_hh
#define sbndaq_artdaq_Generators_CAENV1730Readout_hh

#include "messagefacility/MessageLogger/MessageLogger.h"
#include "fhiclcpp/fwd.h"
#include "artdaq-core/Data/Fragment.hh"
#include "artdaq/Generators/CommandableFragmentGenerator.hh"
#include "sbndaq-artdaq-core/Overlays/Common/CAENV1730Fragment.hh"

#include "CAENDigitizer.h"
#include "CAENDigitizerType.h"

#include "sbndaq-artdaq/Generators/Common/CAENConfiguration.hh"
#include "sbndaq-artdaq/Generators/Common/PoolBuffer.hh"
#include "sbndaq-artdaq/Generators/Common/workerThread.hh"

#include <string>
#include <unordered_map>
#include <mutex>

namespace sbndaq
{

  class CAENV1730Readout : public artdaq::CommandableFragmentGenerator{

  public:

    // constructor: initialize and configure
    explicit CAENV1730Readout(fhicl::ParameterSet const& ps);
    // destructor: freeing the buffer
    virtual ~CAENV1730Readout();

    // getNext_ builds fragments from buffer
    bool getNext_(artdaq::FragmentPtrs & output) override;
    // poll hardware status 
    bool checkHWStatus_() override;
    // called at START transition
    void start() override;
    // called at STOP transition
    void stop() override;
    void stopNoMutex() override { stop(); }

  private:

    // support function for getNext_ loop
    bool readSingleWindowFragments(artdaq::FragmentPtrs &);
    // waits for interrupt, puts data into buffer
    bool GetData();
    // support function for GetData loop
    bool readWindowDataBlocks();

    // support function for configuration
    // called from constructor
    void Configure();
    void ConfigureInterrupts();
    void ConfigureRecordFormat();    
    void ConfigureDataBuffer();
    void ConfigureTrigger();
    void ConfigureReadout();
    void ConfigureAcquisition();
    void ConfigureLVDS();
    void ConfigureSelfTriggerMode();
    void ConfigureClkToTrgOut();
	
    // print board + CAEN software info
    void GetSWInfo();

    // run ADC self-calibration 
    void RunADCCalibration();
    // lock ADC temperature self-calibration
    void SetLockTempCalibration(bool onOff, uint32_t ch);

    // support function for register read/write operations
    CAEN_DGTZ_ErrorCode WriteSPIRegister(int handle, uint32_t ch, uint32_t address, uint8_t value);
    CAEN_DGTZ_ErrorCode ReadSPIRegister(int handle, uint32_t ch, uint32_t address, uint8_t *value);

    // check readback value from register
    void CheckReadback(std::string,int,uint32_t,uint32_t,int channelID=-1);

    //CAEN pieces
    CAENConfiguration     fCAEN;	    // initialized in the constructor
    CAEN_DGTZ_BoardInfo_t fBoardInfo; // board S/N, firmware relase
    size_t   fNChannels; // number of channels
    int fHandle;         // access handle
    bool fail_GetNext; // tracks GetNext_ failure

    // PoolBuffer implementation
    sbndaq::PoolBuffer fPoolBuffer;  		
    std::unique_ptr<uint16_t[]> fBuffer; 
    uint32_t fBufferSize;

    // GetData worker thread
    share::WorkerThreadUPtr GetData_thread_;

    //internals in getting the data
    boost::posix_time::ptime fTimePollEnd,fTimePollBegin;
    boost::posix_time::ptime fTimeEpoch;
    boost::posix_time::time_duration fTimeDiffPollBegin, fTimeDiffPollEnd;

    // map fragmen sequence ID to timestamp
    std::unordered_map<uint32_t,artdaq::Fragment::timestamp_t> fTimestampMap;
    mutable std::mutex fTimestampMapMutex;

    // fragment timestamping
    artdaq::Fragment::timestamp_t fTS; 
    uint64_t fMeanPollTime;
    uint64_t fMeanPollTimeNS;
    uint32_t fTTT;
    long fTTT_ns;

    // max event number internal to the V1730 board
    static constexpr uint32_t EVENT_COUNTER_MASK = 0xFFFFFFu; // 24-bit
    // last event counter seen in GetData() thread
    uint32_t last_rcv_event_counter;

    // count overflows of V1730 event counter
    uint32_t fOverflowCounter; 
    // last fragment event counter sent
    uint32_t last_sent_event_counter;
    // last fragment sequence id sent
    uint64_t last_sent_seqid;
    // last fragment timestamps sent
    artdaq::Fragment::timestamp_t last_sent_ts;

    // hardware status check
    uint32_t ch_temps[CAENConfiguration::MAX_CHANNELS];
    uint32_t ch_status[CAENConfiguration::MAX_CHANNELS];
    // number of board memory buffers, read back from BUFFER_ORGANIZATION
    uint32_t fNumBoardBuffers;
    
    typedef enum {
      BOARD_CONFIG_READ  = 0x8000, // board configuration read register
      BOARD_CONFIG_SET   = 0x8004, // board configuration set register
      BOARD_CONFIG_CLEAR = 0x8008, // board configuration clear register
      FP_TRG_OUT_CONTROL = 0x8110, // front panel TRG-OUT control
      FP_IO_CONTROL      = 0x811C, // front panel I/O control
      FP_LVDS_CONTROL    = 0x81A0, // front panel LVDS control
      ACQ_CONTROL        = 0x8100, // acquisition control register
      READOUT_CONTROL    = 0xEF00, // readout control
      BUFFER_ORGANIZATION= 0x800C, // number of board memory buffers = 2^N
      EVENT_STORED       = 0x812C, // events currently stored, awaiting readout
      EVENT_SIZE         = 0x814C, // size of the next event, in 32-bit words
      GLB_TRG_MASK       = 0x810C, // global trigger mask
      CH_ENABLE_MASK     = 0x8120, // channel enable mask
      DYNAMIC_RANGE      = 0x8028, // dynamic range control 
      TRG_OUT_WIDTH_CH   = 0x1070, // channel n LVDS pulse width
                                   // 0x1n70 for n=0,..,F
      SLF_TRG_LG_CH      = 0x1084, // couple n self-trigger logic
                                   // 0x1n84 for n=0,2,4,6,8,A,C,E
      ANALOG_MON_MODE    = 0x8144, // analog monitor output mode
    } ADDRESS_t;

    typedef enum 
    {
      TRIGGER_OVERLAP_MASK = 0x0002, // bitmask for trigger overlap
      SLF_TRG_BIT_MASK     = 0x40,   // bitmask for self-trigger polarity
      ENABLE_NEW_LVDS      = 0x100,  // bitmask to enable "new" LVDS features
      TRG_IN_LEVEL         = 0x400,  // bitmask to configure TRG-IN as level/edge
      DISABLE_TRG_OUT_LEMO = 0x2,    // bitmask to disable TRG-OUT LEMO output
      LVDS_IO              = 0x3C,   // bitmask for LVDS I/O pins
      LVDS_BUSY            = 0, // LVDS output is BUSY status
      LVDS_TRIGGER         = 1, // LVDS output is TRIGGER (ICARUS mode)
      LVDS_nBUSY_nVETO     = 2, // LVDS output is nBUSY/nVETO
      LVDS_LEGACY          = 3  // legacy LVDS behavior
    } IO_MASK_t;

    enum 
    {
      TERROR    = TLVL_ERROR,
      TWARNING  = TLVL_WARNING,
      TINFO     = TLVL_INFO,
      TDEBUG    = TLVL_DEBUG,
      TCONFIG   = 9,
      TSTART    = 10,
      TSTOP     = 11,
      TSTATUS   = 12,
      TGETNEXT  = 13,
      TGETDATA  = 14,
      TMAKEFRAG = 15,
      TTEMP     = 30
    };

    enum
    {
      V1730_UNPHYSICAL_TEMPERATURE = 200  // degC
    };

    typedef enum {
      TEST_PATTERN_S=3
    } TEST_PATTERN_t;

  };
}

#endif
