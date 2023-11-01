#define TRACE_NAME "PhysCrateData"
#include "sbndaq-artdaq/Generators/ICARUS/PhysCrateData.hh"
#include "artdaq/Generators/GeneratorMacros.hh"

#include <cerrno>
#include <iomanip>
#include <iterator>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "sbndaq-artdaq-core/Trace/trace_defines.h"

#include "icarus-base/PhysCrate.h"
#include "icarus-base/A2795.h"
#include "CAENComm.h"

icarus::PhysCrateData::PhysCrateData(fhicl::ParameterSet const & ps)
  :
  PhysCrate_GeneratorBase(ps),
  veto_host(ps.get<std::string>("VetoHost")),
  veto_host_port(ps.get<int>("VetoPort")),
  veto_udp(veto_host.c_str(),veto_host_port),
  _testPulse(static_cast<TestPulseType>(ps.get<int>("TestPulseType",(int)TestPulseType::kDisable))),
  _doRedis(ps.get<bool>("doRedis",false)),
  _redisHost(ps.get<std::string>("redisHost","localhost")),
  _redisPort(ps.get<int>("redisPort",6379)),
  _issueStart(ps.get<bool>("issueStart",true)),
  _readTemps(ps.get<bool>("readTemps",true)),
  _compressionScheme(ps.get<size_t>("CompressionScheme",0))
{
  InitializeHardware();
  InitializeVeto();

  assignBoardID_ = ps.get<bool>("AssignBoardID", false);
  if ( !assignBoardID_ ) BoardIDs_.assign( physCr->getBoardIDs().begin(), physCr->getBoardIDs().end() );

  // Set up worker getdata thread.
  share::ThreadFunctor functor = std::bind(&PhysCrateData::GetData, this);
  auto worker_functor = share::WorkerThreadFunctorUPtr(new share::WorkerThreadFunctor(functor,"GetDataWorkerThread"));
  auto GetData_worker = share::WorkerThread::createWorkerThread(worker_functor);
  GetData_thread_.swap(GetData_worker);

  if(_doRedis){
    _redisCtxt = redisConnect(_redisHost.c_str(),_redisPort);
    if (_redisCtxt == NULL || _redisCtxt->err) {
      if (_redisCtxt) {
        TLOG(TLVL_ERROR) <<"Error: " <<  _redisCtxt->errstr;
	// handle error
      } else {
        TLOG(TLVL_DEBUG+20) << "Can't allocate redis context";
      }
      
      throw cet::exception("PhysCrateData") << "Could not setup redis context without error";
    }
  }


}

icarus::PhysCrateData::~PhysCrateData()
{
  if(_doRedis)
    redisFree(_redisCtxt);
}

void icarus::PhysCrateData::InitializeVeto(){
  //veto_state     = true; //defaults to on
  veto_state = false;
  
  TRACE(TLVL_INFO,"IP ADDRESS for veto is %s:%d\n",veto_host.c_str(),veto_host_port);
  
  _doVetoTest    = ps_.get<bool>("DoVetoTest",false);
  if(_doVetoTest){
    _vetoTestPeriod = ps_.get<unsigned int>("VetoTestPeriod",1e6);
    share::ThreadFunctor functor = std::bind(&PhysCrateData::VetoTest,this);
    auto worker_functor = share::WorkerThreadFunctorUPtr(new share::WorkerThreadFunctor(functor,"VetoTestWorkerThread"));
    auto veto_worker = share::WorkerThread::createWorkerThread(worker_functor);
    _vetoTestThread.swap(veto_worker);
  }
}

void icarus::PhysCrateData::ForceReset()
{
  for(int ib=0; ib<physCr->NBoards(); ++ib){
    auto bdhandle = physCr->BoardHandle(ib);
    CAENComm_Write32(bdhandle, A_Signals, 0x1);
  }
}

// if we were to use more types of compressions schema this would likely need an arguement, but as it stands it's just a toggle
void icarus::PhysCrateData::SetCompressionBits()
{
  for(int ib=0; ib<physCr->NBoards(); ++ib){
    auto bdhandle = physCr->BoardHandle(ib);
    uint32_t ctrlReg;
    CAENComm_Read32(bdhandle, A_ControlReg, (uint32_t*) &ctrlReg);
    ctrlReg |= 0x20;
    CAENComm_Write32(bdhandle, A_ControlReg, ctrlReg);
  }
}

void icarus::PhysCrateData::ForceClear()
{
  for(int ib=0; ib<physCr->NBoards(); ++ib){
    auto bdhandle = physCr->BoardHandle(ib);
    CAENComm_Write32(bdhandle, A_Signals, 0x2);
  }
}

void icarus::PhysCrateData::SetDCOffset()
{
  std::vector< uint16_t > dc_offset_c = ps_.get< std::vector< uint16_t > >("DACOffset1", std::vector< uint16_t >( nBoards_, 0x8000 ) );
  std::vector< uint16_t > dc_offset_d = ps_.get< std::vector< uint16_t > >("DACOffset2", std::vector< uint16_t >( nBoards_, 0x8000 ) );

  if ( dc_offset_c.size() < nBoards_ )
    throw cet::exception("PhysCrateData") << "Ask to set DAC offset CH[31..0] for " << dc_offset_c.size() << " boards while "
            << nBoards_ << " boards registered.";
  if ( dc_offset_d.size() < nBoards_ )
    throw cet::exception("PhysCrateData") << "Ask to set DAC offset CH[63..32] for " << dc_offset_d.size() << " boards while "
            << nBoards_ << " boards registered.";


  for(int ib=0; ib<physCr->NBoards(); ++ib){
    auto bdhandle = physCr->BoardHandle(ib);
    
    CAENComm_Write32(bdhandle, A_DAC_CTRL, 0x00020000 | (dc_offset_c[ib] & 0xFFFF));
    CAENComm_Write32(bdhandle, A_DAC_CTRL, 0x00030000 | (dc_offset_d[ib] & 0xFFFF));
    
    uint32_t offset_c, offset_d;
    int res1, res2;
    res1 = CAENComm_Read32( bdhandle, A_DAC_C, (uint32_t*) &offset_c );
    res2 = CAENComm_Read32( bdhandle, A_DAC_D, (uint32_t*) &offset_d );
    TRACEN("PhysCrateData",TLVL_DEBUG+1,"Board %d, Offset 1 (Err,Val)=(%d,%ul), Offset 2 (Err,Val)=(%d,%ul)",ib,res1,offset_c,res2,offset_d);

  }
}

void icarus::PhysCrateData::SetTestPulse()
{

  std::vector< uint16_t > dc_offset_a = ps_.get< std::vector< uint16_t > >("TestPulseAmpODD", std::vector< uint16_t >( nBoards_, 0x8000 ) );
  std::vector< uint16_t > dc_offset_b = ps_.get< std::vector< uint16_t > >("TestPulseAmpEVEN", std::vector< uint16_t >( nBoards_, 0x8000 ) );

  //if not disabling, check to make sure we have the right nBoards
  if(_testPulse!=TestPulseType::kDisable){
    if ( dc_offset_a.size() < nBoards_ )
      throw cet::exception("PhysCrateData") << "Ask to set Internal Test Pulse Amplitude ODD for " << dc_offset_a.size() 
					    << " boards while " << nBoards_ << " boards registered.";
    if ( dc_offset_b.size() < nBoards_ )
      throw cet::exception("PhysCrateData") << "Ask to set Internal Test Pulse Amplitude EVEN for " << dc_offset_b.size() 
					    << " boards while " << nBoards_ << " boards registered.";
  }

  for(int ib=0; ib<physCr->NBoards(); ++ib){
    auto bdhandle = physCr->BoardHandle(ib);

    //if disable, set disable
    if(_testPulse==TestPulseType::kDisable)
      CAENComm_Write32(bdhandle, A_RELE, RELE_TP_DIS);

    //if external, set that
    else if (_testPulse==TestPulseType::kExternal)
      CAENComm_Write32(bdhandle, A_RELE, RELE_TP_EXT);

    //internal pulses...
    else if (_testPulse==TestPulseType::kInternal_Even){
      CAENComm_Write32(bdhandle, A_RELE, RELE_TP_INT);
      usleep(1000000);
      CAENComm_Write32(bdhandle, A_RELE, RELE_TP_EVEN);

      //dc offset for even channel test pulse
      CAENComm_Write32(bdhandle, A_DAC_CTRL, 0x00010000 | (dc_offset_b[ib] & 0xFFFF));
    }

    else if (_testPulse==TestPulseType::kInternal_Odd){
      CAENComm_Write32(bdhandle, A_RELE, RELE_TP_INT);
      usleep(1000000);
      CAENComm_Write32(bdhandle, A_RELE, RELE_TP_ODD);
    
      //dc offset for odd channel test pulse
      CAENComm_Write32(bdhandle, A_DAC_CTRL, 0x00000000 | (dc_offset_a[ib] & 0xFFFF));
    }

  }

}
  
void icarus::PhysCrateData::VetoOn(){
  TRACEN("PhysCrateData",TLVL_DEBUG+2,"VetoOn called.");

  int result = veto_udp.VetoOn();
  TRACEN("PhysCrateData",TLVL_DEBUG+3,"VetoOn called. Result %d",result);
  if(result<0)
    TRACEN("PhysCrateData",TLVL_DEBUG+4,"VetoOn Error: %s",std::strerror(errno));

  veto_state = true;
}

void icarus::PhysCrateData::VetoOff(){
  TRACEN("PhysCrateData",TLVL_DEBUG+2,"VetoOff called.");

  int result = veto_udp.VetoOff();
  TRACEN("PhysCrateData",TLVL_DEBUG+3,"VetoOff called. Result %d",result);
  if(result<0)
    TRACEN("PhysCrateData",TLVL_DEBUG+4,"VetoOff Error: %s",std::strerror(errno));

  veto_state = false;
}

void icarus::PhysCrateData::InitializeHardware(){
  physCr = std::make_unique<PhysCrate>();
  physCr->initialize(pcieLinks_);
  this->nBoards_ = (uint16_t)(physCr->NBoards());
  ForceReset();
  if(_compressionScheme != 0) SetCompressionBits(); // CompressionScheme is set up to mulitple compression schema, but currently there are only two

  SetDCOffset();
  SetTestPulse();
}


void icarus::PhysCrateData::ConfigureStart(){

  _tloop_start = std::chrono::high_resolution_clock::now();
  _tloop_end = std::chrono::high_resolution_clock::now();
  
  if(_issueStart)
    physCr->start();

  if(_doVetoTest)
    _vetoTestThread->start();

  GetData_thread_->start();

}

void icarus::PhysCrateData::ConfigureStop(){
  if(_doVetoTest)
    _vetoTestThread->stop();

  GetData_thread_->stop();
}

bool icarus::PhysCrateData::Monitor(){ 

  char* tmp_str = (char*)malloc(150);
  uint16_t busy_mask=0,gbusy_mask=0;
  int n_busy=0,n_gbusy=0,n_running=0,n_ready=0;

  for(int ib=0; ib<physCr->NBoards(); ++ib){
    auto status = physCr->BoardStatus(ib);
    //std::cout << "Board " << ib << " status is " << status << std::endl;
    if( (status & 0x00000010) ) {
      //std::cout << "BUSY on " << ib << "!" << std::endl;
      ++n_running;
    }
    if( (status & 0x00000020) ) {
      //std::cout << "BUSY on " << ib << "!" << std::endl;
      ++n_ready;
    }
    if( (status & 0x00000040) ) {
      //std::cout << "BUSY on " << ib << "!" << std::endl;
      busy_mask = busy_mask | (0x1 << ib);
      ++n_busy;
    }
    if( (status & 0x00000080) ){
      //std::cout << "GBUSY on " << ib << "!" << std::endl;
      gbusy_mask = gbusy_mask | (0x1 << ib);
      ++n_gbusy;
    }

    if(_readTemps){
      physCr->BoardTemps(ib,BoardTemps1_[ib],BoardTemps2_[ib]);

      sprintf(tmp_str,".Temp.Board%d.Temp1",ib);
      metricMan->sendMetric(tmp_str,BoardTemps1_[ib],"C",11,artdaq::MetricMode::Average);

      sprintf(tmp_str,".Temp.Board%d.Temp2",ib);
      metricMan->sendMetric(tmp_str,BoardTemps2_[ib],"C",11,artdaq::MetricMode::Average);
    }

  }

  struct timeval ts;
  gettimeofday(&ts,NULL);

  if(_doRedis){
    //redisReply reply;
    if(busy_mask){
      //std::cout << "BUSY CONDITION!" << std::endl;
      sprintf(tmp_str,"%s:%s_%s_%ul_%s_%ld.%ld","ARTDAQ","STATUSMSG","ICARUSTPCTEST",busy_mask,"BAD",ts.tv_sec,ts.tv_usec);
      //reply = 
      redisCommand(_redisCtxt, "SET IM:STATUSMSG_ICARUSTPCTEST %s", tmp_str);
    }
    else{
      sprintf(tmp_str,"%s:%s_%s_%ul_%s_%ld.%ld","ARTDAQ","STATUSMSG","ICARUSTPCTEST",busy_mask,"GOOD",ts.tv_sec,ts.tv_usec);
      //reply =
      redisCommand(_redisCtxt, "SET IM:STATUSMSG_ICARUSTPCTEST %s", tmp_str);
    }
  }

  metricMan->sendMetric(".Status.N_Running",n_running,"boards",11,artdaq::MetricMode::Minimum|artdaq::MetricMode::Maximum|artdaq::MetricMode::LastPoint); //use minimum for running
  metricMan->sendMetric(".Status.N_DataReady",n_ready,"boards",11,artdaq::MetricMode::Minimum|artdaq::MetricMode::Maximum|artdaq::MetricMode::LastPoint); //use last point for ready...
  metricMan->sendMetric(".Status.N_Busy",n_busy,"boards",11,artdaq::MetricMode::Minimum|artdaq::MetricMode::Maximum|artdaq::MetricMode::LastPoint); //maximum for the busy signals...
  metricMan->sendMetric(".Status.N_GBusy",n_gbusy,"boards",11,artdaq::MetricMode::Minimum|artdaq::MetricMode::Maximum|artdaq::MetricMode::LastPoint);

  
  /*
  if(!reply)
    std::cout << "WE HAD AN ERROR!" << std::endl;
  */

  free(tmp_str);

  usleep(10000);
  return true; 
}

bool icarus::PhysCrateData::VetoTest(){
  
  if(veto_state)
    VetoOff();
  else
    VetoOn();
  usleep(_vetoTestPeriod);
  
  return true;
}

int icarus::PhysCrateData::GetData(){

  TRACEN("PhysCrateData",TLVL_DEBUG+5,"GetData called.");

  physCr->ArmTrigger();

  size_t data_size_bytes = 0;

  //end loop timer
  _tloop_end = std::chrono::high_resolution_clock::now();
  UpdateDuration();
  TRACEN("PhysCrateData",TLVL_DEBUG+6,"GetData : waitData loop time was %lf seconds",_tloop_duration.count());
  metricMan->sendMetric(".GetData.ReturnTime",_tloop_duration.count()*1000.,"ms",11,
			artdaq::MetricMode::LastPoint | artdaq::MetricMode::Maximum | artdaq::MetricMode::Average);

  TRACEN("PhysCrateData",TLVL_DEBUG+7,"GetData : Calling waitData()");
  physCr->waitData();

  //start loop timer
  _tloop_start = std::chrono::high_resolution_clock::now();

  _tloop_duration = std::chrono::duration_cast< std::chrono::duration<double> >(_tloop_start-_tloop_end);
  TRACEN("PhysCrateData",TLVL_DEBUG+8,"GetData : waitData call time was %lf seconds",_tloop_duration.count());
  metricMan->sendMetric(".GetData.WaitTime",_tloop_duration.count()*1000.,"ms",11,
			artdaq::MetricMode::LastPoint | artdaq::MetricMode::Maximum | artdaq::MetricMode::Average);

  // Yun-Tse: ugly and tentative workaround at this moment...  need to change!!
  // int iBoard = 0, nBoards_ = 2;
      
  while(physCr->dataAvail()){
    TRACEN("PhysCrateData",TLVL_DEBUG+9,"GetData : DataAvail!");
    auto data_ptr = physCr->getData();
    
    size_t const this_data_size_bytes = ntohl( data_ptr->Header.packSize );
    TLOG(TLVL_DEBUG+10) << "PhysCrateData::GetData : Data acquired! Size is " << this_data_size_bytes << " bytes, with " 
                     << data_size_bytes << " bytes already acquired.";

    if( this_data_size_bytes == 32 ) continue;

    // ++iBoard;
    
    TLOG(TLVL_DEBUG+11) << "PhysCrateData: data_size_bytes: " << std::dec << data_size_bytes 
              << ", this_data_size_bytes: " << this_data_size_bytes
              << ", token: " << std::hex << data_ptr->Header.token << ", info1: " << data_ptr->Header.info1 
              << ", info2: " << data_ptr->Header.info2 << ", info3: " << data_ptr->Header.info3 
              << ", timeinfo: " << data_ptr->Header.timeinfo << ", chID: " << data_ptr->Header.chID;

   
    // auto ev_ptr = reinterpret_cast<uint32_t*>(data_ptr->data);    
    // TRACEN("PhysCrateData",TLVL_DEBUG,"GetData : Data event number is %#8X",*ev_ptr);
    
    auto const* board_block = reinterpret_cast< A2795DataBlock const * >( data_ptr->data );
    TLOG(TLVL_DEBUG+12) << "PhysCrateData: event_number: " << board_block->header.event_number 
              << ", time_stamp: " << board_block->header.time_stamp;

    // if ( iBoard == nBoards_ ) {
    //   fCircularBuffer.Insert( data_size, reinterpret_cast<uint16_t const*>(data_ptr) );
    //   iBoard = 0;
    //   data_size = 0;
    // }
    fCircularBuffer.Insert( this_data_size_bytes/sizeof(uint16_t), reinterpret_cast<uint16_t const*>(data_ptr) );
    data_size_bytes += this_data_size_bytes;
    TLOG(TLVL_DEBUG+13) << "PhysCrateData::GetData: Data copied! Size was " << this_data_size_bytes << " bytes, with "
                     << data_size_bytes << " bytes now acquired.";

  }
  
  TRACEN("PhysCrateData",TLVL_DEBUG+14,"GetData completed. Status %d, Data size %lu bytes",0,data_size_bytes);
  metricMan->sendMetric(".GetData.DataAcquiredSize",data_size_bytes,"bytes",11,artdaq::MetricMode::Average | artdaq::MetricMode::Maximum | artdaq::MetricMode::Minimum);

  //start loop timer
  _tloop_start = std::chrono::high_resolution_clock::now();

  _tloop_duration = std::chrono::duration_cast< std::chrono::duration<double> >(_tloop_start-_tloop_end);
  TRACEN("PhysCrateData",TLVL_DEBUG+15,"GetData : waitData fill time was %lf seconds",_tloop_duration.count());
  metricMan->sendMetric(".GetData.FillTime",_tloop_duration.count()*1000.,"ms",11,
			artdaq::MetricMode::LastPoint | artdaq::MetricMode::Maximum | artdaq::MetricMode::Average);

  metricMan->sendMetric(".GetData.CircularBufferOccupancy",fCircularBuffer.Size()/2,"bytes",11,
			artdaq::MetricMode::LastPoint|artdaq::MetricMode::Maximum|artdaq::MetricMode::Minimum|artdaq::MetricMode::Average);

  if(data_size_bytes==0 && veto_state)
    VetoOff();

  return 0;
}

void icarus::PhysCrateData::FillStatPack( statpack & pack )
{
  memcpy(&(pack.token), "STAT", 4);
  //pack->crateid = htonl(158);
  pack.crateid = htonl(physCr->getId());
  pack.memstat1 = 0;
  pack.memstat2 = 0;
  pack.size = htonl(28);

  TRACEN("PhysCrateData",TLVL_DEBUG+16,"statpack initilized...");

  //return pack;
}

DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(icarus::PhysCrateData) 
