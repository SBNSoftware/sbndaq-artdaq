//
//  CAENV1740Configuration.h   (based on CAENConfiguration.h)
//  jcrespo: to do: generalize CAENConfiguration for both V1730 and V1740
//

#ifndef _CAENV1740Configuration_h
#define _CAENV1740Configuration_h

#include "CAENDecoder.hh"
#include "fhiclcpp/ParameterSet.h"
#include <iostream>

namespace sbndaq
{
class CAENV1740Configuration
{
  public:
  enum
  {
    MAX_BOARDS = 4,
    MAX_CHANNELS = 64
  };

  virtual ~CAENV1740Configuration() {}
  CAENV1740Configuration(fhicl::ParameterSet const & ps);

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

    uint32_t  channelEnableMask;
  //  int  ovthValue;         
    int  triggerLogic;  
    int  majorityLevel; 
    int  majorityCoincidenceWindow;

    void print(std::ostream & os = std::cout);
};
}

std::ostream& operator<<(std::ostream& os, const sbndaq::CAENV1740Configuration& e);

#include <trace.h>
namespace {
template <>
inline TraceStreamer &TraceStreamer::
operator<<(const sbndaq::CAENV1740Configuration &r) {
  std::ostringstream s;
  s << r;
  return *this;
}
} // namespace

#endif
