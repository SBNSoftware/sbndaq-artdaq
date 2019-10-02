//
//  sbndaq-artdaq/Generators/Common/CAENV1730Readout.hh
//

#ifndef sbndaq_artdaq_Generators_CAENV1730Readout_hh
#define sbndaq_artdaq_Generators_CAENV1730Readout_hh

#include "messagefacility/MessageLogger/MessageLogger.h"
#include "fhiclcpp/fwd.h"
#include "artdaq-core/Data/Fragment.hh"
#include "artdaq/Generators/CommandableFragmentGenerator.hh"

#include "CAENDigitizer.h"
#include "CAENDigitizerType.h"
#include "sbndaq-artdaq-core/Overlays/Common/CAENV1730Fragment.hh"

#include "CAENConfiguration.hh"

//#include "CircularBuffer.hh"
#include "PoolBuffer.hh"
#include "workerThread.hh"

#include <string>

namespace sbndaq
{

  class CAENV1730Readout : public artdaq::CommandableFragmentGenerator{

  public:

    explicit CAENV1730Readout(fhicl::ParameterSet const& ps);
    virtual ~CAENV1730Readout();

    bool getNext_(artdaq::FragmentPtrs & output) override;
    bool checkHWStatus_() override;
    void start() override;
    void stop() override;
    void stopNoMutex() override { stop(); }
    //void init();

  private:
    bool readSingleWindowFragments(artdaq::FragmentPtrs &);
    bool readSingleWindowDataBlock();
	
    bool readCombinedWindowFragments(artdaq::FragmentPtrs &);
		
    void loadConfiguration(fhicl::ParameterSet const& ps);
    void configureInterrupts();

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
    CAEN_DGTZ_AcqMode_t   fAcqMode;	// initialized in the constructor


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

    typedef enum {
      DYNAMIC_RANGE      = 0x8028,
      ACQ_CONTROL        = 0x8100,
      GLOBAL_TRIGGER     = 0x810C,
      FP_TRG_OUT_CONTROL = 0x8110,
      FP_IO_CONTROL      = 0x811C,
      FP_LVDS_CONTROL    = 0x81A0,
      READOUT_CONTROL    = 0xEF00,
      // Aiwu add registers for the LVDS logic
      FP_LVDS_Logic_G1   = 0x1084,
      FP_LVDS_Logic_G2   = 0x1284,
      FP_LVDS_Logic_G3   = 0x1484,
      FP_LVDS_Logic_G4   = 0x1684,
      FP_LVDS_Logic_G5   = 0x1884,
      FP_LVDS_Logic_G6   = 0x1A84,
      FP_LVDS_Logic_G7   = 0x1C84,
      FP_LVDS_Logic_G8   = 0x1E84,
      // Aiwu add end
      // Aiwu add registers for the LVDS output width
      FP_LVDS_OutWidth_Ch1   = 0x1070,
      FP_LVDS_OutWidth_Ch2   = 0x1170,
      FP_LVDS_OutWidth_Ch3   = 0x1270,
      FP_LVDS_OutWidth_Ch4   = 0x1370,
      FP_LVDS_OutWidth_Ch5   = 0x1470,
      FP_LVDS_OutWidth_Ch6   = 0x1570,
      FP_LVDS_OutWidth_Ch7   = 0x1670,
      FP_LVDS_OutWidth_Ch8   = 0x1770,
      FP_LVDS_OutWidth_Ch9   = 0x1870,
      FP_LVDS_OutWidth_Ch10   = 0x1970,
      FP_LVDS_OutWidth_Ch11   = 0x1A70,
      FP_LVDS_OutWidth_Ch12   = 0x1B70,
      FP_LVDS_OutWidth_Ch13   = 0x1C70,
      FP_LVDS_OutWidth_Ch14   = 0x1D70,
      FP_LVDS_OutWidth_Ch15   = 0x1E70,
      FP_LVDS_OutWidth_Ch16   = 0x1F70

    } ADDRESS_t;

    typedef enum 
    {
      ENABLE_LVDS_TRIGGER   = 0x20000000,
      ENABLE_EXT_TRIGGER    = 0x40000000,
      ENABLE_NEW_LVDS       = 0x100,
      ENABLE_TRG_OUT        = 0xFF,
      TRIGGER_LOGIC         = 0x1F00,
      DISABLE_TRG_OUT_LEMO  = 0x2,
      LVDS_IO               = 0x3C,
      LVDS_BUSY             = 0,
      LVDS_TRIGGER          = 1,
      LVDS_nBUSY_nVETO      = 2,
      LVDS_LEGACY           = 3,
      MAJORITY_TIME_WINDOW_S = 20,
      MAJORITY_LEVEL_S      = 24
    } IO_MASK_t;

    enum {
      TERROR    = 0,
      TWARNING  = 1,
      TINFO     = 2,
      TDEBUG    = 3,
      TCONFIG   = 4,
      TSTART    = 5,
      TSTOP     = 6,
      TSTATUS   = 7,
      TGETNEXT  = 8,
      TGETDATA  = 9,
      TMAKEFRAG = 10,
      TTEMP     = 30
    };

    //fhicl parameters
    int fVerbosity;
    int fBoardChainNumber;
    uint8_t  fInterruptLevel;
    uint16_t fInterruptEventNumber;
    uint32_t fInterruptStatusID;
    uint32_t fGetNextSleep;
    uint32_t fGetNextFragmentBunchSize;
    bool     fSWTrigger;
    uint32_t fSelfTriggerMode;
    uint32_t fSelfTriggerMask;
    uint32_t fModeLVDS;
    bool     fCombineReadoutWindows;
    bool     fCalibrateOnConfig;
    uint32_t fFragmentID;
    uint32_t fMajorityLevel;
    uint32_t fMajorityTimeWindow;

    // Aiwu add fhicl parameters - LVDS logic
    uint32_t fLVDSLogicValueG1;
    uint32_t fLVDSLogicValueG2;
    uint32_t fLVDSLogicValueG3;
    uint32_t fLVDSLogicValueG4;
    uint32_t fLVDSLogicValueG5;
    uint32_t fLVDSLogicValueG6;
    uint32_t fLVDSLogicValueG7;
    uint32_t fLVDSLogicValueG8;
    // Aiwu add end
    // Aiwu add fhicl parameters - LVDS output pulse width
    uint32_t fLVDSOutWidthC1;
    uint32_t fLVDSOutWidthC2;
    uint32_t fLVDSOutWidthC3;
    uint32_t fLVDSOutWidthC4;
    uint32_t fLVDSOutWidthC5;
    uint32_t fLVDSOutWidthC6;
    uint32_t fLVDSOutWidthC7;
    uint32_t fLVDSOutWidthC8;
    uint32_t fLVDSOutWidthC9;
    uint32_t fLVDSOutWidthC10;
    uint32_t fLVDSOutWidthC11;
    uint32_t fLVDSOutWidthC12;
    uint32_t fLVDSOutWidthC13;
    uint32_t fLVDSOutWidthC14;
    uint32_t fLVDSOutWidthC15;
    uint32_t fLVDSOutWidthC16;
    // Aiwu add end

    //internals
    size_t   fNChannels;
    uint32_t fBoardID;
    bool     fOK;
    bool     fail_GetNext;
    uint32_t fEvCounter; // set to zero at the beginning
    uint32_t last_rcvd_rwcounter;
    uint32_t last_sent_rwcounter;
    uint32_t total_data_size;
    //uint32_t event_size;	
    uint32_t n_readout_windows;
    uint32_t ch_temps[CAENConfiguration::MAX_CHANNELS];
    
    //functions
    void Configure();

    void ConfigureRecordFormat();    
    void ConfigureDataBuffer();
    void ConfigureTrigger();
    void ConfigureReadout();
    void ConfigureAcquisition();
    void ConfigureLVDS();
    void ConfigureSelfTriggerMode();
    void ConfigureMajorityLogic();
    void RunADCCalibration();

    bool WaitForTrigger();
    bool GetData();
    share::WorkerThreadUPtr GetData_thread_;
    sbndaq::PoolBuffer fPoolBuffer; 		
    size_t fCircularBufferSize;
    std::unique_ptr<uint16_t[]> fBuffer;

    void CheckReadback(std::string,int,uint32_t,uint32_t,int channelID=-1);

    CAEN_DGTZ_ErrorCode	WriteRegisterBitmask(int32_t handle, uint32_t address,
					     uint32_t data, uint32_t bitmask); 
    
  };

}

#endif
