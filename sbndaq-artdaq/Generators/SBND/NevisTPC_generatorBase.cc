//
// sbndaq-readout/Generators/NevisTPC_generatorBase.cc (D.Cianci,W.Badgett)
//
#define TRACE_NAME "NevisTPCGenerator"
#include "artdaq/DAQdata/Globals.hh"

#include "sbndaq-artdaq/Generators/SBND/NevisTPC_generatorBase.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"

#include <fstream>
#include <iomanip>
#include <iterator>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

sbndaq::NevisTPC_generatorBase::NevisTPC_generatorBase(fhicl::ParameterSet const & ps): CommandableFragmentGenerator(ps), ps_(ps){
	
	Initialize();
}

sbndaq::NevisTPC_generatorBase::~NevisTPC_generatorBase(){

	stopAll();
}

void sbndaq::NevisTPC_generatorBase::Initialize(){
  
  // Read out stuff from fhicl
  FEMIDs_ = ps_.get< std::vector<uint64_t> >("FEMIDs",{0});
  CircularBufferSizeBytes_ = ps_.get<uint32_t>("CircularBufferSizeBytes_",1e9); 
  EventsPerSubrun_ = ps_.get<uint32_t>("EventsPerSubrun",100);
  
  DMABufferSizeBytes_ = ps_.get<uint32_t>("DMABufferSize",1e6);	
  DMABuffer_.reset(new uint16_t[DMABufferSizeBytes_]);
  
  // Build our buffer
  if( CircularBufferSizeBytes_%sizeof(uint16_t)!=0)
    TRACE(TWARNING,"NevisTPC::Initialize() : CircularBufferSize_ not multiple of size uint16_t. Rounding down.");
  CircularBuffer_ = CircularBuffer(CircularBufferSizeBytes_/sizeof(uint16_t));
  
  // Set up worker getdata thread.
  share::ThreadFunctor functor = std::bind(&NevisTPC_generatorBase::GetData,this);
  auto worker_functor = share::WorkerThreadFunctorUPtr(new share::WorkerThreadFunctor(functor,"GetDataWorkerThread"));
  auto GetData_worker = share::WorkerThread::createWorkerThread(worker_functor);
  GetData_thread_.swap(GetData_worker);
}

void sbndaq::NevisTPC_generatorBase::start(){
  
  current_subrun_ = 0;
  events_seen_ = 0;
  look_for_xmit_header_ = true;
  
  ConfigureStart();
  
  // Initialize our buffer
  CircularBuffer_.Init();	
  
  // Magically start getdata thread
  GetData_thread_->start();
}

void sbndaq::NevisTPC_generatorBase::stopAll(){
  
  GetData_thread_->stop();
}

void sbndaq::NevisTPC_generatorBase::stop(){
  
  stopAll();
}

void sbndaq::NevisTPC_generatorBase::stopNoMutex(){
  
  stopAll();
}

size_t sbndaq::NevisTPC_generatorBase::CircularBuffer::Insert(size_t n_words, std::unique_ptr<uint16_t[]> const& dataptr){
  
  TRACE(TDEBUG,"Inserting %lu words. Currently %lu/%lu in buffer.",
	n_words,buffer.size(),buffer.capacity());
  
  //don't fill while we wait for available capacity...
  while( (buffer.capacity()-buffer.size()) < n_words){ usleep(10); }
  //obtain the lock
  std::unique_lock<std::mutex> lock(*(mutexptr));
  TRACE(TDEBUG,"Obtained circular buffer lock for insert.");

  buffer.insert(buffer.end(),&(dataptr[0]),&(dataptr[n_words]));
  TRACE(TDEBUG,"Inserted %lu words. Currently have %lu/%lu in buffer.",
	n_words,buffer.size(),buffer.capacity());  
  return buffer.size();
}

size_t sbndaq::NevisTPC_generatorBase::CircularBuffer::Erase(size_t n_words){
  
  TRACE(TDEBUG,"Erasing %lu words. Currently %lu/%lu in buffer.",
	n_words,buffer.size(),buffer.capacity());

  std::unique_lock<std::mutex> lock(*(mutexptr));
  TRACE(TDEBUG,"Obtained circular buffer lock for erase.");

  buffer.erase_begin(n_words);
  TRACE(TDEBUG,"Erased %lu words. Currently have %lu/%lu in buffer.",
	n_words,buffer.size(),buffer.capacity());  

  return buffer.size();	
}

bool sbndaq::NevisTPC_generatorBase::GetData(){
  
  TRACE(TGETDATA,"GetData() called");
  
  size_t n_words = GetFEMCrateData()/sizeof(uint16_t);
  TRACE(TGETDATA,"GetFEMCrateData() return %lu words",n_words);
  if(n_words==0)
      return false;

  size_t new_buffer_size = CircularBuffer_.Insert(n_words,DMABuffer_);	
  
  TRACE(TGETDATA,"Successfully inserted %lu words. Buffer occupancy now %lu",n_words,new_buffer_size);

  return true;
}

bool sbndaq::NevisTPC_generatorBase::getNext_(artdaq::FragmentPtrs & frags){
  
  while(true)
    if(!FillFragment(frags)) break;
  
  return true;
}

bool sbndaq::NevisTPC_generatorBase::FillFragment(artdaq::FragmentPtrs &frags){
  
  size_t new_buffer_size=0;

  // Check that we've got at least 32 bits of data
  if(CircularBuffer_.buffer.size()*sizeof(uint16_t) < sizeof(uint32_t)){
    return false;	
  }

  // look for xmit header
  if(look_for_xmit_header_){
    if( *(reinterpret_cast<uint32_t const*>(&CircularBuffer_.buffer[0])) != 0xffffffff ){
      TRACE(TFILLFRAG,"EXPECTED XMIT HEADER BUT FOUND %d. Sleep and try again?",
	    *(reinterpret_cast<uint32_t const*>(&CircularBuffer_.buffer[0])));
      usleep(10000);
      return false;
    }
    TRACE(TFILLFRAG,"FOUND AN XMIT HEADER");
    new_buffer_size = CircularBuffer_.Erase(2);
    look_for_xmit_header_ = false;
    TRACE(TFILLFRAG,"Successfully erased %d words. Buffer occupancy now %lu",2,new_buffer_size);
  }
  
  if(CircularBuffer_.buffer.size()*sizeof(uint16_t) < sizeof(uint32_t)){
    return false;
  }

  // look for xmit footer
  if( *(reinterpret_cast<uint32_t const*>(&CircularBuffer_.buffer[0])) == 0xe0000000 ){
    TRACE(TFILLFRAG,"FOUND AN XMIT TRAILER");
    new_buffer_size = CircularBuffer_.Erase(2);
    look_for_xmit_header_ = true;
    TRACE(TFILLFRAG,"Successfully erased %d words. Buffer occupancy now %lu",2,new_buffer_size);
    return true;
  }
  
  if(CircularBuffer_.buffer.size()*sizeof(uint16_t) < sizeof(NevisTPCHeader)){
    TRACE(TFILLFRAG,"Not enough data for NevisTPCHeader. Return and try again.");
    return false;
  }  
  auto header = reinterpret_cast<NevisTPCHeader const*>(&CircularBuffer_.buffer[0]);
  size_t expected_size = sizeof(NevisTPCHeader) + header->getADCWordCount()*sizeof(uint16_t);

  TRACE(TFILLFRAG,"TPC data with total expected size %lu,FEMID=%u,Slot=%u,ADCWordCount=%u,Event=%u,Frame=%u",
	expected_size,header->getFEMID(),header->getSlot(),
	header->getADCWordCount(),
	header->getEventNum(),
	header->getFrameNum());
  
  if(CircularBuffer_.buffer.size()*sizeof(uint16_t) < expected_size){
    TRACE(TFILLFRAG,"Not enough data for expected size %lu. Return and try again.",expected_size);
    return false;
  }  

  // Sweet, now, let's actually fill stuff
  if(events_seen_%EventsPerSubrun_==0)
    ++current_subrun_;

  // To do: last boolean here is compression flag -- take from FHICL
  metadata_ = NevisTPCFragmentMetadata(RunNumber_,current_subrun_,header->getEventNum(),true);
  frags.emplace_back( artdaq::Fragment::FragmentBytes(expected_size,  
                                                      metadata_.EventNumber(),
                                                      (2&0xffff)+((1&0xffff)<16),
                                                      detail::FragmentType::NevisTPC, metadata_) );
  std::copy(CircularBuffer_.buffer.begin(),
	    CircularBuffer_.buffer.begin()+(expected_size/sizeof(uint16_t)),
	    (uint16_t*)(frags.back()->dataBegin()));
  TRACE(TFILLFRAG,"Created fragment with sequenceID=%lu, fragmentID=%u, TimeStamp=%lu",
	frags.back()->sequenceID(),frags.back()->fragmentID(),frags.back()->timestamp());

  new_buffer_size = CircularBuffer_.Erase(expected_size/sizeof(uint16_t));
  TRACE(TFILLFRAG,"Successfully erased %lu words. Buffer occupancy now %lu",
	expected_size/sizeof(uint16_t),new_buffer_size);

  ++events_seen_;
  return true;
}
