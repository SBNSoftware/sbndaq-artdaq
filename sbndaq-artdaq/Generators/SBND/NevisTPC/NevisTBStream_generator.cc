#define TRACE_NAME "NevisTBStreamGenerator"

#include "artdaq/DAQdata/Globals.hh"
#include "artdaq/Generators/GeneratorMacros.hh"
#include "sbndaq-artdaq/Generators/SBND/NevisTPC/NevisTBStream.hh"


#include <chrono>
#include <ctime>

void sbndaq::NevisTBStream::ConfigureNTBStart() {
  TLOG(TLVL_INFO) << "ConfigureStart NTB";

  fNTBChunkSize = ps_.get<int>("NTBChunkSize", 96);

  fDumpNTBBinary = ps_.get<bool>("DumpNTBBinary", false);
  fDumpNTBBinaryDir = ps_.get<std::string>("DumpNTBBinaryDir", ".");

 

  NTBDMABuffer_.reset(new uint16_t[fNTBChunkSize]);
  NTBCircularBuffer_ = CircularBuffer(1e9/sizeof(uint16_t)); // to do: define in fcl                                             
                                           
  NTBCircularBuffer_.Init();
  NTBBuffer_ = new uint16_t[fNTBChunkSize];
 
  if( fDumpNTBBinary ){
    // Get timestamp for binary file name                                                                                                                                  
    time_t t = time(0);
    struct tm ltm = *localtime( &t );

    sprintf(binFileNameNevisTB, "%s/sbndrawbin_run%06i_%4i.%02i.%02i-%02i.%02i.%02i_NevisTB.dat",
            fDumpNTBBinaryDir.c_str(), sbndaq::NevisTB_generatorBase::run_number(),
            ltm.tm_year + 1900, ltm.tm_mon + 1, ltm.tm_mday, ltm.tm_hour, ltm.tm_min, ltm.tm_sec);
    binFileNTB.open( binFileNameNevisTB, std::ofstream::out | std::ofstream::binary ); // temp                                                                         

    TLOG(TLVL_INFO)<< "Opening raw binary file " << binFileNameNevisTB;

  }

  TLOG(TLVL_INFO) << "Configure NTB PCIe card";
  fntbreader->configureReader();
  fntbreader->initializePCIeCard();
  fntbreader->setupTXModeRegister();
  TLOG(TLVL_INFO) << "Configured NTB PCIe card";

}

/*
// Set up worker GetNTBData thread.                                                                                                                           
            
share::ThreadFunctor GetNTBData_functor = std::bind( &NevisTBStream::GetNTBData, this );
auto GetNTBData_worker_functor = share::WorkerThreadFunctorUPtr( new share::WorkerThreadFunctor( GetNTBData_functor, "GetNTBDataWorkerThread" ) );
auto GetNTBData_worker = share::WorkerThread::createWorkerThread( GetNTBData_worker_functor );
GetNTBData_thread_.swap(GetNTBData_worker);
if(fDumpNTBBinary){
  GetNTBData_thread_->start();
  TLOG(TLVL_INFO) << "Started GetNTBData thread" << TLOG_ENDL;
 }


// Set up worker WriteSNData thread.                                                                                                                                     
share::ThreadFunctor WriteNTBData_functor = std::bind( &NevisTBStream::WriteNTBData, this );
auto WriteNTBData_worker_functor = share::WorkerThreadFunctorUPtr( new share::WorkerThreadFunctor( WriteNTBData_functor, "WriteNTBDataWorkerThread" ) );
auto WriteNTBData_worker = share::WorkerThread::createWorkerThread( WriteNTBData_worker_functor );
WriteNTBData_thread_.swap(WriteNTBData_worker);
if( fDumpNTBBinary ){
  WriteNTBData_thread_->start();
  TLOG(TLVL_INFO) << "Started WriteNTBData thread" << TLOG_ENDL;
 }

*/

void sbndaq::NevisTBStream::ConfigureNTBStop() {

  if( fDumpNTBBinary ){
    TLOG(TLVL_INFO)<< "Closig raw binary file " << binFileNameNevisTB;
    binFileNTB.close();
    //GetNTBData_thread_->stop();
    //WriteNTBData_thread_->stop();
  }

  //  delete[] NTBBuffer_;


  TLOG(TLVL_INFO)<< "Successful " << __func__ ;
  mf::LogInfo("NevisTBStream") << "Successful " << __func__;
}

size_t sbndaq::NevisTBStream::GetNevisTBData() {

  //  std::streamsize bytesRead = fCrate->getNevisTBStreamReader()->readsome(reinterpret_cast<char*>(&DMABufferNTB_[0]), fNTBChunkSize);
  TLOG(TLVL_INFO)<< "GetNevisTBData";

  std::streamsize bytesRead = fntbreader->readsome(reinterpret_cast<char*>(&DMABufferNTB_[0]), fNTBChunkSize);  
  TLOG(TLVL_INFO)<< "NTB bytes:" << bytesRead;
  //  if(bytesRead>0){

    binFileNTB.write( (char*)(&DMABufferNTB_[0]), bytesRead); //fNTBChunkSize );  

  // size_t n_words = bytesRead/sizeof(uint16_t); 
  //size_t new_buffer_size = NTBCircularBuffer_.Insert(n_words, NTBDMABuffer_);
  //if( NTBCircularBuffer_.buffer.size() < fNTBChunkSize ) return false;
  //std::copy(NTBCircularBuffer_.buffer.begin(), NTBCircularBuffer_.buffer.begin() + fNTBChunkSize, NTBBuffer_); 

  //binFileNTB.write((char*)NTBBuffer_, fNTBChunkSize );                                                                                                     
  binFileNTB.flush();
  //}                                                                                                                                        
  return bytesRead;

}

/*
bool sbndaq::NevisTBStream::GetNTBData() {
  std::streamsize bytesRead = fCrate->getNevisTBStreamReader()->readsome(reinterpret_cast<char*>(&NTBDMABuffer_[0]), fNTBChunkSize);
  size_t n_words = bytesRead/sizeof(uint16_t);
  size_t new_buffer_size = NTBCircularBuffer_.Insert(n_words, NTBDMABuffer_);

  TLOG(TGETDATA)<< "Successfully inserted " << n_words << " . SN Buffer occupancy now " << new_buffer_size;
  return true;
}

bool sbndaq::NevisTBStream::WriteNTBData() {
  if( NTBCircularBuffer_.buffer.size() < fNTBChunkSize ) return false;

  std::copy(NTBCircularBuffer_.buffer.begin(), NTBCircularBuffer_.buffer.begin() + fNTBChunkSize, NTBBuffer_);

  binFileNTB.write((char*)NTBBuffer_, fNTBChunkSize );
  binFileNTB.flush();
  size_t new_buffer_size = NTBCircularBuffer_.Erase(fNTBChunkSize);
  TLOG(TFILLFRAG)<< "Successfully erased " << fNTBChunkSize << " . NTB Buffer occupancy now " << new_buffer_size;

  return true;
}
*/

DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(sbndaq::NevisTBStream)
