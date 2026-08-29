//
//  sbndaq-artdaq/Generators/Common/CAENV1730Readout_generator.cc
//

#define TRACE_NAME "CAENV1730Readout"

#include "artdaq/DAQdata/Globals.hh"
#include "artdaq/Generators/GeneratorMacros.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "sbndaq-artdaq/Generators/Common/CAENV1730Readout.hh"
#include "sbndaq-artdaq/Generators/Common/CAENDecoder.hh"

#include <iostream>
#include <sstream>
#include <time.h>
#include <unistd.h>
#include <algorithm>

#include "boost/date_time/microsec_time_clock.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

using namespace sbndaq;

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

// Constructor of the CAENV1730Readout.
// All configuration parameters are loaded and stored in CAENConfiguration.
// Connection is opened, board is reset and Configure() is called.
sbndaq::CAENV1730Readout::CAENV1730Readout(fhicl::ParameterSet const& ps) :
  CommandableFragmentGenerator(ps),
  fCAEN(ps)
{
  TLOG_ARB(TCONFIG,TRACE_NAME) << "CAENV1730Readout()" << TLOG_ENDL;

  // print-out all configuration parameters
  TLOG(TINFO) << fCAEN.to_string();
  
  CAEN_DGTZ_ErrorCode retcode;

  fail_GetNext=false;
  fNChannels = fCAEN.nChannels;
  fNumBoardBuffers=0;

  TLOG(TCONFIG) << ": Using FragID=" << fCAEN.fragmentId 
    << " BoardID=" << fCAEN.boardId 
    << " with NChannels=" << fNChannels;

  // opening the connection to the digitizer
  retcode = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_OpticalLink, fCAEN.link, 
				    fCAEN.boardChainNumber, 0, &fHandle);

  if(retcode != CAEN_DGTZ_Success)
  {
    sbndaq::CAENDecoder::checkError(retcode,"OpenDigitizer",fCAEN.fragmentId);
		CAEN_DGTZ_CloseDigitizer(fHandle);
    fHandle = -1;
    TLOG(TLVL_ERROR) << "(fragID=" << fCAEN.fragmentId 
      << ") Fatal error configuring CAEN board at link=" << fCAEN.link
      << ", boardChainNumber=" << fCAEN.boardChainNumber 
      << ". Terminating process...";
    abort();
  }
 
  // prints current firmware/software versions
  // prints VME bridge firmware and driver 
  GetSWInfo();
  
  // software reset signal: clears all registers and buffers
  // should clear any running or busy state
  retcode = CAEN_DGTZ_Reset(fHandle);
  sbndaq::CAENDecoder::checkError(retcode,"Reset",fCAEN.fragmentId);
  
  // initiate board configuration
  sleep(1);
  Configure();

  // after configuration is over, read and print registers
  // this is a summary snapshot of the board config
  uint32_t data;

  // board configuration register
  retcode = CAEN_DGTZ_ReadRegister(fHandle,BOARD_CONFIG_READ,&data);
  TLOG(TLVL_INFO) << "Board configuration (BOARD_CONFIG_ADDR=0x" 
	          << std::hex << static_cast<unsigned int>(BOARD_CONFIG_READ) << ") = 0x" << data;
  
  // front panel TRG-OUT control register
  retcode = CAEN_DGTZ_ReadRegister(fHandle,FP_TRG_OUT_CONTROL,&data);
  TLOG(TLVL_INFO) << "Front panel TRG-OUT control (FP_TRG_OUT_CONTROL=0x" 
                  << std::hex << static_cast<unsigned int>(FP_TRG_OUT_CONTROL) << ") = 0x" << data;

  // front panel I/O control register
  retcode = CAEN_DGTZ_ReadRegister(fHandle,FP_IO_CONTROL,&data);
  TLOG(TLVL_INFO) << "Front panel I/O control (FP_IO_CONTROL=0x" 
	          << std::hex << static_cast<unsigned int>(FP_IO_CONTROL) << ") = 0x" << data;

  // front panel LVDS I/O control register
  retcode = CAEN_DGTZ_ReadRegister(fHandle,FP_LVDS_CONTROL,&data);
  TLOG(TLVL_INFO) << "Front panel LVDS I/O control (FP_LVDS_CONTROL=0x" 
	          << std::hex << static_cast<unsigned int>(FP_LVDS_CONTROL) << ") = 0x" << data;

  // acquisition control register
  retcode = CAEN_DGTZ_ReadRegister(fHandle,ACQ_CONTROL,&data);
  TLOG(TLVL_INFO) << "Acquisition control (ACQ_CONTROL=0x" 
                  << std::hex << static_cast<unsigned int>(ACQ_CONTROL) << ") = 0x" << data;

  // readout control register (interrupts)
  retcode = CAEN_DGTZ_ReadRegister(fHandle,READOUT_CONTROL,&data);
  TLOG(TLVL_INFO) << "Readout interrupts control  (READOUT_CONTROL=0x" 
                  << std::hex << static_cast<unsigned int>(READOUT_CONTROL) << ") = 0x" << data;
  
  // global trigger mask
  retcode = CAEN_DGTZ_ReadRegister(fHandle,GLB_TRG_MASK,&data);
  TLOG(TLVL_INFO) << "Global trigger mask (GLB_TRG_MASK=0x" 
	  	  << std::hex << static_cast<unsigned int>(GLB_TRG_MASK) << ") = 0x" << data;

  // channel enable mask
  retcode = CAEN_DGTZ_ReadRegister(fHandle,CH_ENABLE_MASK,&data);
  TLOG(TLVL_INFO) << "Channel enable mask (CH_ENABLE_MASK=0x" 
                  << std::hex << static_cast<unsigned int>(CH_ENABLE_MASK) << ") = 0x" << data
                  << std::dec;

  // Set up worker getdata thread.
  share::ThreadFunctor functor = std::bind(&CAENV1730Readout::GetData,this);
  auto worker_functor = share::WorkerThreadFunctorUPtr(new share::WorkerThreadFunctor(functor,"GetDataWorkerThread"));
  auto GetData_worker = share::WorkerThread::createWorkerThread(worker_functor);
  GetData_thread_.swap(GetData_worker);
  TLOG_ARB(TCONFIG,TRACE_NAME) << "GetData worker thread setup." << TLOG_ENDL;

  TLOG(TLVL_INFO) << "Configuration complete!" << TLOG_ENDL;

  //epoch time
  fTimeEpoch = boost::posix_time::ptime(boost::gregorian::date(1970,1,1));
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

sbndaq::CAENV1730Readout::~CAENV1730Readout()
{
  TLOG_ARB(TCONFIG,TRACE_NAME) << "~CAENV1730Readout()" << TLOG_ENDL;

  if(fBuffer != NULL){
    fBuffer.reset();
  }

  TLOG_ARB(TCONFIG,TRACE_NAME) << "~CAENV1730Readout() done." << TLOG_ENDL;
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

void sbndaq::CAENV1730Readout::Configure()
{
  TLOG_ARB(TCONFIG,TRACE_NAME) << "Configure()" << TLOG_ENDL;

  CAEN_DGTZ_ErrorCode retcode;
  uint32_t readback;

  // Make sure DAQ run is off first
  // This should be garantueed by the previous reset, but still
  // Set software control acquistion, then stop
  TLOG_ARB(TCONFIG,TRACE_NAME) << "Set Acquisition Mode to SW" << TLOG_ENDL;
  retcode = CAEN_DGTZ_SetAcquisitionMode(fHandle,CAEN_DGTZ_SW_CONTROLLED);
  sbndaq::CAENDecoder::checkError(retcode,"SetAcquisitionMode",fCAEN.fragmentId);

  retcode = CAEN_DGTZ_GetAcquisitionMode(fHandle,(CAEN_DGTZ_AcqMode_t *)&readback);
  CheckReadback("SetAcquisitionMode",fCAEN.fragmentId,(uint32_t)CAEN_DGTZ_SW_CONTROLLED ,readback);

  TLOG_ARB(TCONFIG,TRACE_NAME) << "Stop Acquisition" << TLOG_ENDL;
  retcode = CAEN_DGTZ_SWStopAcquisition(fHandle);
  sbndaq::CAENDecoder::checkError(retcode,"SWStopAcquisition",fCAEN.fragmentId);

  // reset again to clear acquisition mode
  retcode = CAEN_DGTZ_Reset(fHandle);
  sleep(2);

  // Configuration is carried out in different functions
  // - CAENDecoder::checkError is applied every time: if writing fails, exception is thrown
  // - CheckReadback() is called when possible 
  ConfigureReadout();
  ConfigureRecordFormat();
  ConfigureTrigger();  
  ConfigureAcquisition();
  ConfigureInterrupts();

  // if requested, run ADC self-calibration
  if (fCAEN.calibrateOnConfig){ 
    RunADCCalibration();  
  }

  // if requested, lock temperature calibration
  // avoids recalibrating mid-run and causing baseline jumps
  if (fCAEN.lockTempCalibration )  
  { 
    TLOG_ARB(TINFO,TRACE_NAME) << "Locking mid-run temperature calibration adjustements..." << TLOG_ENDL;
    for ( uint32_t ch=0; ch<fNChannels; ch++)
    {
      SetLockTempCalibration(true,ch);
    }
  }

  TLOG_ARB(TCONFIG,TRACE_NAME) << "Configure() done." << TLOG_ENDL;
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

void sbndaq::CAENV1730Readout::ConfigureReadout()
{
  TLOG_ARB(TCONFIG,TRACE_NAME) << "ConfigureReadout()" << TLOG_ENDL;

  CAEN_DGTZ_ErrorCode retcode;
  uint32_t readback;
  uint32_t addr,mask;
  
  // sets run synchronization mode
  TLOG_ARB(TCONFIG,TRACE_NAME) << "SetRunSyncMode " << (CAEN_DGTZ_RunSyncMode_t)(fCAEN.runSyncMode) << TLOG_ENDL;
  retcode = CAEN_DGTZ_SetRunSynchronizationMode(fHandle,(CAEN_DGTZ_RunSyncMode_t)(fCAEN.runSyncMode));
  sbndaq::CAENDecoder::checkError(retcode,"SetRunSynchronizationMode",fCAEN.fragmentId);
  retcode = CAEN_DGTZ_GetRunSynchronizationMode(fHandle,(CAEN_DGTZ_RunSyncMode_t*)&readback);
  CheckReadback("SetRunSynchronizationMode",fCAEN.fragmentId,fCAEN.runSyncMode,readback);
  
 // test pattern Enable (default value is 0).
 // this bit enables a triangular (0<– >3FFF) test wave 
 // to be provided at the ADCs input for debug purposes
  mask = ( 1 << TEST_PATTERN_t::TEST_PATTERN_S );
  addr = (fCAEN.testPattern)
    ? BOARD_CONFIG_SET    // writing a 1 to a bit sets that bit
    : BOARD_CONFIG_CLEAR; // writing a 1 to a bit clears that bit
  TLOG_ARB(TCONFIG,TRACE_NAME) << "SetTestPattern addr=" << addr << ", mask=" << mask << TLOG_ENDL;
  retcode = CAEN_DGTZ_WriteRegister(fHandle,addr,mask);
  sbndaq::CAENDecoder::checkError(retcode,"SetTestPattern",fCAEN.fragmentId);

  // sets dynamic range control
  TLOG_ARB(TCONFIG,TRACE_NAME) << "SetDynamicRange " << fCAEN.dynamicRange << TLOG_ENDL;
  mask = (uint32_t)(fCAEN.dynamicRange);
  retcode = CAEN_DGTZ_WriteRegister(fHandle,DYNAMIC_RANGE,mask);
  sbndaq::CAENDecoder::checkError(retcode,"SetDynamicRange",fCAEN.fragmentId);

  // sets bits in the acquisition control register
  // later call to SetAcquisitionMode can override mode
  // 0x28 --> bit[3] = 1, bit[5] = 1 else if default
  // bit[3]=trigger counting mode selection; 1=all triggers are counted
  // bit[5]=memory full mode selection; 1=one buffer free (full if N-1 are full)
  TLOG_ARB(TCONFIG,TRACE_NAME) << "SetTriggerMode (addr=" << std::hex << ACQ_CONTROL << ")" << std::hex << uint32_t{0x28} << std::dec; 
  retcode = CAEN_DGTZ_WriteRegister(fHandle,ACQ_CONTROL,uint32_t{0x28});
  sbndaq::CAENDecoder::checkError(retcode,"SetTriggerMode",fCAEN.fragmentId);
  retcode = CAEN_DGTZ_ReadRegister(fHandle,ACQ_CONTROL,&readback);
  sbndaq::CAENDecoder::checkError(retcode,"GetTriggerMode",fCAEN.fragmentId);
  CheckReadback("SetTriggerMode",fCAEN.fragmentId,uint32_t{0x28},readback);

  // sets channel DC offeset/pedestal
  for(uint32_t ch=0; ch<fNChannels; ++ch){
    TLOG_ARB(TCONFIG,TRACE_NAME) << "Set channel " << ch << " DC offset to " << fCAEN.pedestal[ch] << TLOG_ENDL;
    retcode = CAEN_DGTZ_SetChannelDCOffset(fHandle,ch,fCAEN.pedestal[ch]);
    sbndaq::CAENDecoder::checkError(retcode,"SetChannelDCOffset",fCAEN.fragmentId);
    retcode = CAEN_DGTZ_GetChannelDCOffset(fHandle,ch,&readback);
    CheckReadback("SetChannelDCOffset",fCAEN.fragmentId,fCAEN.pedestal[ch],readback,ch);
  }

  TLOG_ARB(TCONFIG,TRACE_NAME) << "ConfigureReadout() done." << TLOG_ENDL;
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

void sbndaq::CAENV1730Readout::ConfigureRecordFormat()
{
  TLOG_ARB(TCONFIG,TRACE_NAME) << "ConfigureRecordFormat()" << TLOG_ENDL;

  CAEN_DGTZ_ErrorCode retcode;
  uint32_t readback;

  // channel masks for readout(?)
  TLOG_ARB(TCONFIG,TRACE_NAME) << "SetChannelEnableMask " << fCAEN.channelEnableMask << TLOG_ENDL;
  retcode = CAEN_DGTZ_SetChannelEnableMask(fHandle,fCAEN.channelEnableMask);
  sbndaq::CAENDecoder::checkError(retcode,"SetChannelEnableMask",fCAEN.fragmentId);
  retcode = CAEN_DGTZ_GetChannelEnableMask(fHandle,&readback);
  sbndaq::CAENDecoder::checkError(retcode,"GetChannelEnableMask",fCAEN.fragmentId);
  CheckReadback("CHANNEL_ENABLE_MASK", fCAEN.fragmentId, fCAEN.channelEnableMask, readback);

  // record length (number of sample)
  TLOG_ARB(TCONFIG,TRACE_NAME) << "SetRecordLength " << fCAEN.recordLength << TLOG_ENDL;
  retcode = CAEN_DGTZ_SetRecordLength(fHandle,fCAEN.recordLength);
  sbndaq::CAENDecoder::checkError(retcode,"SetRecordLength",fCAEN.fragmentId);
  retcode = CAEN_DGTZ_GetRecordLength(fHandle,&readback);
  sbndaq::CAENDecoder::checkError(retcode,"GetRecordLength",fCAEN.fragmentId);
  CheckReadback("RECORD_LENGTH", fCAEN.fragmentId, fCAEN.recordLength, readback);

  // post trigger size
  TLOG_ARB(TCONFIG,TRACE_NAME) << "SetPostTriggerSize " << (unsigned int)(fCAEN.postPercent) << TLOG_ENDL;
  retcode = CAEN_DGTZ_SetPostTriggerSize(fHandle,(unsigned int)(fCAEN.postPercent));
  sbndaq::CAENDecoder::checkError(retcode,"SetPostTriggerSize",fCAEN.fragmentId);
  retcode = CAEN_DGTZ_GetPostTriggerSize(fHandle,&readback);
  sbndaq::CAENDecoder::checkError(retcode,"GetPostTriggerSize",fCAEN.fragmentId);
  CheckReadback("POST_TRIGGER_SIZE", fCAEN.fragmentId, fCAEN.postPercent, readback);

  TLOG_ARB(TCONFIG,TRACE_NAME) << "ConfigureRecordFormat() done." << TLOG_ENDL;
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

void sbndaq::CAENV1730Readout::ConfigureTrigger()
{
  TLOG_ARB(TCONFIG,TRACE_NAME) << "ConfigureTrigger()" << TLOG_ENDL;

  CAEN_DGTZ_ErrorCode retcode;
  uint32_t readback;
  uint32_t addr;

  // set the software trigger mode
  TLOG_ARB(TCONFIG,TRACE_NAME) << "SetSWTriggerMode" << fCAEN.swTrgMode << TLOG_ENDL;
  retcode = CAEN_DGTZ_SetSWTriggerMode(fHandle,(CAEN_DGTZ_TriggerMode_t)(fCAEN.swTrgMode));
  sbndaq::CAENDecoder::checkError(retcode,"SetSWTriggerMode",fCAEN.fragmentId);
  retcode = CAEN_DGTZ_GetSWTriggerMode(fHandle,(CAEN_DGTZ_TriggerMode_t *)&readback);
  CheckReadback("SetSWTriggerMode", fCAEN.fragmentId,fCAEN.swTrgMode,readback);

  // set the external trigger mode
  TLOG_ARB(TCONFIG,TRACE_NAME) << "SetExtTriggerMode" << fCAEN.extTrgMode << TLOG_ENDL;
  retcode = CAEN_DGTZ_SetExtTriggerInputMode(fHandle,(CAEN_DGTZ_TriggerMode_t)(fCAEN.extTrgMode));
  sbndaq::CAENDecoder::checkError(retcode,"SetExtTriggerInputMode",fCAEN.fragmentId);
  retcode = CAEN_DGTZ_GetExtTriggerInputMode(fHandle,(CAEN_DGTZ_TriggerMode_t *)&readback);
  CheckReadback("SetExtTriggerInputMode",fCAEN.fragmentId,fCAEN.extTrgMode,readback);

  for(uint32_t ch=0; ch<fNChannels; ++ch)
  {
    TLOG_ARB(TCONFIG,TRACE_NAME) << "Set channel " << ch 
          << " trigger threshold to " << fCAEN.triggerThresholds[ch] << TLOG_ENDL;
    retcode = CAEN_DGTZ_SetChannelTriggerThreshold(fHandle,ch,fCAEN.triggerThresholds[ch]);
    sbndaq::CAENDecoder::checkError(retcode,"SetChannelTriggerThreshold",fCAEN.fragmentId);
    retcode = CAEN_DGTZ_GetChannelTriggerThreshold(fHandle,ch,&readback);
    CheckReadback("SetChannelTriggerThreshold",fCAEN.fragmentId,fCAEN.triggerThresholds[ch],readback,ch);

    // GVS: the following configuration parameters are for SBND.
    // fCAEN.modeLVDS must be 0
    if(fCAEN.modeLVDS==0)
    {
      TLOG_ARB(TCONFIG,TRACE_NAME) << "Set Trigger Polarity " << fCAEN.triggerPolarity << " to channel: " << ch << TLOG_ENDL;
      retcode = CAEN_DGTZ_SetTriggerPolarity(fHandle, ch,(CAEN_DGTZ_TriggerPolarity_t)(fCAEN.triggerPolarity));
      sbndaq::CAENDecoder::checkError(retcode,"SetTriggerPolarity",fCAEN.fragmentId);
      retcode = CAEN_DGTZ_GetTriggerPolarity(fHandle, ch,(CAEN_DGTZ_TriggerPolarity_t *)&readback);
      CheckReadback("SetTriggerPolarity",fCAEN.fragmentId,fCAEN.triggerPolarity,readback, ch);

      // GVS: pulse width must be set per channel, not per pair of channel. 
      // This contradicts what manual says!
      TLOG_ARB(TCONFIG,TRACE_NAME) << "Set channels " << ch << " trigger pulse width to " << fCAEN.triggerPulseWidth << TLOG_ENDL;
      retcode = CAEN_DGTZ_WriteRegister(fHandle,TRG_OUT_WIDTH_CH+(ch<<8),fCAEN.triggerPulseWidth);
      sbndaq::CAENDecoder::checkError(retcode,"SetChannelTriggerPulseWidth",fCAEN.fragmentId);
      retcode = CAEN_DGTZ_ReadRegister(fHandle,TRG_OUT_WIDTH_CH+(ch<<8),&readback);
      CheckReadback("SetChannelTriggerPulseWidth",fCAEN.fragmentId,fCAEN.triggerPulseWidth,readback, ch);
    }
  }

  // LVDS configuration for ICARUS
  // LVDS mode must be > 0
  if(fCAEN.modeLVDS!=0)
  { 
    ConfigureLVDS();
  }

  // for clock synchronization studies
  if( fCAEN.outputClk || fCAEN.outputClkPhase )
  { 
    ConfigureClkToTrgOut(); 
  } 

  // Self trigger configuration
  // also LVDS config for SBND
  ConfigureSelfTriggerMode();
 
  // enable the trigger overlap (merging of trigger events)
  TLOG_ARB(TCONFIG,TRACE_NAME) << "SetTriggerOverlap" << fCAEN.allowTriggerOverlap << TLOG_ENDL;
  addr = (fCAEN.allowTriggerOverlap)
    ? BOARD_CONFIG_SET    // writing a 1 to a bit sets that bit
    : BOARD_CONFIG_CLEAR; // writing a 1 to a bit clears that bit
  retcode = CAEN_DGTZ_WriteRegister(fHandle, addr, TRIGGER_OVERLAP_MASK);
  sbndaq::CAENDecoder::checkError(retcode,"SetTriggerOverlap",fCAEN.fragmentId);

  // I/O level: TTL=1, NIM=0
  TLOG_ARB(TCONFIG,TRACE_NAME) << "SetIOLevel " << (CAEN_DGTZ_IOLevel_t)(fCAEN.ioLevel) << TLOG_ENDL;
  retcode = CAEN_DGTZ_SetIOLevel(fHandle,(CAEN_DGTZ_IOLevel_t)(fCAEN.ioLevel));
  sbndaq::CAENDecoder::checkError(retcode,"SetIOLevel",fCAEN.fragmentId);
  retcode = CAEN_DGTZ_GetIOLevel(fHandle,(CAEN_DGTZ_IOLevel_t *)&readback);
  CheckReadback("SetIOLevel",fCAEN.fragmentId,fCAEN.ioLevel,readback);

}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

void sbndaq::CAENV1730Readout::ConfigureLVDS()
{
  CAEN_DGTZ_ErrorCode retcod = CAEN_DGTZ_Success;
  uint32_t data,readBack,ioMode;

  // Construct mode mask: repeat chosen modeLVDS 4 times
  // FP_LVDS_CONTROL groups multiple LVDS pins into separate 4-bit fields
  data = fCAEN.modeLVDS | (fCAEN.modeLVDS << 4) | (fCAEN.modeLVDS << 8) | (fCAEN.modeLVDS << 12);
  TLOG(TCONFIG) << "ModeLVDS: 0x" << std::hex << data << std::dec;
  
  retcod = CAEN_DGTZ_WriteRegister(fHandle, FP_LVDS_CONTROL, data);
  sbndaq::CAENDecoder::checkError(retcod,"WriteLVDSOutputConfig",fCAEN.fragmentId);
  retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_LVDS_CONTROL, &readBack);
  sbndaq::CAENDecoder::checkError(retcod,"ReadLVDSOutputConfig",fCAEN.fragmentId);
  CheckReadback("LVDSOutputConfig", fCAEN.fragmentId, data, readBack);

  // reads the FP_IO_CONTROL register (front-panel I/O configuration) into ioMode
  // this is the base configuration -- it will be modified and written back
  retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_IO_CONTROL, &ioMode);
  sbndaq::CAENDecoder::checkError(retcod,"ReadFPOutputConfig",fCAEN.fragmentId);

  // If TRIGGER mode, send them out TRG-OUT NIM
  if ( fCAEN.modeLVDS == LVDS_TRIGGER )
  {
    // Put LVDS into OUTPUT mode and send to TRG-OUT
    ioMode |= (LVDS_IO | ENABLE_NEW_LVDS);
    ioMode &= ~DISABLE_TRG_OUT_LEMO ;
  }
  else
  {
    // Put LVDS into INPUT mode
    ioMode &= ~(LVDS_IO | DISABLE_TRG_OUT_LEMO);
  }

  // sets TRG-IN to level or edge
  if ( fCAEN.trigInLevel )
  {
    ioMode |= TRG_IN_LEVEL;
  }
  else
  {
    ioMode &= ~(TRG_IN_LEVEL);
  }

  // finally write back to FP_IO_CONTROL the update value
  // this should be the final configuration for this register
  TLOG(TCONFIG) << "FPOutputConfig: 0x" << std::hex << ioMode << std::dec;
  retcod = CAEN_DGTZ_WriteRegister(fHandle, FP_IO_CONTROL, ioMode);
  sbndaq::CAENDecoder::checkError(retcod,"WriteFPOutputConfig",fCAEN.fragmentId);
  retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_IO_CONTROL, &readBack);
  sbndaq::CAENDecoder::checkError(retcod,"ReadFPOutputConfig",fCAEN.fragmentId);
  CheckReadback("FPOutputConfig", fCAEN.fragmentId, ioMode, readBack);

  // set/read registers for LVDS logic values setting
  // values are set in pairs (8 groups)
  for (size_t gr = 0; gr < 8; ++gr) 
  {
    // 0x1n84 with n = 0,2,4,6,..,E
    uint32_t regAddr = SLF_TRG_LG_CH + (gr * 0x200); 
    TLOG_ARB(TCONFIG,TRACE_NAME) << "LVDS logic value for G" << gr+1 
      << " (0x" << std::hex << regAddr << ") : "
      << std::dec << fCAEN.LVDSLogicValue[gr] << TLOG_ENDL;

    retcod = CAEN_DGTZ_WriteRegister(fHandle, regAddr, fCAEN.LVDSLogicValue[gr]);
    sbndaq::CAENDecoder::checkError(retcod, "WriteLVDSLogicValue", fCAEN.fragmentId);
    retcod = CAEN_DGTZ_ReadRegister(fHandle, regAddr, &readBack);
    sbndaq::CAENDecoder::checkError(retcod,"ReadLVDSLogicValue", fCAEN.fragmentId);
    CheckReadback("LVDSLogicValue", fCAEN.fragmentId, fCAEN.LVDSLogicValue[gr], readBack, gr);
  }

  // set/read registers for LVDS output width values setting
  // values are set per channel
  for (size_t ch = 0; ch < fNChannels; ++ch) 
  {
    // 0x1n70 with n = 0,1,2,3,..,F
    uint32_t regAddr = TRG_OUT_WIDTH_CH + (ch * 0x100); 
    TLOG_ARB(TCONFIG,TRACE_NAME) << "LVDS logic value for Ch" << ch 
      << " (0x" << std::hex << regAddr << ") : " 
      << std::dec << fCAEN.LVDSOutWidth[ch] << TLOG_ENDL;

    retcod = CAEN_DGTZ_WriteRegister(fHandle, regAddr, fCAEN.LVDSOutWidth[ch]);
    sbndaq::CAENDecoder::checkError(retcod, "WriteLVDSOutWidth", fCAEN.fragmentId);
    retcod = CAEN_DGTZ_ReadRegister(fHandle, regAddr, &readBack);
    sbndaq::CAENDecoder::checkError(retcod,"ReadLVDSOutWidth", fCAEN.fragmentId);
    CheckReadback("LVDSOutWidth", fCAEN.fragmentId, fCAEN.LVDSOutWidth[ch], readBack, ch);
  }

  // set self-trigger polarity
  TLOG_ARB(TCONFIG,TRACE_NAME) << "SetSelfTrigBit" << fCAEN.selfTrgBit << TLOG_ENDL;
  uint32_t addr = (fCAEN.selfTrgBit)
    ? BOARD_CONFIG_SET    // writing a 1 to a bit sets that bit
    : BOARD_CONFIG_CLEAR; // writing a 1 to a bit clears that bit
  retcod = CAEN_DGTZ_WriteRegister(fHandle, addr, SLF_TRG_BIT_MASK);
  sbndaq::CAENDecoder::checkError(retcod, "WriteSelfTrigBit", fCAEN.fragmentId);
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

void sbndaq::CAENV1730Readout::ConfigureClkToTrgOut()
{
  /* Check to output ONLY CLK OR CLK PHASE */
  if ( fCAEN.outputClk && fCAEN.outputClkPhase ){
    TLOG(TLVL_ERROR) << "(FragID=" << fCAEN.fragmentId << ")"
                     << " Error configuring output clock: Cannot output clock and its phase at the same time." << TLOG_ENDL;
    abort();
  } 

  CAEN_DGTZ_ErrorCode retcod = CAEN_DGTZ_Success;
  uint32_t data;

  /* Check the output of the 0x811C */
  retcod = CAEN_DGTZ_ReadRegister(fHandle,FP_IO_CONTROL, &data);
  sbndaq::CAENDecoder::checkError(retcod,"ClkToTrgOutCheckError",fCAEN.fragmentId);

  uint32_t value16 = 0x1; 
  uint32_t value18 = 0x0;
  if (fCAEN.outputClk) value18 = 0x1;
  if (fCAEN.outputClkPhase) value18 = 0x2;
  data |= ((value16 & 0x3)<<16) + ((value18 &0x3)<<18);

  retcod = CAEN_DGTZ_WriteRegister(fHandle, FP_IO_CONTROL, data);
  sbndaq::CAENDecoder::checkError(retcod,"ClkToTrgOutCheckError",fCAEN.fragmentId);

  TLOG_ARB(TCONFIG,TRACE_NAME) << "Front Panel IO Control address 0x811C, new value: 0x" << std::hex << data << std::dec;
  TLOG(TINFO) << "Front Panel IO Control address 0x811C, new value: 0x" << std::hex << data << std::dec;
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

// GVS: new ConfigureSelfTriggerMode() function
void sbndaq::CAENV1730Readout::ConfigureSelfTriggerMode()
{
  CAEN_DGTZ_ErrorCode retcod = CAEN_DGTZ_Success;

  // sets the channel self-trigger mode
  retcod = CAEN_DGTZ_SetChannelSelfTrigger(fHandle,
					   (CAEN_DGTZ_TriggerMode_t)fCAEN.selfTrgMode,
					   fCAEN.selfTrgMask);
  sbndaq::CAENDecoder::checkError(retcod,"SetChannelSelfTriggerMode",fCAEN.fragmentId);
  
  // GVS: the following configuration parameters are for SBND. 
  // fCAEN.modeLVDS must be 0
  if(fCAEN.modeLVDS==0)
  { 
    uint32_t data, data2, bitpair, readBack, aux, aux2;
  
    // GVS: verify the SelfTrigger values set in each channel.
    for(uint32_t chn=0; chn<fNChannels; ++chn)
    {
      retcod = CAEN_DGTZ_GetChannelSelfTrigger(fHandle, chn, (CAEN_DGTZ_TriggerMode_t *)&readBack);
      sbndaq::CAENDecoder::checkError(retcod,"GetChannelSelfTriggerMode",fCAEN.fragmentId);
      CheckReadback("ChannelSelfTriggerMode", fCAEN.fragmentId, fCAEN.selfTrgMode, readBack, chn);

      // GVS: inserted triggerLogic for each PAIR of channels.
      if(chn%2==0)
      {
        retcod = CAEN_DGTZ_ReadRegister(fHandle,SLF_TRG_LG_CH+(chn<<8),&aux);
        TLOG_ARB(TCONFIG,TRACE_NAME) << "Self-trigger logic to channel " << chn << " old value " << std::hex << aux << std::dec;

        TLOG_ARB(TCONFIG,TRACE_NAME) << "Set channels " << chn << "/" << chn+1 << " self trigger logic to " << fCAEN.triggerLogic
				/* << " self trigger pulse type to " << fCAEN.ovthValue*/ << TLOG_ENDL;
        retcod = CAEN_DGTZ_WriteRegister(fHandle,SLF_TRG_LG_CH+(chn<<8),
                 (fCAEN.triggerLogic & 0x3)/* + ((fCAEN.ovthValue & 0x1) <<2)*/);

        sbndaq::CAENDecoder::checkError(retcod,"SelfTriggerPulseType",fCAEN.fragmentId);
        retcod = CAEN_DGTZ_ReadRegister(fHandle,SLF_TRG_LG_CH+(chn<<8),&aux2);
      
        TLOG_ARB(TCONFIG,TRACE_NAME) << "Self-trigger logic to channel " << chn << " new value " << std::hex << aux2 << std::dec;
        CheckReadback("SelfTriggerPulseType", fCAEN.fragmentId, aux2, (fCAEN.triggerLogic & 0x3) /*+ ((fCAEN.ovthValue & 0x1) <<2)*/,chn);
      }
    }
  
    // GVS: read TRG_OUT register to monitor enabled/disabled pair of channels.
    retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_TRG_OUT_CONTROL, &bitpair);
    TLOG_ARB(TCONFIG,TRACE_NAME) << "Front Panel TRG-OUT address 0x8110, value: 0x" << std::hex << bitpair << std::dec;
				    
    /* Set Majority Mode and Majority Coincidence Window */
    retcod = CAEN_DGTZ_ReadRegister(fHandle, GLB_TRG_MASK, &data);
    TLOG_ARB(TCONFIG,TRACE_NAME) << "Global Trigger Mask address 0x810C, old value: 0x" << std::hex << data << std::dec;
  
    TLOG_ARB(TCONFIG,TRACE_NAME)  << " Set Majority Level to " << fCAEN.majorityLevel << TLOG_ENDL;
    TLOG_ARB(TCONFIG,TRACE_NAME)  << " Set Maj Coincidence Window to " << fCAEN.majorityCoincidenceWindow << TLOG_ENDL;
				   
    data |= ((fCAEN.majorityLevel & 0x7)<<24) + ((fCAEN.majorityCoincidenceWindow & 0xF) <<20);
				   
    retcod = CAEN_DGTZ_WriteRegister(fHandle,GLB_TRG_MASK, data);

    sbndaq::CAENDecoder::checkError(retcod,"SetMajCoincWindow",fCAEN.fragmentId);
    retcod = CAEN_DGTZ_ReadRegister(fHandle,GLB_TRG_MASK,&data2);
      
    TLOG_ARB(TCONFIG,TRACE_NAME) << "Global Trigger Mask address 0x810C, new value: 0x" << std::hex << data2 << std::dec;
    CheckReadback("SetMajCoincWindow", fCAEN.fragmentId, data, data2);
  }
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

void sbndaq::CAENV1730Readout::ConfigureAcquisition()
{
  TLOG_ARB(TCONFIG,TRACE_NAME) << "ConfigureAcquisition()" << TLOG_ENDL;

  CAEN_DGTZ_ErrorCode retcode;
  uint32_t readback;

  // sets the acquisition mode
  TLOG_ARB(TCONFIG,TRACE_NAME) << "SetAcqMode " << (CAEN_DGTZ_AcqMode_t)(fCAEN.acqMode) << TLOG_ENDL;
  retcode = CAEN_DGTZ_SetAcquisitionMode(fHandle,(CAEN_DGTZ_AcqMode_t)(fCAEN.acqMode));
  sbndaq::CAENDecoder::checkError(retcode,"SetAcquisitionMode",fCAEN.fragmentId);
  retcode = CAEN_DGTZ_GetAcquisitionMode(fHandle,(CAEN_DGTZ_AcqMode_t*)&readback);
  CheckReadback("SetAcquisitionMode",fCAEN.fragmentId,fCAEN.acqMode,readback);

  // sets analog monitor output mode
  // GetAnalogMonOutput function does not work for V1730s -- still true?
  // use register access instead 
  TLOG_ARB(TCONFIG,TRACE_NAME) << "SetAnalogMonOutputMode " << (CAEN_DGTZ_AnalogMonitorOutputMode_t)(fCAEN.analogMode) << TLOG_ENDL;
  retcode = CAEN_DGTZ_SetAnalogMonOutput(fHandle,(CAEN_DGTZ_AnalogMonitorOutputMode_t)(fCAEN.analogMode));
  sbndaq::CAENDecoder::checkError(retcode,"SetAnalogMonOutputMode",fCAEN.fragmentId);
  retcode = CAEN_DGTZ_ReadRegister(fHandle,ANALOG_MON_MODE,&readback);
  CheckReadback("SetAnalogMonOutputMode",fCAEN.fragmentId,fCAEN.analogMode,readback);

  TLOG_ARB(TCONFIG,TRACE_NAME) << "ConfigureAcquisition() done." << TLOG_ENDL;
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

void sbndaq::CAENV1730Readout::ConfigureInterrupts() 
{
  CAEN_DGTZ_EnaDis_t  state, stateOut;
  uint8_t             interruptLevel, interruptLevelOut;
  uint32_t            statusId, statusIdOut;
  uint16_t            eventNumber, eventNumberOut;
  CAEN_DGTZ_IRQMode_t mode, modeOut;
  CAEN_DGTZ_ErrorCode retcode;

  interruptLevel  = 1; // For CONET, level should be kept to 1
  statusId        = 0; // For CONET, statusId is meaningless
  mode            = CAEN_DGTZ_IRQ_MODE_RORA; // For CONET, only RORA = Release on Register Access
  eventNumber     = fCAEN.interruptEventNumber; // Number of recorded events to generate interrupts

  state = (fCAEN.interruptEnable>0)
    ? CAEN_DGTZ_ENABLE    // Enable interrupts
    : CAEN_DGTZ_DISABLE;; // Disable interrupts

  TLOG(TINFO) << "Configuring Interrupts state=" << uint32_t{state} << " ("
	      << sbndaq::CAENDecoder::EnaDisMode((CAEN_DGTZ_EnaDis_t)state) << ")"
        << ", mode=" << uint32_t{mode} << " (" << sbndaq::CAENDecoder::IRQMode((CAEN_DGTZ_IRQMode_t)mode) << ")" 
        << ", interruptLevel=" << uint32_t{interruptLevel} 
        << ", statusId=" << uint32_t{statusId}
        << ", eventNumber="<< uint16_t{eventNumber};

  retcode = CAEN_DGTZ_SetInterruptConfig(fHandle,state,interruptLevel,statusId,eventNumber,mode);
  CAENDecoder::checkError(retcode,"SetInterruptConfig",fCAEN.fragmentId);

  retcode = CAEN_DGTZ_GetInterruptConfig(fHandle,&stateOut,&interruptLevelOut,&statusIdOut,&eventNumberOut,&modeOut);
  CAENDecoder::checkError(retcode,"GetInterruptConfig",fCAEN.fragmentId);

  // check returned value for inconsistencies
  // skip statusId, interruptLevel, mode: these are meaningless for optical links
  // what matters is state and eventNumber

  if (state != stateOut)
  {
    TLOG_WARNING("CAENV1730Readout") << "Interrupt State was not setup properly, state write/read="
                                     << uint32_t{state} << "/" << uint32_t{stateOut};
  }
  if (eventNumber != eventNumberOut)
  {
    TLOG_WARNING("CAENV1730Readout") << "Interrupt eventNumber was not setup properly, eventNumber write/read="
                                     << uint32_t{eventNumber} << "/" << uint32_t{eventNumberOut};
  }              
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

void sbndaq::CAENV1730Readout::RunADCCalibration()
{
  TLOG_ARB(TINFO,TRACE_NAME) << "Running calibration..." << TLOG_ENDL;
  auto retcode = CAEN_DGTZ_Calibrate(fHandle);
  sbndaq::CAENDecoder::checkError(retcode,"Calibrate",fCAEN.fragmentId);
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

// Following SPI code is from CAEN
CAEN_DGTZ_ErrorCode CAENV1730Readout::ReadSPIRegister(int handle, uint32_t ch, uint32_t address, uint8_t *value)
{
  uint32_t SPIBusy = 1;
  CAEN_DGTZ_ErrorCode retcod = CAEN_DGTZ_Success;
  uint32_t SPIBusyAddr        = 0x1088 + (ch<<8);
  uint32_t addressingRegAddr  = 0x80B4;
  uint32_t valueRegAddr       = 0x10B8 + (ch<<8);
  uint32_t val;

  while(SPIBusy) 
  {
    if((retcod = CAEN_DGTZ_ReadRegister(handle, SPIBusyAddr, &SPIBusy)) != CAEN_DGTZ_Success)
    {
      return CAEN_DGTZ_CommError;
    }
    SPIBusy = (SPIBusy>>2)&0x1;
    if (!SPIBusy) 
    {
      if((retcod = CAEN_DGTZ_WriteRegister(handle, addressingRegAddr, address)) != CAEN_DGTZ_Success)
      { return CAEN_DGTZ_CommError;}

      if((retcod = CAEN_DGTZ_ReadRegister(handle, valueRegAddr, &val)) != CAEN_DGTZ_Success)
      { return CAEN_DGTZ_CommError;}
    }
    *value = (uint8_t)val;
    usleep(1000);
  }
  return CAEN_DGTZ_Success;
}

CAEN_DGTZ_ErrorCode CAENV1730Readout::WriteSPIRegister(int handle, uint32_t ch, uint32_t address, uint8_t value)
{
  uint32_t SPIBusy = 1;
  CAEN_DGTZ_ErrorCode retcod = CAEN_DGTZ_Success;
  uint32_t SPIBusyAddr        = 0x1088 + (ch<<8);
  uint32_t addressingRegAddr  = 0x80B4;
  uint32_t valueRegAddr       = 0x10B8 + (ch<<8);

  while (SPIBusy) 
  {
    if((retcod = CAEN_DGTZ_ReadRegister(handle, SPIBusyAddr, &SPIBusy)) != CAEN_DGTZ_Success)
    {
      return CAEN_DGTZ_CommError;
    }
    SPIBusy = (SPIBusy>>2)&0x1;
    if (!SPIBusy) 
    {
      if((retcod = CAEN_DGTZ_WriteRegister(handle, addressingRegAddr, address)) != CAEN_DGTZ_Success)
      {  return CAEN_DGTZ_CommError;}
      if((retcod = CAEN_DGTZ_WriteRegister(handle, valueRegAddr, (uint32_t)value)) != CAEN_DGTZ_Success)
      { return CAEN_DGTZ_CommError;}
    }
    usleep(1000);
  }
  return CAEN_DGTZ_Success;
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

void sbndaq::CAENV1730Readout::SetLockTempCalibration(bool onOff, uint32_t ch)
{
  CAEN_DGTZ_ErrorCode retcod;
  uint8_t lock, ctrl;
  TLOG_ARB(TCONFIG,TRACE_NAME) << "Locking Temperature Calibration Adjustments, channel " << ch << TLOG_ENDL;

  // Following code comes from CAEN
  // enter engineering functions
  retcod = WriteSPIRegister(fHandle, ch, (uint32_t)0x7A, (uint8_t)0x59);
  sbndaq::CAENDecoder::checkError(retcod,"LockTempCalibration",fCAEN.fragmentId);
  retcod = WriteSPIRegister(fHandle, ch, (uint32_t)0x7A, (uint8_t)0x1A);
  sbndaq::CAENDecoder::checkError(retcod,"LockTempCalibration",fCAEN.fragmentId);
  retcod = WriteSPIRegister(fHandle, ch, (uint32_t)0x7A, (uint8_t)0x11);
  sbndaq::CAENDecoder::checkError(retcod,"LockTempCalibration",fCAEN.fragmentId);
  retcod = WriteSPIRegister(fHandle, ch, (uint32_t)0x7A, (uint8_t)0xAC);
  sbndaq::CAENDecoder::checkError(retcod,"LockTempCalibration",fCAEN.fragmentId);
  
  // read lock value
  retcod = ReadSPIRegister (fHandle, ch, (uint32_t)0xA7, &lock);
  sbndaq::CAENDecoder::checkError(retcod,"LockTempCalibration",fCAEN.fragmentId);

  // write lock value
  retcod = WriteSPIRegister(fHandle, ch, (uint32_t)0xA5, lock);
  sbndaq::CAENDecoder::checkError(retcod,"LockTempCalibration",fCAEN.fragmentId);

  // enable lock
  retcod = ReadSPIRegister (fHandle, ch, (uint32_t)0xA4, &ctrl);
  sbndaq::CAENDecoder::checkError(retcod,"LockTempCalibration",fCAEN.fragmentId);

  if (onOff) { ctrl |= 0x4;}  // set bit 2
  else       { ctrl &= ~0x4;}
  retcod = WriteSPIRegister(fHandle, ch, (uint32_t)0xA4, ctrl);
  sbndaq::CAENDecoder::checkError(retcod,"LockTempCalibration",fCAEN.fragmentId);

  retcod = ReadSPIRegister (fHandle, ch, (uint32_t)0xA4, &ctrl);
  sbndaq::CAENDecoder::checkError(retcod,"LockTempCalibration",fCAEN.fragmentId);
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

void sbndaq::CAENV1730Readout::ConfigureDataBuffer()
{
  TLOG_ARB(TSTART,TRACE_NAME) << "ConfigureDataBuffer()" << TLOG_ENDL;

  CAEN_DGTZ_ErrorCode retcode;

  // sets maximum number of events to be transferred
  retcode = CAEN_DGTZ_SetMaxNumEventsBLT(fHandle,fCAEN.maxEventsPerTransfer);
  sbndaq::CAENDecoder::checkError(retcode,"SetMaxNumEventsBLT",fCAEN.fragmentId);

  // we do this shenanigans so we can get the BufferSize. We then allocate our own...
  // first, calls CAEN API to allocate a temporary readout buffer
  char* myBuffer=NULL;
  retcode = CAEN_DGTZ_MallocReadoutBuffer(fHandle,&myBuffer,&fBufferSize);
  sbndaq::CAENDecoder::checkError(retcode,"MallocReadoutBuffer",fCAEN.fragmentId);
  
  // now we got its buffer size, allocate our own
  fBuffer.reset(new uint16_t[fBufferSize/sizeof(uint16_t)]);
  TLOG_ARB(TSTART,TRACE_NAME) << "Created Buffer of size " << fBufferSize << TLOG_ENDL;  

  // free the temporary buffer allocated by CAEN
  retcode = CAEN_DGTZ_FreeReadoutBuffer(&myBuffer);
  sbndaq::CAENDecoder::checkError(retcode,"FreeReadoutBuffer",fCAEN.fragmentId);

  // how many events the board itself can hold: BUFFER_ORGANIZATION gives
  // 2^N buffers, and it is programmed indirectly by SetRecordLength, so it
  // has to be read back rather than computed
  uint32_t bufferOrganization = 0;
  retcode = CAEN_DGTZ_ReadRegister(fHandle,BUFFER_ORGANIZATION,&bufferOrganization);
  sbndaq::CAENDecoder::checkError(retcode,"ReadBufferOrganization",fCAEN.fragmentId);
  fNumBoardBuffers = 1u<<(bufferOrganization & 0xF); // code is in bits [3:0]
  TLOG(TLVL_INFO) << "Buffer organization (BUFFER_ORGANIZATION=0x"
                  << std::hex << static_cast<unsigned int>(BUFFER_ORGANIZATION) << ") = 0x"
                  << bufferOrganization << std::dec
                  << " -> " << fNumBoardBuffers << " buffers of "
                  << fCAEN.recordLength << " samples";

  // now we prepare the pool buffer on the boardreader side
  // single block size is CAEN buffer size
  TLOG_ARB(TSTART,TRACE_NAME) << "Configuring PoolBuffer of size " << fCAEN.poolBufferSize << TLOG_ENDL;
  fPoolBuffer.allocate(fBufferSize,fCAEN.poolBufferSize,true);
  fPoolBuffer.debugInfo();

  // lock a mutex protecting fTimestampMap and clear it
  std::lock_guard<std::mutex> lock(fTimestampMapMutex);
  fTimestampMap.clear();
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

void sbndaq::CAENV1730Readout::CheckReadback(std::string label,
					      int fragID,
					      uint32_t wrote,
					      uint32_t readback,
					      int channelID)
{
  if (wrote != readback){

    std::stringstream channelLabel(" ");
    if (channelID >= 0)
      channelLabel << " Channel/Group " << channelID;
    
    std::stringstream text;
    text << " " << label << " ReadBack error"
      << channelLabel.str() << " wrote " << wrote << " read " << readback;

    TLOG(TLVL_ERROR ) << "(FragID=" << fragID << ")" << text.str();
  }
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

void sbndaq::CAENV1730Readout::start()
{
  TLOG_INFO("CAENV1730Readout") << "start()" << TLOG_ENDL;
  
  // configure pool buffer 
  ConfigureDataBuffer();
  
  if((CAEN_DGTZ_AcqMode_t)(fCAEN.acqMode)==CAEN_DGTZ_AcqMode_t::CAEN_DGTZ_SW_CONTROLLED)
  {
    CAEN_DGTZ_ErrorCode retcode;
    TLOG_ARB(TSTART,TRACE_NAME) << "SWStartAcquisition" << TLOG_ENDL;
    retcode = CAEN_DGTZ_SWStartAcquisition(fHandle);
    sbndaq::CAENDecoder::checkError(retcode,"SWStartAcquisition",fCAEN.fragmentId);
  }
  
  fOverflowCounter=0;
  last_rcv_event_counter=0x0;
  last_sent_event_counter=0x0;
  last_sent_seqid =0x0;
  last_sent_ts=0x0;

  // Manual calibration added by Animesh - DELETE?
  // "its origin and purpose is still a total mistery"
  // this overwrites ADC calibration parameters
  if (fCAEN.writeCalibration)  
  { 
    CAEN_DGTZ_ErrorCode retcod;
    for ( uint32_t ch=0; ch<fNChannels; ++ch)
    {
      retcod = WriteSPIRegister(fHandle, ch, 0xFE, 0x00);      
      // write offset
      retcod = WriteSPIRegister(fHandle, ch, 0x20, 114);
      retcod = WriteSPIRegister(fHandle, ch, 0x21, 107);
      retcod = WriteSPIRegister(fHandle, ch, 0x26, 122);
      retcod = WriteSPIRegister(fHandle, ch, 0x27, 76);
      // write gain
      retcod = WriteSPIRegister(fHandle, ch, 0x22, 14);
      retcod = WriteSPIRegister(fHandle, ch, 0x23, 128);
      retcod = WriteSPIRegister(fHandle, ch, 0x24, 127);
      retcod = WriteSPIRegister(fHandle, ch, 0x28, 14);
      retcod = WriteSPIRegister(fHandle, ch, 0x29, 135);
      retcod = WriteSPIRegister(fHandle, ch, 0x2A, 125);
      // write skew
      retcod = WriteSPIRegister(fHandle, ch, 0x70, 129);    
      // Update parameters
      retcod = WriteSPIRegister(fHandle, ch, 0xFE, 0x01);
      retcod = WriteSPIRegister(fHandle, ch, 0xFE, 0x00);
    }
  }
    
  fTimePollBegin = boost::posix_time::microsec_clock::universal_time();
  GetData_thread_->start();
  
  TLOG_ARB(TSTART,TRACE_NAME) << "start() done." << TLOG_ENDL;
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

// This is really the DAQ part where the boardreader reads data from 
// the card and stores in its internal pool buffer
// - this is executed through the GetData worker thread
// - at this stage, data is identified by the CAEN event number in the header
// - timestamp map (key=CAEN event num) is used to later package the timestamp

bool sbndaq::CAENV1730Readout::GetData()
{
  TLOG(TGETDATA)<< "Begin of GetData()";

  CAEN_DGTZ_ErrorCode retcod;

  // if the software trigger is on, send one
  if(fCAEN.swTrigger) {
    usleep(fCAEN.getNextSleep);
    TLOG(TGETDATA) << "Sending SW trigger..." << TLOG_ENDL;
    retcod = CAEN_DGTZ_SendSWtrigger(fHandle);
    TLOG(TGETDATA) << "CAEN_DGTZ_SendSWtrigger returned " << int{retcod};
  }

  // read the data from the buffer of the card
  return readWindowDataBlocks();
}

bool sbndaq::CAENV1730Readout::readWindowDataBlocks() {

  if(fail_GetNext) {
    TLOG(TLVL_ERROR) << "(FragID=" << fCAEN.fragmentId << ")"
                     << "Not calling CAEN_DGTZ_ReadData due a previous critical error...";
    ::usleep(50000);
    return false;
  }

  TLOG(TGETDATA) << "(FragID=" << fCAEN.fragmentId << ")" << "Begin.";

  // (re)-arm interrupt before waiting for one
  // this is required for links through a5818 cards
  // but causes stability issues for a3818 links...
  if(fCAEN.aX818 == 5)
  {
    CAEN_DGTZ_ErrorCode retcode = CAEN_DGTZ_RearmInterrupt(fHandle);
    if(retcode < 0){
      TLOG(TLVL_WARNING) << "(FragID=" << fCAEN.fragmentId << ")"
                         << " RearmInterrupt() failed: " << retcode;
    } 
  }

  //wait for one event, then interrupt
  CAEN_DGTZ_ErrorCode retcode = CAEN_DGTZ_IRQWait(fHandle, fCAEN.IRQTimeoutMS);

  //if we have a timeout condition, return
  if (retcode == CAEN_DGTZ_Timeout) {

    //end of this poll
    fTimePollEnd = boost::posix_time::microsec_clock::universal_time();
    
    TLOG(TGETDATA) << "(FragID=" << fCAEN.fragmentId << ")"
		   << "Exiting after a timeout. Poll time was " 
		   << (fTimePollEnd - fTimePollBegin).total_milliseconds() << " ms.";
    
    //update the polling time for the next poll
    fTimePollBegin = fTimePollEnd;

    //go again!
    return true;
  }

  // catch any other IRQWait failure
  if (retcode != CAEN_DGTZ_Success) {
    TLOG(TLVL_WARNING) << "(FragID=" << fCAEN.fragmentId << ")"
                     << " CAEN_DGTZ_IRQWait failed; return code=" << int{retcode}
                     << "; not calling ReadData.";
    return true; // go again, do not stop for a transient failure
  }

  TLOG(TGETDATA) << "(FragID=" << fCAEN.fragmentId << ")"
		 << "No timeout. TimePollBegin=" 
		 << fTimePollBegin << " TimePollEnd=" << fTimePollEnd;

  uint32_t read_data_size = 1;
  size_t n_reads=0;

  //gianluca won't let me do a do while
  //we want to do ReadData until there is no more data to read

  TLOG(TGETDATA) << "(FragID=" << fCAEN.fragmentId << ")"
		 << "Start while loop read. " << read_data_size; 

  while(read_data_size!=0)
  {
    TLOG(TGETDATA) << "(FragID=" << fCAEN.fragmentId << ")"
		   << "Last read data size was " << read_data_size; 

    //reset read_data_size to 0, just in case
    read_data_size = 0;

    //get a block of data from the PoolBuffer. Hopefully doesn't take very long.
    auto block =  fPoolBuffer.takeFreeBlock();

    if(!block) {
      TLOG(TLVL_ERROR) << "(FragID=" << fCAEN.fragmentId << ")"
                       << "PoolBuffer is empty; last received trigger eventCounter=" << last_rcv_event_counter;
      TLOG(TLVL_ERROR) << "(FragID=" << fCAEN.fragmentId << ")"
                       << "PoolBuffer status: freeBlockCount=" << fPoolBuffer.freeBlockCount()
                       << "(FragID=" << fCAEN.fragmentId << ")"
                       << ", activeBlockCount=" << fPoolBuffer.activeBlockCount();
      TLOG(TLVL_ERROR) << "(FragID=" << fCAEN.fragmentId << ")"
                       << "Critical error; aborting boardreader process....";
      fail_GetNext = true;
      std::this_thread::yield();
      return false;
    }

    TLOG(TGETDATA) << "(FragID=" << fCAEN.fragmentId << ")"
                   << "Got a free DataBlock from PoolBuffer";

    //call ReadData
    TLOG(TGETDATA) << "(FragID=" << fCAEN.fragmentId << ")"
                   << "Calling ReadData(fHandle="<<fHandle<< ",bufp=" << (void*)block->begin
                   << ",&block.size="<<(void*)&(block->size) << ")";

    retcode = CAEN_DGTZ_ReadData(fHandle,CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT,
                                (char*)block->begin,&read_data_size);

    // 1) check to make sure no errors on readout
    if (retcode != CAEN_DGTZ_Success) {
      uint32_t stored=0, eventSize=0, acqStatus=0;
      CAEN_DGTZ_ReadRegister(fHandle,EVENT_STORED,&stored);
      CAEN_DGTZ_ReadRegister(fHandle,EVENT_SIZE,&eventSize);
      CAEN_DGTZ_ReadRegister(fHandle,CAEN_DGTZ_ACQ_STATUS_ADD,&acqStatus);
      TLOG(TLVL_ERROR) << "(FragID=" << fCAEN.fragmentId << ")"
                       << " CAEN_DGTZ_ReadData returned non zero return code; return code=" << int{retcode}
                       << " (" << sbndaq::CAENDecoder::CAENError(retcode) << ")"
                       << ", EVENT_STORED=" << stored << "/" << fNumBoardBuffers
                       << ", EVENT_SIZE=" << eventSize*sizeof(uint32_t) << " bytes"
                       << ", ACQ_STATUS=0x" << std::hex << acqStatus << std::dec
                       << " (eventFull=" << bool(acqStatus & 0x10)
                       << ", eventReady=" << bool(acqStatus & 0x8)
                       << "), n_reads this poll=" << n_reads;
      fPoolBuffer.returnFreeBlock(block);
      std::this_thread::yield();
      return false;
    }

    // 2) check for no data
    // a zero-length read can be the normal exit of this loop once the board has been
    // drained, so only warn when it happens on the first iteration, i.e. IRQWait
    // said an event was ready and ReadData then returned nothing
    if(read_data_size==0) {
      if(n_reads==0) {
        TLOG(TLVL_WARNING) << "(FragID=" << fCAEN.fragmentId << ")"
                           << " ReadData returned 0 bytes after a successful IRQWait.";
      }
      fPoolBuffer.returnFreeBlock(block);
      break;
    }

    // 3) check if data is within buffer boundaries
    if(read_data_size > block->size){
      TLOG(TLVL_ERROR) << "(FragID=" << fCAEN.fragmentId << ")"
                       << " CAENReadData() tried to write " << read_data_size
                       << " bytes into a " << block->size << "-byte buffer; dropping.";
      fPoolBuffer.returnFreeBlock(block);
      break;
    }

    TLOG(TGETDATA) << "(FragID=" << fCAEN.fragmentId << ")"
		   << "This read data size was " << read_data_size; 
    ++n_reads;

    // 4) update data size in block and verify_redzone
    block->data_size= read_data_size;
    block->verify_redzone();

    fTimePollEnd = boost::posix_time::microsec_clock::universal_time();
    
    TLOG(TGETDATA) << "(FragID=" << fCAEN.fragmentId << ")"
		   << "CAEN_DGTZ_ReadData complete with returned data size " << block->data_size
                   << " retcod=" << int{retcode};

    // now time to read off what we got in the header
    const auto header = reinterpret_cast<CAENV1730EventHeader const *>(block->begin);
    
    TLOG(TGETDATA) << "(FragID=" << fCAEN.fragmentId << ")"
		   << ": PMT_EVENT_COUNTER=" << header->eventCounter
		   << ", PMT_EVENT_SIZE=" << header->eventSize
		   << ", PMT_TIME_TAG=" << header->triggerTimeTag;

    const size_t header_event_size = sizeof(uint32_t)* header->eventSize; 

    // does the size reported in the header much the actual size?
    if(block->data_size != header_event_size ) {
      TLOG(TLVL_ERROR) << "(FragID=" << fCAEN.fragmentId << ")"
                       << " Wrong event size; returned=" << block->data_size 
                       << ", header=" << header_event_size
                       << ". PMT_EVENT_COUNTER=" << header->eventCounter
                       << ", PMT_EVENT_SIZE=" << header->eventSize
                       << ", PMT_TIME_TAG=" << header->triggerTimeTag 
                       << ". DROPPING THIS FRAGMENT.";
      fPoolBuffer.returnFreeBlock(block);
      break;
    }

    //do all the timestamp assignment
    //first reference against epoch
    fTimeDiffPollBegin = fTimePollBegin - fTimeEpoch;
    fTimeDiffPollEnd = fTimePollEnd - fTimeEpoch;

    //then calculate the mean poll time
    fMeanPollTime = fTimeDiffPollBegin.total_nanoseconds()/2 + fTimeDiffPollEnd.total_nanoseconds()/2;
    fMeanPollTimeNS = fMeanPollTime%(1000000000);
    fTTT=0;
    fTTT_ns = -1;

    if(fCAEN.useTimeTagForTimeStamp){

      fTTT = uint32_t{header->triggerTimeTag}; 
      fTTT_ns = fTTT*8;      

      // Scheme borrowed from what Antoni developed for CRT.
      // See https://sbn-docdb.fnal.gov/cgi-bin/private/DisplayMeeting?sessionid=7783
      fTS = fMeanPollTime - fMeanPollTimeNS + fTTT_ns
          + (fTTT_ns - (long)fMeanPollTimeNS < -500000000) * 1000000000
          - (fTTT_ns - (long)fMeanPollTimeNS >  500000000) * 1000000000
          - fCAEN.timeOffsetNanoSec;

    } else if(fCAEN.useTimeTagShiftForTimeStamp){

      fTTT = uint32_t{header->triggerTimeTag}; // 
      // TTT is 8 ticks/ns, record length is 2 ticks/ns. See CAEN V1730 manuals for details
      fTTT_ns = (fTTT*8.0) - (((double)fCAEN.recordLength * 2.0) * ((double)fCAEN.postPercent / 100.0)); //in 1 ns

      // Scheme borrowed from what Antoni developed for CRT.
      // See https://sbn-docdb.fnal.gov/cgi-bin/private/DisplayMeeting?sessionid=7783
      fTS = fMeanPollTime - fMeanPollTimeNS + fTTT_ns
          + (fTTT_ns - (long)fMeanPollTimeNS < -500000000) * 1000000000
          - (fTTT_ns - (long)fMeanPollTimeNS >  500000000) * 1000000000
          - fCAEN.timeOffsetNanoSec;

    } else{
      fTS = fTimeDiffPollEnd.total_nanoseconds() - fCAEN.timeOffsetNanoSec;
    }

    //put lock in local scope
    {
      std::lock_guard<std::mutex> lock(fTimestampMapMutex);
      fTimestampMap[uint32_t{header->eventCounter}] = fTS;
    }

    //print out timestamping info
    TLOG(TGETDATA) << "(FragID=" << fCAEN.fragmentId << ")"
		   << "TIMESTAMP " << fCAEN.fragmentId 
		   << ": Poll begin/end/mean/ns = " << fTimeDiffPollBegin.total_nanoseconds()
		   << "/" << fTimeDiffPollEnd.total_nanoseconds() 
		   << "/" << fMeanPollTime
		   << "/" << fMeanPollTimeNS;
    TLOG(TGETDATA) << "(FragID=" << fCAEN.fragmentId << ")"
		   << "TIMESTAMP " << fCAEN.fragmentId 
		   << ": TTT/TTT_ns/TS_ns = " << fTTT << "/" << fTTT_ns << "/" << fTS;
    TLOG(TGETDATA) << "(FragID=" << fCAEN.fragmentId << ")"
		   << "TIMESTAMP " << fCAEN.fragmentId 
		   << ": Timestamp for event " << header->eventCounter << " = " << fTS;

    // check trigger event counter gaps: this is a 24-bit counter in the CAEN board
    // compute gap: look only at first 24bits to handle possible overflows
    uint32_t current_event_counter = uint32_t{header->eventCounter};
    uint32_t gap  = (current_event_counter - last_rcv_event_counter) & EVENT_COUNTER_MASK;
    // trigger rate: the board counts ALL triggers, accepted or not (ACQ_CONTROL
    // bit[3], set in ConfigureReadout()), so Rate mode sums gap over the
    // reporting interval and counts every trigger, event or not
    metricMan->sendMetric("BoardEventRate", double(gap), "Hz", 11, artdaq::MetricMode::Rate);

    if(gap > 1u)
    {
      TLOG (TLVL_WARNING) << "(FragID=" << fCAEN.fragmentId << ")"
        << " Missing triggers; current trigger eventCounter=" << current_event_counter
        << ", previous trigger eventCounter / gap  = " << last_rcv_event_counter << " / " << gap
        << ", freeBlockCount=" << fPoolBuffer.freeBlockCount()
        << ", activeBlockCount=" << fPoolBuffer.activeBlockCount()
        << ", fullyDrainedCount=" << fPoolBuffer.fullyDrainedCount();
    }

    // update
    last_rcv_event_counter = current_event_counter;

    //return active block
    fPoolBuffer.returnActiveBlock(block);
    
    TLOG(TGETDATA) << "(FragID=" << fCAEN.fragmentId << ")"
		   << "CAEN_DGTZ_ReadData returned DataBlock header.eventCounter=" 
		   << header->eventCounter << ", header.eventSize=" << header_event_size;

  }//end while read_data_size is not zero

  TLOG(TGETDATA) << "(FragID=" << fCAEN.fragmentId << ")"
		 << "n_reads=" << n_reads;

  //update the polling time for the next poll
  fTimePollBegin = fTimePollEnd;

  //and go again!
  return true;
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

// This is the part of the boardared that packages the fragments
// - getNext_ is called internally by the artdaq framework
// - available data in the pool buffers are packaged into fragments
// - fragment timestamp is assigned through the timestamp map

bool sbndaq::CAENV1730Readout::getNext_(artdaq::FragmentPtrs & fragments)
{
  if(fail_GetNext){
    throw std::runtime_error("Critical error; stopping boardreader process...." );
  }
  return readSingleWindowFragments(fragments);
}

bool sbndaq::CAENV1730Readout::readSingleWindowFragments(artdaq::FragmentPtrs & fragments)
{
  TLOG(TGETNEXT) << "Begin of readSingleWindowFragments()" ;

  // this value would persist across function calls (static)
  // but we update it manually at the end of each read cycle
  static auto start= std::chrono::steady_clock::now();

  // measure the time delta between the end of previous fragment creation
  // this tells how "laggy" getNext is in creating fragments
  // threshold is configurable -> find a better name?
  std::chrono::duration<double> delta = std::chrono::steady_clock::now()-start;
  if (delta.count() > 0.005*fCAEN.getNextFragmentBunchSize)
  {
    metricMan->sendMetric("Laggy getNext",1,"count",11,artdaq::MetricMode::Accumulate);
    TLOG (TLVL_DEBUG) << "Time spent outside of getNext_() " << delta.count()*1000 << " ms. Last seen fragment sequenceID=" << last_sent_seqid;
  }

  // check if there are any active blocks inside the pool buffer
  // if not, wait a bit and retry
  if(fPoolBuffer.activeBlockCount() == 0)
  {
    TLOG(TGETNEXT) << "PoolBuffer has no data.  Laast last seen fragment sequenceID=" << last_sent_seqid
                   << "; Sleep for " << fCAEN.getNextSleep << " us and return.";
    ::usleep(fCAEN.getNextSleep);
    start= std::chrono::steady_clock::now();
    return true;
  }

  double max_fragment_create_time = 0.0;
  double min_fragment_create_time = 10000.0;
  struct timespec now;
  clock_gettime(CLOCK_REALTIME,&now); // get current server time

  // prepare V1730 fragment metadata structure
  const auto metadata = CAENV1730FragmentMetadata(fNChannels,fCAEN.recordLength,now.tv_sec,now.tv_nsec,ch_temps);
  const auto fragment_datasize_bytes = metadata.ExpectedDataSize();
  TLOG(TMAKEFRAG)<< "Created CAENV1730FragmentMetadata with expected data size of "
                 << fragment_datasize_bytes << " bytes.";

  //just get anything that's there...
  while(fPoolBuffer.activeBlockCount())
  {
    start= std::chrono::steady_clock::now();

    // create an artdaq::Fragment buffer to receive the data
    // set some identifiers as required 
    auto fragment_uptr=artdaq::Fragment::FragmentBytes(fragment_datasize_bytes,last_sent_seqid,fCAEN.fragmentId,sbndaq::detail::FragmentType::CAENV1730,metadata);

    // create a data range that points into the fragment
    // now poolbuffer "knows" where it needs to write the payload
    using sbndaq::PoolBuffer;
    PoolBuffer::DataRange<decltype(artdaq::Fragment())> range{fragment_uptr->dataBegin(),fragment_uptr->dataEnd()};

    // copy next block from the pool buffer into the fragment
    // if it fails, no more usable data so break out of the loop
    if(!fPoolBuffer.read(range)) break;

    // initial part of the fragmet is casted to the V1730 event header
    // this is so we can read values off of it
    const auto header = reinterpret_cast<CAENV1730EventHeader const *>(fragment_uptr->dataBeginBytes());
   
    // get eventCounter, which is the key to get the correct timestamp from the map
    // for longer runs it can overflow (24 bit) --> play safe and don't use it as sequence id
    uint32_t current_event_counter = uint32_t {header->eventCounter};

    // detect overflow: 
    // 1. current counter smaller than previous one
    // 2. difference is large (larger then 0.9 overflow...)
    if (current_event_counter < last_sent_event_counter && 
       (last_sent_event_counter - current_event_counter) > (EVENT_COUNTER_MASK * 9 / 10)) 
    {
      ++fOverflowCounter;
    }

    // build sequence id keeping track of overflows (avoids repeated seqIDs in the same run)
    // of course 64 bit can still overflow, but much more unlikely
    uint64_t current_sequence_id = uint64_t{current_event_counter} + uint64_t{fOverflowCounter} * (uint64_t{EVENT_COUNTER_MASK} + 1u);
    
    // update the sequence_id for the fragment
    fragment_uptr->setSequenceID(current_sequence_id);

    // now look up the timestamp from the map
    size_t ts_count=0;
    {
      std::lock_guard<std::mutex> lock(fTimestampMapMutex);
      ts_count = fTimestampMap.count(current_event_counter);
    }

    // if no timestamp is found, retry 3 times -- sleeping in between 
    int ts_loop=0;
    while(ts_loop<3 && ts_count==0)
    {
      TLOG(TLVL_WARNING) << "(FragID=" << fCAEN.fragmentId << ")"
                         << " TIMESTAMP FOR SEQID " << current_sequence_id << " EVCOUNTER " << current_event_counter << " not found in fTimestampMap!"
			 << " Will sleep for 200 ms and try again. Times tried = " << ts_loop;
      ::usleep(200000);
      ts_loop++;
      {
	      std::lock_guard<std::mutex> lock(fTimestampMapMutex);
	      ts_count = fTimestampMap.count(current_event_counter);
      }
    }

    // check where we are now in time
    // this is taking the current server clock
    artdaq::Fragment::timestamp_t ts_frag, ts_now;
    {
      using namespace boost::gregorian;
      using namespace boost::posix_time;
      
      ptime t_now(microsec_clock::universal_time());
      ptime time_t_epoch(date(1970,1,1));
      time_duration diff = t_now - time_t_epoch;
      
      ts_now = diff.total_nanoseconds();
    }

    // if the map contains a timestamp entry
    // fetch it and erase it from the map
    if(ts_count>0)
    {
      std::lock_guard<std::mutex> lock(fTimestampMapMutex);      
      ts_frag = fTimestampMap.at(current_event_counter);
      fTimestampMap.erase(current_event_counter);
    }
    // if no timestamp is found (weird..)
    // generate a new timestamp based on the current time
    // use similar procedure to getData, just different "now"
    // no "polling" correction being applied here
    else
    {
      TLOG(TLVL_ERROR) << "(FragID=" << fCAEN.fragmentId << ")"
                       << " TIMESTAMP FOR SEQID " << current_sequence_id << " EVCOUNTER " << current_event_counter << " not found in fTimestampMap!"
		       << " Will generate new one now...";

      if(fCAEN.useTimeTagForTimeStamp)
      {
	      const auto TTT = uint32_t {header->triggerTimeTag};
	
        using namespace boost::gregorian;
        using namespace boost::posix_time;
	
        ptime t_now(second_clock::universal_time());
        ptime time_t_epoch(date(1970,1,1));
        time_duration diff = t_now - time_t_epoch;
        uint32_t t_offset_s = diff.total_seconds();
        uint64_t t_offset_ticks = diff.total_seconds()*125000000; //in 8ns ticks
        uint64_t t_truetriggertime = t_offset_ticks + TTT;

        TLOG_ARB(TMAKEFRAG,TRACE_NAME) << "time offset = " << t_offset_ticks << " ns since the epoch" << TLOG_ENDL;
        ts_frag = (t_truetriggertime*8); //in 1ns ticks
      }
      else if(fCAEN.useTimeTagShiftForTimeStamp)
      {
	      const auto TTT = uint32_t {header->triggerTimeTag};
	
        using namespace boost::gregorian;
        using namespace boost::posix_time;
	
        ptime t_now(second_clock::universal_time());
        ptime time_t_epoch(date(1970,1,1));
        time_duration diff = t_now - time_t_epoch;
        uint32_t t_offset_s = diff.total_seconds();
        uint64_t t_offset_ticks = diff.total_seconds()*125000000; //in 8ns ticks
        uint64_t t_truetriggertime = t_offset_ticks + TTT;
        TLOG_ARB(TMAKEFRAG,TRACE_NAME) << "time offset = " << t_offset_ticks << " ns since the epoch"<< TLOG_ENDL;

        // TTT is 8 ticks/ns, record length is 2 ticks/ns. See CAEN V1730 manuals for details
        ts_frag = (t_truetriggertime*8.0) - (((double)fCAEN.recordLength * 2.0) * ((double)fCAEN.postPercent / 100.0)); //in 1ns ticks
      }
      else
      {
        using namespace boost::gregorian;
        using namespace boost::posix_time;
        
        ptime t_now(microsec_clock::universal_time());
        ptime time_t_epoch(date(1970,1,1));
        time_duration diff = t_now - time_t_epoch;
        
        ts_frag = diff.total_nanoseconds() - fCAEN.timeOffsetNanoSec;
      }
    }

    TLOG_ARB(TMAKEFRAG,TRACE_NAME) << "Fragment timestamp in 1ns ticks = " << ts_frag << TLOG_ENDL;
    TLOG_ARB(TMAKEFRAG,TRACE_NAME) << "Difference to now in ns is = " << (ts_now - ts_frag) << TLOG_ENDL;

    // assumng ts_frag was read from the timestamp map
    // we can check ordering is correct and how much time it took
    // note: ts_frag is a mix of server time + CAEN TTT
    // bad values in TTT (board not getting PPS reset properly) might trigger a problem here
    if( ts_frag>ts_now )
      TLOG(TLVL_WARNING) << "(FragID=" << fCAEN.fragmentId << ")"
			 << " Fragment assigned timestamp is after timestamp from fragment creation! Causality problem!!"
			 << "ts_frag - ts_now = " << ts_frag - ts_now << " ns!"
			 << TLOG_ENDL;

    else if( (ts_now-ts_frag)>5e9 ){
      TLOG(TLVL_ERROR) << "(FragID=" << fCAEN.fragmentId << ")"
		       << " Fragment being packged more than 5 seconds after timestamp: "
		       << "ts_now - ts_frag = " << ts_now-ts_frag << " ns!"
		       << TLOG_ENDL;
    }
    else if( (ts_now-ts_frag)>1e9 ){
      TLOG(TLVL_WARNING) << "(FragID=" << fCAEN.fragmentId << ")"
			 << " Fragment being packged more than 1 second after timestamp: "
			 << "ts_now - ts_frag = " << ts_now-ts_frag << " ns!"
			 << TLOG_ENDL;
    }

    metricMan->sendMetric("FragmentCreationGapMax", (ts_now-ts_frag), "ns", 12, artdaq::MetricMode::Maximum);
    metricMan->sendMetric("FragmentCreationGapAvg", (ts_now-ts_frag), "ns", 12, artdaq::MetricMode::Average);

    // finally, set the timestamp to the fragment 
    fragment_uptr->setTimestamp( ts_frag );

    TLOG(TMAKEFRAG) << "Created fragment " << fCAEN.fragmentId << " sequenceID " << current_sequence_id << " for event " << current_event_counter
                    << " triggerTimeTag " << header->triggerTimeTag << " ts=" << ts_frag;
    
    // check for possible gaps in the sequence IDs: compare current one with last sent
    // throw errors if gap > 1 or order is not correct
    // no handling of possible 64 bit overflow -- FIX?
    uint64_t sequence_id_gap = current_sequence_id - last_sent_seqid;

    // look for gaps in the sequence_id
    if( sequence_id_gap > 1u )
    {
      TLOG (TLVL_WARNING) << "(FragID=" << fCAEN.fragmentId << ")"
         << " Missing data; current fragment sequenceID=" << current_sequence_id
         << ", previous fragment sequenceID / gap  = " << last_sent_seqid << " / " << sequence_id_gap;
      metricMan->sendMetric("Missing Fragments", uint64_t{sequence_id_gap}, "frags", 11, artdaq::MetricMode::Accumulate);
    }

    // look for bad ordering
    if( current_sequence_id < last_sent_seqid )
    {
      TLOG(TLVL_ERROR) << "(FragID=" << fCAEN.fragmentId << ")"
                       << " SequenceIDs processed out of order!! " << current_sequence_id << " < " << last_sent_seqid << TLOG_ENDL;
    }
    if( last_sent_ts > ts_frag)
    {
      TLOG(TLVL_ERROR) << "(FragID=" << fCAEN.fragmentId << ")"
                       << " Timestamps out of order!! Last event later than current one." << ts_frag << " < " << last_sent_ts << TLOG_ENDL;
    }

    // finally push the fragments to the eventbuilders
    // clear and them move in the fragments vector
    fragments.emplace_back(nullptr);
    std::swap(fragments.back(),fragment_uptr);
 
    last_sent_event_counter = current_event_counter;
    last_sent_seqid = current_sequence_id;
    last_sent_ts = ts_frag;

    // keep track of how much time it took
    // also update min/max time of each loop call
    delta = std::chrono::steady_clock::now()-start;
    min_fragment_create_time=std::min(delta.count(),min_fragment_create_time);
    max_fragment_create_time=std::max(delta.count(),max_fragment_create_time);

    if (delta.count() >0.0005) 
    {
      metricMan->sendMetric("Laggy Fragments",1,"frags",11,artdaq::MetricMode::Maximum);
      TLOG (TLVL_DEBUG+1) << "Creating a fragment with setSequenceID=" << last_sent_seqid << " took " << delta.count()*1000 << " ms";
    }

  }

  metricMan->sendMetric("Fragment Create Time Max",max_fragment_create_time,"s",11,artdaq::MetricMode::Accumulate);
  //metricMan->sendMetric("Fragment Create Time  Min" ,min_fragment_create_time,"s",1,artdaq::MetricMode::Accumulate);

  TLOG(TGETNEXT) << "End of readSingleWindowFragments(); returning " << fragments.size() << " fragments.";

  // reset the start time
  start= std::chrono::steady_clock::now();
  return true;
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

void sbndaq::CAENV1730Readout::stop()
{
  TLOG_INFO("CAENV1730Readout") << "stop()" << TLOG_ENDL;

  GetData_thread_->stop();

  CAEN_DGTZ_ErrorCode retcode;
  TLOG_ARB(TSTOP,TRACE_NAME) << "SWStopAcquisition" << TLOG_ENDL;
  retcode = CAEN_DGTZ_SWStopAcquisition(fHandle);
  sbndaq::CAENDecoder::checkError(retcode,"SWStopAcquisition",fCAEN.fragmentId);

  if(fBuffer != NULL){
    fBuffer.reset();
  }
  TLOG_ARB(TSTOP,TRACE_NAME) << "stop() done." << TLOG_ENDL;
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

// Checks the hardware status and sends corresponding metrics to Grafana
// This function is called internally by the framework, no need to call it manually
// Relevant fcl parameters are: 
// - "poll_hardware_status" (true/false)
// - "hardware_poll_interval_us" (interval in us)
bool sbndaq::CAENV1730Readout::checkHWStatus_(){

  // first, check the acqusition status register: CAEN_DGTZ_ACQ_STATUS_ADD
  // this provides overall status of the board
  uint32_t data;
  auto ret = CAEN_DGTZ_ReadRegister(fHandle,CAEN_DGTZ_ACQ_STATUS_ADD,&data);
  if ( ret == CAEN_DGTZ_Success ){
    // unpacking only "interesting" values
    bool run  = data & 0x0004; // is running?
    bool full = data & 0x0010; // is busy?
    bool shut = data & 0x80000; // is shutting down? 
    bool pll  = data & 0x0080; // is the PLL locked?

    metricMan->sendMetric("Running", int(run), "", 11, artdaq::MetricMode::LastPoint);
    metricMan->sendMetric("Busy", int(full), "", 11, artdaq::MetricMode::LastPoint);
    metricMan->sendMetric("Shutdown", int(shut), "", 11, artdaq::MetricMode::LastPoint);
    metricMan->sendMetric("PLLLock", int(pll), "", 11, artdaq::MetricMode::LastPoint);

  } else {
    TLOG(TLVL_WARNING) << "(FragID=" << fCAEN.fragmentId << ") "
                       << "Failed reading CAEN_DGTZ_ACQ_STATUS_ADD register!";
  }

  // How close is the board to refusing triggers?
  uint32_t eventsStored = 0;
  ret = CAEN_DGTZ_ReadRegister(fHandle,EVENT_STORED,&eventsStored);
  if(ret == CAEN_DGTZ_Success){
    metricMan->sendMetric("BoardEventsStored", uint64_t{eventsStored}, "events", 11,
                          artdaq::MetricMode::Maximum);
    // fNumBoardBuffers is only known once ConfigureDataBuffer() has run, so a
    // poll before the first start would divide by zero
    if(fNumBoardBuffers > 0){
      metricMan->sendMetric("BoardBufferFillPercent",
                            100.0*double(eventsStored)/double(fNumBoardBuffers), "%", 11,
                            artdaq::MetricMode::Maximum);
    }
  } else {
    TLOG(TLVL_WARNING) << "(FragID=" << fCAEN.fragmentId << ") "
                       << "Failed reading EVENT_STORED register!";
  }

  // second, check individual channel status
  // this provides temperature reading + channel memory full
  for(size_t ch=0; ch<fNChannels; ++ch)
  {
    std::ostringstream tempStream; 
    tempStream << "Channel" << ch << ".Temp"; 
    
    std::ostringstream memfullStream;
    memfullStream << "Channel" << ch << ".MemoryFull"; 
   
    CAEN_DGTZ_ErrorCode retcod;

    // temperature readout enabled for this channel?
    if ( fCAEN.temperatureCheckMask & ( 1 << ch ) ) 
    {
      retcod = CAEN_DGTZ_ReadTemperature(fHandle, ch, &(ch_temps[ch]));
      TLOG_ARB(TTEMP,TRACE_NAME) << "(fragID=" << fCAEN.fragmentId << ")"
         << tempStream.str() << ": " 
         << ch_temps[ch] << "  C"
				 << TLOG_ENDL;
      
      metricMan->sendMetric(tempStream.str(), int(ch_temps[ch]), "C", 11, artdaq::MetricMode::Average);

      // Need 2 requirements for measurement to be meaningfull 
      // 1. Can successfully read temperature: return code = 0 since S/N 164 can fail to read temperature during acquisition
      // 2. Temperature < non_physical temperature, 200C since S/N 164 can produce non-physical temperature
      if ( retcod == CAEN_DGTZ_Success && ch_temps[ch] < V1730_UNPHYSICAL_TEMPERATURE )
      {
        // V1730(S) shuts down at 70(85) celsius, give a error ahead of that
        // threshold temperature is set via fhicl parameter
        if ( ch_temps[ch] > fCAEN.maxTemp ) {
          TLOG(TLVL_ERROR) << "SHUTTING DOWN CAENV1730 fragID=" << fCAEN.fragmentId << " : "
			     << " Channel " << ch
			     << " temperature " << ch_temps[ch]
			     << " > " << fCAEN.maxTemp << " degrees Celsius."
			     << " ReadTemperature Return Code = " << retcod
			     << TLOG_ENDL;
          } 
      } else {
        // Ignore unphysical temperatures/bad return codes from S/N 164.
        // CAEN advises not to read temperatures while the readout is running, but we cannot do that.
        // Only one sensors on one V1730 has ever malfunctioned.
        // S/N 164 sometimes returns a non-physical temperature, ignore it and move on
        TLOG(TLVL_WARNING) << "CAENV1730 fragID=" << fCAEN.fragmentId << " : "
			   << " Channel " << ch
			   << " unphysical temperature " << ch_temps[ch] << " degrees Celsius."
			   << " ReadTemperature Return Code = " << retcod
			   << TLOG_ENDL;
      }
    }

    // now check if channel memory is full
    uint32_t CHANNEL_STATUS_ADD = 0x1088 + (ch<<8);
    retcod = CAEN_DGTZ_ReadRegister(fHandle, CHANNEL_STATUS_ADD, &(ch_status[ch]));

    if(retcod == CAEN_DGTZ_Success){
      // unpack only memory full status
      bool full = ch_status[ch] & 0x1;

      TLOG_ARB(TTEMP,TRACE_NAME) << "(fragID=" << fCAEN.fragmentId << ")"
         << memfullStream.str() << ": " 
         << int(full) << TLOG_ENDL;

      metricMan->sendMetric(memfullStream.str(), int(full), "", 11, artdaq::MetricMode::LastPoint);
    } else {
      TLOG(TLVL_WARNING) << "(FragID=" << fCAEN.fragmentId << ") "
                         << "Failed reading CHANNEL_STATUS register for channel " << ch;
    }
  } //end channel loop
  return true;
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

// this function queries software and board information
// board S/N + firmware, CAEN libraries versions, VME bridge firmware/driver
void sbndaq::CAENV1730Readout::GetSWInfo(){

  int retcod=0;
  CAEN_DGTZ_BoardInfo_t info;

  // CAEV1730 S/N and firmware
  retcod = CAEN_DGTZ_GetInfo(fHandle,&info);
  if( retcod == CAEN_DGTZ_Success ){
    TLOG(TLVL_INFO) << info.ModelName << " S/N: " << info.SerialNumber 
                    << "\nFirmware ROC: " << info.ROC_FirmwareRel 
                    << "\nFirmware AMC: " << info.AMC_FirmwareRel;
  }

  // CAEN software releases
  char CommSWrel[30], VMESWrel[30], DGTZSWrel[30];
  retcod = CAEN_DGTZ_SWRelease( DGTZSWrel );
  retcod = CAENVME_SWRelease( VMESWrel );
  retcod = CAENComm_SWRelease( CommSWrel );
  TLOG(TLVL_INFO) << "Software releases"
		  << "\ncaendigitizer: " << DGTZSWrel
                  << "\ncaenvme: " << VMESWrel
                  << "\ncaencomm: " << CommSWrel;
  
  // AX818 firmware / driver
  short Device = 0;
  int32_t BHandle;

  CVBoardTypes cvAX818;
  switch (fCAEN.aX818)
  {
    case 3:
      cvAX818 = cvA3818;
      break;
    case 5:
      cvAX818 = cvA5818;
      break;
    default:
      TLOG(TLVL_ERROR) << "(FragID=" << fCAEN.fragmentId << ")"
                       << " Do not know how to handle fCAEN.aX818 == " << fCAEN.aX818;
      return;
  }

  if( CAENVME_Init2(cvAX818, &fCAEN.link, Device, &BHandle) == cvSuccess ) {
  
    char fwrev[100];
    char drrev[100];
    auto ret = CAENVME_BoardFWRelease(BHandle,fwrev);

    std::ostringstream aX818Stream;
    aX818Stream << "A" << fCAEN.aX818 << "818 firmware: ";
    if (!ret) aX818Stream << fwrev << "\n";
    else aX818Stream << CAENVME_DecodeError(ret) << "\n";

    ret = CAENVME_DriverRelease( BHandle, drrev );
    aX818Stream << "A" << fCAEN.aX818 << "818 driver: ";
    if (!ret) aX818Stream << drrev;
    else aX818Stream << CAENVME_DecodeError(ret);
   
    TLOG(TLVL_INFO) << aX818Stream.str();

    CAENVME_End(BHandle);
  }
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(sbndaq::CAENV1730Readout)
