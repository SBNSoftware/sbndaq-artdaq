#ifndef sbndaq_artdaq_Generators_CAENV1730Readout_hh
#define sbndaq_artdaq_Generators_CAENV1730Readout_hh

#include "messagefacility/MessageLogger/MessageLogger.h"
#include "fhiclcpp/fwd.h"
#include "artdaq-core/Data/Fragment.hh" 
#include "artdaq/Application/CommandableFragmentGenerator.hh"

#include "CAENDigitizer.h"
#include "CAENDigitizerType.h"
#include "sbndaq-artdaq-core/Overlays/Common/CAENV1730Fragment.hh"

#include "CAENConfiguration.hh"

#include "CircularBuffer.hh"
#include "workerThread.hh"

#include <string>

namespace sbndaq
{

  class CAENV1730Readout : public artdaq::CommandableFragmentGenerator{

  public:

    explicit CAENV1730Readout(fhicl::ParameterSet const& ps);
    virtual ~CAENV1730Readout();

    bool getNext_(artdaq::FragmentPtrs & output) override;
    void start() override;
    void stop() override;
    void stopNoMutex() override { stop(); }
    //void init();

  private:

    typedef enum 
    { 
      CONFIG_READ_ADDR     = 0x8000,
      CONFIG_SET_ADDR      = 0x8004,
      CONFIG_CLEAR_ADDR    = 0x8008,
      TRIGGER_OVERLAP_MASK = 0x0002
    } REGISTERS_t;

    //CAEN pieces
    CAENConfiguration     fCAEN;	// initialized in the constructor
    int                   fHandle;
    CAEN_DGTZ_BoardInfo_t fBoardInfo;
    //char*                 fBuffer;
    uint32_t              fBufferSize;
    //uint32_t              fCircularBufferSize;
    CAEN_DGTZ_AcqMode_t   fAcqMode;		// initialized in the constructor

    typedef enum {
      TEST_PATTERN_S=3
    } TEST_PATTERN_t;
    
    typedef enum {
      BOARD_READY  = 0x0100,
      PLL_STATUS   = 0x0080,
      PLL_BYPASS   = 0x0040,
      CLOCK_SOURCE = 0x0020,
      EVENT_FULL   = 0x0010,
      EVENT_READY  = 0x0008,
      RUN_ENABLED  = 0x0004
    } ACQ_STATUS_MASK_t;

    enum {
      TERROR      = 0,
	TWARNING  = 1,
	TINFO     = 2,
	TDEBUG    = 3,
	
	TCONFIG   = 4,
	TSTART    = 5,
	TSTOP     = 6,
	TSTATUS   = 7,
	TGETNEXT  = 8,
	TGETDATA  = 9,
	TMAKEFRAG = 10
    };

    //fhicl parameters
    int fVerbosity;
    int fBoardChainNumber;
    uint8_t  fInterruptLevel;
    uint16_t fInterruptEventNumber;
    uint32_t fInterruptStatusID;
    uint32_t fGetNextSleep;
    bool     fSWTrigger;
    bool     fCombineReadoutWindows;
    
    //internals
    size_t   fNChannels;
    uint32_t fBoardID;
    bool     fOK;
    uint32_t fEvCounter; // set to zero at the beginning
    int32_t  prev_rwcounter;
    uint32_t total_data_size;
    uint32_t event_size;	
    uint32_t n_readout_windows;



    //functions
    void Configure();

    void ConfigureRecordFormat();    
    void ConfigureDataBuffer();
    void ConfigureTrigger();
    void ConfigureReadout();
    void ConfigureAcquisition();

    bool WaitForTrigger();
    bool GetData();
    share::WorkerThreadUPtr GetData_thread_;
    sbndaq::CircularBuffer<uint16_t> fCircularBuffer;
    size_t fCircularBufferSize;
    std::unique_ptr<uint16_t[]> fBuffer;

    void CheckReadback(std::string,int,uint32_t,uint32_t,int channelID=-1);
    
  };

}

#endif
