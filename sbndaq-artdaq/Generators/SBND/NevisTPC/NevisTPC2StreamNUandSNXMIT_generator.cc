#define TRACE_NAME "NevisTPCGenerator2StreamNUandSNXMIT"

#include "artdaq/DAQdata/Globals.hh"
#include "artdaq/Generators/GeneratorMacros.hh"
#include "sbndaq-artdaq/Generators/SBND/NevisTPC/NevisTPC2StreamNUandSNXMIT.hh"

#include <chrono>
#include <ctime>

void sbndaq::NevisTPC2StreamNUandSNXMIT::ConfigureStart() {
  TLOG(TLVL_INFO) << "ConfigureStart";

  fChunkSize = ps_.get<int>("ChunkSize", 4096);
  fNTBChunkSize = ps_.get<int>("NTBChunkSize", 96);
  fMonitorPeriod = ps_.get<int>("MonitorPeriod", 12);
  fCALIBFreq = ps_.get<double>("CALIBTriggerFrequency", -1);
  fControllerTriggerFreq = ps_.get<double>("ControllerTriggerFrequency", -1);
  fDumpBinary = ps_.get<bool>("DumpBinary", false);
  fDumpBinaryDir = ps_.get<std::string>("DumpBinaryDir", ".");
  fSNReadout = ps_.get<bool>("DoSNReadout", true);
  fSNChunkSize = ps_.get<int>("SNChunkSize", 100000);
  fGPSTimeFreq = ps_.get<double>("GPSTimeFrequency", -1);

  SNDMABuffer_.reset(new uint16_t[fSNChunkSize]);
  SNCircularBuffer_ = CircularBuffer(1e9/sizeof(uint16_t)); // to do: define in fcl
  SNCircularBuffer_.Init();
  SNBuffer_ = new uint16_t[fSNChunkSize];

  if( fDumpBinary ){
    // Get timestamp for binary file name
    time_t t = time(0);
    struct tm ltm = *localtime( &t );
    sprintf(binFileNameNU, "%s/sbndrawbin_run%06i_%4i.%02i.%02i-%02i.%02i.%02i_TPC_NU.dat",
	    fDumpBinaryDir.c_str(), sbndaq::NevisTPC_generatorBase::run_number(), 
	    ltm.tm_year + 1900, ltm.tm_mon + 1, ltm.tm_mday, ltm.tm_hour, ltm.tm_min, ltm.tm_sec);
    
    TLOG(TLVL_INFO)<< "Opening raw binary file " << binFileNameNU;
    binFileNU.open( binFileNameNU, std::ofstream::out | std::ofstream::binary ); // temp


    sprintf(binFileNameNevisTB, "%s/sbndrawbin_run%06i_%4i.%02i.%02i-%02i.%02i.%02i_NevisTB.dat",                                                                 
	   fDumpBinaryDir.c_str(), sbndaq::NevisTPC_generatorBase::run_number(),                                                                                   
	   ltm.tm_year + 1900, ltm.tm_mon + 1, ltm.tm_mday, ltm.tm_hour, ltm.tm_min, ltm.tm_sec);                                                                  
    binFileNevisTB.open( binFileNameNevisTB, std::ofstream::out | std::ofstream::binary ); // temp                                                                  
                                                                                                                                                                    
    TLOG(TLVL_INFO)<< "Opening raw binary file " << binFileNameNevisTB;

    if( fSNReadout ){ 
      sprintf(binFileNameSN, "%s/sbndrawbin_run%06i_%4i.%02i.%02i-%02i.%02i.%02i_TPC_SN.dat",
	      fDumpBinaryDir.c_str(), sbndaq::NevisTPC_generatorBase::run_number(), 
	      ltm.tm_year + 1900, ltm.tm_mon + 1, ltm.tm_mday, ltm.tm_hour, ltm.tm_min, ltm.tm_sec);
    
      TLOG(TLVL_INFO)<< "Opening raw binary file " << binFileNameSN;
      binFileSN.open( binFileNameSN, std::ofstream::out | std::ofstream::binary ); // temp
      // to do: Send SN data to a dedicated Event Builder? Does it require its own BoardReader?
      // Otherwise use MicroBooNE example to make several files with the binary dump instead of a giant file
    }
  }

  // Create Crate object
  fCrate = std::make_shared<nevistpc::Crate>( fControllerModule, fNUXMITReader, ps_, fSNXMITReader );
  
  // Run configuration recipe
  if( fCALIBFreq > 0 && fControllerTriggerFreq <= 0 ) fSNReadout? fCrate->runCalib2Stream( ps_ ) : fCrate->runCalib( ps_ );
  else if( fControllerTriggerFreq > 0 && fCALIBFreq <= 0 ) fCrate->runControllerTrigger2Stream ( ps_ ); // implement NU-only stream if needed
  else if( fCALIBFreq > 0 && fControllerTriggerFreq > 0){
    // Prevent running with two internal trigger sources as it could have unpredicted behavior
    TLOG(TLVL_ERROR) << "Two internal trigger sources (CALIB and Controller) are enabled simultaneously. Exit..." << TLOG_ENDL;
    mf::LogInfo("NevisTPC2StreamNUandSNXMIT") << "Two internal trigger sources (CALIB and Controller) are enabled simultaneously. Exit...";
    std::exit (EXIT_FAILURE);
  } else fSNReadout? fCrate->run2Stream( ps_ ) : fCrate->runNUStream( ps_ );

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
  if( fCALIBFreq > 0 ){
    FireCALIB_thread_->start();
    TLOG(TLVL_INFO) << "Started FireCALIB thread" << TLOG_ENDL;
  }
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
  if( fGPSTimeFreq > 0 ) GPSTime_thread_->start();
  TLOG(TLVL_INFO) << "Started GPS thread" << TLOG_ENDL;
  TLOG(TLVL_INFO)<< "Successful " << __func__ ;
  mf::LogInfo("NevisTPC2StreamNUandSNXMIT") << "Successful " << __func__;

}

void sbndaq::NevisTPC2StreamNUandSNXMIT::ConfigureStop() {
  if( fSNReadout ){
    GetSNData_thread_->stop();
    WriteSNData_thread_->stop();
  }
  FireCALIB_thread_->stop();
  FireController_thread_->stop();
  MonitorCrate_thread_->stop();

  if( fDumpBinary ){
    TLOG(TLVL_INFO)<< "Closig raw binary file " << binFileNameNU;
    binFileNU.close(); // temp
    TLOG(TLVL_INFO)<< "Closig raw binary file " << binFileNameNevisTB;                                                                                        
    binFileNevisTB.close();      

    if( fSNReadout ){
      TLOG(TLVL_INFO)<< "Closig raw binary file " << binFileNameSN;
      binFileSN.close(); // temp
    }
  }
  delete[] SNBuffer_;

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
  TLOG(TSTATUS) << "Called " << __func__ ;
  // To do: add other board status checks. Follow uboonedaq/projects/sebs/configManager_CrateMonitor.cpp
  next_monitor_cycle_time = std::chrono::steady_clock::now() + std::chrono::seconds( fMonitorPeriod );

  return true;
}

bool sbndaq::NevisTPC2StreamNUandSNXMIT::GPSTime() {
  static int fGPSTimePeriod_us = 1./fGPSTimeFreq * 1e6; //convert frequency to period in us 
  static std::chrono::steady_clock::time_point next_check_time{std::chrono::steady_clock::now() + std::chrono::microseconds(fGPSTimePeriod_us)};
  //create time point                                                                                                                                    
  static nevistpc::TriggerModuleGPSStamp lastGPSStamp = fCrate->getTriggerModule()->getLastGPSClockRegister(); //get most recent GPS stamp   
  if(fGPSTimeFreq < 0 || next_check_time > std::chrono::steady_clock::now() ) return false;
  //otherwise get the current gps stamp                                                                                                                
  nevistpc::TriggerModuleGPSStamp nowGPSStamp = fCrate->getTriggerModule()->getLastGPSClockRegister();

  struct timespec unixtime;
  clock_gettime(CLOCK_REALTIME, &unixtime);
  time_t ntp_time = unixtime.tv_sec + (unixtime.tv_nsec*1e-9);
  // Check if the new gps time/frame is different from the old one                                                                                     

  TLOG(TLVL_INFO) << "NTP time " << unixtime.tv_sec << " , " << unixtime.tv_nsec  << " , " << ntp_time << TLOG_ENDL;
  TLOG(TLVL_INFO) << "Check on conditions 1 : " << nowGPSStamp.gps_frame << " , " << lastGPSStamp.gps_frame << TLOG_ENDL;
  TLOG(TLVL_INFO) << "Check on conditions 2 : " <<nowGPSStamp.gps_sample << " , " << lastGPSStamp.gps_sample << TLOG_ENDL;
  TLOG(TLVL_INFO) << "Check on conditions 3 : " <<nowGPSStamp.gps_sample_div << " , " << lastGPSStamp.gps_sample_div << TLOG_ENDL;

  if( (nowGPSStamp.gps_frame != lastGPSStamp.gps_frame) ||
      (nowGPSStamp.gps_sample != lastGPSStamp.gps_sample) ||
      (nowGPSStamp.gps_sample_div != lastGPSStamp.gps_sample_div) ){

      double diff_time = nowGPSStamp.gps_frame -  lastGPSStamp.gps_frame;
    //std::map<int32_t, unsigned long> timeMap;
      //  timeMap.insert(std::pair<int32_t, unsigned long>(nowGPSStamp.gps_frame*0.00128, ntp_time));
      //update stamps                                                                                                                                        
      lastGPSStamp = nowGPSStamp;

      TLOG(TLVL_INFO) << "updated time stamp " << TLOG_ENDL;
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

  //std::streamsize bytesRead = fNUXMITReader->readsome(reinterpret_cast<char*>(buffer), fChunkSize);
  std::streamsize bytesRead = fNUXMITReader->readsome(reinterpret_cast<char*>(&DMABuffer_[0]), fChunkSize);
  //unsigned wordsRead = bytesRead * sizeof(char) / sizeof(uint16_t);
  //std::copy(buffer, buffer + wordsRead, &DMABuffer_[0]);
  //if( fDumpBinary ) binFileNU.write( (char*)buffer, fChunkSize );
  if( fDumpBinary ) binFileNU.write( (char*)(&DMABuffer_[0]), fChunkSize );

  binFileNU.flush();
  //delete[] buffer;

  return bytesRead;
}


size_t sbndaq::NevisTPC2StreamNUandSNXMIT::GetNevisTBData() {                                                                                        
  std::streamsize ntbbytesRead = fCrate->getNevisTBStreamReader()->readsome(reinterpret_cast<char*>(&DMABufferNTB_[0]), fNTBChunkSize);              
  binFileNevisTB.write( (char*)(&DMABufferNTB_), fNTBChunkSize );                                                                                    
  binFileNevisTB.flush();                                                                                                                            
  TLOG(TLVL_INFO) << "NevisTB bytes read: " << ntbbytesRead;                                                                                         
  return ntbbytesRead;                                                                                                                               
}                       

bool sbndaq::NevisTPC2StreamNUandSNXMIT::GetSNData() {
  
  TLOG(TGETDATA)<< "GetSNData";

  // Just for tests
  // Taken from NevisTPCFile_generator and adapted to use an XMITReader
  // To be reviewed
  // uint16_t* SNBuffer_ = new uint16_t[fSNChunkSize];

  std::streamsize bytesRead = fSNXMITReader->readsome(reinterpret_cast<char*>(&SNDMABuffer_[0]), fSNChunkSize);
  size_t n_words = bytesRead/sizeof(uint16_t);
  size_t new_buffer_size = SNCircularBuffer_.Insert(n_words, SNDMABuffer_);

  TLOG(TGETDATA)<< "Successfully inserted " << n_words << " . SN Buffer occupancy now " << new_buffer_size;

  //  if( fDumpBinary ) binFileSN.write( (char*)(&SNDMABuffer_[0]), fSNChunkSize );
  
  //delete[] SNBuffer_;
  //memset(SNBuffer_, 0, fSNChunkSize*sizeof(uint16_t)); // avoid clearing?

  return true;
}

bool sbndaq::NevisTPC2StreamNUandSNXMIT::WriteSNData() {

  if( SNCircularBuffer_.buffer.size() < fSNChunkSize ) return false;

  std::copy(SNCircularBuffer_.buffer.begin(), SNCircularBuffer_.buffer.begin() + fSNChunkSize, SNBuffer_);

  binFileSN.write((char*)SNBuffer_, fSNChunkSize );
  binFileSN.flush();
  size_t new_buffer_size = SNCircularBuffer_.Erase(fSNChunkSize);
  TLOG(TFILLFRAG)<< "Successfully erased " << fSNChunkSize << " . SN Buffer occupancy now " << new_buffer_size;

  return true;
}

DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(sbndaq::NevisTPC2StreamNUandSNXMIT)
