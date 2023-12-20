//
//  CAENConfiguration.h   (W.Badgett)
//

#ifndef _CAENConfiguration_h
#define _CAENConfiguration_h

#include "CAENDecoder.hh"
#include "fhiclcpp/ParameterSet.h"
#include <iostream>

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

  virtual ~CAENConfiguration() {}
  CAENConfiguration(fhicl::ParameterSet const & ps);

    int  link;
    int  nBoards;
    int  enableReadout;
    int  boardId;
    int  recordLength;
    int  postPercent;
    int  eventsPerInterrupt;
    int  irqWaitTime;
    bool allowTriggerOverlap;
    int  dynamicRange;
    int  ioLevel;
    int  nChannels;
    int  triggerPolarity;
    uint16_t triggerThresholds[MAX_CHANNELS];
    uint8_t   triggerPulseWidth;
    int  extTrgMode;
    int  swTrgMode;
    int  selfTrgMode;
    int  acqMode;
    int  debugLevel;
    int  runSyncMode;
    int  readoutMode;
    int  analogMode;
    int  testPattern;
    int  pedestal[MAX_CHANNELS];
    int  channelEnable[MAX_CHANNELS];
    int  channelSelfTrgLogic[MAX_CHANNELS/2];
    int  channelSelfTrgPulseType[MAX_CHANNELS/2];

    uint32_t  channelEnableMask;
    uint32_t  channelSelfTrgMask;
  //  int  ovthValue;         
    int  triggerLogic;  
    int  majorityLevel; 
    int  majorityCoincidenceWindow;
    uint32_t  maxTemp;

    void print(std::ostream & os = std::cout);
};
}

std::ostream& operator<<(std::ostream& os, const sbndaq::CAENConfiguration& e);

#include <trace.h>
namespace {
template <>
inline TraceStreamer &TraceStreamer::
operator<<(const sbndaq::CAENConfiguration &r) {
  std::ostringstream s;
  s << r;
  return *this;
}
} // namespace

#endif
