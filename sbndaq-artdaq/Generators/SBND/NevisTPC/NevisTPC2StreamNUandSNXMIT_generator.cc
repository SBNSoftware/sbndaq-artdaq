#define TRACE_NAME "NevisTPCGenerator2StreamNUandSNXMIT"

#include "artdaq/DAQdata/Globals.hh"
#include "artdaq/Generators/GeneratorMacros.hh"
#include "sbndaq-artdaq/Generators/SBND/NevisTPC/NevisTPC2StreamNUandSNXMIT.hh"

#include <chrono>
#include <ctime>
#include <zmq.hpp>
#include <fstream>
#include <unistd.h>

void sbndaq::NevisTPC2StreamNUandSNXMIT::ConfigureStart() {
  TLOG(TLVL_INFO) << "ConfigureStart";

  fChunkSize             = ps_.get<int>("ChunkSize", 4096);
  fMonitorPeriod         = ps_.get<int>("MonitorPeriod", 12);
  fCALIBFreq             = ps_.get<double>("CALIBTriggerFrequency", -1);
  fControllerTriggerFreq = ps_.get<double>("ControllerTriggerFrequency", -1);
  fDumpBinary            = ps_.get<bool>("DumpBinary", false);
  fDumpSNBinary            = ps_.get<bool>("DumpSNBinary", false);
  fDumpBinaryDir         = ps_.get<std::string>("DumpBinaryDir", ".");
  fSNReadout             = ps_.get<bool>("DoSNReadout", true);
  fDisableNUStream       = ps_.get<bool>("DisableNUStream", false);
  fSNChunkSize           = ps_.get<int>("SNChunkSize", 100000);
  fGPSTimeFreq           = ps_.get<double>("GPSTimeFrequency", -1);
  fGPSZMQPortNTB         = ps_.get<std::string>("GPSZMQPortNTB", "tcp://10.226.36.6:11212");
  fUseZMQ                = ps_.get<bool>("UseZMQ",false);

  SNDMABuffer_.reset(new uint16_t[fSNChunkSize]);
  SNCircularBuffer_ = CircularBuffer(1e9/sizeof(uint16_t)); // to do: define in fcl
  SNCircularBuffer_.Init();
  SNBuffer_ = new uint16_t[fSNChunkSize];
  N_SNDMAs = 0;
  N_NUDMAs = 0;
  N_SNWrites = 0;
  SNDMATransferCnt_ = 0;
  SNBinSubFileNum_  = -1;
  t = time(0);
  ltm = *localtime( &t );


  //std::string connectionString = "tcp://10.226.36.6:" + std::to_string(fGPSZMQPortNTB);

  if(fUseZMQ){
      _zmqGPSPublisher.bind(fGPSZMQPortNTB);} // This port can be configured in fcl file and need to change the localhost to -daq subnet  to find daq subnet, ifconfig and choose ino2 10.226.36.6
      // Any port > 10000 can be used by artdaq (netstat -lpnu4 --> this will tell you the used ports)
      //    publisher.bind("udp://127.0.0.1:7620");

  if( fDumpBinary ){
    // Get timestamp for binary file name
   // time_t t = time(0);
   // struct tm ltm = *localtime( &t );
    sprintf(binFileNameNU, "%s/sbndrawbin_run%06i_%4i.%02i.%02i-%02i.%02i.%02i_TPC_NU.dat",
	    fDumpBinaryDir.c_str(), sbndaq::NevisTPC_generatorBase::run_number(), 
	    ltm.tm_year + 1900, ltm.tm_mon + 1, ltm.tm_mday, ltm.tm_hour, ltm.tm_min, ltm.tm_sec);
    
    TLOG(TLVL_INFO)<< "Opening raw binary file " << binFileNameNU;
    binFileNU.open( binFileNameNU, std::ofstream::out | std::ofstream::binary ); // temp

//    if( fSNReadout ){ 
//      sprintf(binFileNameSN, "%s/sbndrawbin_run%06i_%4i.%02i.%02i-%02i.%02i.%02i_TPC_SN.dat",
//	      fDumpBinaryDir.c_str(), sbndaq::NevisTPC_generatorBase::run_number(), 
//	      ltm.tm_year + 1900, ltm.tm_mon + 1, ltm.tm_mday, ltm.tm_hour, ltm.tm_min, ltm.tm_sec);
    
  //    TLOG(TLVL_INFO)<< "Opening raw binary file " << binFileNameSN;
    //  binFileSN.open( binFileNameSN, std::ofstream::out | std::ofstream::binary ); // temp
      // to do: Send SN data to a dedicated Event Builder? Does it require its own BoardReader?
      // Otherwise use MicroBooNE example to make several files with the binary dump instead of a giant file
   // }
  }

  // Create Crate object
  fCrate = std::make_shared<nevistpc::Crate>( fControllerModule, fNUXMITReader, ps_, fSNXMITReader );
  
  // Run configuration recipe
  if( fCALIBFreq > 0 && fControllerTriggerFreq <= 0 ){
 
    // if (fDisableNUStream) {
     // TLOG(TLVL_ERROR) << "DisableNUStream=true with CALIBTriggerFrequency>0 is not supported. "
      //                 << "Either enable NU or disable CALIB trigger." << TLOG_ENDL;
     //  std::exit(EXIT_FAILURE);
     // }

fSNReadout? fCrate->runCalib2Stream( ps_ ) : fCrate->runCalib( ps_ );
}  else if( fControllerTriggerFreq > 0 && fCALIBFreq <= 0 ){

    // if (fDisableNUStream) {
       // ControllerTrigger sends NU triggers, so SN-only doesn't apply here
      //  TLOG(TLVL_ERROR) << "DisableNUStream=true with ControllerTriggerFrequency>0 is not supported. "
        //                 << "Controller triggers are NU-trigger based." << TLOG_ENDL;
    //  std::exit(EXIT_FAILURE);
    //  }

      fCrate->runControllerTrigger2Stream ( ps_ ); // implement NU-only stream if needed

 } else if( fCALIBFreq > 0 && fControllerTriggerFreq > 0){
    // Prevent running with two internal trigger sources as it could have unpredicted behavior
    TLOG(TLVL_ERROR) << "Two internal trigger sources (CALIB and Controller) are enabled simultaneously. Exit..." << TLOG_ENDL;
    mf::LogInfo("NevisTPC2StreamNUandSNXMIT") << "Two internal trigger sources (CALIB and Controller) are enabled simultaneously. Exit...";
    std::exit (EXIT_FAILURE);
  } else{

    //  if (fDisableNUStream) {
      //   if (!fSNReadout) {
        //    TLOG(TLVL_ERROR) << "DisableNUStream=true but DoSNReadout=false. No stream enabled. Exit..." << TLOG_ENDL;
          //  std::exit(EXIT_FAILURE);
         // }
       //  fCrate->runSNStream( ps_ );
       // } else {
       fSNReadout? fCrate->run2Stream( ps_ ) : fCrate->runNUStream( ps_ );
      // }
   }

  // To do: nevistpc::Crate should have a general runConfiguration function
  // The specific Crate configuration function to run should be specified in a fcl file
  // Therefore, only one common generator would be need for all configurations that run the same GetFEMCrateData() function

  // Set up worker GetSNData thread.
  share::ThreadFunctor GetSNData_functor = std::bind( &NevisTPC2StreamNUandSNXMIT::GetSNData, this );
  auto GetSNData_worker_functor = share::WorkerThreadFunctorUPtr( new share::WorkerThreadFunctor( GetSNData_functor, "GetSNDataWorkerThread" ) );
  auto GetSNData_worker = share::WorkerThread::createWorkerThread( GetSNData_worker_functor );
  GetSNData_thread_.swap(GetSNData_worker);
  if( fSNReadout ){
    GetSNData_thread_->start();
    TLOG(TLVL_INFO) << "Started GetSNData thread" << TLOG_ENDL;
  }

  // Set up worker WriteSNData thread.
  share::ThreadFunctor WriteSNData_functor = std::bind( &NevisTPC2StreamNUandSNXMIT::WriteSNData, this );
  auto WriteSNData_worker_functor = share::WorkerThreadFunctorUPtr( new share::WorkerThreadFunctor( WriteSNData_functor, "WriteSNDataWorkerThread" ) );
  auto WriteSNData_worker = share::WorkerThread::createWorkerThread( WriteSNData_worker_functor );
  WriteSNData_thread_.swap(WriteSNData_worker);
  if( fSNReadout ){
    WriteSNData_thread_->start();
    TLOG(TLVL_INFO) << "Started WriteSNData thread" << TLOG_ENDL;
  }

  // Set up worker MonitorCrate thread.
  share::ThreadFunctor MonitorCrate_functor = std::bind( &NevisTPC2StreamNUandSNXMIT::MonitorCrate, this );
  auto MonitorCrate_worker_functor = share::WorkerThreadFunctorUPtr( new share::WorkerThreadFunctor( MonitorCrate_functor, "MonitorCrateWorkerThread" ) );
  auto MonitorCrate_worker = share::WorkerThread::createWorkerThread( MonitorCrate_worker_functor );
  MonitorCrate_thread_.swap( MonitorCrate_worker );
  MonitorCrate_thread_->start();
  TLOG(TLVL_INFO) << "Started MonitorCrate thread" << TLOG_ENDL;

  // Set up worker FireCALIB thread.
  share::ThreadFunctor FireCALIB_functor = std::bind( &NevisTPC2StreamNUandSNXMIT::FireCALIB, this );
  auto FireCALIB_worker_functor = share::WorkerThreadFunctorUPtr( new share::WorkerThreadFunctor( FireCALIB_functor, "FireCALIBWorkerThread" ) );
  auto FireCALIB_worker = share::WorkerThread::createWorkerThread( FireCALIB_worker_functor );
  FireCALIB_thread_.swap(FireCALIB_worker);
  //  if( fCALIBFreq > 0 ){
  // FireCALIB_thread_->start();
  // TLOG(TLVL_INFO) << "Started FireCALIB thread" << TLOG_ENDL;
  //}

  // Set up worker FireController thread.
  share::ThreadFunctor FireController_functor = std::bind( &NevisTPC2StreamNUandSNXMIT::FireController, this );
  auto FireController_worker_functor = share::WorkerThreadFunctorUPtr( new share::WorkerThreadFunctor( FireController_functor, "FireControllerWorkerThread" ) );
  auto FireController_worker = share::WorkerThread::createWorkerThread( FireController_worker_functor );
  FireController_thread_.swap(FireController_worker);
  if( fControllerTriggerFreq > 0 ){
    FireController_thread_->start();
    TLOG(TLVL_INFO) << "Started FireController thread" << TLOG_ENDL;
  }
  //set up thread GPS time                                                                                                                               
  share::ThreadFunctor GPSTime_functor = std::bind( &NevisTPC2StreamNUandSNXMIT::GPSTime, this );
  auto GPSTime_worker_functor = share::WorkerThreadFunctorUPtr( new share::WorkerThreadFunctor( GPSTime_functor, "GPSTimeWorkerThread" ) );
  auto GPSTime_worker = share::WorkerThread::createWorkerThread( GPSTime_worker_functor );
  GPSTime_thread_.swap(GPSTime_worker);
  if( fGPSTimeFreq > 0 && fUseZMQ ) GPSTime_thread_->start();
  TLOG(TLVL_INFO) << "Started GPS thread" << TLOG_ENDL;

}

void sbndaq::NevisTPC2StreamNUandSNXMIT::runonsyncon() {
  if( fCALIBFreq > 0 ){
    fCrate->getTriggerModule()->runOnSyncOn();
    TLOG(TLVL_INFO) << "called runonsyncon for CALIB trigger" << TLOG_ENDL;
  }

  if( fCALIBFreq < 0 and fControllerTriggerFreq < 0 ){
    if(fCrate->hasTrigger){
      fCrate->getTriggerModule()->enableTriggers();
      fCrate->getTriggerModule()->runOnSyncOn();

      TLOG(TLVL_INFO) << "called runonsyncon for EXT trigger" << TLOG_ENDL;
    }}

  if( fControllerTriggerFreq > 0 ){
    fCrate->getControllerModule()->runOn();
    TLOG(TLVL_INFO) << "called runonsyncon for Controller trigger" << TLOG_ENDL;
  }
}

void sbndaq::NevisTPC2StreamNUandSNXMIT::startFireCalibTrig() {
  if( fCALIBFreq > 0 ){                                                                                         
    FireCALIB_thread_->start();
    TLOG(TLVL_INFO) << "Started FireCALIB thread" << TLOG_ENDL;
}

  if( fControllerTriggerFreq > 0 ){
    FireController_thread_->start();
    TLOG(TLVL_INFO) << "Started FireController thread" << TLOG_ENDL;
  }

  //  if( fControllerTriggerFreq < 0 and  ){

}

void sbndaq::NevisTPC2StreamNUandSNXMIT::ConfigureStop() {
  if( fSNReadout ){
    GetSNData_thread_->stop();
    WriteSNData_thread_->stop();
  }
  //  FireCALIB_thread_->stop();
  if( fControllerTriggerFreq > 0 ){//only stop thread if it met conditions to get started
    FireController_thread_->stop();
  }
  MonitorCrate_thread_->stop();

  if( fDumpBinary ){
    TLOG(TLVL_INFO)<< "Closing raw binary file " << binFileNameNU;
    binFileNU.close(); // temp

    if( fDumpSNBinary ){
      TLOG(TLVL_INFO)<< "Closing raw binary file " << binFileNameSN;
      binFileSN.close(); // temp
    }
  }
  delete[] SNBuffer_;
  fNUXMITReader->dmaStop();

  TLOG(TLVL_INFO)<< "Successful " << __func__ ;
  mf::LogInfo("NevisTPC2StreamNUandSNXMIT") << "Successful " << __func__;
}

bool sbndaq::NevisTPC2StreamNUandSNXMIT::FireCALIB() {
  static int fCALIBPeriod_us = 1./fCALIBFreq * 1e6;
  static std::chrono::steady_clock::time_point next_trigger_time{std::chrono::steady_clock::now() + std::chrono::microseconds(fCALIBPeriod_us)};
  if( next_trigger_time > std::chrono::steady_clock::now() ) return false;

  fCrate->getTriggerModule()->sendOutCalibTrigger();
  mf::LogInfo("NevisTPC2StreamNUandSNXMIT") << "CALIB Trigger ";
  TLOG(TLVL_INFO) << "Called " << __func__ ;
  next_trigger_time = std::chrono::steady_clock::now() + std::chrono::microseconds( fCALIBPeriod_us );
  usleep(10000);

  return true;
}

bool sbndaq::NevisTPC2StreamNUandSNXMIT::FireController() {
  static int fControllerTriggerPeriod_us = 1./fControllerTriggerFreq * 1e6;
  static std::chrono::steady_clock::time_point next_controller_trigger_time{std::chrono::steady_clock::now() + std::chrono::microseconds(fControllerTriggerPeriod_us)};
  if( next_controller_trigger_time > std::chrono::steady_clock::now() ) return false;

  fCrate->getControllerModule()->sendNUTrigger(); // Controller-driven trigger
  mf::LogInfo("NevisTPC2StreamNUandSNXMIT") << "Controller Trigger ";
  TLOG(TLVL_INFO) << "Called " << __func__ ;
  next_controller_trigger_time = std::chrono::steady_clock::now() + std::chrono::microseconds( fControllerTriggerPeriod_us );
  usleep(10000);

  return true;
}

bool sbndaq::NevisTPC2StreamNUandSNXMIT::MonitorCrate() {
  static std::chrono::steady_clock::time_point next_monitor_cycle_time{ std::chrono::steady_clock::now() };

  if( next_monitor_cycle_time > std::chrono::steady_clock::now() ) return false;

  fCrate->getXMITModule()->readStatus();
  //  fCrate->FEMtestfunction();
  TLOG(TSTATUS) << "Called " << __func__ ;
  // To do: add other board status checks. Follow uboonedaq/projects/sebs/configManager_CrateMonitor.cpp
  next_monitor_cycle_time = std::chrono::steady_clock::now() + std::chrono::seconds( fMonitorPeriod );

  return true;
}

bool sbndaq::NevisTPC2StreamNUandSNXMIT::GPSTime() {
  //static int fGPSTimePeriod_us = 0.25/fGPSTimeFreq * 1e6; //convert frequency to period in us 
  static int fGPSTimePeriod_us = 100; //convert frequency to period in us

  static std::chrono::steady_clock::time_point next_check_time{std::chrono::steady_clock::now() + std::chrono::microseconds(fGPSTimePeriod_us)};
  //create time point                                                                                                                                    
  static nevistpc::TriggerModuleGPSStamp lastGPSStamp = fCrate->getTriggerModule()->getLastGPSClockRegister(); //get most recent GPS stamp   
  if(fGPSTimeFreq < 0 || next_check_time > std::chrono::steady_clock::now() ) return false;
  //otherwise get the current gps stamp                                                                                                                
  nevistpc::TriggerModuleGPSStamp nowGPSStamp = fCrate->getTriggerModule()->getLastGPSClockRegister();
  //const auto start = std::chrono::steady_clock::now();
  struct timespec unixtime;
  clock_gettime(CLOCK_REALTIME, &unixtime);

  // Check if the new gps time/frame is different from the old one                                                                                     
  if( (nowGPSStamp.gps_frame != lastGPSStamp.gps_frame) ||
      (nowGPSStamp.gps_sample != lastGPSStamp.gps_sample) ||
      (nowGPSStamp.gps_sample_div != lastGPSStamp.gps_sample_div) ){
    
      //update stamps                                                                                                                                        
      lastGPSStamp = nowGPSStamp;
      
      //create message to send
      std::string message = std::to_string(lastGPSStamp.gps_frame) + ","
	+ std::to_string(lastGPSStamp.gps_sample) + ","
	+ std::to_string(lastGPSStamp.gps_sample_div);  
      
      zmq::message_t zmqMessage(message.size());

      //create timestamp to send with message. this will help correct NTP time to GPS time (we just need the "second" part) 
      long remainder = unixtime.tv_nsec % 1000000000;
      // Adjust the timespec to the nearest second
      if (remainder >= 500000000) {
        // Round up to the next second
        unixtime.tv_sec += 1;
      }

      long long timestamp = unixtime.tv_sec;
      zmq::message_t zmqTimestamp(sizeof(timestamp));

      memcpy(zmqTimestamp.data(), &timestamp, sizeof(timestamp));
      _zmqGPSPublisher.send(zmqTimestamp, ZMQ_SNDMORE);

      memcpy(zmqMessage.data(), message.c_str(), message.size());
      _zmqGPSPublisher.send(zmqMessage);

      //const auto end   = std::chrono::steady_clock::now();
      //auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start2).count();
      //TLOG(TLVL_INFO) << "ZMQ Message sending latency: " << duration << TLOG_ENDL;
      //auto duration2 = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
      //TLOG(TLVL_INFO) << "latency from time GPS is received to after message is sent: " << duration2 << TLOG_ENDL;

  }
  //update check time                                                                                                                                  
  next_check_time = std::chrono::steady_clock::now() + std::chrono::microseconds( fGPSTimePeriod_us );
  return true;
}

size_t sbndaq::NevisTPC2StreamNUandSNXMIT::GetFEMCrateData() {
  
  TLOG(TGETDATA)<< "GetFEMCrateData";

  // Just for tests
  // Taken from NevisTPCFile_generator and adapted to use an XMITReader
  // To be reviewed
  // uint16_t* buffer = new uint16_t[fChunkSize];
  TLOG(TGETDATA) << "Going to call Readsome function";
  //std::streamsize bytesRead = fNUXMITReader->readsome(reinterpret_cast<char*>(buffer), fChunkSize);
  std::streamsize bytesRead = fNUXMITReader->readsome(reinterpret_cast<char*>(&DMABuffer_[0]), fChunkSize);
  //unsigned wordsRead = bytesRead * sizeof(char) / sizeof(uint16_t);
  TLOG(TGETDATA) << "Number of bytes read:" << int(bytesRead) ;

  //std::copy(buffer, buffer + wordsRead, &DMABuffer_[0]);

  //if( fDumpBinary ) binFileNU.write( (char*)buffer, fChunkSize );
  if( fDumpBinary ) binFileNU.write( (char*)(&DMABuffer_[0]), fChunkSize );

  binFileNU.flush();
  //delete[] buffer;

  return bytesRead;
}

bool sbndaq::NevisTPC2StreamNUandSNXMIT::GetSNData() {
  
  TLOG(TGETDATA)<< "GetSNData";
  auto start = std::chrono::high_resolution_clock::now();

  static std::ofstream dbg;
  static bool dbg_init = false;
  if (!dbg_init) {
    dbg.open("/home/sbnd/DAQ_SPACK_DevAreas/DAQ_2026-01-23_SCHUNG_v1_10_08/DAQInterface/sn_getsndata_debug.log", std::ios::out | std::ios::app);
    dbg_init = true;
    dbg << "=== GetSNData debug start, pid=" << getpid() << " ===\n";
    dbg.flush();
   }


  // Just for tests
  // Taken from NevisTPCFile_generator and adapted to use an XMITReader
  // To be reviewed
  // uint16_t* SNBuffer_ = new uint16_t[fSNChunkSize];

  std::streamsize bytesRead = fSNXMITReader->readsome(reinterpret_cast<char*>(&SNDMABuffer_[0]), fSNChunkSize);
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::micro> duration_us = end - start;

  if (bytesRead <=0) return false;

  if (bytesRead == fSNChunkSize){
    ++N_SNDMAs;
    dbg << "Number of SN DMAs: " << N_SNDMAs ;
    dbg << "\n";
  }

  
  size_t n_words = bytesRead/sizeof(uint16_t);
  size_t new_buffer_size = SNCircularBuffer_.Insert(n_words, SNDMABuffer_);
  

  TLOG(TGETDATA)<< "Successfully inserted " << n_words << " . SN Buffer occupancy now " << new_buffer_size;


  dbg << "Successfully inserted " << n_words << " . SN Buffer occupancy now " << new_buffer_size;
  dbg << "\n";

  total_words_inserted += n_words;
  dbg << "Inserted " << n_words << " words. Total inserted: " << total_words_inserted;
  dbg << "\n";
  auto end1 = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::micro> duration_us1 = end1 - end;
  dbg << "SN DMA read time (us): " << duration_us.count();
  dbg << "SN Buffer Write time (us): "  << duration_us1.count();
  dbg << "\n";


  dbg << "SNCircularBuffer_.buffer: " << SNCircularBuffer_.buffer.size() ; 
  //  if( fDumpBinary ) binFileSN.write( (char*)(&SNDMABuffer_[0]), fSNChunkSize );
  //    //delete[] SNBuffer_;
  //      //memset(SNBuffer_, 0, fSNChunkSize*sizeof(uint16_t)); // avoid clearing?
  //  dbg << "SNCircularBuffer_.buffer.size() " << SNCircularBuffer_.buffer.size() ; 
  //        if(metricMan != nullptr) {
  //          //send SN metrics
  //            metricMan->sendMetric(
  //                "SN_DMA_Count",
  //                    N_SNDMAs,
  //                        "SN_dma_count", 11, artdaq::MetricMode::LastPoint);    
  //
  dbg << "\n";                        
  dbg.flush();

  //  if( fDumpBinary ) binFileSN.write( (char*)(&SNDMABuffer_[0]), fSNChunkSize );
  //delete[] SNBuffer_;
  //memset(SNBuffer_, 0, fSNChunkSize*sizeof(uint16_t)); // avoid clearing?

  return true;
}

bool sbndaq::NevisTPC2StreamNUandSNXMIT::WriteSNData() {

  auto start = std::chrono::high_resolution_clock::now();  


  static std::ofstream wdbg;
  static bool wdbg_init = false;
  if (!wdbg_init) {
    wdbg.open("/home/sbnd/DAQ_SPACK_DevAreas/DAQ_2026-01-23_SCHUNG_v1_10_08/DAQInterface/sn_writesndata_debug.log", std::ios::out | std::ios::app);
    wdbg_init = true;
    wdbg << "=== WriteSNData debug start, pid=" << getpid() << " ===\n";
    wdbg.flush();
   } 


  if( SNCircularBuffer_.buffer.size() < fSNChunkSize/ sizeof(uint16_t) ) return false;

// add for subfile
  if (SNDMATransferCnt_ == 1000) SNDMATransferCnt_ = 0;

  if (SNDMATransferCnt_ == 0) {

    if (fDumpSNBinary && fSNReadout) {
   if (binFileSN.is_open()) {
     TLOG(TLVL_INFO) << "Closing raw SN binary file " << binFileNameSN;
     binFileSN.close();
     }
    
    ++SNBinSubFileNum_;
    
    sprintf(binFileNameSN,"%s/sbndrawbin_run%06i_%4i.%02i.%02i-%02i.%02i.%02i_subfile%i_TPC_SN.dat",
                         fDumpBinaryDir.c_str(),sbndaq::NevisTPC_generatorBase::run_number(),
                         ltm.tm_year + 1900, ltm.tm_mon + 1, ltm.tm_mday,ltm.tm_hour, ltm.tm_min, ltm.tm_sec,SNBinSubFileNum_);
    
    TLOG(TLVL_INFO) << "Opening raw SN binary file " << binFileNameSN;
    binFileSN.open(binFileNameSN, std::ofstream::out | std::ofstream::binary);   
    }
      }


 // auto end = std::chrono::high_resolution_clock::now();
 // std::chrono::duration<double, std::micro> duration_us = end - start;
 // wdbg <<  "SN buffer to disk write time (us): " << duration_us.count;
 // wdbg << "\n";

 // std::copy(SNCircularBuffer_.buffer.begin(), SNCircularBuffer_.buffer.begin() + fSNChunkSize, SNBuffer_);
  std::copy(SNCircularBuffer_.buffer.begin(), SNCircularBuffer_.buffer.begin() + (fSNChunkSize / sizeof(uint16_t)), SNBuffer_);


  binFileSN.write(reinterpret_cast<char*>(SNBuffer_), fSNChunkSize );
  ++N_SNWrites;
  ++SNDMATransferCnt_;

  binFileSN.flush();

  size_t n_words_written = fSNChunkSize / sizeof(uint16_t);  
  total_words_written += n_words_written;  

  wdbg << "Wrote " << n_words_written << " words (" << fSNChunkSize << " bytes) to binary file. "                                            
		    << "Total written: " << total_words_written;
  wdbg << "\n";


  size_t new_buffer_size = SNCircularBuffer_.Erase(fSNChunkSize/sizeof(uint16_t));
  TLOG(TFILLFRAG)<< "Successfully erased " << fSNChunkSize/sizeof(uint16_t) << " . SN Buffer occupancy now " << new_buffer_size;

  wdbg << "Successfully erased " << fSNChunkSize/sizeof(uint16_t) << " . SN Buffer occupancy now " << new_buffer_size;
  wdbg << "\n"; 
  wdbg.flush();

  return true;
}

DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(sbndaq::NevisTPC2StreamNUandSNXMIT)
