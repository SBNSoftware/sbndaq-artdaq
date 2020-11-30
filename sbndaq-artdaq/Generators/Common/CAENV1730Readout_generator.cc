//
//  sbndaq-artdaq/Generators/Common/CAENV1730Readout_generator.cc
//

#define TRACE_NAME "CAENV1730Readout"
#include "artdaq/DAQdata/Globals.hh"

#include "artdaq/Generators/GeneratorMacros.hh"
#include "sbndaq-artdaq/Generators/Common/CAENV1730Readout.hh"

#include <iostream>
#include <sstream>
#include <time.h>
#include <unistd.h>

#include <algorithm>
#include "CAENDecoder.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
using namespace sbndaq;

// constructor of the CAENV1730Readout. It wants the param set 
// which means the fhicl paramters in CAENV1730Readout.hh

sbndaq::CAENV1730Readout::CAENV1730Readout(fhicl::ParameterSet const& ps) :
  CommandableFragmentGenerator(ps),
  fCAEN(ps),
  fAcqMode(CAEN_DGTZ_SW_CONTROLLED)
{
  uint32_t data;

  TLOG_ARB(TCONFIG,TRACE_NAME) << "CAENV1730Readout()" << TLOG_ENDL;
  TLOG(TCONFIG) << fCAEN;
  loadConfiguration(ps);
  
  last_rcvd_rwcounter=0x0;
  last_sent_rwcounter=0x1;
  CAEN_DGTZ_ErrorCode retcode;

  fail_GetNext=false;

  fNChannels = fCAEN.nChannels;
  fBoardID = fCAEN.boardId;

  TLOG(TCONFIG) << ": Using BoardID=" << fBoardID << " with NChannels=" 
		<< fNChannels;

  retcode = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_OpticalLink, fCAEN.link, 
				    fBoardChainNumber, 0, &fHandle);

  fOK=true;

  if(retcode != CAEN_DGTZ_Success)
  {
    sbndaq::CAENDecoder::checkError(retcode,"OpenDigitizer",fBoardID);
		CAEN_DGTZ_CloseDigitizer(fHandle);
    fHandle = -1;
    fOK = false;
    TLOG(TLVL_ERROR) << ": Fatal error configuring CAEN board at " << 
      fCAEN.link << ", " << fBoardChainNumber;
    TLOG(TLVL_ERROR) << __func__ << ": Terminating process";
    abort();
  }

  retcode = CAEN_DGTZ_Reset(fHandle);
  sbndaq::CAENDecoder::checkError(retcode,"Reset",fBoardID);
  
  sleep(1);
  Configure();

  retcode = CAEN_DGTZ_ReadRegister(fHandle,FP_TRG_OUT_CONTROL,&data);
  TLOG(TLVL_INFO) << "Reg:0x" << std::hex << FP_TRG_OUT_CONTROL << 
    "=0x" << data;

  retcode = CAEN_DGTZ_ReadRegister(fHandle,FP_IO_CONTROL,&data);
  TLOG(TLVL_INFO) << "Reg:0x" << std::hex << FP_IO_CONTROL << "=0x" << data;

  retcode = CAEN_DGTZ_ReadRegister(fHandle,FP_LVDS_CONTROL,&data);
  TLOG(TLVL_INFO) << "Reg:0x" << std::hex << FP_LVDS_CONTROL << "=0x" << 
    data << std::dec;

  if(!fOK)
  {
    CAEN_DGTZ_CloseDigitizer(fHandle);
    TLOG(TLVL_ERROR) << ": Fatal error configuring CAEN board at " << 
      fCAEN.link << ", " << fBoardChainNumber;
    TLOG(TLVL_ERROR) << __func__ << ": Terminating process";
    abort();
  }

  // Set up worker getdata thread.
  share::ThreadFunctor functor = std::bind(&CAENV1730Readout::GetData,this);
  auto worker_functor = share::WorkerThreadFunctorUPtr(new share::WorkerThreadFunctor(functor,"GetDataWorkerThread"));
  auto GetData_worker = share::WorkerThread::createWorkerThread(worker_functor);
  GetData_thread_.swap(GetData_worker);
  TLOG_ARB(TCONFIG,TRACE_NAME) << "GetData worker thread setup." << TLOG_ENDL;

  TLOG(TCONFIG) << "Configuration complete with OK=" << fOK << TLOG_ENDL;
}

void sbndaq::CAENV1730Readout::configureInterrupts() 
{
  CAEN_DGTZ_EnaDis_t  state,stateOut;
  uint8_t             interruptLevel,interruptLevelOut;
  uint32_t            statusId __attribute__((unused)),statusIdOut __attribute__((unused));
 //uint32_t            statusIdOut;
  uint16_t            eventNumber,eventNumberOut;
  CAEN_DGTZ_IRQMode_t mode,modeOut;

  fInterruptLevel =1; //FIXME:GAL remove this line

  if(fInterruptLevel>0){
    state           = CAEN_DGTZ_ENABLE;
    interruptLevel  = 1;
    statusId        = 1;
    eventNumber     = 1;
    mode            = CAEN_DGTZ_IRQ_MODE_RORA; 
  }
  else{
    state           = CAEN_DGTZ_DISABLE;
    interruptLevel  = 0;
    statusId        = 0;
    eventNumber     = 0;
    mode            = CAEN_DGTZ_IRQ_MODE_ROAK; // Default mode
  }

  uint32_t readback=0;
  CAEN_DGTZ_ErrorCode retcode = CAEN_DGTZ_ReadRegister(fHandle,READOUT_CONTROL,&readback);
  CAENDecoder::checkError(retcode,"GetInterruptConfig",fBoardID);
  TLOG(TLVL_INFO) << "CAEN_DGTZ_ReadRegister reg=0xef00 value=" <<  std::bitset<32>(readback) ;

  retcode = CAEN_DGTZ_GetInterruptConfig(fHandle, &stateOut, &interruptLevelOut, &statusIdOut, &eventNumberOut, &modeOut);
  CAENDecoder::checkError(retcode,"GetInterruptConfig",fBoardID);

  uint32_t bitmask = (uint32_t)(0x1FF);

  uint32_t data = (uint32_t)(0x9); //RORA,irq link enabled,vme baseaddress relocation disabled,VME Bus error enabled,level 1
  uint32_t addr = READOUT_CONTROL;
  uint32_t value = 0;
  retcode = CAEN_DGTZ_ReadRegister(fHandle,addr,&value);
  TLOG(TCONFIG) << "CAEN_DGTZ_ReadRegister prior to overwrite of addr=" << std::hex << addr << ", returned value=" << std::bitset<32>(value) ; 
  TLOG_ARB(TCONFIG,TRACE_NAME) << "Setting I/O control register 0x811C " << TLOG_ENDL;
  retcode = sbndaq::CAENV1730Readout::WriteRegisterBitmask(fHandle,addr,data,bitmask);
  sbndaq::CAENDecoder::checkError(retcode,"SetIOControl",fBoardID);
  retcode = CAEN_DGTZ_ReadRegister(fHandle,addr,&value);
  TLOG(TCONFIG) << "CAEN_DGTZ_ReadRegister addr=" << std::hex << addr << 
    " and bitmask=" << std::bitset<32>(bitmask)
                << ", returned value=" << std::bitset<32>(value); 

  TLOG(TLVL_INFO)  << "state=" << uint32_t{ stateOut} << 
    ", interruptLevel=" << uint32_t{interruptLevelOut} 
    << ", statusId=" << uint32_t{statusIdOut} << ", eventNumber=" <<
    uint32_t{eventNumberOut}<<", mode="<< int32_t{modeOut};	
/*
  retcode = CAEN_DGTZ_SetInterruptConfig(fHandle,
                                        state,
                                        interruptLevel,
                                        statusId,
                                        eventNumber,
                                        mode);
  CAENDecoder::checkError(retcode,"SetInterruptConfig",fBoardID);
	*/
  retcode = CAEN_DGTZ_ReadRegister(fHandle,READOUT_CONTROL,&readback);
  CAENDecoder::checkError(retcode,"GetInterruptConfig",fBoardID);
  TLOG(TLVL_INFO) << "CAEN_DGTZ_ReadRegiste reg=READOUT_CONTROL value=" <<  std::bitset<32>(readback) ;

  retcode = CAEN_DGTZ_GetInterruptConfig (fHandle, &stateOut, &interruptLevelOut, &statusIdOut, &eventNumberOut, &modeOut);
  CAENDecoder::checkError(retcode,"GetInterruptConfig",fBoardID);

  if (state != stateOut){
    TLOG_WARNING("CAENV1730Readout") << "Interrupt State was not setup properly, state write/read="
                                     << int32_t{ state} <<"/"<< int32_t{ stateOut};
  }
  if (interruptLevel != interruptLevelOut){
    TLOG_WARNING("CAENV1730Readout") << "Interrupt State was not setup properly, interruptLevel write/read="
                                     << uint32_t{interruptLevel}<< "/" << uint32_t{interruptLevelOut};
  }
  if (eventNumber != eventNumberOut){
    TLOG_WARNING("CAENV1730Readout")
      << "Interrupt State was not setup properly, eventNumber write/read="
      << uint32_t {eventNumber} <<"/"<< uint32_t {eventNumberOut};
  }
  if (mode != modeOut){
    TLOG_WARNING("CAENV1730Readout")
      << "Interrupt State was not setup properly, mode write/read="
      << int32_t { mode }<< "/"<<int32_t { modeOut };
  }

}

void sbndaq::CAENV1730Readout::loadConfiguration(fhicl::ParameterSet const& ps)
{
  // initialize the fhicl parameters (see CAENV1730Readout.hh)
  // the obj ps has a member method that gets he private members
  // fVerbosity, etc.. are priv memb in CAENV1730Readout.hh
  //
  // wes, 16Jan2018: disabling default parameters
  ///
  fFragmentID = ps.get<uint32_t>("fragment_id");
  TLOG(TINFO)<< __func__ << ": fFragmentID=" << fFragmentID;

  fVerbosity = ps.get<int>("Verbosity"); //-1
  TLOG(TINFO) << __func__<< ": Verbosity=" << fVerbosity;

  fBoardChainNumber = ps.get<int>("BoardChainNumber"); //0
  TLOG(TINFO)<<__func__ << ": BoardChainNumber=" << fBoardChainNumber;

  fInterruptLevel = ps.get<uint8_t>("InterruptLevel"); //1
  TLOG(TINFO) << __func__ << ": InterruptLevel=" << fInterruptLevel;

  fInterruptStatusID = ps.get<uint32_t>("InterruptStatusID"); //0
  TLOG(TINFO) << __func__ << ": InterruptStatusID=" << fInterruptStatusID;

  fInterruptEventNumber = ps.get<uint16_t>("InterruptEventNumber"); //1
  TLOG(TINFO) << __func__<< ": InterruptEventNumber=" << fInterruptEventNumber;

  fSWTrigger = ps.get<bool>("SWTrigger"); //false
  TLOG(TINFO)<<__func__ << ": SWTrigger=" << fSWTrigger ;

  fModeLVDS = ps.get<uint32_t>("ModeLVDS"); // LVDS output mode
  TLOG(TINFO)<<__func__ << ": ModeLVDS=" << fModeLVDS;

  fTrigInLevel  = ps.get<uint32_t>("TrigInLevel",0); // TRG_IN on level (1) or edge (0)
  TLOG(TINFO)<<__func__ << ": TrigInLevel=" << fTrigInLevel;

  fSelfTriggerMode = ps.get<uint32_t>("SelfTriggerMode"); 
  TLOG(TINFO)<<__func__ << ": SelfTriggerMode=" << fSelfTriggerMode;

  fSelfTriggerMask = ps.get<uint32_t>("SelfTriggerMask"); 
  TLOG(TINFO)<<__func__ << ": SelfTriggerMask=" << std::hex 
	     << fSelfTriggerMask << std::dec;

  fGetNextSleep = ps.get<uint32_t>("GetNextSleep"); //1000000
  TLOG(TINFO) << __func__<< ": GetNextSleep=" << fGetNextSleep;

  fCircularBufferSize = ps.get<uint32_t>("CircularBufferSize"); //1000000
  TLOG(TINFO) << __func__<< ": CircularBufferSize=" << fCircularBufferSize;

  fCombineReadoutWindows = ps.get<bool>("CombineReadoutWindows");
  TLOG(TINFO) <<__func__ << ": CombineReadoutWindows=" << fCombineReadoutWindows;

  fCalibrateOnConfig = ps.get<bool>("CalibrateOnConfig");
  TLOG(TINFO) <<__func__ <<": CalibrateOnConfig=" << fCalibrateOnConfig;

  fLockTempCalibration = ps.get<bool>("LockTempCalibration");
  TLOG(TINFO) <<__func__ <<": LockTempCalibration=" << fLockTempCalibration;

  fGetNextFragmentBunchSize  = ps.get<uint32_t>("GetNextFragmentBunchSize");
  TLOG(TINFO) <<__func__ <<": fGetNextFragmentBunchSize=" << fGetNextFragmentBunchSize;

  //Animesh & Aiwu add - for LVDS logic settings
  fLVDSLogicValueG1 = ps.get<uint32_t>("LVDSLogicValueG1"); // LVDS logic value for G1
  TLOG(TINFO)<<__func__ << ": LVDSLogicValueG1=" << fLVDSLogicValueG1;
  fLVDSLogicValueG2 = ps.get<uint32_t>("LVDSLogicValueG2"); // LVDS logic value for G2
  TLOG(TINFO)<<__func__ << ": LVDSLogicValueG2=" << fLVDSLogicValueG2;
  fLVDSLogicValueG3 = ps.get<uint32_t>("LVDSLogicValueG3"); // LVDS logic value for G3
  TLOG(TINFO)<<__func__ << ": LVDSLogicValueG3=" << fLVDSLogicValueG3;
  fLVDSLogicValueG4 = ps.get<uint32_t>("LVDSLogicValueG4"); // LVDS logic value for G4
  TLOG(TINFO)<<__func__ << ": LVDSLogicValueG4=" << fLVDSLogicValueG4;
  fLVDSLogicValueG5 = ps.get<uint32_t>("LVDSLogicValueG5"); // LVDS logic value for G5
  TLOG(TINFO)<<__func__ << ": LVDSLogicValueG5=" << fLVDSLogicValueG5;
  fLVDSLogicValueG6 = ps.get<uint32_t>("LVDSLogicValueG6"); // LVDS logic value for G6
  TLOG(TINFO)<<__func__ << ": LVDSLogicValueG6=" << fLVDSLogicValueG6;
  fLVDSLogicValueG7 = ps.get<uint32_t>("LVDSLogicValueG7"); // LVDS logic value for G7
  TLOG(TINFO)<<__func__ << ": LVDSLogicValueG7=" << fLVDSLogicValueG7;
  fLVDSLogicValueG8 = ps.get<uint32_t>("LVDSLogicValueG8"); // LVDS logic value for G8
  TLOG(TINFO)<<__func__ << ": LVDSLogicValueG8=" << fLVDSLogicValueG8;
  //Animesh & Aiwu add end
  //Animesh & Aiwu add - for LVDS output width
  fLVDSOutWidthC1 = ps.get<uint32_t>("LVDSOutWidthC1"); // LVDS output width Ch1
  TLOG(TINFO)<<__func__ << ": LVDSOutWidthC1=" << fLVDSOutWidthC1;
  fLVDSOutWidthC2 = ps.get<uint32_t>("LVDSOutWidthC2"); // LVDS output width Ch2
  TLOG(TINFO)<<__func__ << ": LVDSOutWidthC2=" << fLVDSOutWidthC2;
  fLVDSOutWidthC3 = ps.get<uint32_t>("LVDSOutWidthC3"); // LVDS output width Ch3
  TLOG(TINFO)<<__func__ << ": LVDSOutWidthC3=" << fLVDSOutWidthC3;
  fLVDSOutWidthC4 = ps.get<uint32_t>("LVDSOutWidthC4"); // LVDS output width Ch4
  TLOG(TINFO)<<__func__ << ": LVDSOutWidthC4=" << fLVDSOutWidthC4;
  fLVDSOutWidthC5 = ps.get<uint32_t>("LVDSOutWidthC5"); // LVDS output width Ch5
  TLOG(TINFO)<<__func__ << ": LVDSOutWidthC5=" << fLVDSOutWidthC5;
  fLVDSOutWidthC6 = ps.get<uint32_t>("LVDSOutWidthC6"); // LVDS output width Ch6
  TLOG(TINFO)<<__func__ << ": LVDSOutWidthC6=" << fLVDSOutWidthC6;
  fLVDSOutWidthC7 = ps.get<uint32_t>("LVDSOutWidthC7"); // LVDS output width Ch7
  TLOG(TINFO)<<__func__ << ": LVDSOutWidthC7=" << fLVDSOutWidthC7;
  fLVDSOutWidthC8 = ps.get<uint32_t>("LVDSOutWidthC8"); // LVDS output width Ch8
  TLOG(TINFO)<<__func__ << ": LVDSOutWidthC8=" << fLVDSOutWidthC8;
  fLVDSOutWidthC9 = ps.get<uint32_t>("LVDSOutWidthC9"); // LVDS output width Ch9
  TLOG(TINFO)<<__func__ << ": LVDSOutWidthC9=" << fLVDSOutWidthC9;
  fLVDSOutWidthC10 = ps.get<uint32_t>("LVDSOutWidthC10"); // LVDS output width Ch10
  TLOG(TINFO)<<__func__ << ": LVDSOutWidthC10=" << fLVDSOutWidthC10;
  fLVDSOutWidthC11 = ps.get<uint32_t>("LVDSOutWidthC11"); // LVDS output width Ch11
  TLOG(TINFO)<<__func__ << ": LVDSOutWidthC11=" << fLVDSOutWidthC11;
  fLVDSOutWidthC12 = ps.get<uint32_t>("LVDSOutWidthC12"); // LVDS output width Ch12
  TLOG(TINFO)<<__func__ << ": LVDSOutWidthC12=" << fLVDSOutWidthC12;
  fLVDSOutWidthC13 = ps.get<uint32_t>("LVDSOutWidthC13"); // LVDS output width Ch13
  TLOG(TINFO)<<__func__ << ": LVDSOutWidthC13=" << fLVDSOutWidthC13;
  fLVDSOutWidthC14 = ps.get<uint32_t>("LVDSOutWidthC14"); // LVDS output width Ch14
  TLOG(TINFO)<<__func__ << ": LVDSOutWidthC14=" << fLVDSOutWidthC14;
  fLVDSOutWidthC15 = ps.get<uint32_t>("LVDSOutWidthC15"); // LVDS output width Ch15
  TLOG(TINFO)<<__func__ << ": LVDSOutWidthC15=" << fLVDSOutWidthC15;
  fLVDSOutWidthC16 = ps.get<uint32_t>("LVDSOutWidthC16"); // LVDS output width Ch16
  TLOG(TINFO)<<__func__ << ": LVDSOutWidthC16=" << fLVDSOutWidthC16;
  //Animesh & Aiwu add end
  //Animesh & Aiwu add - self trigger polarity
  fSelfTrigBit = ps.get<uint32_t>("SelfTrigBit"); // LVDS output width Ch16
  TLOG(TINFO)<<__func__ << ": SelfTrigBit=" << fSelfTrigBit;
  fChargePedstalBitCh1 = ps.get<uint32_t>("ChargePedstalBitCh1"); // DPP algorithm feature
  TLOG(TINFO)<<__func__ << ": ChargePedstalBitCh1=" << fChargePedstalBitCh1;
  // dc offset or baseline
  fBaselineCh1 = ps.get<uint32_t>("BaselineCh1"); // ch1 baseline
  TLOG(TINFO)<<__func__ << ": BaselineCh1=" << fBaselineCh1;
  fBaselineCh2 = ps.get<uint32_t>("BaselineCh2"); // ch2 baseline
  TLOG(TINFO)<<__func__ << ": BaselineCh2=" << fBaselineCh2;
  fBaselineCh3 = ps.get<uint32_t>("BaselineCh3"); // ch3 baseline
  TLOG(TINFO)<<__func__ << ": BaselineCh3=" << fBaselineCh3;
  fBaselineCh4 = ps.get<uint32_t>("BaselineCh4"); // ch4 baseline
  TLOG(TINFO)<<__func__ << ": BaselineCh4=" << fBaselineCh4;
  fBaselineCh5 = ps.get<uint32_t>("BaselineCh5"); // ch5 baseline
  TLOG(TINFO)<<__func__ << ": BaselineCh5=" << fBaselineCh5;
  fBaselineCh6 = ps.get<uint32_t>("BaselineCh6"); // ch6 baseline
  TLOG(TINFO)<<__func__ << ": BaselineCh6=" << fBaselineCh6;
  fBaselineCh7 = ps.get<uint32_t>("BaselineCh7"); // ch7 baseline
  TLOG(TINFO)<<__func__ << ": BaselineCh7=" << fBaselineCh7;
  fBaselineCh8 = ps.get<uint32_t>("BaselineCh8"); // ch8 baseline
  TLOG(TINFO)<<__func__ << ": BaselineCh8=" << fBaselineCh8;
  fBaselineCh9 = ps.get<uint32_t>("BaselineCh9"); // ch9 baseline
  TLOG(TINFO)<<__func__ << ": BaselineCh9=" << fBaselineCh9;
  fBaselineCh10 = ps.get<uint32_t>("BaselineCh10"); // ch10 baseline
  TLOG(TINFO)<<__func__ << ": BaselineCh10=" << fBaselineCh10;
  fBaselineCh11 = ps.get<uint32_t>("BaselineCh11"); // ch11 baseline
  TLOG(TINFO)<<__func__ << ": BaselineCh11=" << fBaselineCh11;
  fBaselineCh12 = ps.get<uint32_t>("BaselineCh12"); // ch12 baseline
  TLOG(TINFO)<<__func__ << ": BaselineCh12=" << fBaselineCh12;
  fBaselineCh13 = ps.get<uint32_t>("BaselineCh13"); // ch13 baseline
  TLOG(TINFO)<<__func__ << ": BaselineCh13=" << fBaselineCh13;
  fBaselineCh14 = ps.get<uint32_t>("BaselineCh14"); // ch14 baseline
  TLOG(TINFO)<<__func__ << ": BaselineCh14=" << fBaselineCh14;
  fBaselineCh15 = ps.get<uint32_t>("BaselineCh15"); // ch15 baseline
  TLOG(TINFO)<<__func__ << ": BaselineCh15=" << fBaselineCh15;
  fBaselineCh16 = ps.get<uint32_t>("BaselineCh16"); // ch16 baseline
  TLOG(TINFO)<<__func__ << ": BaselineCh16=" << fBaselineCh16;
  //Aimesh & Aiwu addd end

  fUseTimeTagForTimeStamp = ps.get<bool>("UseTimeTagForTimeStamp",true);
  TLOG(TINFO) <<__func__ <<": fUseTimeTagForTimeStamp=" << fUseTimeTagForTimeStamp;

  fTimeOffsetNanoSec = ps.get<uint32_t>("TimeOffsetNanoSec",0); //0ms by default
  TLOG(TINFO) <<__func__ <<": fTimeOffsetNanoSec=" << fTimeOffsetNanoSec;
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
  sleep(2);

  ConfigureReadout();
  ConfigureRecordFormat();
  ConfigureTrigger();

  if(fAcqMode==CAEN_DGTZ_SW_CONTROLLED){
    TLOG_ARB(TCONFIG,TRACE_NAME) << "Stop Acquisition" << TLOG_ENDL;
    retcode = CAEN_DGTZ_SWStopAcquisition(fHandle);
    sbndaq::CAENDecoder::checkError(retcode,"SWStopAcquisition",fBoardID);
  }

  ConfigureAcquisition();
  configureInterrupts();

  if (fCalibrateOnConfig)     { RunADCCalibration();  }
  // Does lock bit clear on V1730 reset?   If not, always call this routine
  if (fLockTempCalibration )  
  { 
    for ( uint32_t ch=0; ch<CAENConfiguration::MAX_CHANNELS; ch++)
    {
      SetLockTempCalibration(true,ch);
    }
  }

  TLOG_ARB(TCONFIG,TRACE_NAME) << "Configure() done." << TLOG_ENDL;
}

void sbndaq::CAENV1730Readout::RunADCCalibration()
{
  TLOG_ARB(TINFO,TRACE_NAME) << "Running calibration..." << TLOG_ENDL;
  auto retcode = CAEN_DGTZ_Calibrate(fHandle);
  sbndaq::CAENDecoder::checkError(retcode,"Calibrate",fBoardID);
}

//Animesh & Aiwu added function
void sbndaq::CAENV1730Readout::ConfigureOthers(){
  CAEN_DGTZ_ErrorCode retcod = CAEN_DGTZ_Success;
  uint32_t data,readBack,ioMode;

  // Always set output to "New features"

  //retcod = CAEN_DGTZ_WriteRegister(fHandle, FP_LVDS_Logic_G8, fLVDSLogicValueG8);

  retcod = CAEN_DGTZ_WriteRegister(fHandle, DPP_Alo_Feature_Ch1, fChargePedstalBitCh1);
  retcod = CAEN_DGTZ_ReadRegister(fHandle, DPP_Alo_Feature_Ch1, &readBack);
  sbndaq::CAENDecoder::checkError(retcod,"Read_DPP_Algorithm_feature_0x1n80",fBoardID);
  TLOG(TINFO) << "DPP Algorithm feature ch1: 0x" << std::hex << readBack << std::dec;

  retcod = CAEN_DGTZ_ReadRegister(fHandle, DPP_Alo_Feature_Ch2, &readBack);
  TLOG(TINFO) << "DPP Algorithm feature ch2: 0x" << std::hex << readBack << std::dec;

  // set DC offset, or baseline
  //retcod = CAEN_DGTZ_WriteRegister(fHandle, Baseline_Ch1, fBaselineCh1);
  retcod = CAEN_DGTZ_ReadRegister(fHandle, Baseline_Ch1, &readBack);
  TLOG(TINFO) << "DC offset, or baseline of ch1: 0x" << std::hex << readBack << std::dec;
  //retcod = CAEN_DGTZ_WriteRegister(fHandle, Baseline_Ch2, fBaselineCh2);
  retcod = CAEN_DGTZ_ReadRegister(fHandle, Baseline_Ch2, &readBack);
  TLOG(TINFO) << "DC offset, or baseline of ch2: 0x" << std::hex << readBack << std::dec;
  //retcod = CAEN_DGTZ_WriteRegister(fHandle, Baseline_Ch3, fBaselineCh3);
  retcod = CAEN_DGTZ_ReadRegister(fHandle, Baseline_Ch3, &readBack);
  TLOG(TINFO) << "DC offset, or baseline of ch3: 0x" << std::hex << readBack << std::dec;
  //retcod = CAEN_DGTZ_WriteRegister(fHandle, Baseline_Ch4, fBaselineCh4);
  retcod = CAEN_DGTZ_ReadRegister(fHandle, Baseline_Ch4, &readBack);
  TLOG(TINFO) << "DC offset, or baseline of ch4: 0x" << std::hex << readBack << std::dec;
  //retcod = CAEN_DGTZ_WriteRegister(fHandle, Baseline_Ch5, fBaselineCh5);
  retcod = CAEN_DGTZ_ReadRegister(fHandle, Baseline_Ch5, &readBack);
  TLOG(TINFO) << "DC offset, or baseline of ch5: 0x" << std::hex << readBack << std::dec;
  //retcod = CAEN_DGTZ_WriteRegister(fHandle, Baseline_Ch6, fBaselineCh6);
  retcod = CAEN_DGTZ_ReadRegister(fHandle, Baseline_Ch6, &readBack);
  TLOG(TINFO) << "DC offset, or baseline of ch6: 0x" << std::hex << readBack << std::dec;
  //retcod = CAEN_DGTZ_WriteRegister(fHandle, Baseline_Ch7, fBaselineCh7);
  retcod = CAEN_DGTZ_ReadRegister(fHandle, Baseline_Ch7, &readBack);
  TLOG(TINFO) << "DC offset, or baseline of ch7: 0x" << std::hex << readBack << std::dec;
  //retcod = CAEN_DGTZ_WriteRegister(fHandle, Baseline_Ch8, fBaselineCh8);
  retcod = CAEN_DGTZ_ReadRegister(fHandle, Baseline_Ch8, &readBack);
  TLOG(TINFO) << "DC offset, or baseline of ch8: 0x" << std::hex << readBack << std::dec;
  //retcod = CAEN_DGTZ_WriteRegister(fHandle, Baseline_Ch9, fBaselineCh9);
  retcod = CAEN_DGTZ_ReadRegister(fHandle, Baseline_Ch9, &readBack);
  TLOG(TINFO) << "DC offset, or baseline of ch9: 0x" << std::hex << readBack << std::dec;
  //retcod = CAEN_DGTZ_WriteRegister(fHandle, Baseline_Ch10, fBaselineCh10);
  retcod = CAEN_DGTZ_ReadRegister(fHandle, Baseline_Ch10, &readBack);
  TLOG(TINFO) << "DC offset, or baseline of ch10: 0x" << std::hex << readBack << std::dec;
  //retcod = CAEN_DGTZ_WriteRegister(fHandle, Baseline_Ch11, fBaselineCh11);
  retcod = CAEN_DGTZ_ReadRegister(fHandle, Baseline_Ch11, &readBack);
  TLOG(TINFO) << "DC offset, or baseline of ch11: 0x" << std::hex << readBack << std::dec;
  //retcod = CAEN_DGTZ_WriteRegister(fHandle, Baseline_Ch12, fBaselineCh12);
  retcod = CAEN_DGTZ_ReadRegister(fHandle, Baseline_Ch12, &readBack);
  TLOG(TINFO) << "DC offset, or baseline of ch12: 0x" << std::hex << readBack << std::dec;
  //retcod = CAEN_DGTZ_WriteRegister(fHandle, Baseline_Ch13, fBaselineCh13);
  retcod = CAEN_DGTZ_ReadRegister(fHandle, Baseline_Ch13, &readBack);
  TLOG(TINFO) << "DC offset, or baseline of ch13: 0x" << std::hex << readBack << std::dec;
  //retcod = CAEN_DGTZ_WriteRegister(fHandle, Baseline_Ch14, fBaselineCh14);
  retcod = CAEN_DGTZ_ReadRegister(fHandle, Baseline_Ch14, &readBack);
  TLOG(TINFO) << "DC offset, or baseline of ch14: 0x" << std::hex << readBack << std::dec;
  //retcod = CAEN_DGTZ_WriteRegister(fHandle, Baseline_Ch15, fBaselineCh15);
  retcod = CAEN_DGTZ_ReadRegister(fHandle, Baseline_Ch15, &readBack);
  TLOG(TINFO) << "DC offset, or baseline of ch15: 0x" << std::hex << readBack << std::dec;
  //retcod = CAEN_DGTZ_WriteRegister(fHandle, Baseline_Ch16, fBaselineCh16);
  retcod = CAEN_DGTZ_ReadRegister(fHandle, Baseline_Ch16, &readBack);
  TLOG(TINFO) << "DC offset, or baseline of ch16: 0x" << std::hex << readBack << std::dec;

  // test software trigger
  //retcod = CAEN_DGTZ_ReadRegister(fHandle, SWTriggerValue, &readBack);
  //TLOG(TINFO) << "send a software trigger (whatever value is fine): 0x" << std::hex << readBack << std::dec;

}
//Animesh & Aiwu add end

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


void sbndaq::CAENV1730Readout::SetLockTempCalibration(bool onOff, uint32_t ch)
{
  CAEN_DGTZ_ErrorCode retcod;
  uint8_t lock, ctrl;
  TLOG_ARB(TINFO,TRACE_NAME) << "Locking Temperature Calibration Adjustments, channel " << ch << TLOG_ENDL;

  // Following code comes from CAEN
  // enter engineering functions
  retcod = WriteSPIRegister(fHandle, ch, (uint32_t)0x7A, (uint8_t)0x59);
  sbndaq::CAENDecoder::checkError(retcod,"LockTempCalibration",fBoardID);

  retcod = WriteSPIRegister(fHandle, ch, (uint32_t)0x7A, (uint8_t)0x1A);
  sbndaq::CAENDecoder::checkError(retcod,"LockTempCalibration",fBoardID);

  retcod = WriteSPIRegister(fHandle, ch, (uint32_t)0x7A, (uint8_t)0x11);
  sbndaq::CAENDecoder::checkError(retcod,"LockTempCalibration",fBoardID);

  retcod = WriteSPIRegister(fHandle, ch, (uint32_t)0x7A, (uint8_t)0xAC);
  sbndaq::CAENDecoder::checkError(retcod,"LockTempCalibration",fBoardID);
  
  // read lock value
  retcod = ReadSPIRegister (fHandle, ch, (uint32_t)0xA7, &lock);
  sbndaq::CAENDecoder::checkError(retcod,"LockTempCalibration",fBoardID);

  // write lock value
  retcod = WriteSPIRegister(fHandle, ch, (uint32_t)0xA5, lock);
  sbndaq::CAENDecoder::checkError(retcod,"LockTempCalibration",fBoardID);

  // enable lock
  retcod = ReadSPIRegister (fHandle, ch, (uint32_t)0xA4, &ctrl);
  sbndaq::CAENDecoder::checkError(retcod,"LockTempCalibration",fBoardID);

  if (onOff) { ctrl |= 0x4;}  // set bit 2
  else       { ctrl &= ~0x4;}
  retcod = WriteSPIRegister(fHandle, ch, (uint32_t)0xA4, ctrl);
  sbndaq::CAENDecoder::checkError(retcod,"LockTempCalibration",fBoardID);

  retcod = ReadSPIRegister (fHandle, ch, (uint32_t)0xA4, &ctrl);
  sbndaq::CAENDecoder::checkError(retcod,"LockTempCalibration",fBoardID);
}

void sbndaq::CAENV1730Readout::ConfigureSelfTriggerMode()
{
  CAEN_DGTZ_ErrorCode retcod = CAEN_DGTZ_Success;
  uint32_t data,readBack;

  retcod = CAEN_DGTZ_SetChannelSelfTrigger(fHandle,
					   (CAEN_DGTZ_TriggerMode_t)fSelfTriggerMode,
					   fSelfTriggerMask);
  sbndaq::CAENDecoder::checkError(retcod,"SetSelfTriggerMask",fBoardID);
}

void sbndaq::CAENV1730Readout::ConfigureLVDS()
{
  CAEN_DGTZ_ErrorCode retcod = CAEN_DGTZ_Success;
  uint32_t data,readBack,ioMode;

  // Always set output to "New LVDS features"
  retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_IO_CONTROL, &ioMode);
  sbndaq::CAENDecoder::checkError(retcod,"ReadFPOutputConfig",fBoardID);

  // Construct mode mask
  data = fModeLVDS | (fModeLVDS << 4) | (fModeLVDS << 8) | (fModeLVDS << 12);
  TLOG(TINFO) << __func__ << " ModelLVDS: 0x" << 
      std::hex << data << std::dec;
  retcod = CAEN_DGTZ_WriteRegister(fHandle, FP_LVDS_CONTROL, data);
  sbndaq::CAENDecoder::checkError(retcod,"WriteLVDSOutputConfig",fBoardID);

  retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_LVDS_CONTROL, &readBack);
  sbndaq::CAENDecoder::checkError(retcod,"ReadLVDSOutputConfig",fBoardID);

  CheckReadback("LVDSOutputConfig", fBoardID, data, readBack);

  // If TRIGGER mode, send them out TRG-OUT NIM
  if ( fModeLVDS == LVDS_TRIGGER )
  {
    retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_TRG_OUT_CONTROL, &data);
    sbndaq::CAENDecoder::checkError(retcod,"ReadTRGOutputConfig",fBoardID);

    //    data |= ( ENABLE_LVDS_TRIGGER | ENABLE_TRG_OUT );
    // wes and bill commenting out 10/14/2020 to get DaisyChain to work
    //data |= ( ENABLE_TRG_OUT );
    //data &= ~ TRIGGER_LOGIC ; // Choose OR Logic

    retcod = CAEN_DGTZ_WriteRegister(fHandle, FP_TRG_OUT_CONTROL, data);
    sbndaq::CAENDecoder::checkError(retcod,"WriteTRGOutputConfig",fBoardID);

    retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_TRG_OUT_CONTROL, &readBack);
    sbndaq::CAENDecoder::checkError(retcod,"ReadTRGOutputConfig",fBoardID);
    
    TLOG(TINFO) << __func__ << " TrgOutputConfig: 0x" << 
      std::hex << data << std::dec;
    CheckReadback("TRGOutputConfig", fBoardID, data, readBack);

    // Put LVDS into OUTPUT mode and send to TRG-OUT
    ioMode |= (LVDS_IO | ENABLE_NEW_LVDS);
    ioMode &= ~DISABLE_TRG_OUT_LEMO ;
  }
  else
  {
    // Put LVDS into INPUT mode
    ioMode &= ~(LVDS_IO | DISABLE_TRG_OUT_LEMO);
  }

  if ( fTrigInLevel )
  {
    ioMode |= TRG_IN_LEVEL;
  }
  else
  {
    ioMode &= ~(TRG_IN_LEVEL);
  }

  TLOG(TINFO) << __func__ << " FPOutputConfig: 0x" << 
    std::hex << ioMode << std::dec;
  retcod = CAEN_DGTZ_WriteRegister(fHandle, FP_IO_CONTROL, ioMode);
  sbndaq::CAENDecoder::checkError(retcod,"WriteFPOutputConfig",fBoardID);

  retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_IO_CONTROL, &readBack);
  sbndaq::CAENDecoder::checkError(retcod,"ReadFPOutputConfig",fBoardID);

  CheckReadback("FPOutputConfig", fBoardID, ioMode, readBack);



 // Animesh added 

  retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_LVDS_Logic_G1, &readBack);
  TLOG(TINFO) << "Register for G1: 0x" << std::hex << readBack << std::dec;
  retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_LVDS_Logic_G2, &readBack);
  TLOG(TINFO) << "Register for G2: 0x" << std::hex << readBack << std::dec;
  retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_LVDS_Logic_G3, &readBack);
  TLOG(TINFO) << "Register for G3: 0x" << std::hex << readBack << std::dec;
  retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_LVDS_Logic_G4, &readBack);
  TLOG(TINFO) << "Register for G4: 0x" << std::hex << readBack << std::dec;
  retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_LVDS_Logic_G5, &readBack);
  TLOG(TINFO) << "Register for G5: 0x" << std::hex << readBack << std::dec;
  retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_LVDS_Logic_G6, &readBack);
  TLOG(TINFO) << "Register for G6: 0x" << std::hex << readBack << std::dec;
  retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_LVDS_Logic_G7, &readBack);
  TLOG(TINFO) << "Register for G7: 0x" << std::hex << readBack << std::dec;
  retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_LVDS_Logic_G8, &readBack);
  TLOG(TINFO) << "Register for G8: 0x" << std::hex << readBack << std::dec;


  //Animesh & Aiwu add - to set/read registers for LVDS logic values setting
  retcod = CAEN_DGTZ_WriteRegister(fHandle, FP_LVDS_Logic_G1, fLVDSLogicValueG1);
  retcod = CAEN_DGTZ_WriteRegister(fHandle, FP_LVDS_Logic_G2, fLVDSLogicValueG2);
  retcod = CAEN_DGTZ_WriteRegister(fHandle, FP_LVDS_Logic_G3, fLVDSLogicValueG3);
  retcod = CAEN_DGTZ_WriteRegister(fHandle, FP_LVDS_Logic_G4, fLVDSLogicValueG4);
  retcod = CAEN_DGTZ_WriteRegister(fHandle, FP_LVDS_Logic_G5, fLVDSLogicValueG5);
  retcod = CAEN_DGTZ_WriteRegister(fHandle, FP_LVDS_Logic_G6, fLVDSLogicValueG6);
  retcod = CAEN_DGTZ_WriteRegister(fHandle, FP_LVDS_Logic_G7, fLVDSLogicValueG7);
  retcod = CAEN_DGTZ_WriteRegister(fHandle, FP_LVDS_Logic_G8, fLVDSLogicValueG8);

  retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_LVDS_Logic_G1, &readBack);
  TLOG(TINFO) << "LVDS Logic for G1: 0x" << std::hex << readBack << std::dec;
  retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_LVDS_Logic_G2, &readBack);
  TLOG(TINFO) << "LVDS  Logic for G2: 0x" << std::hex << readBack << std::dec;
  retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_LVDS_Logic_G3, &readBack);
  TLOG(TINFO) << "LVDS  Logic for G3: 0x" << std::hex << readBack << std::dec;
  retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_LVDS_Logic_G4, &readBack);
  TLOG(TINFO) << "LVDS  Logic for G4: 0x" << std::hex << readBack << std::dec;
  retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_LVDS_Logic_G5, &readBack);
  TLOG(TINFO) << "LVDS  Logic for G5: 0x" << std::hex << readBack << std::dec;
  retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_LVDS_Logic_G6, &readBack);
  TLOG(TINFO) << "LVDS  Logic for G6: 0x" << std::hex << readBack << std::dec;
  retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_LVDS_Logic_G7, &readBack);
  TLOG(TINFO) << "LVDS  Logic for G7: 0x" << std::hex << readBack << std::dec;
  retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_LVDS_Logic_G8, &readBack);
  TLOG(TINFO) << "LVDS  Logic for G8: 0x" << std::hex << readBack << std::dec;
  //Animesh & Aiwu add ends

  //Animesh & Aiwu add - to set/read registers for LVDS output width values setting
  retcod = CAEN_DGTZ_WriteRegister(fHandle, FP_LVDS_OutWidth_Ch1, fLVDSOutWidthC1);
  retcod = CAEN_DGTZ_WriteRegister(fHandle, FP_LVDS_OutWidth_Ch2, fLVDSOutWidthC2);
  retcod = CAEN_DGTZ_WriteRegister(fHandle, FP_LVDS_OutWidth_Ch3, fLVDSOutWidthC3);
  retcod = CAEN_DGTZ_WriteRegister(fHandle, FP_LVDS_OutWidth_Ch4, fLVDSOutWidthC4);
  retcod = CAEN_DGTZ_WriteRegister(fHandle, FP_LVDS_OutWidth_Ch5, fLVDSOutWidthC5);
  retcod = CAEN_DGTZ_WriteRegister(fHandle, FP_LVDS_OutWidth_Ch6, fLVDSOutWidthC6);
  retcod = CAEN_DGTZ_WriteRegister(fHandle, FP_LVDS_OutWidth_Ch7, fLVDSOutWidthC7);
  retcod = CAEN_DGTZ_WriteRegister(fHandle, FP_LVDS_OutWidth_Ch8, fLVDSOutWidthC8);
  retcod = CAEN_DGTZ_WriteRegister(fHandle, FP_LVDS_OutWidth_Ch9, fLVDSOutWidthC9);
  retcod = CAEN_DGTZ_WriteRegister(fHandle, FP_LVDS_OutWidth_Ch10, fLVDSOutWidthC10);
  retcod = CAEN_DGTZ_WriteRegister(fHandle, FP_LVDS_OutWidth_Ch11, fLVDSOutWidthC11);
  retcod = CAEN_DGTZ_WriteRegister(fHandle, FP_LVDS_OutWidth_Ch12, fLVDSOutWidthC12);
  retcod = CAEN_DGTZ_WriteRegister(fHandle, FP_LVDS_OutWidth_Ch13, fLVDSOutWidthC13);
  retcod = CAEN_DGTZ_WriteRegister(fHandle, FP_LVDS_OutWidth_Ch14, fLVDSOutWidthC14);
  retcod = CAEN_DGTZ_WriteRegister(fHandle, FP_LVDS_OutWidth_Ch15, fLVDSOutWidthC15);
  retcod = CAEN_DGTZ_WriteRegister(fHandle, FP_LVDS_OutWidth_Ch16, fLVDSOutWidthC16);
  
  retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_LVDS_OutWidth_Ch1, &readBack);
  TLOG(TINFO) << "LVDS  Logic output width for Ch1: 0x" << std::hex << readBack << std::dec;
  retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_LVDS_OutWidth_Ch2, &readBack);
  TLOG(TINFO) << "LVDS  Logic output width for Ch2: 0x" << std::hex << readBack << std::dec;
  retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_LVDS_OutWidth_Ch3, &readBack);
  TLOG(TINFO) << "LVDS  Logic output width for Ch3: 0x" << std::hex << readBack << std::dec;
  retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_LVDS_OutWidth_Ch4, &readBack);
  TLOG(TINFO) << "LVDS  Logic output width for Ch4: 0x" << std::hex << readBack << std::dec;
  retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_LVDS_OutWidth_Ch5, &readBack);
  TLOG(TINFO) << "LVDS  Logic output width for Ch5: 0x" << std::hex << readBack << std::dec;
  retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_LVDS_OutWidth_Ch6, &readBack);
  TLOG(TINFO) << "LVDS  Logic output width for Ch6: 0x" << std::hex << readBack << std::dec;
  retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_LVDS_OutWidth_Ch7, &readBack);
  TLOG(TINFO) << "LVDS  Logic output width for Ch7: 0x" << std::hex << readBack << std::dec;
  retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_LVDS_OutWidth_Ch8, &readBack);
  TLOG(TINFO) << "LVDS  Logic output width for Ch8: 0x" << std::hex << readBack << std::dec;
  retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_LVDS_OutWidth_Ch9, &readBack);
  TLOG(TINFO) << "LVDS  Logic output width for Ch9: 0x" << std::hex << readBack << std::dec;
  retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_LVDS_OutWidth_Ch10, &readBack);
  TLOG(TINFO) << "LVDS  Logic output width for Ch10: 0x" << std::hex << readBack << std::dec;
  retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_LVDS_OutWidth_Ch11, &readBack);
  TLOG(TINFO) << "LVDS  Logic output width for Ch11: 0x" << std::hex << readBack << std::dec;
  retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_LVDS_OutWidth_Ch12, &readBack);
  TLOG(TINFO) << "LVDS  Logic output width for Ch12: 0x" << std::hex << readBack << std::dec;
  retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_LVDS_OutWidth_Ch13, &readBack);
  TLOG(TINFO) << "LVDS  Logic output width for Ch13: 0x" << std::hex << readBack << std::dec;
  retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_LVDS_OutWidth_Ch14, &readBack);
  TLOG(TINFO) << "LVDS  Logic output width for Ch14: 0x" << std::hex << readBack << std::dec;
  retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_LVDS_OutWidth_Ch15, &readBack);
  TLOG(TINFO) << "LVDS  Logic output width for Ch15: 0x" << std::hex << readBack << std::dec;
  retcod = CAEN_DGTZ_ReadRegister(fHandle, FP_LVDS_OutWidth_Ch16, &readBack);
  TLOG(TINFO) << "LVDS  Logic output width for Ch16: 0x" << std::hex << readBack << std::dec;
  //Animesh & Aiwu add ends

  //Animesh & Aiwu add - test self trigger polarity
  retcod = CAEN_DGTZ_WriteRegister(fHandle, CONFIG_READ_ADDR, fSelfTrigBit);
  retcod = CAEN_DGTZ_ReadRegister(fHandle, CONFIG_READ_ADDR, &readBack);
  TLOG(TINFO) << "Address 0x8000, values inside: 0x" << std::hex << readBack << std::dec;
  //Animesh & Aiwu end
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

//Taken from wavedump
//  handle : Digitizer handle
//  address: register address
//  data   : value to write to register
//  bitmask: bitmask to override only the bits that need to change while leaving the rest unchanged
CAEN_DGTZ_ErrorCode sbndaq::CAENV1730Readout::WriteRegisterBitmask(int32_t handle, uint32_t address, 
								   uint32_t data, uint32_t bitmask) 
{
  //int32_t ret = CAEN_DGTZ_Success;
  CAEN_DGTZ_ErrorCode  ret = CAEN_DGTZ_Success;
  uint32_t d32 = 0xFFFFFFFF;
  uint32_t d32Out;

  ret = CAEN_DGTZ_ReadRegister(handle, address, &d32);
  if(ret != CAEN_DGTZ_Success){
    TLOG(TLVL_ERROR) << __func__ << ": Failed reading a register; address=0x" << std::hex << address;
    abort();
  }

  data &= bitmask;
  d32 &= ~bitmask;
  d32 |= data;
  ret = CAEN_DGTZ_WriteRegister(handle, address, d32);

  if(ret != CAEN_DGTZ_Success) {
    TLOG(TLVL_ERROR) << __func__ << ": Failed writing a register; address=0x" << std::hex << address;
    abort();
  }

  ret = CAEN_DGTZ_ReadRegister(handle, address, &d32Out);
  if(ret != CAEN_DGTZ_Success){
    TLOG(TLVL_ERROR) << __func__ << ": Failed reading a register; address=0x" << std::hex << address
                     << ", value=" << std::bitset<32>(d32) << ", bitmask=" << std::bitset<32>(bitmask);
    abort();
  }

  if( d32 != d32Out ) {
    TLOG(TLVL_ERROR) << __func__ << ": Read and write values disagree; address=0x" << std::hex << address
                     <<", read value=" << std::bitset<32>(d32Out) << ", write value="<< std::bitset<32>(d32)
                     << ", bitmask="<< std::bitset<32>(bitmask);
    abort();
  }

  return ret;
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
	fPoolBuffer.allocate(fBufferSize,fCircularBufferSize,true);
	fPoolBuffer.debugInfo();
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

  for(uint32_t ch=0; ch<fNChannels; ++ch)
  {

    TLOG_ARB(TCONFIG,TRACE_NAME) << "Set channel " << ch
				 << " trigger threshold to " << fCAEN.triggerThresholds[ch] << TLOG_ENDL;
    retcode = CAEN_DGTZ_SetChannelTriggerThreshold(fHandle,ch,fCAEN.triggerThresholds[ch]); //0x8000
    sbndaq::CAENDecoder::checkError(retcode,"SetChannelTriggerThreshold",fBoardID);
    retcode = CAEN_DGTZ_GetChannelTriggerThreshold(fHandle,ch,&readback);
    CheckReadback("SetChannelTriggerThreshold",fBoardID,fCAEN.triggerThresholds[ch],readback);

    //pulse width only set in pairs
    /* //Aiwu commented out this because LVDS output width is set elsewhere in the ConfigureLVDS() function
    if(ch%2==0)
    {
      TLOG_ARB(TCONFIG,TRACE_NAME) << "Set channels " << ch << "/" << ch+1 
				   << " trigger pulse width to " << fCAEN.triggerPulseWidth << TLOG_ENDL;
      retcode = CAEN_DGTZ_WriteRegister(fHandle,0x1070+(ch<<8),fCAEN.triggerPulseWidth);
      sbndaq::CAENDecoder::checkError(retcode,"SetChannelTriggerPulseWidth",fBoardID);
      retcode = CAEN_DGTZ_ReadRegister(fHandle,0x1070+(ch<<8),&readback);
      CheckReadback("SetChannelTriggerPulseWidth",fBoardID,fCAEN.triggerPulseWidth,readback);
    }
    */
  }
  TLOG_ARB(TCONFIG,TRACE_NAME) << "Set global trigger pulse width to " << fCAEN.triggerPulseWidth << TLOG_ENDL;
  retcode = CAEN_DGTZ_WriteRegister(fHandle,TRG_OUT_WIDTH,fCAEN.triggerPulseWidth);
  sbndaq::CAENDecoder::checkError(retcode,"SetGlobalTriggerPulseWidth",fBoardID);
  // Readback must be channel by channel (see reg doc)
  for ( uint32_t ch=0; ch<CAENConfiguration::MAX_CHANNELS; ch++)
  {
    uint32_t address = TRG_OUT_WIDTH_CH | ( ch << 8 );  
    retcode = CAEN_DGTZ_ReadRegister(fHandle,address,&readback);
    CheckReadback("SetGlobalTriggerPulseWidth",fBoardID,fCAEN.triggerPulseWidth,readback);
  }

  ConfigureLVDS();
  ConfigureOthers(); // Animesh & Aiwu added new function
  ConfigureSelfTriggerMode();

  TLOG_ARB(TCONFIG,TRACE_NAME) << "SetTriggerMode" << fCAEN.extTrgMode << TLOG_ENDL;
  retcode = CAEN_DGTZ_SetExtTriggerInputMode(fHandle,(CAEN_DGTZ_TriggerMode_t)(fCAEN.extTrgMode));
  sbndaq::CAENDecoder::checkError(retcode,"SetExtTriggerInputMode",fBoardID);
  retcode = CAEN_DGTZ_GetExtTriggerInputMode(fHandle,(CAEN_DGTZ_TriggerMode_t *)&readback);
  CheckReadback("SetExtTriggerInputMode", fBoardID,fCAEN.extTrgMode,readback);  

  TLOG_ARB(TCONFIG,TRACE_NAME) << "SetTriggerOverlap" << fCAEN.allowTriggerOverlap << TLOG_ENDL;
  if ( fCAEN.allowTriggerOverlap )
  {
    addr = CONFIG_SET_ADDR;
  }
  else
  {
    addr = CONFIG_CLEAR_ADDR;
  }
  retcode = CAEN_DGTZ_WriteRegister(fHandle, addr, TRIGGER_OVERLAP_MASK);
  sbndaq::CAENDecoder::checkError(retcode,"SetTriggerOverlap",fBoardID);

  //level=1 for TTL, =0 for NIM
  TLOG_ARB(TCONFIG,TRACE_NAME) << "SetIOLevel " << (CAEN_DGTZ_IOLevel_t)(fCAEN.ioLevel) << TLOG_ENDL;
  retcode = CAEN_DGTZ_SetIOLevel(fHandle,(CAEN_DGTZ_IOLevel_t)(fCAEN.ioLevel));
  sbndaq::CAENDecoder::checkError(retcode,"SetIOLevel",fBoardID);
  retcode = CAEN_DGTZ_GetIOLevel(fHandle,(CAEN_DGTZ_IOLevel_t *)&readback);
  CheckReadback("SetIOLevel", fBoardID,fCAEN.ioLevel,readback);

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

  //Global Registers
  TLOG_ARB(TCONFIG,TRACE_NAME) << "SetDyanmicRange " << fCAEN.dynamicRange << TLOG_ENDL;
  mask = (uint32_t)(fCAEN.dynamicRange);
  addr = DYNAMIC_RANGE;
  retcode = CAEN_DGTZ_WriteRegister(fHandle,addr,mask);
  sbndaq::CAENDecoder::checkError(retcode,"SetDynamicRange",fBoardID);

  addr = ACQ_CONTROL;
  retcode = CAEN_DGTZ_WriteRegister(fHandle,addr,uint32_t{0x28});
  sbndaq::CAENDecoder::checkError(retcode,"SetTriggerMode",fBoardID);

  uint32_t value =0;
  retcode = CAEN_DGTZ_ReadRegister(fHandle,addr,&value);
  sbndaq::CAENDecoder::checkError(retcode,"GetTriggerMode",fBoardID);

  TLOG(TCONFIG) << "CAEN_DGTZ_ReadRegister addr=" << std::hex << addr << ", returned value=" << std::bitset<32>(value) ; 

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
  last_sent_rwcounter = 0;

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

  for(size_t ch=0; ch<CAENConfiguration::MAX_CHANNELS; ++ch){
    CAEN_DGTZ_ReadTemperature(fHandle, ch, &(ch_temps[ch]));
    TLOG_ARB(TTEMP,TRACE_NAME) << "Card: " << fBoardID
                               << ", Channel: " << ch
                               << ", temp: " << ch_temps[ch] << "  C"
                               << TLOG_ENDL;
    std::ostringstream tempStream;
    tempStream << "Card: " << fBoardID
               << ", Channel: " << ch << " temp.";
    metricMan->sendMetric(tempStream.str(), int(ch_temps[ch]), "C", 1,
                          artdaq::MetricMode::Average, "CAENV1730");
  }

  return true;
}

bool sbndaq::CAENV1730Readout::GetData() {
  TLOG(TGETDATA)<< __func__ << ": Begin of GetData()";

  CAEN_DGTZ_ErrorCode retcod;

  if(fSWTrigger) {
    usleep(fGetNextSleep);
    TLOG(TGETDATA) << "Sending SW trigger..." << TLOG_ENDL;
    retcod = CAEN_DGTZ_SendSWtrigger(fHandle);
    TLOG(TGETDATA) << "CAEN_DGTZ_SendSWtrigger returned " << int{retcod};
  }

  // read the data from the buffer of the card
  // this_data_size is the size of the acq window
  if(fCAEN.interruptLevel > 0){
  }
  else {
   return readSingleWindowDataBlock();
  }

  return true;
}// CAENV1730Readout::GetData()

bool sbndaq::CAENV1730Readout::readSingleWindowDataBlock() {
  if(fail_GetNext) {
    TLOG(TLVL_ERROR) << __func__ << " : Not calling CAEN_DGTZ_ReadData due a previous critical error...";
    ::usleep(50000);
    return false;
  }

  TLOG(TGETDATA) << __func__<< ": Begin of readSingleWindowDataBlock()";

  metricMan->sendMetric("Free DataBlocks",fPoolBuffer.freeBlockCount(),"fragments",1,artdaq::MetricMode::LastPoint);
  CAEN_DGTZ_ErrorCode retcode = CAEN_DGTZ_IRQWait(fHandle, 500);

  if (retcode == CAEN_DGTZ_Timeout) {
    TLOG(TGETDATA) << __func__ <<  ": Exiting after a timeout";
    return true;
  }
  else if(retcode !=CAEN_DGTZ_Success) {
    TLOG(TLVL_ERROR) << __func__ << ": CAEN_DGTZ_IRQWait returned non zero return code; return code=" << int{retcode};
    std::this_thread::yield();
    return false;
  }

  auto fragment_count=fGetNextFragmentBunchSize;

  while (--fragment_count) {
    auto block =  fPoolBuffer.takeFreeBlock();

    if(!block) {
      TLOG(TLVL_ERROR) << __func__ <<": PoolBuffer is empty; last received trigger sequenceID=" <<last_rcvd_rwcounter;
      TLOG(TLVL_ERROR) << __func__ <<": PoolBuffer status: freeBlockCount=" << fPoolBuffer.freeBlockCount()
                       <<", activeBlockCount=" << fPoolBuffer.activeBlockCount();
      TLOG(TLVL_ERROR) << __func__ <<": Critical error; aborting boardreader process....";				

      fail_GetNext = true;

      std::this_thread::yield();
      return false;
    }

    TLOG(TGETDATA) << __func__ << ": Got a free DataBlock from PoolBuffer";

    uint32_t read_data_size =0;

    TLOG(TGETDATA) << __func__ << ": Calling ReadData(fHandle="<<fHandle<< ",bufp=" << (void*)block->begin
                   << ",&block.size="<<(void*)&(block->size) << ")";

    retcode = CAEN_DGTZ_ReadData(fHandle,CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT,
                                (char*)block->begin,&read_data_size);

    block->verify_redzone();
    block->data_size= read_data_size;

    if (retcode !=CAEN_DGTZ_Success) {
        TLOG(TLVL_ERROR) << __func__ << ": CAEN_DGTZ_ReadData returned non zero return code; return code=" << int{retcode};
        fPoolBuffer.returnFreeBlock(block);
				std::this_thread::yield();
        return false;
    }

    TLOG(TGETDATA) << __func__ <<  ": CAEN_DGTZ_ReadData complete with returned data size " << block->data_size
                   << " retcod=" << int{retcode};

    if (block->data_size == 0) { 
      TLOG(TGETDATA)<< __func__ << ": CAEN_DGTZ_ReadData returned zero data size";
      fPoolBuffer.returnFreeBlock(block);
			std::this_thread::yield();
      return false;
    }

    const auto header = reinterpret_cast<CAENV1730EventHeader const *>(block->begin);
    const size_t header_event_size = sizeof(uint32_t)* header->eventSize; 
    if(block->data_size != header_event_size ) {
      TLOG(TLVL_ERROR)<<__func__ << ": Wrong event size; returned="
                       << block->data_size << ", header=" << header_event_size;
    }


		auto readoutwindow_trigger_counter_gap= uint32_t{header->eventCounter} - last_rcvd_rwcounter;

		if( readoutwindow_trigger_counter_gap > 1u ){
  	  TLOG (TLVL_ERROR) << __func__ << " : Missing triggers; previous trigger sequenceID / gap  = " << last_rcvd_rwcounter << " / "
      << readoutwindow_trigger_counter_gap <<", freeBlockCount=" <<fPoolBuffer.freeBlockCount() 
			<< ", activeBlockCount=" <<fPoolBuffer.activeBlockCount() << ", fullyDrainedCount=" << fPoolBuffer.fullyDrainedCount();
		}

    last_rcvd_rwcounter = uint32_t{header->eventCounter};
    fPoolBuffer.returnActiveBlock(block);
    
    TLOG(TGETDATA) << __func__ << ": CAEN_DGTZ_ReadData returned DataBlock header.eventCounter=" 
			<< header->eventCounter << ", header.eventSize=" << header_event_size;
		}

    return true;
}

// this is really the DAQ part where the server reads data from 
// the card and stores them
bool sbndaq::CAENV1730Readout::getNext_(artdaq::FragmentPtrs & fragments){
  if(fail_GetNext) throw std::runtime_error("Critical error; stopping boardreader process...." ) ;
  return fCombineReadoutWindows ? readCombinedWindowFragments(fragments): readSingleWindowFragments(fragments);
}

bool sbndaq::CAENV1730Readout::readSingleWindowFragments(artdaq::FragmentPtrs & fragments){
  TLOG(TGETNEXT) << __func__ << ": Begin of readSingleWindowFragments()" ;

	static auto start= std::chrono::steady_clock::now();

 	std::chrono::duration<double> delta = std::chrono::steady_clock::now()-start;

  if (delta.count() >0.005*fGetNextFragmentBunchSize) {
     metricMan->sendMetric("Laggy getNext",1,"count",1,artdaq::MetricMode::Accumulate);
     TLOG (TLVL_DEBUG) << __func__ << ": Time spent outside of getNext_() " << delta.count()*1000 << " ms. Last seen fragment sequenceID=" << last_sent_rwcounter;
   }

  if(fPoolBuffer.activeBlockCount() == 0){
    TLOG(TGETNEXT) << __func__ << ": PoolBuffer has no data.  Laast last seen fragment sequenceID=" <<last_sent_rwcounter
                   << "; Sleep for " << fGetNextSleep << " us and return.";
    ::usleep(fGetNextSleep);
    start= std::chrono::steady_clock::now();
    return true;
  }

  double max_fragment_create_time = 0.0;
  double min_fragment_create_time = 10000.0;
  struct timespec now;
  clock_gettime(CLOCK_REALTIME,&now);
  const auto metadata = CAENV1730FragmentMetadata(fNChannels,fCAEN.recordLength,now.tv_sec,now.tv_nsec,ch_temps);
  const auto fragment_datasize_bytes = metadata.ExpectedDataSize();
  TLOG(TMAKEFRAG)<< __func__ << ": Created CAENV1730FragmentMetadata with expected data size of "
                 << fragment_datasize_bytes << " bytes.";

  auto fragment_count=fGetNextFragmentBunchSize;

  while(--fragment_count && fPoolBuffer.activeBlockCount()){

    start= std::chrono::steady_clock::now();
//    TLOG(21) << __func__ << ": b4 FragmentBytes";
    auto fragment_uptr=artdaq::Fragment::FragmentBytes(fragment_datasize_bytes,fEvCounter,fFragmentID,sbndaq::detail::FragmentType::CAENV1730,metadata);


    using sbndaq::PoolBuffer;
    PoolBuffer::DataRange<decltype(artdaq::Fragment())> range{fragment_uptr->dataBegin(),fragment_uptr->dataEnd()};

//	TLOG(21) << __func__ << ": b4 fPoolBuffer.read(range)";
    if(!fPoolBuffer.read(range)) break;

    const auto header = reinterpret_cast<CAENV1730EventHeader const *>(fragment_uptr->dataBeginBytes());
    const auto readoutwindow_event_counter = uint32_t {header->eventCounter};
    fragment_uptr->setSequenceID(readoutwindow_event_counter);

    if(fUseTimeTagForTimeStamp){
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
      
      artdaq::Fragment::timestamp_t ts = (t_truetriggertime*8); //in 1ns ticks
      TLOG_ARB(TMAKEFRAG,TRACE_NAME) << "fragment timestamp in 1ns ticks = " << ts << TLOG_ENDL;
      fragment_uptr->setTimestamp( ts );
    }
    else{
      using namespace boost::gregorian;
      using namespace boost::posix_time;
      
      ptime t_now(microsec_clock::universal_time());
      ptime time_t_epoch(date(1970,1,1));
      time_duration diff = t_now - time_t_epoch;

      artdaq::Fragment::timestamp_t ts = diff.total_nanoseconds() - fTimeOffsetNanoSec;;

      TLOG_ARB(TMAKEFRAG,TRACE_NAME) << "fragment timestamp in 1ns ticks = " << ts << TLOG_ENDL;
      fragment_uptr->setTimestamp( ts );
    }

    auto readoutwindow_event_counter_gap= readoutwindow_event_counter - last_sent_rwcounter;

    TLOG(TMAKEFRAG)<<__func__ << ": Created fragment " << fFragmentID << " for event " << readoutwindow_event_counter
                   << " triggerTimeTag " << header->triggerTimeTag ;


    if( readoutwindow_event_counter_gap > 1u ){
      if ( last_sent_rwcounter > 0 )
      {
	TLOG (TLVL_ERROR) << __func__ << ": Missing data; previous fragment sequenceID / gap  = " << last_sent_rwcounter << " / "
                        << readoutwindow_event_counter_gap;
	metricMan->sendMetric("Missing Fragments", uint64_t{readoutwindow_event_counter_gap}, "frags", 1, artdaq::MetricMode::Accumulate);
      }
    }

    fragments.emplace_back(nullptr);
    std::swap(fragments.back(),fragment_uptr);

    fEvCounter++;
    last_sent_rwcounter = readoutwindow_event_counter;
    delta = std::chrono::steady_clock::now()-start;

    min_fragment_create_time=std::min(delta.count(),min_fragment_create_time);
    max_fragment_create_time=std::max(delta.count(),max_fragment_create_time);

    if (delta.count() >0.0005 ) {
      metricMan->sendMetric("Laggy Fragments",1,"frags",1,artdaq::MetricMode::Maximum);
      TLOG (TLVL_DEBUG) << __func__ << ": Creating a fragment with setSequenceID=" << last_sent_rwcounter <<  " took " << delta.count()*1000 << " ms";
//TRACE_CNTL("modeM", 0);
    }
  }

  metricMan->sendMetric("Fragment Create Time  Max",max_fragment_create_time,"s",1,artdaq::MetricMode::Accumulate);
 // metricMan->sendMetric("Fragment Create Time  Min" ,min_fragment_create_time,"s",1,artdaq::MetricMode::Accumulate);

  checkHWStatus_();

  TLOG(TGETNEXT) << __func__<< ": End of readSingleWindowFragments(); returning " << fragments.size() << " fragments.";

  start= std::chrono::steady_clock::now();

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
    TLOG(TLVL_ERROR ) << __func__ <<": " << text.str();

    //sbndaq::CAENException e(CAEN_DGTZ_DigitizerNotReady,
    //			     text.str(), boardId);
    //throw(e);
  }
  
}

DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(sbndaq::CAENV1730Readout)
