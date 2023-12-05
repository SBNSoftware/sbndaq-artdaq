//
// sbndaq-artdaq/Generators/SBND/NevisTPC_generatorBase.cc (D.Cianci,W.Badgett)
//
#define TRACE_NAME "NevisTPCGenerator"
#include "artdaq/DAQdata/Globals.hh"

#include "sbndaq-artdaq/Generators/SBND/NevisTPC/NevisTPC_generatorBase.hh"
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
  use_xmit_ = true;
  look_for_xmit_header_ = true;

  // Read out stuff from fhicl
  FEMIDs_ = ps_.get< std::vector<uint64_t> >("FEMIDs",{0});
  fragment_ids = ps_.get< std::vector<artdaq::Fragment::fragment_id_t> >("fragment_ids");

  CircularBufferSizeBytes_ = ps_.get<uint32_t>("CircularBufferSizeBytes_",1e9); 
  EventsPerSubrun_ = ps_.get<int32_t>("EventsPerSubrun",-1);

  fSamplesPerChannel = ps_.get<uint32_t>("SamplesPerChannel",9600);
  fNChannels         = ps_.get<uint32_t>("NChannels",64);
  fUseCompression    = ps_.get<bool>("UseCompression",false);
  
  DMABufferSizeBytes_ = ps_.get<uint32_t>("DMABufferSize",1e6);	
  DMABuffer_.reset(new uint16_t[DMABufferSizeBytes_]);

  desyncCrash = ps_.get<bool>("desyncCrash",false);
  current_subrun_ = 0;
  events_seen_ = 0;

  // intialize event counting                                                                                                       
  _subrun_event_0 = -1;
  _this_event = -1;
  

  // Build our buffer
  if( CircularBufferSizeBytes_%sizeof(uint16_t)!=0)
    TRACE(TWARNING,"NevisTPC::Initialize() : CircularBufferSize_ not multiple of size uint16_t. Rounding down.");
  CircularBuffer_ = CircularBuffer(CircularBufferSizeBytes_/sizeof(uint16_t));
 
 // Initialize our buffer                                                                                                          
  CircularBuffer_.Init();
  
  // Set up worker getdata thread.
  share::ThreadFunctor functor = std::bind(&NevisTPC_generatorBase::GetData,this);
  auto worker_functor = share::WorkerThreadFunctorUPtr(new share::WorkerThreadFunctor(functor,"GetDataWorkerThread"));
  auto GetData_worker = share::WorkerThread::createWorkerThread(worker_functor);
  GetData_thread_.swap(GetData_worker);
}

void sbndaq::NevisTPC_generatorBase::start(){
  
  // Magically start getdata thread
  GetData_thread_->start();
}

void sbndaq::NevisTPC_generatorBase::stopAll(){
  
  GetData_thread_->stop();
}

void sbndaq::NevisTPC_generatorBase::stop(){
  ConfigureStop();  
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

  /*
 // If no data is received for ~3 sec, issue an error                                                                  
  while (n_words == 0) {
    auto elapsed_getdatatime = std::chrono::steady_clock::now() - start_getdatatime;

    // Timeout duration is set to 3 seconds                                                                             
    if (std::chrono::duration_cast<std::chrono::seconds>(elapsed_getdatatime).count() >= 3) {
      TRACE(TGETDATA, "Timeout: No data received for 3 seconds");
      return false;
    }
    n_words = GetFEMCrateData() / sizeof(uint16_t);
  }
  */


  size_t new_buffer_size = CircularBuffer_.Insert(n_words,DMABuffer_);	
  
  TRACE(TGETDATA,"Successfully inserted %lu words. Buffer occupancy now %lu",n_words,new_buffer_size);

  return true;
}

bool sbndaq::NevisTPC_generatorBase::getNext_(artdaq::FragmentPtrs & frags){
  
  while(true)
    if(!FillFragment(frags)) break;
  
  return true;
}

bool sbndaq::NevisTPC_generatorBase::FillFragment(artdaq::FragmentPtrs &frags, bool clear_buffer[[gnu::unused]]){

  int wiggle_room = 5;	// How many words before and after ADC count to correctly identify ending of data packet
  size_t new_buffer_size=0;

  // Check that we've got at least 32 bits of data
  if(CircularBuffer_.buffer.size()*sizeof(uint16_t) < sizeof(uint32_t)){
    return false;	
  }

  // Since we can no longer trust XMIT headers or footers...
  if( *(reinterpret_cast<uint32_t const*>(&CircularBuffer_.buffer[0])) == 0xffffffff ){
    TRACE(TFILLFRAG,"FOUND AN XMIT HEADER");
    new_buffer_size = CircularBuffer_.Erase(2);
    TRACE(TFILLFRAG,"Successfully erased %d words. Buffer occupancy now %lu",2,new_buffer_size);
    current_event = -1;
    current_framenum = -1;
  }
  
  if(CircularBuffer_.buffer.size()*sizeof(uint16_t) < sizeof(uint32_t)){
    return false;
  }

  if( *(reinterpret_cast<uint32_t const*>(&CircularBuffer_.buffer[0])) == 0xe0000000 ){
    TRACE(TFILLFRAG,"FOUND AN XMIT TRAILER");
    new_buffer_size = CircularBuffer_.Erase(2);
    //std::cout << "BAAAADA BING" << std::endl;
    TRACE(TFILLFRAG,"Successfully erased %d words. Buffer occupancy now %lu",2,new_buffer_size);
    return true;
  }

  if(CircularBuffer_.buffer.size()*sizeof(uint16_t) < sizeof(NevisTPCHeader)){
    TRACE(TFILLFRAG,"Not enough data for NevisTPCHeader. Return and try again.");
    return false;
  }  
 // Theoretically, we should have a header, but there may be a problem with the data. Sometimes we get a big discrepancy between the number of ADC words described in the header and the actual number of words and it causes the next header to be out of line. So let's check that the header is  lined up right. Otherwise, we'll just throw a fit and crash the run.
  if(CircularBuffer_.buffer[0] != 0xFFFF)
  {
    char line [132];
    sprintf(line,"Header out of sync: %X", CircularBuffer_.buffer[0]);
    TRACE(TERROR,line);
    throw std::runtime_error(line);
    return false;
  }

  auto header = reinterpret_cast<NevisTPCHeader const*>(&CircularBuffer_.buffer[0]);
  size_t expected_size = sizeof(NevisTPCHeader) + header->getADCWordCount()*sizeof(uint16_t) + sizeof(uint16_t);
  size_t expected_plus_wiggle = sizeof(NevisTPCHeader) + header->getADCWordCount()*sizeof(uint16_t) + sizeof(uint16_t) * (wiggle_room + 1);

  if(desyncCrash){
    // Some more consistency checks! If the event or framenumber are out of sync between FEMs within a single event, tank the run
    if(current_event < 0){
      current_event = header->getEventNum();
    }
    else if((uint)current_event != header->getEventNum())
    {
      char line[132];
      sprintf(line,"FEM event num out of sync, tanking the run. Current: %d, header: %d",current_event,header->getEventNum());
      TRACE(TERROR,line);
      throw std::runtime_error(line);
      return false;
    }

    if(current_framenum < 0){
      current_framenum = header->getFrameNum();
    }
    else if((uint)current_framenum != header->getFrameNum()){
      char line[132];
      sprintf(line,"FEM framenum out of sync, tanking the run. Current: %d, Header :%d", current_framenum,header->getFrameNum());
      TRACE(TERROR,line);
      throw std::runtime_error(line);
      return false;
    }
  }

  if (!use_xmit_) {
    expected_size += sizeof(uint16_t);
  }

  TRACE(TFILLFRAG,"TPC data with total expected size %lu,FEMID=%u,Slot=%u,ADCWordCount=%u,Event=%u,Frame=%u",
	expected_size,
	header->getFEMID(),
	header->getSlot(),
	header->getADCWordCount(),
	header->getEventNum(),
	header->getFrameNum());
  
  if(CircularBuffer_.buffer.size()*sizeof(uint16_t) < expected_plus_wiggle){
    TRACE(TFILLFRAG,"Not enough data for expected size %lu. Return and try again.",expected_size);
    return false;
  }

  // First, let's figure out how big our data packet actually is since we can't trust the FEM
  for(int i = 0; i < wiggle_room * 2; i++){
    int word_i = header->getADCWordCount() + 12 - wiggle_room; 
    if(CircularBuffer_.buffer[word_i + i] == 0xFFFF){
      expected_size = (word_i + i)*sizeof(uint16_t);
      break;
    }
    // Check for XMIT trailer
    else if(CircularBuffer_.buffer[word_i + i] == 0xe000 && CircularBuffer_.buffer[word_i + i - 1] == 0x0000){
      continue;
    }
    else if(CircularBuffer_.buffer[word_i + i] == 0x0000 && CircularBuffer_.buffer[word_i + i + 1] == 0xe000){
      expected_size = (word_i + i)*sizeof(uint16_t);
      //std::cout << "LAST WORD: " << std::hex << CircularBuffer_.buffer[word_i - i - 1] << CircularBuffer_.buffer[word_i -i - 2] << std::endl;
      break;
    }
  }

  // Sweet, now, let's actually fill stuff
  _this_event = metadata_.EventNumber();

  // set the subrun event 0 if it has never been set before
  if (_subrun_event_0 == -1) {
    _subrun_event_0 = _this_event;
  }

  //std::cout << std::hex << CircularBuffer_.buffer[1] << CircularBuffer_.buffer[0] << " " << CircularBuffer_.buffer[3] << CircularBuffer_.buffer[2] << " " << CircularBuffer_.buffer[5] << CircularBuffer_.buffer[4] << std::endl;

  // System-wide realtime clock timestamp
  struct timespec unixtime;
  clock_gettime(CLOCK_REALTIME, &unixtime);
  artdaq::Fragment::timestamp_t unixtime_ns = static_cast<artdaq::Fragment::timestamp_t>(unixtime.tv_sec)*1000000000 + 
    static_cast<artdaq::Fragment::timestamp_t>(unixtime.tv_nsec);

  artdaq::Fragment::fragment_id_t fragment_id;
  uint32_t index = header->getSlot() - FEM_BASE_SLOT;

  if (( header->getSlot() >= FEM_BASE_SLOT ) && ( index < fragment_ids.size()))
  {
    fragment_id = fragment_ids[index];
  }
  else
  {
    TRACE(TERROR,"NevisTPC::FillFragment() : illegal FEM slot number");
    return(false);
  }

  metadata_ = NevisTPCFragmentMetadata(header->getEventNum(),fNChannels,fSamplesPerChannel,fUseCompression);
  frags.emplace_back( artdaq::Fragment::FragmentBytes(expected_size,
                                                      metadata_.EventNumber(),          // Sequence ID
						      fragment_id,                      // Fragment ID
                                                      detail::FragmentType::NevisTPC,   // Fragment Type
						      metadata_,
						      unixtime_ns) );
  std::copy(CircularBuffer_.buffer.begin(),
	    CircularBuffer_.buffer.begin()+(expected_size/sizeof(uint16_t)),
	    (uint16_t*)(frags.back()->dataBegin()));
  TRACE(TFILLFRAG,"Created fragment with sequenceID=%lu, fragmentID=%u, TimeStamp=%lu",
	frags.back()->sequenceID(),frags.back()->fragmentID(),frags.back()->timestamp());

  new_buffer_size = CircularBuffer_.Erase(expected_size/sizeof(uint16_t));
  TRACE(TFILLFRAG,"Successfully erased %lu words. Buffer occupancy now %lu",
	expected_size/sizeof(uint16_t),new_buffer_size);

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
}
