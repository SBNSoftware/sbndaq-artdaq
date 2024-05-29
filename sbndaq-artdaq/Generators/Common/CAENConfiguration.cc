//
//  CAENConfiguration.cc
//
//  Class to read and contain SBND CAEN configuration 
//   parameters
//
//
#define TRACE_NAME "CAENConfiguration"
#include "artdaq/DAQdata/Globals.hh"

#include "CAENConfiguration.hh"

// Constructor
sbndaq::CAENConfiguration::CAENConfiguration(fhicl::ParameterSet const & ps):
  link(0),
  nBoards(0),
  enableReadout(0),
  boardId(0),
  recordLength(0),
  postPercent(0),
  irqWaitTime(0),
  allowTriggerOverlap(true),
  dynamicRange(0),
  ioLevel(0),
  nChannels(0),
  triggerPolarity(0),
  extTrgMode(0),
  swTrgMode(0),
  acqMode(0),
  debugLevel(0),
  runSyncMode(0),
  analogMode(0),
  testPattern(0),
  //ovthValue(0),
  triggerLogic(0),
  majorityLevel(0),
  majorityCoincidenceWindow(0),
  maxTemp(80),
  temperatureCheckMask(0xFFFF)
{
  link                 = ps.get<int>("link");
  enableReadout        = ps.get<int>("enableReadout");
  boardId              = ps.get<int>("board_id");
  recordLength         = ps.get<int>("recordLength");
  runSyncMode          = ps.get<int>("runSyncMode");
  allowTriggerOverlap  = ps.get<bool>("allowTriggerOverlap");
  dynamicRange         = ps.get<int>("dynamicRange");
  ioLevel              = ps.get<int>("ioLevel");
  nChannels            = ps.get<int>("nChannels");
  extTrgMode           = ps.get<int>("extTrgMode");
  swTrgMode            = ps.get<int>("swTrgMode");
  acqMode              = ps.get<int>("acqMode");
  triggerPolarity      = ps.get<int>("triggerPolarity");
  triggerPulseWidth    = ps.get<uint8_t>("triggerPulseWidth");
  debugLevel           = ps.get<int>("debugLevel");
  postPercent          = ps.get<int>("postPercent");
  irqWaitTime          = ps.get<int>("irqWaitTime");
  readoutMode          = ps.get<int>("readoutMode");
  analogMode           = ps.get<int>("analogMode");
  testPattern          = ps.get<int>("testPattern");
 // ovthValue            = ps.get<int>("ovthValue");         
  triggerLogic         = ps.get<int>("triggerLogic");  
  majorityLevel        = ps.get<int>("majorityLevel"); 
  majorityCoincidenceWindow = ps.get<int>("majorityCoincidenceWindow"); 
  maxTemp              = ps.get<uint32_t>("maxTempCelsius",80);
  temperatureCheckMask = ps.get<uint32_t>("temperatureCheckMask",0xFFFF);

  char tag[1024];
  channelEnableMask = 0;

  for ( int j=0; j<MAX_CHANNELS; j++){
    sprintf(tag,"triggerThreshold%d", j);
    triggerThresholds[j] = ps.get<uint16_t>(tag);
  }

  if ( enableReadout ){
    for ( int j=0; j<MAX_CHANNELS; j++){
      sprintf(tag,"channelEnable%d", j);
      channelEnable[j] = ps.get<bool>(tag);
      if ( channelEnable[j] )
      {
	channelEnableMask |= ( 1 << j );
	sprintf(tag,"channelPedestal%d", j);
	pedestal[j] = ps.get<int>(tag);
      }

      /*
      sprintf(tag,"channelSelfTrgMode%d", j);
      channelSelfTrgMode[j] = ps.get<bool>(tag);
      if ( channelSelfTrgMode[j] )
      {
	channelEnableMask |= ( 1 << j );
	sprintf(tag,"channelPedestal%d", j);
	pedestal[j] = ps.get<int>(tag);
      }
      */
    }
  }
}


void sbndaq::CAENConfiguration::print(std::ostream& os) 
{
  os << *this;
}

std::ostream& operator<<(std::ostream& os, const sbndaq::CAENConfiguration& e)
{
  os << "CAEN Configuration" << std::endl;
  os << "  Link                  " << e.link << std::endl;
  os << "  nBoards               " << e.nBoards << std::endl;
  os << "  EnableReadout         " << e.enableReadout << std::endl;
  os << "  RecordLength          " << e.recordLength << std::endl;
  os << "  AllowTriggerOverlap   " << e.allowTriggerOverlap << std::endl;
  os << "  DynamicRange          " << e.dynamicRange << std::endl;
  os << "  nChannels             " << e.nChannels << std::endl;
  os << "  PostPercent           " << e.postPercent << "%" << std::endl;
  os << "  IrqWaitTime           " << e.irqWaitTime << std::endl;
  os << "  IOLevel (NIM or TTL)  " << e.ioLevel << " " 
     << sbndaq::CAENDecoder::IOLevel((CAEN_DGTZ_IOLevel_t)e.ioLevel) << std::endl;
  os << "  TriggerPolarity       " << e.triggerPolarity << " " 
     << sbndaq::CAENDecoder::TriggerPolarity((CAEN_DGTZ_TriggerPolarity_t)e.triggerPolarity) << std::endl;
  os << "  TriggerPulseWidth     " << e.triggerPulseWidth << std::endl;
  os << "  ExtTrgMode            " << e.extTrgMode << " " 
     << sbndaq::CAENDecoder::TriggerMode((CAEN_DGTZ_TriggerMode_t)e.extTrgMode) << std::endl;
  os << "  SWTrgMode             " << e.swTrgMode << " " 
     << sbndaq::CAENDecoder::TriggerMode((CAEN_DGTZ_TriggerMode_t)e.swTrgMode) << std::endl;
  for ( int j=0; j<sbndaq::CAENConfiguration::MAX_CHANNELS; j++)
      os << "    Channel " << j << " Threshold " << e.triggerThresholds[j] << std::endl;
  os << "  AcqMode               " << e.acqMode << " " 
     << sbndaq::CAENDecoder::AcquisitionMode((CAEN_DGTZ_AcqMode_t)e.acqMode) << std::endl;
  os << "  DebugLevel            " << e.debugLevel << std::endl;
  os << "  ReadoutMode           " << e.readoutMode << " " 
     << sbndaq::CAENDecoder::EnaDisMode((CAEN_DGTZ_EnaDis_t)e.readoutMode) << std::endl;
  os << "  AnalogMode            " << e.analogMode << std::endl;
  os << "  TestPattern           " << e.testPattern << std::endl;
 // os << "  OvthValue             " << e.ovthValue << std::endl;
  os << "  TriggerLogic          " << e.triggerLogic << std::endl;
  os << "  MajorityLevel         " << e.majorityLevel << std::endl;
  os << "  MajorityCoincidenceWindow " << e.majorityCoincidenceWindow << std::endl;
  os << "  MaxTempCelsius        " << e.maxTemp << std::endl;
  os << "  TemperatureCheckMask  0x" << std::hex << e.temperatureCheckMask << std::dec << std::endl;
  os << "  BoardId               " << e.boardId << 
      "  EnableReadout " << e.enableReadout << std::endl;
  if ( e.enableReadout )
  {
    os << "    ChannelEnableMask   0x" << std::hex << e.channelEnableMask << std::dec << std::endl;
    for ( int j=0; j<sbndaq::CAENConfiguration::MAX_CHANNELS; j++)
    {
      if ( e.channelEnable[j] )
      {
	os << "    Channel " << j << " Pedestal " << e.pedestal[j] << std::endl;
      }
    }
  }
  return(os);
}
