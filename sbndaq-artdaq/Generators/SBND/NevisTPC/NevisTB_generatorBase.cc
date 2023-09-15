//
// sbndaq-artdaq/Generators/SBND/NevisTB_generatorBase.cc (D.Kalra)
//
#define TRACE_NAME "NevisTBGenerator"
#include "artdaq/DAQdata/Globals.hh"

#include "sbndaq-artdaq/Generators/SBND/NevisTPC/NevisTB_generatorBase.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"

#include <fstream>
#include <iomanip>
#include <iterator>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

sbndaq::NevisTB_generatorBase::NevisTB_generatorBase(fhicl::ParameterSet const & ps): CommandableFragmentGenerator(ps), ps_(ps){
  
  Initialize();
}


sbndaq::NevisTB_generatorBase::~NevisTB_generatorBase(){

  stopAll();
}

void sbndaq::NevisTB_generatorBase::Initialize(){
  CircularBufferSizeBytesNTB_ = ps_.get<uint32_t>("CircularBufferSizeBytesNTB_",1e9);  
  DMABufferSizeBytesNTB_ = ps_.get<uint32_t>("DMABufferSizeNTB",1e4);                                                                                
  DMABufferNTB_.reset(new uint16_t[DMABufferSizeBytesNTB_]);       
  current_subrun_ = 0;
  events_seen_ = 0;
  _subrun_event_0 = -1;
  _this_event = -1;


  pseudo_ntbfragment = 1;  

  //Build buffer for NTB        
  if( CircularBufferSizeBytesNTB_%sizeof(uint16_t)!=0)                                                                                                
    TRACE(TWARNING,"NevisTB::Initialize() : CircularBufferSizeNTB_ not multiple of size uint16_t. Rounding down.");                                   
  CircularBufferNTB_ = CircularBuffer(CircularBufferSizeBytesNTB_/sizeof(uint16_t));                                                                  
  // Initialize our buffer 
  CircularBufferNTB_.Init();  

  // Set up worker getNTBdata thread. 
  share::ThreadFunctor ntbfunctor = std::bind(&NevisTB_generatorBase::GetNTBData,this);                                                              
  auto worker_ntbfunctor = share::WorkerThreadFunctorUPtr(new share::WorkerThreadFunctor(ntbfunctor,"GetNTBDataWorkerThread"));                       
  auto GetNTBData_worker = share::WorkerThread::createWorkerThread(worker_ntbfunctor);                                                                
  GetNTBData_thread_.swap(GetNTBData_worker);

}

void sbndaq::NevisTB_generatorBase::start(){
  GetNTBData_thread_->start();  
}

void sbndaq::NevisTB_generatorBase::stopAll(){
  GetNTBData_thread_->stop();   
}


void sbndaq::NevisTB_generatorBase::stop(){
  ConfigureNTBStop();
  stopAll();
}

void sbndaq::NevisTB_generatorBase::stopNoMutex(){

  stopAll();
}

size_t sbndaq::NevisTB_generatorBase::CircularBuffer::Insert(size_t n_words, std::unique_ptr<uint16_t[]> const& dataptr){

  TRACE(TDEBUG,"Inserting %lu words. Currently %lu/%lu in buffer.",n_words,buffer.size(),buffer.capacity());
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


size_t sbndaq::NevisTB_generatorBase::CircularBuffer::Erase(size_t n_words){

  TRACE(TDEBUG,"Erasing %lu words. Currently %lu/%lu in buffer.",
        n_words,buffer.size(),buffer.capacity());

  std::unique_lock<std::mutex> lock(*(mutexptr));
  TRACE(TDEBUG,"Obtained circular buffer lock for erase.");

  buffer.erase_begin(n_words);
  TRACE(TDEBUG,"Erased %lu words. Currently have %lu/%lu in buffer.",
        n_words,buffer.size(),buffer.capacity());

  return buffer.size();
}

bool sbndaq::NevisTB_generatorBase::GetNTBData(){                                                                                                     
                                                                                                                                                        
  size_t ntb_words = GetNevisTBData()/sizeof(uint16_t);                                                                                                 
  TLOG(TLVL_INFO)<< "NTB words" << ntb_words;                                                                                                           
  if(ntb_words==0) return false;                                                                                                                        
  size_t new_NTBbuffer_size = CircularBufferNTB_.Insert(ntb_words,DMABufferNTB_);                                                                       
                                                                                                                                                        
  return true;                                                                                                                                          
}                                          


bool sbndaq::NevisTB_generatorBase::getNext_(artdaq::FragmentPtrs & frags){
  //this function doesn't run over event by event.                                                                                                      
  while(true)
    if(!FillNTBFragment(frags)) break; // and !FillNTBFragment(frags)) break;                                                                              

  return true;
}


bool sbndaq::NevisTB_generatorBase::FillNTBFragment(artdaq::FragmentPtrs &frags, bool clear_buffer[[gnu::unused]]){                                    
                                                                                                                                                        
  size_t new_NTBbuffer_size=0;  
  // Check that we've got at least 4*32 bits of data for NevisTB                                                                                        
  // We need 4*32bit data because for NTB, the trailer end marker is the fixed one 0xffffffff unlike the TPCs where we have XMIT header that starts with 0xffffffff                                                                                                                                           
  if(CircularBufferNTB_.buffer.size()*sizeof(uint16_t) < 4*sizeof(uint32_t)){                                                                         
    TLOG(TLVL_INFO)<< "Size of NTB Circular buffer:" << CircularBufferNTB_.buffer.size();                                                             
    return false;                                                                                                                                     
  }            

 //Can we reset event and frame number counter here. For TPC, it is reset during start of header but for NTB, it's tricky because we can only determine event trigger data end marker. Confirm with Gennadiy.         
  current_ntbevent = -1;        
  current_ntbframenum = -1;     
  if( *(reinterpret_cast<uint32_t const*>(&CircularBufferNTB_.buffer[0])) == 0xffffffff ){                                                            
  TRACE(TFILLFRAG,"FOUND EVENT TRIGGER DATA END MARKER (NTB DATA MARKER)");                                                                           
   new_NTBbuffer_size = CircularBufferNTB_.Erase(8); //Deleting 8 words. because we read 8 16 bit words when a trigger is issued.                     
    TRACE(TFILLFRAG,"Successfully erased %d words. Buffer occupancy now %lu",2,new_NTBbuffer_size);                                                   
    return true;                                                                                                                                      
  }
  //Define NevisTB header 
  auto ntbheader = reinterpret_cast<NevisTrigger_Header const*>(&CircularBufferNTB_.buffer[0]);
  size_t expected_size = sizeof(NevisTrigger_Header) + sizeof(NevisTrigger_Trailer) + sizeof(NevisTrigger_Data);                                      
  if(current_ntbevent < 0){                                                                                                                           
    current_ntbevent = ntbheader->getTriggerNumber();  //this function is defined in NevisTB_dataFormat                                               
 
  }                                                                                                                                                   
 
  else if((uint)current_ntbevent != ntbheader->getTriggerNumber())                                                                                    
 
    {                                                                                                                                                 
 
      char line[132];  
      sprintf(line,"NTB eventnum out of sync, tanking the run. Current: %d, header: %d",current_ntbevent,ntbheader->getTriggerNumber());                
      TRACE(TERROR,line);                                                                                                                               
      throw std::runtime_error(line);                                                                                                                   
      return false;                                                                                                                                     
    }                                                                                                                                                   
                                                                                                                                                        
  if(current_ntbframenum < 0){                                                                                                                          
    current_ntbframenum = ntbheader->getFrame();                                                                                                        
  }                                                                                                                                                     
                                                                                                                                                        
  else if((uint)current_ntbframenum != ntbheader->getFrame()){                                                                                          
    char line[132];    
    sprintf(line,"NTB framenum out of sync, tanking the run. Current: %d, Header :%d", current_ntbframenum,ntbheader->getFrame());                      
    TRACE(TERROR,line);                                                                                                                                 
    throw std::runtime_error(line);                                                                                                                     
    return false;                                                                                                                                       
  }                                                                                                                                                     
                                                                                                                                                        
                                                                                                                                                        
  if(CircularBufferNTB_.buffer.size()*sizeof(uint16_t) < expected_size){                                                                                   
    TRACE(TFILLFRAG,"Not enough NTB data for expected size %lu. Return and try again.",expected_size);                                                  
    return false;                                                                                                                                       
  }  
  // Sweet, now, let's actually fill stuff                                                                                                              
  _this_event = ntbmetadata_.EventNumber();                                                                                                             
                                                                                                                                                        
  // set the subrun event 0 if it has never been set before                                                                                             
  if (_subrun_event_0 == -1) {                                                                                                                          
    _subrun_event_0 = _this_event;  

  }
    struct timespec unixtime;                                                                                                                             
    clock_gettime(CLOCK_REALTIME, &unixtime);                                                                                                             
    artdaq::Fragment::timestamp_t unixtime_ns = static_cast<artdaq::Fragment::timestamp_t>(unixtime.tv_sec)*1000000000 +                                  
    static_cast<artdaq::Fragment::timestamp_t>(unixtime.tv_nsec);                                                                                       
                                                                                                                                                        
    ntbmetadata_ = NevisTBFragmentMetadata(ntbheader->getTriggerNumber(),ntbheader->getFrame(), ntbheader->get2MHzSampleNumber());  
    frags.emplace_back( artdaq::Fragment::FragmentBytes(expected_size,                                                                                    
							ntbmetadata_.EventNumber(),       // Sequence ID                                                  
							pseudo_ntbfragment,                                                                               
							detail::FragmentType::NevisTB,   // Fragment Type                                                 
							ntbmetadata_,                                                                                     
							unixtime_ns) );                                                                                   
                                                                                                                                                        
    std::copy(CircularBufferNTB_.buffer.begin(),                                                                                                          
	      CircularBufferNTB_.buffer.begin()+(expected_size/sizeof(uint16_t)),                                                                         
	      (uint16_t*)(frags.back()->dataBegin()));     
    new_NTBbuffer_size = CircularBufferNTB_.Erase(expected_size/sizeof(uint16_t));                                                                        

    TRACE(TFILLFRAG,"Successfully erased %lu words. Buffer occupancy now %lu",
	  expected_size/sizeof(uint16_t),new_NTBbuffer_size);

    // bump the subrun
    if(EventsPerSubrun_ > 0 && _subrun_event_0 != _this_event && _this_event % EventsPerSubrun_== 0) {
      TRACE(TFILLFRAG, "Bumping artdaq subrun number from %u to %u. Last subrun spans events %i to %i.", current_subrun_,current_subrun_ + 1, _subrun_event_0, _this_event); 
      _subrun_event_0 = _this_event;
      ++current_subrun_;
      artdaq::FragmentPtr endOfSubrunFrag(new artdaq::Fragment(static_cast<size_t>(ceil(sizeof(my_rank) / static_cast<double>(sizeof(artdaq::Fragment::value_type))))));
      endOfSubrunFrag->setSystemType(artdaq::Fragment::EndOfSubrunFragmentType);
      endOfSubrunFrag->setSequenceID(_this_event);
      *endOfSubrunFrag->dataBegin() = my_rank;
      frags.emplace_back(std::move(endOfSubrunFrag));
    }

    ++events_seen_;

    return true;                                                                                                                                          
                                                                                                                                                          
  } //End of FillNTBFragment                                                                                                                              

    
