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
#include <deque>
#include <vector>
#include <ostream>

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

    // ------------------------------------------------------------------
    // error-22 (CAEN_DGTZ_OutOfMemory) test instrumentation.
    // TEST CODE: everything is hardcoded (ring depth, dump directory,
    // sentinel, FIFO pop count); see error22_analysis_2026-09-18.md, sec. 7.
    // ------------------------------------------------------------------
    struct RawEventRecord {
      uint64_t hostPollBeginNs;    // fTimePollBegin, ns since epoch
      uint64_t hostPollEndNs;      // fTimePollEnd (when ReadData returned), ns since epoch
      uint32_t eventCounter;       // from the CAEN header
      uint32_t eventSizeWords;     // from the CAEN header
      uint32_t triggerTimeTag;     // from the CAEN header (8 ns ticks)
      uint32_t dTTT;               // TTT - previous good TTT (8 ns ticks); 0xFFFFFFFF if unknown
      uint32_t nReadsInPoll;       // 0-based index of this read within its poll
      uint32_t eventsStoredAtPoll; // EVENT_STORED read once at poll start; 0xFFFFFFFF if read failed
      uint32_t returnedBytes;      // read_data_size reported by ReadData
      uint32_t flags;              // bit0: dropped by the header size check
      std::vector<uint8_t> data;   // raw copy of the returned bytes
    };
    static constexpr size_t   kRawRingDepth  = 16;          // events kept
    static constexpr uint32_t kSentinelWord  = 0xDEADBEEFu; // block pre-fill before ReadData
    static constexpr size_t   kFifoPopWords  = 4096;        // single-word pops of 0x0000 after failure
    static constexpr uint32_t kTTTMask       = 0x7FFFFFFFu; // TTT is 31 bits
    static constexpr uint32_t kUnknown32     = 0xFFFFFFFFu;

    std::deque<RawEventRecord> fRawRing;
    bool     fIncidentDumped;          // one dump per run
    uint64_t fConsecutiveReadErrors;   // throttles the -22 log spam
    bool     fHaveLastTTT;
    uint32_t fLastTTT;
    uint32_t fExpectedEventSizeWords;  // 4 + Nch * recordLength/2
    uint64_t fAnomalousEventLogCount;  // throttles short/overlapped event warnings

    void   fillSentinel(uint8_t* begin, size_t bytes);
    size_t sentinelOverwriteExtent(const uint8_t* begin, size_t bytes, size_t& changedWords) const;
    void   recordRawEvent(const uint8_t* begin, size_t bytes, size_t readIndexInPoll,
                          uint32_t storedAtPoll, uint32_t flags);
    void   handleReadDataError(CAEN_DGTZ_ErrorCode retcode, uint8_t* blockBegin, size_t blockSize,
                               size_t blockIndex, size_t n_reads, uint32_t storedAtPoll);
    void   snapshotRegisters(std::ostream& os, const char* label);
    void   dumpIncident(CAEN_DGTZ_ErrorCode retcode, const uint8_t* blockBegin, size_t blockSize,
                        size_t blockIndex, size_t extent, size_t changedWords, size_t n_reads,
                        uint32_t storedAtPoll, uint32_t stored, uint32_t eventSize, uint32_t acqStatus);
    
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
      // error-22 instrumentation
      READOUT_BUFFER     = 0x0000, // event readout buffer (single D32 read pops one word)
      RECORD_LENGTH_REG  = 0x8020, // custom record length
      POST_TRIGGER_REG   = 0x8114, // post trigger
      BOARD_FAILURE_STAT = 0x8178, // board failure status
      READOUT_STATUS     = 0xEF04, // readout status (event ready, BERR, ...)
      CHANNEL_STATUS_CH0 = 0x1088, // 0x1n88 channel n status
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
