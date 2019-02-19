#define TRACE_NAME "CAENV1730Readout"
#include "artdaq/DAQdata/Globals.hh"

#include "sbndaq-artdaq/Generators/Common/CAENV1730Readout.hh"
#include "artdaq/Application/GeneratorMacros.hh"

#include <iostream>
#include <sstream>
#include <time.h>

#include "CAENDecoder.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"

// constructor of the CAENV1730Readout. It wants the param set 
// which means the fhicl paramters in CAENV1730Readout.hh
sbndaq::CAENV1730Readout::CAENV1730Readout(fhicl::ParameterSet const& ps) : 
  CommandableFragmentGenerator(ps),
  fCAEN(ps),								     
  fAcqMode(CAEN_DGTZ_SW_CONTROLLED)
{

  TLOG_ARB(TCONFIG,TRACE_NAME) << "CAENV1730Readout()" << TLOG_ENDL;

  // initialize the fhicl parameters (see CAENV1730Readout.hh)
  // the obj ps has a member method that gets he private members
  // fVerbosity, etc.. are priv memb in CAENV1730Readout.hh
  //
  // wes, 16Jan2018: disabling default parameters
  ///
  fVerbosity = ps.get<int>("Verbosity"); //-1
  TLOG_ARB(TCONFIG,TRACE_NAME) << "Verbosity=" << fVerbosity << TLOG_ENDL;

  fBoardChainNumber = ps.get<int>("BoardChainNumber"); //0
  TLOG_ARB(TCONFIG,TRACE_NAME) << "BoardChainNumber=" << fBoardChainNumber << TLOG_ENDL;

  fInterruptLevel = ps.get<uint8_t>("InterruptLevel"); //1
  TLOG_ARB(TCONFIG,TRACE_NAME) << "InterruptLevel=" << fInterruptLevel << TLOG_ENDL;

  fInterruptStatusID = ps.get<uint32_t>("InterruptStatusID"); //0
  TLOG_ARB(TCONFIG,TRACE_NAME) << "InterruptStatusID=" << fInterruptStatusID << TLOG_ENDL;

  fInterruptEventNumber = ps.get<uint16_t>("InterruptEventNumber"); //1
  TLOG_ARB(TCONFIG,TRACE_NAME) << "InterruptEventNumber=" << fInterruptEventNumber << TLOG_ENDL;
  fSWTrigger = ps.get<bool>("SWTrigger"); //false
  TLOG_ARB(TCONFIG,TRACE_NAME) << "SWTrigger=" << fSWTrigger << TLOG_ENDL;

  fGetNextSleep = ps.get<uint32_t>("GetNextSleep"); //1000000
  TLOG_ARB(TCONFIG,TRACE_NAME) << "GetNextSleep=" << fGetNextSleep << TLOG_ENDL;

  fCircularBufferSize = ps.get<uint32_t>("CircularBufferSize"); //1000000
  TLOG_ARB(TCONFIG,TRACE_NAME) << "CircularBufferSize=" << fCircularBufferSize << TLOG_ENDL;

  fCombineReadoutWindows = ps.get<bool>("CombineReadoutWindows");
  TLOG_ARB(TCONFIG,TRACE_NAME) << "CombineReadoutWindows=" << fCombineReadoutWindows << TLOG_ENDL;

  fCalibrateOnConfig = ps.get<bool>("CalibrateOnConfig",false);
  TLOG_ARB(TCONFIG,TRACE_NAME) << "CalibrateOnConfig=" << fCalibrateOnConfig << TLOG_ENDL;


  CAEN_DGTZ_ErrorCode retcode;

  if(fVerbosity>0)
    TLOG_INFO("CAENV1730Readout") << fCAEN << TLOG_ENDL;
  
  TLOG_ARB(TCONFIG,TRACE_NAME) << fCAEN << TLOG_ENDL;

  fNChannels = fCAEN.nChannels;
  fBoardID = fCAEN.boardId;

  TLOG_ARB(TCONFIG,TRACE_NAME) << "Using BoardID=" << fBoardID << " with NChannels=" << fNChannels  << TLOG_ENDL;

  retcode = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_OpticalLink,
				    fCAEN.link,
				    fBoardChainNumber,
				    0,
				    &fHandle);

  fOK=false;

  if(retcode != CAEN_DGTZ_Success){
    TLOG_ERROR("CAENV1730Readout")
      << "*** Fatal error opening CAEN board at "
      << fCAEN.link << ", " << fBoardChainNumber << TLOG_ENDL;
    sbndaq::CAENDecoder::checkError(retcode,"OpenDigitizer",fBoardID);
    fHandle = -1;
    fOK = false;
  }
  else{
    retcode = CAEN_DGTZ_Reset(fHandle);
    sbndaq::CAENDecoder::checkError(retcode,"Reset",fBoardID);
    fOK = true;
    sleep(1);
    Configure();
  }

  CAEN_DGTZ_EnaDis_t  state;
  uint8_t             interruptLevel;
  uint32_t            statusId;
  uint16_t            eventNumber;
  CAEN_DGTZ_IRQMode_t mode;

  CAEN_DGTZ_EnaDis_t  stateOut;
  uint8_t             interruptLevelOut;
  uint32_t            statusIdOut;
  uint16_t            eventNumberOut;
  CAEN_DGTZ_IRQMode_t modeOut;

  if(fInterruptLevel>0){
    state           = CAEN_DGTZ_ENABLE;
    interruptLevel  = fInterruptLevel;
    statusId        = 0;
    eventNumber     = 1;
    mode            = CAEN_DGTZ_IRQ_MODE_ROAK; // Default mode
  }
  else{
    state           = CAEN_DGTZ_DISABLE;
    interruptLevel  = fInterruptLevel;
    statusId        = 0;
    eventNumber     = 1;
    mode            = CAEN_DGTZ_IRQ_MODE_ROAK; // Default mode
  }

  retcode = CAEN_DGTZ_SetInterruptConfig(fHandle,
                                        state,
                                        interruptLevel,
                                        statusId,
                                        eventNumber,
                                        mode);
  CAENDecoder::checkError(retcode,"SetInterruptConfig",fBoardID);

  retcode = CAEN_DGTZ_GetInterruptConfig (fHandle,
                                          &stateOut,
                                          &interruptLevelOut,
                                          &statusIdOut,
                                          &eventNumberOut,
                                          &modeOut);
  CAENDecoder::checkError(retcode,"GetInterruptConfig",fBoardID);
  
  if (state != stateOut){
    TLOG_WARNING("CAENV1730Readout")
      << "Interrupt State was not setup properly." << TLOG_ENDL;
  }
  if (interruptLevel != interruptLevelOut){
    TLOG_WARNING("CAENV1730Readout")
      << "Interrupt State was not setup properly." << TLOG_ENDL;
  }
  if (eventNumber != eventNumberOut){
    TLOG_WARNING("CAENV1730Readout")
      << "Interrupt State was not setup properly." << TLOG_ENDL;
  }
  if (mode != modeOut){
    TLOG_WARNING("CAENV1730Readout")
      << "Interrupt State was not setup properly." << TLOG_ENDL;
  }

  // Set up worker getdata thread.
  share::ThreadFunctor functor = std::bind(&CAENV1730Readout::GetData,this);
  auto worker_functor = share::WorkerThreadFunctorUPtr(new share::WorkerThreadFunctor(functor,"GetDataWorkerThread"));
  auto GetData_worker = share::WorkerThread::createWorkerThread(worker_functor);
  GetData_thread_.swap(GetData_worker);
  TLOG_ARB(TCONFIG,TRACE_NAME) << "GetData worker thread setup." << TLOG_ENDL;

  TLOG_ARB(TCONFIG,TRACE_NAME) << "Configuration complete with OK=" << fOK << TLOG_ENDL;
}

void sbndaq::CAENV1730Readout::Configure()
{
  TLOG_ARB(TCONFIG,TRACE_NAME) << "Configure()" << TLOG_ENDL;

  CAEN_DGTZ_ErrorCode retcode;
  uint32_t readback;

  //Make sure DAQ is off first
  TLOG_ARB(TCONFIG,TRACE_NAME) << "Set Acquisition Mode to SW" << TLOG_ENDL;
  retcode = CAEN_DGTZ_SetAcquisitionMode(fHandle,CAEN_DGTZ_SW_CONTROLLED);
  sbndaq::CAENDecoder::checkError(retcode,"SetAcquisitionMode",fBoardID);
  retcode = CAEN_DGTZ_GetAcquisitionMode(fHandle,(CAEN_DGTZ_AcqMode_t *)&readback);
  CheckReadback("SetAcquisitionMode", fBoardID,(uint32_t)CAEN_DGTZ_SW_CONTROLLED ,readback);
 
  TLOG_ARB(TCONFIG,TRACE_NAME) << "Stop Acquisition" << TLOG_ENDL;
  retcode = CAEN_DGTZ_SWStopAcquisition(fHandle);
  sbndaq::CAENDecoder::checkError(retcode,"SWStopAcquisition",fBoardID);

  //get info, make sure board is in good communicative state
  retcode = CAEN_DGTZ_GetInfo(fHandle,&fBoardInfo);
  fOK = (retcode==CAEN_DGTZ_Success);

  retcode = CAEN_DGTZ_Reset(fHandle);

  ConfigureReadout();
  ConfigureRecordFormat();
  ConfigureTrigger();
  
  if(fAcqMode==CAEN_DGTZ_SW_CONTROLLED){
    TLOG_ARB(TCONFIG,TRACE_NAME) << "Stop Acquisition" << TLOG_ENDL;
    retcode = CAEN_DGTZ_SWStopAcquisition(fHandle);
    sbndaq::CAENDecoder::checkError(retcode,"SWStopAcquisition",fBoardID);
  }

  ConfigureAcquisition();

  if(fCalibrateOnConfig)
    RunADCCalibration();

  TLOG_ARB(TCONFIG,TRACE_NAME) << "Configure() done." << TLOG_ENDL;
}

void sbndaq::CAENV1730Readout::RunADCCalibration()
{
  TLOG_ARB(TCONFIG,TRACE_NAME) << "Running calibration..." << TLOG_ENDL;
  auto retcode = CAEN_DGTZ_Calibrate(fHandle);
  sbndaq::CAENDecoder::checkError(retcode,"Calibrate",fBoardID);
}

void sbndaq::CAENV1730Readout::ConfigureRecordFormat()
{
  TLOG_ARB(TCONFIG,TRACE_NAME) << "ConfigureRecordFormat()" << TLOG_ENDL;
  CAEN_DGTZ_ErrorCode retcode;
  uint32_t readback;

  //channel masks for readout(?)
  TLOG_ARB(TCONFIG,TRACE_NAME) << "SetChannelEnableMask " << fCAEN.channelEnableMask << TLOG_ENDL;
  retcode = CAEN_DGTZ_SetChannelEnableMask(fHandle,fCAEN.channelEnableMask);
  sbndaq::CAENDecoder::checkError(retcode,"SetChannelEnableMask",fBoardID);
  retcode = CAEN_DGTZ_GetChannelEnableMask(fHandle,&readback);
  sbndaq::CAENDecoder::checkError(retcode,"GetChannelEnableMask",fBoardID);
  CheckReadback("CHANNEL_ENABLE_MASK", fBoardID, fCAEN.channelEnableMask, readback);

  //record length
  TLOG_ARB(TCONFIG,TRACE_NAME) << "SetRecordLength " << fCAEN.recordLength << TLOG_ENDL;
  retcode = CAEN_DGTZ_SetRecordLength(fHandle,fCAEN.recordLength);
  sbndaq::CAENDecoder::checkError(retcode,"SetRecordLength",fBoardID);
  retcode = CAEN_DGTZ_GetRecordLength(fHandle,&readback);
  sbndaq::CAENDecoder::checkError(retcode,"GetRecordLength",fBoardID);
  CheckReadback("RECORD_LENGTH", fBoardID, fCAEN.recordLength, readback);

  //post trigger size
  TLOG_ARB(TCONFIG,TRACE_NAME) << "SetPostTriggerSize " << (unsigned int)(fCAEN.postPercent) << TLOG_ENDL;
  retcode = CAEN_DGTZ_SetPostTriggerSize(fHandle,(unsigned int)(fCAEN.postPercent));
  sbndaq::CAENDecoder::checkError(retcode,"SetPostTriggerSize",fBoardID);
  retcode = CAEN_DGTZ_GetPostTriggerSize(fHandle,&readback);
  sbndaq::CAENDecoder::checkError(retcode,"GetPostTriggerSize",fBoardID);
  CheckReadback("POST_TRIGGER_SIZE", fBoardID, fCAEN.postPercent, readback);

  //number of events
  TLOG_ARB(TCONFIG,TRACE_NAME) << "SetMaxNumEventsBLT " << fCAEN.maxEventsPerTransfer << TLOG_ENDL;
  retcode = CAEN_DGTZ_SetMaxNumEventsBLT(fHandle,fCAEN.maxEventsPerTransfer);
  sbndaq::CAENDecoder::checkError(retcode,"SetMaxNumEventsBLT",fBoardID);
  retcode = CAEN_DGTZ_GetMaxNumEventsBLT(fHandle,&readback);
  CheckReadback("SetMaxNumEventsBLT", fBoardID,fCAEN.maxEventsPerTransfer ,readback);

  TLOG_ARB(TCONFIG,TRACE_NAME) << "ConfigureRecordFormat() done." << TLOG_ENDL;
}

sbndaq::CAENV1730Readout::~CAENV1730Readout()
{
  TLOG_ARB(TCONFIG,TRACE_NAME) << "~CAENV1730Readout()" << TLOG_ENDL;

  if(fBuffer != NULL){
    fBuffer.reset();
  }

  TLOG_ARB(TCONFIG,TRACE_NAME) << "~CAENV1730Readout() done." << TLOG_ENDL;
}

void sbndaq::CAENV1730Readout::ConfigureDataBuffer()
{
  TLOG_ARB(TSTART,TRACE_NAME) << "ConfigureDataBuffer()" << TLOG_ENDL;

  CAEN_DGTZ_ErrorCode retcode;

  //we do this shenanigans so we can get the BufferSize. We then allocate our own...
  char* myBuffer=NULL;
  retcode = CAEN_DGTZ_MallocReadoutBuffer(fHandle,&myBuffer,&fBufferSize);
  sbndaq::CAENDecoder::checkError(retcode,"MallocReadoutBuffer",fBoardID);
  
  fBuffer.reset(new uint16_t[fBufferSize/sizeof(uint16_t)]);

  TLOG_ARB(TSTART,TRACE_NAME) << "Created Buffer of size " << fBufferSize << std::endl << TLOG_ENDL;  

  retcode = CAEN_DGTZ_FreeReadoutBuffer(&myBuffer);
  sbndaq::CAENDecoder::checkError(retcode,"FreeReadoutBuffer",fBoardID);

  TLOG_ARB(TSTART,TRACE_NAME) << "Configuring Circular Buffer of size " << fCircularBufferSize << TLOG_ENDL;
  fCircularBuffer = sbndaq::CircularBuffer<uint16_t>(fCircularBufferSize/sizeof(uint16_t)); 
  TLOG_ARB(TSTART,TRACE_NAME) << "Created Circular Buffer of size " 
			      << fCircularBuffer.Buffer().capacity()*sizeof(uint16_t) 
			      << std::endl << TLOG_ENDL;
  
}

void sbndaq::CAENV1730Readout::ConfigureTrigger()
{
  TLOG_ARB(TCONFIG,TRACE_NAME) << "ConfigureTrigger()" << TLOG_ENDL;

  CAEN_DGTZ_ErrorCode retcode;
  uint32_t readback;
  uint32_t addr;

  //set the trigger configurations
  TLOG_ARB(TCONFIG,TRACE_NAME) << "SetSWTriggerMode" << fCAEN.swTrgMode << TLOG_ENDL;
  retcode = CAEN_DGTZ_SetSWTriggerMode(fHandle,(CAEN_DGTZ_TriggerMode_t)(fCAEN.swTrgMode));
  sbndaq::CAENDecoder::checkError(retcode,"SetSWTriggerMode",fBoardID);
  retcode = CAEN_DGTZ_GetSWTriggerMode(fHandle,(CAEN_DGTZ_TriggerMode_t *)&readback);
  CheckReadback("SetSWTriggerMode", fBoardID,fCAEN.swTrgMode,readback);

  TLOG_ARB(TCONFIG,TRACE_NAME) << "SetExtTriggerMode" << fCAEN.extTrgMode << TLOG_ENDL;
  retcode = CAEN_DGTZ_SetExtTriggerInputMode(fHandle,(CAEN_DGTZ_TriggerMode_t)(fCAEN.extTrgMode));
  sbndaq::CAENDecoder::checkError(retcode,"SetExtTriggerInputMode",fBoardID);
  retcode = CAEN_DGTZ_GetExtTriggerInputMode(fHandle,(CAEN_DGTZ_TriggerMode_t *)&readback);
  CheckReadback("SetExtTriggerInputMode", fBoardID,fCAEN.extTrgMode,readback);

  /*
  TLOG_ARB(TCONFIG,TRACE_NAME) << "SetSelfTriggerMode" << fCAEN.selfTrgMode 
			       << " on channel mask " << fCAEN.channelSelfTrgMask << TLOG_ENDL;
  retcode = CAEN_DGTZ_SetChannelSelfTrigger(fHandle,(CAEN_DGTZ_TriggerMode_t)(fCAEN.selfTrgMode));
  sbndaq::CAENDecoder::checkError(retcode,"SetChannelSelfTrigger",fBoardID);
  */
  //retcode = CAEN_DGTZ_GetExtTriggerInputMode(fHandle,(CAEN_DGTZ_TriggerMode_t *)&readback);
  //CheckReadback("SetExtTriggerInputMode", fBoardID,fCAEN.extTrgMode,readback);

  for(uint32_t ch=0; ch<fNChannels; ++ch){

    TLOG_ARB(TCONFIG,TRACE_NAME) << "Set channel " << ch
				 << " trigger threshold to " << fCAEN.triggerThresholds[ch] << TLOG_ENDL;
    retcode = CAEN_DGTZ_SetChannelTriggerThreshold(fHandle,ch,fCAEN.triggerThresholds[ch]); //0x8000
    sbndaq::CAENDecoder::checkError(retcode,"SetChannelTriggerThreshold",fBoardID);
    retcode = CAEN_DGTZ_GetChannelTriggerThreshold(fHandle,ch,&readback);
    CheckReadback("SetChannelTriggerThreshold",fBoardID,fCAEN.triggerThresholds[ch],readback);

    //pulse width only set in pairs
    if(ch%2==0){
      TLOG_ARB(TCONFIG,TRACE_NAME) << "Set channels " << ch << "/" << ch+1 
				   << " trigger pulse width to " << fCAEN.triggerPulseWidth << TLOG_ENDL;
      retcode = CAEN_DGTZ_WriteRegister(fHandle,0x1070+(ch<<8),fCAEN.triggerPulseWidth);
      sbndaq::CAENDecoder::checkError(retcode,"SetChannelTriggerPulseWidth",fBoardID);
      retcode = CAEN_DGTZ_ReadRegister(fHandle,0x1070+(ch<<8),&readback);
      CheckReadback("SetChannelTriggerPulseWidth",fBoardID,fCAEN.triggerPulseWidth,readback);
      /*
      TLOG_ARB(TCONFIG,TRACE_NAME) << "Set channels " << ch << "/" << ch+1 
				   << " self trigger logic to " << fCAEN.channelSelfTrgLogic[ch/2]
				   << " self trigger pulse type to " << fCAEN.channelSelfTrgPulseType[ch/2] << TLOG_ENDL;
      retcode = CAEN_DGTZ_WriteRegister(fHandle,0x1084+(ch<<8),
					((fCAEN.channelSelfTrgLogic[ch/2] & 0x3) + (fCAEN.channelSelfTrgPulseType[ch/2] & 0x1)<<2) );

      sbndaq::CAENDecoder::checkError(retcode,"SetSelfTriggerLogic",fBoardID);
      retcode = CAEN_DGTZ_ReadRegister(fHandle,0x1084+(ch<<8),&readback);
      CheckReadback("SetSelfTriggerLogic",fBoardID,
		    (fCAEN.channelSelfTrgLogic[ch/2] & 0x3) + (fCAEN.channelSelfTrgPulseType[ch/2] & 0x1)<<2)
		    ,readback);
      */
    }
      
  }
  
  TLOG_ARB(TCONFIG,TRACE_NAME) << "SetTriggerMode" << fCAEN.extTrgMode << TLOG_ENDL;
  retcode = CAEN_DGTZ_SetExtTriggerInputMode(fHandle,(CAEN_DGTZ_TriggerMode_t)(fCAEN.extTrgMode));
  sbndaq::CAENDecoder::checkError(retcode,"SetExtTriggerInputMode",fBoardID);
  retcode = CAEN_DGTZ_GetExtTriggerInputMode(fHandle,(CAEN_DGTZ_TriggerMode_t *)&readback);
  CheckReadback("SetExtTriggerInputMode", fBoardID,fCAEN.extTrgMode,readback);  

  TLOG_ARB(TCONFIG,TRACE_NAME) << "SetTriggerOverlap" << fCAEN.allowTriggerOverlap << TLOG_ENDL;
  if ( fCAEN.allowTriggerOverlap ) { addr = CONFIG_SET_ADDR;}
  else                             { addr = CONFIG_CLEAR_ADDR;}
  retcode = CAEN_DGTZ_WriteRegister(fHandle,addr,TRIGGER_OVERLAP_MASK);
  sbndaq::CAENDecoder::checkError(retcode,"SetTriggerOverlap",fBoardID);

  //level=1 for TTL, =0 for NIM
  TLOG_ARB(TCONFIG,TRACE_NAME) << "SetIOLevel " << (CAEN_DGTZ_IOLevel_t)(fCAEN.ioLevel) << TLOG_ENDL;
  retcode = CAEN_DGTZ_SetIOLevel(fHandle,(CAEN_DGTZ_IOLevel_t)(fCAEN.ioLevel));
  sbndaq::CAENDecoder::checkError(retcode,"SetIOLevel",fBoardID);
  retcode = CAEN_DGTZ_GetIOLevel(fHandle,(CAEN_DGTZ_IOLevel_t *)&readback);
  CheckReadback("SetIOLevel", fBoardID,fCAEN.ioLevel,readback);

  TLOG_ARB(TCONFIG,TRACE_NAME) << "ConfigureTrigger() done." << TLOG_ENDL;
}

void sbndaq::CAENV1730Readout::ConfigureReadout()
{
  TLOG_ARB(TCONFIG,TRACE_NAME) << "ConfigureReadout()" << TLOG_ENDL;

  CAEN_DGTZ_ErrorCode retcode;
  uint32_t readback;
  uint32_t addr,mask;
  
  TLOG_ARB(TCONFIG,TRACE_NAME) << "SetRunSyncMode " << (CAEN_DGTZ_RunSyncMode_t)(fCAEN.runSyncMode) << TLOG_ENDL;
  retcode = CAEN_DGTZ_SetRunSynchronizationMode(fHandle,
						(CAEN_DGTZ_RunSyncMode_t)(fCAEN.runSyncMode));
  sbndaq::CAENDecoder::checkError(retcode,"SetRunSynchronizationMode",fBoardID);
  retcode = CAEN_DGTZ_GetRunSynchronizationMode(fHandle,(CAEN_DGTZ_RunSyncMode_t*)&readback);
  CheckReadback("SetRunSynchronizationMode",fBoardID,fCAEN.runSyncMode,readback);
  
  mask = ( 1 << TEST_PATTERN_t::TEST_PATTERN_S );
  addr = (fCAEN.testPattern)
    ? CAEN_DGTZ_BROAD_CH_CONFIGBIT_SET_ADD
    : CAEN_DGTZ_BROAD_CH_CLEAR_CTRL_ADD;
  TLOG_ARB(TCONFIG,TRACE_NAME) << "SetTestPattern addr=" << addr << ", mask=" << mask << TLOG_ENDL;
  retcode = CAEN_DGTZ_WriteRegister(fHandle,addr,mask);
  sbndaq::CAENDecoder::checkError(retcode,"SetTestPattern",fBoardID);

  TLOG_ARB(TCONFIG,TRACE_NAME) << "SetDyanmicRange " << fCAEN.dynamicRange << TLOG_ENDL;
  mask = (uint32_t)(fCAEN.dynamicRange);
  addr = 0x8028;
  retcode = CAEN_DGTZ_WriteRegister(fHandle,addr,mask);
  sbndaq::CAENDecoder::checkError(retcode,"SetDynamicRange",fBoardID);

  for(uint32_t ch=0; ch<fNChannels; ++ch){
    TLOG_ARB(TCONFIG,TRACE_NAME) << "Set channel " << ch << " DC offset to " << fCAEN.pedestal[ch] << TLOG_ENDL;
    retcode = CAEN_DGTZ_SetChannelDCOffset(fHandle,ch,fCAEN.pedestal[ch]);
    sbndaq::CAENDecoder::checkError(retcode,"SetChannelDCOffset",fBoardID);
    retcode = CAEN_DGTZ_GetChannelDCOffset(fHandle,ch,&readback);
    CheckReadback("SetChannelDCOffset",fBoardID,fCAEN.pedestal[ch],readback,ch);
  }

  TLOG_ARB(TCONFIG,TRACE_NAME) << "ConfigureReadout() done." << TLOG_ENDL;
}

void sbndaq::CAENV1730Readout::ConfigureAcquisition()
{
  TLOG_ARB(TCONFIG,TRACE_NAME) << "ConfigureAcquisition()" << TLOG_ENDL;

  CAEN_DGTZ_ErrorCode retcode;
  uint32_t readback;
  
  TLOG_ARB(TCONFIG,TRACE_NAME) << "SetAcqMode " << (CAEN_DGTZ_AcqMode_t)(fCAEN.acqMode) << TLOG_ENDL;
  retcode = CAEN_DGTZ_SetAcquisitionMode(fHandle,(CAEN_DGTZ_AcqMode_t)(fCAEN.acqMode));
  sbndaq::CAENDecoder::checkError(retcode,"SetAcquisitionMode",fBoardID);
  retcode = CAEN_DGTZ_GetAcquisitionMode(fHandle,(CAEN_DGTZ_AcqMode_t*)&readback);
  CheckReadback("SetAcquisitionMode",fBoardID,fCAEN.acqMode,readback);
  

  //TLOG_ARB(TCONFIG,TRACE_NAME) << "SetAnalogMonOutputMode " << (CAEN_DGTZ_AnalogMonitorOutputMode_t)(fCAEN.analogMode) << TLOG_ENDL;
  //retcode = CAEN_DGTZ_SetAnalogMonOutput(fHandle,(CAEN_DGTZ_AnalogMonitorOutputMode_t)(fCAEN.analogMode));
  //sbndaq::CAENDecoder::checkError(retcode,"SetAnalogMonOutputMode",fBoardID);
  //retcode = CAEN_DGTZ_GetAnalogMonOutput(fHandle,(CAEN_DGTZ_AnalogMonitorOutputMode_t*)&readback);
  //CheckReadback("SetAnalogMonOutputMode",fBoardID,fCAEN.analogMode,readback);


  TLOG_ARB(TCONFIG,TRACE_NAME) << "ConfigureAcquisition() done." << TLOG_ENDL;
}

bool sbndaq::CAENV1730Readout::WaitForTrigger()
{

  TLOG_ARB(TSTATUS,TRACE_NAME) << "WaitForTrigger()" << TLOG_ENDL;

  CAEN_DGTZ_ErrorCode retcode;

  uint32_t acqStatus;
  retcode = CAEN_DGTZ_ReadRegister(fHandle,
				   CAEN_DGTZ_ACQ_STATUS_ADD,
				   &acqStatus);
  if(retcode!=CAEN_DGTZ_Success){
    TLOG_WARNING("CAENV1730Readout")
      << "Trying ReadRegister ACQUISITION_STATUS again." << TLOG_ENDL;
    retcode = CAEN_DGTZ_ReadRegister(fHandle,
				     CAEN_DGTZ_ACQ_STATUS_ADD,
				     &acqStatus);
  }
  sbndaq::CAENDecoder::checkError(retcode,"ReadRegister ACQ_STATUS",fBoardID);

  TLOG_ARB(TSTATUS,TRACE_NAME) << " Acq status = " << acqStatus << TLOG_ENDL;
  return (acqStatus & ACQ_STATUS_MASK_t::EVENT_READY);

}

void sbndaq::CAENV1730Readout::start()
{
  if(fVerbosity>0)
    TLOG_INFO("CAENV1730Readout") << "start()" << TLOG_ENDL;
  TLOG_ARB(TSTART,TRACE_NAME) << "start()" << TLOG_ENDL;

  ConfigureDataBuffer();
  total_data_size = 0;
  prev_rwcounter = -1;

  if((CAEN_DGTZ_AcqMode_t)(fCAEN.acqMode)==CAEN_DGTZ_AcqMode_t::CAEN_DGTZ_SW_CONTROLLED)
    {
      CAEN_DGTZ_ErrorCode retcode;
      TLOG_ARB(TSTART,TRACE_NAME) << "SWStartAcquisition" << TLOG_ENDL;
      retcode = CAEN_DGTZ_SWStartAcquisition(fHandle);
      sbndaq::CAENDecoder::checkError(retcode,"SWStartAcquisition",fBoardID);
    }

  fEvCounter=0;

  GetData_thread_->start();

  TLOG_ARB(TSTART,TRACE_NAME) << "start() done." << TLOG_ENDL;
}

void sbndaq::CAENV1730Readout::stop()
{
  if(fVerbosity>0)
    TLOG_INFO("CAENV1730Readout") << "stop()" << TLOG_ENDL;
  TLOG_ARB(TSTOP,TRACE_NAME) << "stop()" << TLOG_ENDL;

  GetData_thread_->stop();

  CAEN_DGTZ_ErrorCode retcode;
  TLOG_ARB(TSTOP,TRACE_NAME) << "SWStopAcquisition" << TLOG_ENDL;
  retcode = CAEN_DGTZ_SWStopAcquisition(fHandle);
  sbndaq::CAENDecoder::checkError(retcode,"SWStopAcquisition",fBoardID);

  if(fBuffer != NULL){
    fBuffer.reset();
  }
  TLOG_ARB(TSTOP,TRACE_NAME) << "stop() done." << TLOG_ENDL;
}

bool sbndaq::CAENV1730Readout::checkHWStatus_(){

  for(int32_t ch=0; ch<CAENConfiguration::MAX_CHANNELS; ++ch){

    CAEN_DGTZ_ReadTemperature(fHandle,ch,&(ch_temps[ch]));
    TLOG_ARB(TTEMP,TRACE_NAME) << "Channel " << ch
			       <<" temp: " << ch_temps[ch] << "  C"
			       << TLOG_ENDL;
    //metric call here...
  }
  
  return true;
}

bool sbndaq::CAENV1730Readout::GetData()
{
  TLOG(TGETDATA) << "Begin of GetData()" << TLOG_ENDL;

  uint32_t this_data_size=0;  // define this to then pass its adress to the function that reads data
  CAEN_DGTZ_ErrorCode retcod;

  if(fSWTrigger) {
    usleep(fGetNextSleep);
    TLOG(TGETDATA) << "Sending SW trigger..." << TLOG_ENDL;
    retcod = CAEN_DGTZ_SendSWtrigger(fHandle);
    TLOG(TGETDATA) << "CAEN_DGTZ_SendSWtrigger returned " << retcod;
  }

  // read the data from the buffer of the card
  // this_data_size is the size of the acq window
  char *bufp = (char*)(fBuffer.get());
  if(fCAEN.interruptLevel > 0){
    retcod = CAEN_DGTZ_RearmInterrupt(fHandle);
    CAENDecoder::checkError(retcod,"RearmInterrupt",fBoardID);

    retcod = CAEN_DGTZ_IRQWait(fHandle, fGetNextSleep);
    if ( retcod != CAEN_DGTZ_Timeout ) {
      CAENDecoder::checkError(retcod,"IRQWait",fBoardID);
      if (retcod == CAEN_DGTZ_Success){
        TLOG(TGETDATA) << "Calling ReadData(fHandle="<<
            fHandle<< ",bufp=" << (void*)bufp << ",&this_data_size="<<&this_data_size<<")";
        retcod = CAEN_DGTZ_ReadData(fHandle,CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT,
                                    bufp, &this_data_size);
        TLOG(TGETDATA) << "ReadData complete with returned data size "
                       << this_data_size << " retcod=" << retcod;
      }
    }

  }
  else{
    TLOG(TGETDATA) << "Calling ReadData(fHandle="<<fHandle<< ",bufp=" << (void*)bufp
                   << ",&this_data_size="<<&this_data_size<<")";
    retcod = CAEN_DGTZ_ReadData(fHandle,CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT,
                                bufp, &this_data_size);
    TLOG(TGETDATA) << "ReadData complete with returned data size " << this_data_size
                   << " retcod=" << retcod;
  }

  if(this_data_size==0) {
    TLOG(TGETDATA) << "No data. Sleep for " << fGetNextSleep << " us and return.";
    usleep(fGetNextSleep);
    return false;
  }

  TLOG(TGETDATA) << "checking if size()="<<fCircularBuffer.Buffer().size()
                 << " + this_data_size ("<<this_data_size/sizeof(uint16_t)
                 <<") is > capacity ("<<fCircularBuffer.Buffer().capacity()<<")";
  if ((fCircularBuffer.Buffer().size()+(this_data_size/sizeof(uint16_t))) > fCircularBuffer.Buffer().capacity())
	  TLOG(TGETDATA) << "WILL BLOCK -- FIXME WES?BILL";
  //else	
	  fCircularBuffer.Insert(this_data_size/sizeof(uint16_t),fBuffer);
  
  return true;
}// CAENV1730Readout::GetData()



// this is really the DAQ part where the server reads data from 
// the card and stores them
bool sbndaq::CAENV1730Readout::getNext_(artdaq::FragmentPtrs & fragments)
{
	return fCombineReadoutWindows ? readCombinedWindowFragments(fragments): readSingleWindowFragments(fragments);

#if 0	
  TLOG_ARB(TGETNEXT,TRACE_NAME) << "Begin of getNext_()" << TLOG_ENDL;
 
  uint32_t event_size =0;	
  uint32_t this_data_size=0;  // define this to then pass its adress to the function that reads data
  struct timespec zeit;	      // keine anung

  clock_gettime(CLOCK_REALTIME,&zeit);
  
  this_data_size = fCircularBuffer.Buffer().size()*sizeof(uint16_t);

  // if there's no data keep calling getNext_
  if(this_data_size==0){
    TLOG_ARB(TGETNEXT,TRACE_NAME) << "No data. Sleep for " << fGetNextSleep << " us and return." << TLOG_ENDL;
    usleep(fGetNextSleep);
    return true;
  }

  TLOG_ARB(TGETDATA,TRACE_NAME) << "Circular buffer size is " << this_data_size << TLOG_ENDL;

  // I initialize this metadata object
  // because its expected size is = to the size of a rw
  CAENV1730FragmentMetadata metadata(fNChannels,fCAEN.recordLength,
				     zeit.tv_sec,zeit.tv_nsec,
				     ch_temps);
  TLOG_ARB(TMAKEFRAG,TRACE_NAME) << "Created metadata with metadata.ExpectedDataSize() = " 
				 << metadata.ExpectedDataSize() << TLOG_ENDL;

  n_readout_windows = this_data_size/metadata.ExpectedDataSize();

  // this_data_size has to be an integer multiple of metadata.ExpectedDataSize()
  // precisely by a factor of nevents
  if(this_data_size != n_readout_windows*metadata.ExpectedDataSize()){
    TLOG_WARNING("CAENV1730Readout")
      << "Total data size is " << this_data_size 
      << " and n_readout_windows is " << n_readout_windows
      << ", but this does not match per-event size if " << metadata.ExpectedDataSize()
      << ". Sleep for " << fGetNextSleep << " us and return." << TLOG_ENDL;
    usleep(fGetNextSleep);
    return true;
  }

  fCircularBuffer.Linearize();

  // I loop over the readout windows of the acq window
  for(size_t i_rw=0; i_rw<n_readout_windows; ++i_rw){
    
	char * buf_begin = (char*)(&(*fCircularBuffer.Buffer().begin()));
	TLOG(TMAKEFRAG) << "Get Readout Window number - i_rw=" << i_rw
					<< " Buffer().begin()=" << (void*)buf_begin 
					<< " metadata.ExpectedDataSize()="<< metadata.ExpectedDataSize();
    
	// I scan the rw's ponting at them with eventptr
	char* rwptr;
	if(fCombineReadoutWindows)
		rwptr = buf_begin + i_rw*metadata.ExpectedDataSize();
	else
		rwptr = buf_begin;// + i_rw*metadata.ExpectedDataSize();
    //char* rwptr = (char*)(&(*(fCircularBuffer.Buffer().begin())) + i_rw*metadata.ExpectedDataSize()/sizeof(uint16_t));
    //if ((ulong)rwptr > (ulong)(&(*fCircularBuffer.Buffer().end()))) {
    //    TLOG (TMAKEFRAG) << "CAENV1730Readout rwptr beyond end of Buffer() FIXME - should never happen. If it does, give up on life";
	//		event_size += metadata.ExpectedDataSize();
	//	fCircularBuffer.Erase(event_size/sizeof(uint16_t));
	//	return true;
    //}
    
    TLOG (TMAKEFRAG) << "First 4 bytes from CAENV1730Readout header " <<   *((uint32_t*)rwptr);

    CAENV1730EventHeader* header = reinterpret_cast<CAENV1730EventHeader*>(rwptr);
    uint32_t rwcounter = header->eventCounter; // I get the eventcounter for the i_e^th readout window	

    TLOG_ARB(TMAKEFRAG,TRACE_NAME) << "rwcounter = " << rwcounter 
			  << ".  prev_rwcounter (before if) = " << prev_rwcounter << TLOG_ENDL;	
    
    // I update the total size of an event (acq window)
    event_size += metadata.ExpectedDataSize();

    // if the data are not enough to build one event increase the event size							       	
    if(fCombineReadoutWindows && ((int32_t)rwcounter > prev_rwcounter)){
      
      // now, as I loop through the rw's, at some point the 
      // eventcounter will be reset to zero (because the rw belongs 
      // to an other acq wind). So if I keep track of the # of rw's 
      // with prev_eventcounter, when evcounter will be again 0
      // I break the loop and I generate an Event (i.e. smthg that 
      // corresponds to an acq wind)
      TLOG_ARB(TMAKEFRAG,TRACE_NAME) << "i am in the if" << TLOG_ENDL;
      
      prev_rwcounter = rwcounter;
      total_data_size += metadata.ExpectedDataSize();
      
      // here I place memory to store the fragment in the end	
      
      TLOG_ARB(TMAKEFRAG,TRACE_NAME) << "event_size = " << event_size
				     << " total_data_size = " << total_data_size
				     << " i_rw*metadata.ExpectedDataSize() = " << i_rw*metadata.ExpectedDataSize()
				     << " prev_eventcounter = " << prev_rwcounter << TLOG_ENDL;
    }//end if event counter increasing
    else{ 																// now I am ready to build the artdaq fragm
      TLOG(TMAKEFRAG) << "i am in the else\n" 
				     << "event_size = " << event_size 
				     << " total_data_size = " << total_data_size 		
				     << " i_rw*metadata.ExpectedDataSize() = " << i_rw*metadata.ExpectedDataSize() 
				     << " eventptr - fBuffer = " << rwptr - (char*)&*(fCircularBuffer.Buffer().begin())
				     << TLOG_ENDL;
      // fragments is a vector of fragments. 
      // emplace is simuilar to push_back, but it doesn't
      // copy. I don't need to construct an object and put
      // it back. With emplace I construct it AS I push_back
      
      // also, at this point a fragment is a whole event
      // that is why I use the fEvCounter
      fragments.emplace_back( artdaq::Fragment::FragmentBytes(event_size,
							      fEvCounter,fBoardID,
							      sbndaq::detail::FragmentType::CAENV1730,
							      metadata) );
      TLOG_ARB(TMAKEFRAG,TRACE_NAME) << "Created fragment " << fBoardID << "," << fEvCounter << TLOG_ENDL;
      std::copy(fCircularBuffer.Buffer().begin(),
		fCircularBuffer.Buffer().begin()+event_size/sizeof(uint16_t),
		(uint16_t*)(fragments.back()->dataBeginBytes())); // funziona (by Wes)
      TLOG_ARB(TMAKEFRAG,TRACE_NAME) << "Fragment data copied for event " << fEvCounter << "." << TLOG_ENDL;
      fEvCounter++;
      
      //std::copy(eventptr , eventptr + (nevents - i_e)*metadata.ExpectedDataSize(), fBuffer);        
      fCircularBuffer.Erase(event_size/sizeof(uint16_t));
      //TLOG_ARB(TMAKEFRAG,TRACE_NAME) <<"There are " << nevents << " rw(s) left to analyze. Shifted to front of buffer" << TLOG_ENDL;
      
      if(fCombineReadoutWindows)
	event_size = metadata.ExpectedDataSize();
      else
	event_size = 0;
      prev_rwcounter = 0;
    }
  }

  TLOG_ARB(TGETNEXT,TRACE_NAME) << "Finished processing readout windows." << TLOG_ENDL;

  total_data_size = 0;
  usleep(fGetNextSleep);

  TLOG_ARB(TGETNEXT,TRACE_NAME) << "getNext_() done." << TLOG_ENDL;
  return true;
#endif
}


bool sbndaq::CAENV1730Readout::readSingleWindowFragments(artdaq::FragmentPtrs & fragments){
  TLOG_ARB(TGETNEXT,TRACE_NAME) << "Begin of readSingleWindowFragments()" << TLOG_ENDL;

  constexpr auto sizeof_unit16_t = sizeof(uint16_t);
  const auto available_datasize_bytes= fCircularBuffer.Buffer().size()*sizeof_unit16_t;

  if(! available_datasize_bytes ){
    TLOG_ARB(TGETNEXT,TRACE_NAME) << "CircularBuffer is empty. Sleep for " << fGetNextSleep << " us and return." << TLOG_ENDL;
    ::usleep(fGetNextSleep);
    return true;
  }

  TLOG_ARB(TGETDATA,TRACE_NAME) << "CircularBuffer has " <<  available_datasize_bytes << " bytes of new data."<< TLOG_ENDL;

  struct timespec now;
  clock_gettime(CLOCK_REALTIME,&now);
  const auto metadata = CAENV1730FragmentMetadata(fNChannels,fCAEN.recordLength,now.tv_sec,now.tv_nsec,ch_temps);
  const auto fragment_datasize_bytes = metadata.ExpectedDataSize();
  TLOG_ARB(TMAKEFRAG,TRACE_NAME) << "Created CAENV1730FragmentMetadata with expected data size of " << fragment_datasize_bytes << " bytes." TLOG_ENDL;

  if( available_datasize_bytes % fragment_datasize_bytes ){
    TLOG_WARNING("CAENV1730Readout") << "Last readout window is not fully transferred into CircularBuffer; Will retry after " << fGetNextSleep << " us." << TLOG_ENDL;
    ::usleep(fGetNextSleep);
    return true;
  }

  fCircularBuffer.Linearize();

  const auto available_readoutwindow_count = available_datasize_bytes / fragment_datasize_bytes ;
  int remaining_readoutwindow_count = available_readoutwindow_count;

  while (remaining_readoutwindow_count--){
    const auto readoutwindow_begin = fCircularBuffer.Buffer().begin();
    TLOG(TMAKEFRAG) << "Get Readout Window number " << available_readoutwindow_count - remaining_readoutwindow_count << " of "
      << available_readoutwindow_count << " readoutwindow_begin=" << (void*) &*readoutwindow_begin;

    const auto header = reinterpret_cast<CAENV1730EventHeader const *>(&*readoutwindow_begin);
    const auto readoutwindow_event_counter = header->eventCounter;
    TLOG_ARB(TMAKEFRAG,TRACE_NAME) << "Readout window event counters current / gap  = " << readoutwindow_event_counter  << " / "
      << readoutwindow_event_counter - prev_rwcounter  << TLOG_ENDL;

    auto fragment_uptr=artdaq::Fragment::FragmentBytes(fragment_datasize_bytes,fEvCounter,fBoardID,sbndaq::detail::FragmentType::CAENV1730,metadata);
    TLOG_ARB(TMAKEFRAG,TRACE_NAME) << "Created fragment " << fBoardID << " for event " << fEvCounter << TLOG_ENDL;

    const auto fragment_datasize_words = fragment_datasize_bytes/sizeof_unit16_t;
    const auto readoutwindow_end = readoutwindow_begin + fragment_datasize_words ;
    auto fragment_buffer_begin = reinterpret_cast<uint16_t*> (fragment_uptr->dataBeginBytes());

    std::copy(readoutwindow_begin,readoutwindow_end,fragment_buffer_begin);
    fCircularBuffer.Erase(fragment_datasize_words);
    TLOG_ARB(TMAKEFRAG,TRACE_NAME) << "Fragment data copied for event " << fEvCounter << "." << TLOG_ENDL;

    fragments.emplace_back(nullptr);
    std::swap(fragments.back(),fragment_uptr);

    fEvCounter++;
    prev_rwcounter = readoutwindow_event_counter;
  }

  const auto actual_readout_bytes  = std::accumulate( fragments.begin(), fragments.end(), 0ul,[](auto& a, auto const& f){return a+f->dataSizeBytes();});
  if (actual_readout_bytes != available_datasize_bytes) {
    TLOG_ARB(TGETNEXT,TRACE_NAME) << "Error in readSingleWindowFragments actual_readout_bytes != available_datasize_bytes "
      << actual_readout_bytes << " / " << available_datasize_bytes << TLOG_ENDL;
  }

  ::usleep(fGetNextSleep);
  TLOG_ARB(TGETNEXT,TRACE_NAME) << "End of readSingleWindowFragments()"
    << std::string(fragments.size() != available_readoutwindow_count ? "Error: fragment count != readout window count!":"") << TLOG_ENDL;

  return true;
}



bool  sbndaq::CAENV1730Readout::readCombinedWindowFragments(artdaq::FragmentPtrs &){
throw std::runtime_error("Implement me correctly!");
}

void sbndaq::CAENV1730Readout::CheckReadback(std::string label,
					      int boardID,
					      uint32_t wrote,
					      uint32_t readback,
					      int channelID)
{
  if (wrote != readback){

    std::stringstream channelLabel(" ");
    if (channelID >= 0)
      channelLabel << " Ch/Grp " << channelID;
    
    std::stringstream text;
    text << " " << label << 
      " ReadBack error BoardId " << boardID << channelLabel.str() 
	 << " wrote " << wrote << " read " << readback;
    LOG_ERROR("CAENLightReadout") << text.str();

    //sbndaq::CAENException e(CAEN_DGTZ_DigitizerNotReady,
    //			     text.str(), boardId);
    //throw(e);
  }
  
}

DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(sbndaq::CAENV1730Readout)
