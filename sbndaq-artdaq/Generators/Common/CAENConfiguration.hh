//
//  CAENConfiguration.h   (W.Badgett)
//

#ifndef _CAENConfiguration_h
#define _CAENConfiguration_h

#include "fhiclcpp/ParameterSet.h"
#include <iostream>
#include <trace.h>

namespace sbndaq
{
  class CAENConfiguration
  {
  public:
    enum
    {
      MAX_BOARDS = 8,
      MAX_CHANNELS = 16
    };

    /// Constructor
    CAENConfiguration(fhicl::ParameterSet const &ps);
    
    /// Virtual destructor
    virtual ~CAENConfiguration() {}
    
    /// Prints all configuration variables
    void print(std::ostream &os = std::cout);

    int link;                      ///> optical link number
    uint32_t fragmentId;           ///> fragment id
    int boardId;                   ///> board id
    int boardChainNumber;          ///> board optlink chain number 
    int nChannels;                 ///> number of channels
    std::size_t aX818;             ///> vme bridge card (A3818 or A5818)
    bool calibrateOnConfig;        ///> force ADC calibration on config
    bool lockTempCalibration;      ///> disable temperature self-calibration
    bool writeCalibration;         ///> write manual ADC calibration parameters
    int acqMode;                   ///> sets run acquisition mode
    int runSyncMode;               ///> sets run synchronization mode
    int recordLength;              ///> size of the recorded waveform buffer in samples
    int postPercent;               ///> sets the post-trigger buffer fraction
    int ioLevel;                   ///> sets i/o level
    int dynamicRange;              ///> sets the input dynamic range
    int testPattern;               ///> enable debugging test pattern
    int analogMode;                ///> sets signal to output on the Analog Monitor Front Panel
    uint32_t maxEventsPerTransfer; ///> max number of events for each transfer
    std::size_t poolBufferSize;    ///> boardreader pool buffer size
    uint32_t getNextSleep;         ///> sleep time between consecutive GetNext() calls
    uint32_t getNextFragmentBunchSize; ///> unclear use in the code...?
    int extTrgMode;                ///> sets the external trigger input mode
    int swTrgMode;                 ///> sets the software trigger mode
    uint32_t selfTrgMode;          ///> sets the self trigger mode
    uint32_t selfTrgMask;          ///> sets the bit mask to apply the self-trigger mode 
    bool swTrigger;                ///> sends a software trigger
    bool allowTriggerOverlap;      ///> allows overlapping triggers
    uint32_t selfTrgBit;           ///> self trigger polarity bit
    int triggerPolarity;           ///> sets trigger polarity via CAEN_DGTZ library
    int majorityLevel;             ///> majority level for self trigger mode
    int majorityCoincidenceWindow; ///> majority coincident window for self trigger mode
    int triggerLogic;              ///> self trigger logic setting
    uint32_t modeLVDS;             ///> LVDS output mode
    uint32_t trigInLevel;          ///> TRG_IN on level (1) or edge (0)
    uint8_t triggerPulseWidth;     ///> LVDS output pulse width (all channels)
    bool useTimeTagForTimeStamp;       ///> use TTT for fragment timestamp
    bool useTimeTagShiftForTimeStamp;  ///> use TTT shift for fragment timestamp
    uint32_t timeOffsetNanoSec;        ///> add time offset to fragment timestamp (optional)
    bool outputClk;                    ///> output motherboard CLK to TRG-OUT
    bool outputClkPhase;               ///> output motherboard CLK PHASE to TRG-OUT
    uint32_t maxTemp;                  ///> max temperature before warnings are issued
    uint32_t temperatureCheckMask;     ///> 8 bit mask saying which ADCs to perform the temperature check
    uint32_t IRQTimeoutMS;             ///> timeout value waiting for an interrupt in ms
    int channelEnable[MAX_CHANNELS];           ///> sets channel enable mask
    uint32_t channelEnableMask;
    int pedestal[MAX_CHANNELS];                ///> sets channel baseline pedestal
    uint16_t triggerThresholds[MAX_CHANNELS];  ///> sets channel trigger threshold
    uint32_t LVDSOutWidth[MAX_CHANNELS];       ///> sets channel LVDS output pulse width
    uint32_t LVDSLogicValue[MAX_CHANNELS / 2]; ///> sets logic values for LVDS pairs

  };
}

/// Define << operator for easy streaming
std::ostream &operator<<(std::ostream &os, const sbndaq::CAENConfiguration &e);

namespace
{
  template <>
  inline TraceStreamer &TraceStreamer::
  operator<<(const sbndaq::CAENConfiguration &r)
  {
    std::ostringstream s;
    s << r;
    return *this;
  }
}

#endif
