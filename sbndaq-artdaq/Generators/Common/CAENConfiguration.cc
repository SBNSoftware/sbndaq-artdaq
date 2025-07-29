//
//  CAENConfiguration.cc
//
//  Class to read, contain and print CAEN V1730SB
//  configuration parameters.
//

#define TRACE_NAME "CAENConfiguration"

#include "artdaq/DAQdata/Globals.hh"
#include "sbndaq-artdaq/Generators/Common/CAENConfiguration.hh"
#include "sbndaq-artdaq/Generators/Common/CAENDecoder.hh"
#include <iostream>

// Constructor reads fhicl paramaters
// NOTE: do not put defaults on REQUIRED parameters
// so that exception is thrown if missing from config

sbndaq::CAENConfiguration::CAENConfiguration(fhicl::ParameterSet const & ps)
{

  // optical COMET2 link number (range 0-7)
  // this dependes on the aX818 cards/slots  
  link = ps.get<int>("link");

  // Boardreader fragment ID
  fragmentId = ps.get<uint32_t>("fragment_id");

  // Deprecated for artdaq (everything uses fragment id)
  // but still used for error messages somewhere
  boardId = ps.get<int>("board_id");

  // board number along the optical link chain
  // we have one board per link, so always 0  
  boardChainNumber = ps.get<int>("boardChainNumber", 0);

  // number of channels on the board
  nChannels = ps.get<int>("nChannels", MAX_CHANNELS);

  // selects type of VME bridge in use
  // 3 for A3818, 5 for A5818
  aX818 = ps.get<std::size_t>("AX818", 3);

  // run ADC self-calibration during configuration
  // suggestion is to keep it true
  calibrateOnConfig = ps.get<bool>("CalibrateOnConfig");

  // turn on or off automatic mid-run temperature correction
  // known to cause baseline jumps, keep OFF (locked)
  lockTempCalibration = ps.get<bool>("LockTempCalibration");

  // manually write ADC calibration parameters
  // If true, hardcoded values are written in the registers
  // "its origin and purpose are still a total mistery", keep false.
  writeCalibration = ps.get<bool>("WriteCalibration", false);

  // sets the board acquisition mode 
  // 0=Software Controlled, 1=Front Panel S_IN, 2=First trigger
  acqMode = ps.get<int>("acqMode");

  // sets the run synchronization mode
  // currently: 0=CAEN_DGTZ_RUN_SYNC_Disabled 
  runSyncMode = ps.get<int>("runSyncMode");

  // sets the acqusition window width
  // size of the recorded waveform buffer in samples
  recordLength = ps.get<int>("recordLength");

  // sets the post-trigger buffer fraction via CAEN_DGTZ function
  // percent of waveform buffer after the trigger happens
  postPercent = ps.get<int>("postPercent");

  // sets the I/O level via CAEN_DGTZ function
  // 0=NIM, 1=TTL
  ioLevel = ps.get<int>("ioLevel");

  // sets the input dynamic range
  // writes to register 0x8028
  // 0 = 2 Vpp (default), 1 = 0.5 Vpp
  dynamicRange = ps.get<int>("dynamicRange");

  // enable triangular test wave to the ADCs for debugging purposes
  // 1 = writes 0b1000 to 0x8004 (CAEN_DGTZ_BROAD_CH_CONFIGBIT_SET_ADD)
  // 0 = writes 0b1000 to 0x8008 (CAEN_DGTZ_BROAD_CH_CLEAR_CTRL_ADD)
  // keep zero to leave it disabled
  testPattern = ps.get<int>("testPattern");

  // sets signal to output on the Analog Monitor Front Panel
  // 0 = trigger majority, 1 = test, 2 = analog inspection
  // 3 = buffer ocuppancy mode; 4 = voltage level mode
  analogMode = ps.get<int>("analogMode");

  // sets maximum number of events for each transfer
  maxEventsPerTransfer = ps.get<uint32_t>("maxEventsPerTransfer", 1);

  // boardreader poll buffer size (no longer circular!)
  // this is the size in bytes
  poolBufferSize = ps.get<std::size_t>("PoolBufferSize"); 

  // sleep time between consecutive _GetNext() calls in us
  // only in the case of no data in the pollbuffer
  // also used in case of SW trigger mode
  getNextSleep = ps.get<uint32_t>("GetNextSleep"); 

  // unclear use in the code...?
  // controls laggy GetNext metric sent to Grafana
  // currently: 0.005*20 = 0.1 s 
  // TODO: remove? rename?
  getNextFragmentBunchSize = ps.get<uint32_t>("GetNextFragmentBunchSize");

  // sets the external trigger input mode via CAEN_DGTZ function
  // external trigger is tipically enabled for ACQ and TRG-OUT
  // 0=DISABLED; 1=ACQ_ONLY; 2=TRGOUT_ONLY; 3=ACQ_AND_TRGOUT
  extTrgMode = ps.get<int>("extTrgMode");

  // sets the software trigger mode via CAEN_DGZT function
  // software trigger is tipically disabled  
  // 0=DISABLED; 1=ACQ_ONLY; 2=TRGOUT_ONLY; 3=ACQ_AND_TRGOUT
  swTrgMode = ps.get<int>("swTrgMode");

  // sets the self trigger mode for all channels in the bitmask
  // tipically disabled in favor of the external trigger 
  // 0:DISABLED 1:ACQ_ONLY 2:TRGOUT_ONLY 3:ACQ_AND_TRGOUT
  selfTrgMode = ps.get<uint32_t>("SelfTriggerMode"); 

  // sets the bit mask to apply the self-trigger mode 
  selfTrgMask = ps.get<uint32_t>("SelfTriggerMask"); 

  // sends a software trigger inside each getData() calls
  // this is used for debugging/test purpose whe sw trigger is on
  // keep false for production running
  swTrigger = ps.get<bool>("SWTrigger", false);

  // allows overlapping triggers (buffers are merged)
  // writes mask 0x0002 to register 0x8004 
  allowTriggerOverlap = ps.get<bool>("allowTriggerOverlap");

  // self trigger polarity bit for register 0x8000 (used by ICARUS)
  // bit[6]=1 (negative, under threshold) or 
  // bit[6]=0 (positive, over threshold)
  selfTrgBit = ps.get<uint32_t>("SelfTrigBit");
  
  // Exclusive for SBND: keep this parameter = 0
  // sets trigger polarity via CAEN_DGTZ library
  // skipped by code for ICARUS
  triggerPolarity = ps.get<int>("triggerPolarity");

  // Exclusive for SBND: keep this parameter = 0
  // majority level for self trigger mode
  // skipped by the code in ICARUS
  majorityLevel = ps.get<int>("majorityLevel"); 

  // Exclusive for SBND: keep this parameter = 0
  // majority coincident window for self trigger mode
  // skipped by the code in ICARUS
  majorityCoincidenceWindow = ps.get<int>("majorityCoincidenceWindow"); 

  // Exclusive for SBND: keep this parameter = 3
  // self trigger logic setting
  // skipped by the code in ICARUS
  triggerLogic = ps.get<int>("triggerLogic");  

  // LVDS output mode flag
  // difference between ICARUS and SBND
  // ICARUS=1, SBND=0
  modeLVDS = ps.get<uint32_t>("ModeLVDS"); 

  // Sets the TRIG-IN control
  // 1=level, 0=edge
  trigInLevel = ps.get<uint32_t>("TrigInLevel", 0);

  // Exclusive for SBND, ICARUS keep to 20
  // setting 0x1n70 but with the same parameter for all
  // skipped in the code for ICARUS
  triggerPulseWidth = ps.get<uint8_t>("triggerPulseWidth");

  // Allow use of TriggerTimeTag (TTT) to form fragment timestamp
  // exploit last full second of NTP time + TTT from board
  useTimeTagForTimeStamp = ps.get<bool>("UseTimeTagForTimeStamp",true);
  
  // Exclusive for SBND
  // different meaning to the timestamp, do no use for ICARUS!
  useTimeTagShiftForTimeStamp = ps.get<bool>("UseTimeTagShiftForTimeStamp",false);

  // adds optional offset to fragment timestamp in ns
  timeOffsetNanoSec = ps.get<uint32_t>("TimeOffsetNanoSec", 0); 

  // If enabled, motherboard clock is sent to TRG-OUT
  // useful for testing CAENV1730 CLOCK synchronization
  outputClk = ps.get<bool>("OutputClk", false); 

  // If enabled, motherboard clock phase is sent to TRG-OUT
  // useful for testing CAENV1730 CLOCK synchronization
  outputClkPhase = ps.get<bool>("OutputClkPhase", false);

  // Max ADC temperature before sending warnings
  // From CAEN manual: V1730(S) shuts itself down at 70(85) Celsius
  maxTemp = ps.get<uint32_t>("maxTempCelsius", 80);

  // Mask saying which ADCs to perform the temperature check on
  // needed to skip channels in one problematic board (S/N 164)
  temperatureCheckMask = ps.get<uint32_t>("temperatureCheckMask", 0xFFFF);

  // Enables optical link interrupts
  // interrupt mode always CAEN_DGTZ_IRQ_MODE_RORA
  // 0=CAEN_DGTZ_DISABLE, 1=CAEN_DGTZ_ENABLE
  interruptEnable = ps.get<uint8_t>("InterruptEnable");

  // Number of events needed to generate interrupt
  interruptEventNumber = ps.get<uint16_t>("InterruptEventNumber",1);

  // Timeout value waiting for an interrupt in ms
  // set via CAEN_DGTZ_IRQWait
  IRQTimeoutMS = ps.get<uint32_t>("IRQTimeoutMS",500);

  char tag[1024];
  channelEnableMask = 0;

  for ( int j=0; j<MAX_CHANNELS; j++){

    // setting the channel enable mask
    // this is both for readout and trigger features
    // probably setting 0x810C appropriately behind the scenes 
    sprintf(tag,"channelEnable%d", j);
    channelEnable[j] = ps.get<bool>(tag);
    if ( channelEnable[j] ) channelEnableMask |= ( 1 << j );

    // absolute ADC threshold values for triggering
    sprintf(tag,"triggerThreshold%d", j);
    triggerThresholds[j] = ps.get<uint16_t>(tag);

    // DC offset values for baseline setting
    // Writing to registers 0x1n98
    sprintf(tag,"channelPedestal%d", j);
	  pedestal[j] = ps.get<int>(tag);

    // LVDS output signal width in clock periods 
    // Channel pulse width registers: 0x1n70
    // e.g: 20 x 8ns = 160ns
    sprintf(tag,"LVDSOutWidthC%d",j+1); //names go from 1 to 16
    LVDSOutWidth[j] = ps.get<uint32_t>(tag);

  }

  for ( int j=0; j<MAX_CHANNELS / 2; j++){

    // Logic values for the LVDS pairs 
    // Self trigger logic registers: 0x1n84   
    // 01=only first in pair, 10=only second in pair, 00=AND, 11=OR
    // bit[2] kept to 0 to allow custom output width
    sprintf(tag,"LVDSLogicValueG%d",j+1); //names go from 1 to 8
    LVDSLogicValue[j] = ps.get<uint32_t>(tag);

  }
}

void sbndaq::CAENConfiguration::print(std::ostream& os) 
{
  os << *this;
}

std::ostream& operator<<(std::ostream& os, const sbndaq::CAENConfiguration& e)
{
  os << "CAEN Configuration: " << std::endl;
  os << "Link                  " << e.link << std::endl;
  os << "FragmentID            " << e.fragmentId << std::endl;
  os << "BoardId               " << e.boardId << std::endl;
  os << "BoardChainNumber      " << e.boardChainNumber << std::endl;
  os << "nChannels             " << e.nChannels << std::endl;
  os << "AX818                 " << e.aX818 << std::endl;
  os << "CalibrateOnConfig     " << e.calibrateOnConfig << std::endl;
  os << "LockTempCalibration   " << e.lockTempCalibration << std::endl;
  os << "WriteCalibration      " << e.writeCalibration << std::endl;
  os << "AcqMode               " << e.acqMode << " " 
                                   << sbndaq::CAENDecoder::AcquisitionMode((CAEN_DGTZ_AcqMode_t)e.acqMode) 
                                   << std::endl;
  os << "RunSyncMode           " << e.runSyncMode << " " 
                                   << sbndaq::CAENDecoder::RunSynchronizationMode((CAEN_DGTZ_RunSyncMode_t)e.runSyncMode) 
                                   << std::endl;
  os << "RecordLength          " << e.recordLength << " samples" << std::endl;
  os << "PostPercent           " << e.postPercent << "%" << std::endl;
  os << "IOLevel (NIM or TTL)  " << e.ioLevel << " "
                                   << sbndaq::CAENDecoder::IOLevel((CAEN_DGTZ_IOLevel_t)e.ioLevel) 
                                   << std::endl;
  os << "DynamicRange          " << e.dynamicRange << std::endl;
  os << "TestPattern           " << e.testPattern << std::endl;
  os << "AnalogMode            " << e.analogMode << " " 
                                   << sbndaq::CAENDecoder::AnalogMonOutput((CAEN_DGTZ_AnalogMonitorOutputMode_t)e.analogMode) 
                                   << std::endl;
  os << "MaxEventsPerTransfer  " << e.maxEventsPerTransfer << std::endl;
  os << "PoolBufferSize        " << e.poolBufferSize << " bytes" << std::endl;
  os << "GetNextSleep          " << e.getNextSleep << " us" << std::endl;
  os << "GetNextFragmentBunchSize " << e.getNextFragmentBunchSize << std::endl;
  os << "ExtTrgMode            " << e.extTrgMode << " " 
                                   << sbndaq::CAENDecoder::TriggerMode((CAEN_DGTZ_TriggerMode_t)e.extTrgMode) 
                                   << std::endl;
  os << "SWTrgMode             " << e.swTrgMode << " " 
                                   << sbndaq::CAENDecoder::TriggerMode((CAEN_DGTZ_TriggerMode_t)e.swTrgMode) 
                                   << std::endl;
  os << "SelfTrgMode           " << e.selfTrgMode << " " 
                                   << sbndaq::CAENDecoder::TriggerMode((CAEN_DGTZ_TriggerMode_t)e.selfTrgMode) 
                                   << std::endl;                                
  os << "SelfTrgMask         0x" << std::hex << e.selfTrgMask << std::dec << " " << e.selfTrgMask << std::endl;
  os << "SWTrigger             " << e.swTrigger << std::endl;
  os << "AllowTriggerOverlap   " << e.allowTriggerOverlap << std::endl;
  os << "selfTrgBit          0x" << std::hex << e.selfTrgBit << std::dec << " " << e.selfTrgBit << std::endl;
  os << "TriggerPolarity       " << e.triggerPolarity << " " 
                                   << sbndaq::CAENDecoder::TriggerPolarity((CAEN_DGTZ_TriggerPolarity_t)e.triggerPolarity) 
                                   << std::endl;
  os << "MajorityLevel         " << e.majorityLevel << std::endl;
  os << "MajorityCoincidenceWindow " << e.majorityCoincidenceWindow << std::endl;
  os << "TriggerLogic          " << e.triggerLogic << std::endl;
  os << "ModeLVDS              " << e.modeLVDS << std::endl;
  os << "TrigInLevel           " << e.trigInLevel << std::endl;
  os << "TriggerPulseWidth     " << e.triggerPulseWidth << std::endl;
  os << "UseTimeTagForTimeStamp " << e.useTimeTagForTimeStamp << std::endl;
  os << "useTimeTagShiftForTimeStamp " << e.useTimeTagShiftForTimeStamp << std::endl;
  os << "TimeOffsetNanoSec     " << e.timeOffsetNanoSec << " ns" << std::endl;
  os << "OutputClk             " << e.outputClk << std::endl;
  os << "OutputClkPhase        " << e.outputClkPhase << std::endl;
  os << "MaxTempCelsius        " << e.maxTemp << "C" << std::endl;
  os << "TemperatureCheckMask 0x" << std::hex << e.temperatureCheckMask << std::dec << std::endl;
  os << "InterruptEnable       " << e.interruptEnable << " " 
                                 << sbndaq::CAENDecoder::EnaDisMode((CAEN_DGTZ_EnaDis_t)e.interruptEnable)
                                 << std::endl;
  os << "InterruptEventNumber  " << e.interruptEventNumber << std::endl;
  os << "IRQTimeoutMS          " << e.IRQTimeoutMS << " ms" << std::endl;
  os << "ChannelEnableMask   0x" << std::hex << e.channelEnableMask << std::dec << std::endl;

  for ( int j=0; j<sbndaq::CAENConfiguration::MAX_CHANNELS; j++)
  {
    os << "Channel " << j << " EnableReadout " << e.channelEnable[j] 
                          << " Threshold " << e.triggerThresholds[j] 
                          << " Pedestal " << e.pedestal[j] 
                          << " LVDSOutWidth " << e.LVDSOutWidth[j] 
                          << std::endl;
  }

  for ( int j=0; j<sbndaq::CAENConfiguration::MAX_CHANNELS / 2; j++)
  {
    os << "ChannelPair " << j << " LVDSLogicValue " << e.LVDSLogicValue[j] << std::endl;
  }

  return(os);
}