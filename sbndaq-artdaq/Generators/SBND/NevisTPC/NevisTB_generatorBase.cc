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
#include <zmq.hpp>
#include <cmath>

sbndaq::NevisTB_generatorBase::NevisTB_generatorBase(fhicl::ParameterSet const & ps): CommandableFragmentGenerator(ps), rec_context(1), _zmqGPSSubscriber(rec_context, ZMQ_SUB), ps_(ps) {
  
  Initialize();
}


sbndaq::NevisTB_generatorBase::~NevisTB_generatorBase(){

  stopAll();
}

void sbndaq::NevisTB_generatorBase::Initialize(){
  CircularBufferSizeBytesNTB_ = ps_.get<uint32_t>("CircularBufferSizeBytesNTB_",1e9);  
  DMABufferSizeBytesNTB_ = ps_.get<uint32_t>("DMABufferSizeNTB",1e4);                                                                                

  GPSZMQPortNTB_ = ps_.get<std::string>("GPSZMQPortNTB", "tcp://10.226.36.6:11212");
  framesize_ = ps_.get<uint32_t>("framesize", 20479);
  NevisClockFreq_ = ps_.get<uint32_t>("NevisClockFrequency", 15999907);
  EventsPerSubrun_ = ps_.get<int32_t>("EventsPerSubrun",-1);

  DMABufferNTB_.reset(new uint16_t[DMABufferSizeBytesNTB_]);       
  current_subrun_ = 0;
  events_seen_ = 0;
  _subrun_event_0 = -1;
  _this_event = -1;
  GPSinitialized = false;
  pseudo_ntbfragment = 1;
  long long samples_per_frame = 2000000;
  FramesPerSecond_ = samples_per_frame/framesize_;
  TLOG(TLVL_INFO) << "Frames Per Second: " << FramesPerSecond_ ;
  rollCounter = 0;
  prevFrame   = 0;
  //set up zmq to listen for messages from TPC server
  _zmqGPSSubscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);
  _zmqGPSSubscriber.connect(GPSZMQPortNTB_.c_str());
  TLOG(TLVL_INFO) << "NTB Subscriber Connected to port " << GPSZMQPortNTB_ ;

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

  // Set up worker getGPStime thread. 
  share::ThreadFunctor GPStime_functor = std::bind(&NevisTB_generatorBase::GPStime,this);                                                              
  auto worker_GPStime_functor = share::WorkerThreadFunctorUPtr(new share::WorkerThreadFunctor(GPStime_functor,"GetGPStimeWorkerThread"));                       
  auto GPStime_worker = share::WorkerThread::createWorkerThread(worker_GPStime_functor);                                                                
  GPStime_thread_.swap(GPStime_worker);
  TLOG(TLVL_INFO) << "GPStimeWorkerThread Initialized";

}

void sbndaq::NevisTB_generatorBase::start(){
  GetNTBData_thread_->start();  
  GPStime_thread_->start();
}

void sbndaq::NevisTB_generatorBase::stopAll(){
  GetNTBData_thread_->stop();   
  GPStime_thread_->stop();
}


void sbndaq::NevisTB_generatorBase::stop(){
  ConfigureNTBStop();
  stopAll();
}

void sbndaq::NevisTB_generatorBase::stopNoMutex(){
  //  stopAll();
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
  //TLOG(TLVL_INFO)<< "NTB words" << ntb_words;                                                                                                           
  if(ntb_words==0) return false;                                                                                                                        
  size_t new_NTBbuffer_size = CircularBufferNTB_.Insert(ntb_words,DMABufferNTB_);                                                                       
                                                                                                                                                        
  return true;                                                                                                                                          
}                                          

bool sbndaq::NevisTB_generatorBase::GPStime(){                                                                                                     
    //static std::chrono::steady_clock::time_point next_check_time{std::chrono::steady_clock::now() + std::chrono::microseconds(10)};
    zmq::pollitem_t items[] = {{static_cast<void*>(_zmqGPSSubscriber), 0, ZMQ_POLLIN, 0}};

    zmq::poll(items, 1, 1); //timeout after 1 milliseconds (last item)

    if (items[0].revents & ZMQ_POLLIN) {

        zmq::message_t zmqTimestamp;
        _zmqGPSSubscriber.recv(&zmqTimestamp);

        zmq::message_t zmqMessage;
        _zmqGPSSubscriber.recv(&zmqMessage);

        struct timespec unixtime;
        clock_gettime(CLOCK_REALTIME, &unixtime);
        //TLOG(TLVL_INFO) << "NTP time GPS received by NTB " << unixtime.tv_sec << " , " << unixtime.tv_nsec << TLOG_ENDL;

        std::string message(static_cast<char*>(zmqMessage.data()), zmqMessage.size());
        // Remove the prefix and parse the three integers
        std::istringstream iss(message); 
        std::vector<uint32_t> numbers;
        std::string token;

        // Extract timestamp from the received message
        memcpy(&receivedNTPsecond, zmqTimestamp.data(), sizeof(receivedNTPsecond));
        TLOG(TLVL_DEBUG) << "ZMQ Timestamp NTP second received:  " << receivedNTPsecond << TLOG_ENDL;

        while (std::getline(iss, token, ',')) { 
          numbers.push_back(std::stoi(token));
        }
        if(!GPSinitialized || GPSframe != numbers[0] || GPSsample != numbers[1] || GPSdiv != numbers[2]){
          GPSstamp nowGPSstamp(numbers[0], numbers[1], numbers[2]);
          setGPSstamp(nowGPSstamp); 
          //TLOG(TLVL_INFO)<< "Received new time stamp: " << numbers[0] << " " << numbers[1] << " " << numbers[2] ;           
          GPSinitialized = true;
          return true;
        }
        else{
          return false;
        } 
    }
//    }
    return true;
}                                          

void sbndaq::NevisTB_generatorBase::setGPSstamp(GPSstamp currentStamp)
  {  
    GPSframe  = currentStamp.gps_frame;
    GPSsample = currentStamp.gps_sample;
    GPSdiv    = currentStamp.gps_sample_div;
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
    //TLOG(TLVL_INFO)<< "Size of NTB Circular buffer:" << CircularBufferNTB_.buffer.size();                                                             
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
          
  TLOG(TLVL_DEBUG+13) << "NTB Event number from pseudo counter " << _this_event;                                                                                                                                              

  // set the subrun event 0 if it has never been set before
  if (_subrun_event_0 == -1) {
    _subrun_event_0 = _this_event; 
  }
    long long tframe   = ntbheader->getFrame();                  //Get trigger time tagged by nevis clock
    long long tsamp    = ntbheader->get2MHzSampleNumber();       //
    long long tdiv     = ntbheader->get16MHzRemainderNumber();   //

    struct timespec unixtime;
    clock_gettime(CLOCK_REALTIME, &unixtime);

    //When the nevis clock rolls over (frames go back to 0) 
    //treat the timestamp appropriately
    int32_t   frame_diff = abs(ntbheader->getFrame() - GPSframe);
    //long long tolerance  = 16777216 - FramesPerSecond_ - 10; //16777216 = 2^24 is when the rollover happens. frame numbers are stored in 24 bits
    int32_t   tolerance  = 16770000;
    int32_t   corrFrame;

    TLOG(TLVL_INFO) << "Frame difference (between GPS and trigger): " << frame_diff;

    if(tframe < prevFrame){

      rollCounter+=1;
      TLOG(TLVL_INFO) << "Trigger frames rolled over!!!! " << " this many times: " << rollCounter;
    }

    corrFrame = tframe + rollCounter*16777216;
    TLOG(TLVL_INFO) << " Corrected Frame:  " << corrFrame <<  " Uncorrected Frame: " << tframe;


    if(frame_diff > tolerance){  //looks like we rolled over

      TLOG(TLVL_INFO) << "Rolling Over!!!!  Fixing timestamp. trigger frame is "<< tframe << " but will now be: "<< tframe+16777216<<" GPS frame is: " << GPSframe;
      if(tframe < GPSframe){
          tframe += 16777216;
      }
      else{
          GPSframe += 16777216;
      }
      TLOG(TLVL_INFO) << "Last PPS time: " << GPSframe << " " << GPSsample << " " << GPSdiv ;
      TLOG(TLVL_INFO) << "Trigger  time: " << ntbheader->getFrame() << " " << ntbheader->get2MHzSampleNumber() << " " << ntbheader->get16MHzRemainderNumber();
      TLOG(TLVL_INFO) << "Frame difference: " << frame_diff << " " <<  "Tolerance: " << tolerance;
 
    }

    long long t_trig = static_cast<long long>((static_cast<double>(tframe * (framesize_ + 1)   + tsamp * 8 + tdiv) / NevisClockFreq_) * 1000000000);
    long long t_pps  = static_cast<long long>((static_cast<double>(GPSframe * (framesize_ + 1) + GPSsample * 8 + GPSdiv) / NevisClockFreq_) * 1000000000);
    //long long t_trig   = 1000000000*(tframe*(framesize_+1) + tsamp*8 + tdiv)/NevisClockFreq_;   //Convert trigger time from nevis clock ticks to seconds
    //long long t_pps    = (GPSframe*(framesize_+1) + GPSsample*8 + GPSdiv)*1.e9/NevisClockFreq_; //Convert last received pps time from nevis clock ticks to second

    long long pps_offset   = t_trig - t_pps;                                                      //Compute time elapsed from last pps
    //auto old_timestamp = unixtime.tv_sec*1000000000   + unixtime.tv_nsec;
    long long new_timestamp = receivedNTPsecond*1000000000 + pps_offset;                         
                                                                                                 //Add time elapsed to known second when pps was received
    artdaq::Fragment::timestamp_t ntb_fragment_timestamp = static_cast<artdaq::Fragment::timestamp_t>(receivedNTPsecond*1000000000 + pps_offset);   

    if(new_timestamp < prev_timestamp && prev_timestamp-new_timestamp > 1000000000){     
        TLOG(TLVL_WARN) << "NTB Timestamp warning: Current timestamp is more than 1s earlier than the previous one. This should not happen. current: " 
                        << new_timestamp << " previous: " << prev_timestamp << " pps offset: " << pps_offset << " NTP second: " 
                        << receivedNTPsecond << " tframe: " << tframe << " tsamp: " << tsamp << " tdiv: " << tdiv << " t_trig: " << t_trig << " GPSframe: " << GPSframe << " GPSsample: " << GPSsample << " GPSdiv: " << GPSdiv;
    }

    if(prev_timestamp > 0 && new_timestamp > prev_timestamp && new_timestamp-prev_timestamp > 20000000000){     
        TLOG(TLVL_WARN) << "NTB Timestamp warning: Current timestamp is more than 20s later than previous one. current: " 
                        << new_timestamp << " previous: " << prev_timestamp << " pps offset: " << pps_offset << " NTP second: " 
                        << receivedNTPsecond << " t_pps: " << t_pps <<  " tframe: " << tframe << " tsamp: " << tsamp << " tdiv: " << tdiv << " t_trig: " << t_trig << " GPSframe: " << GPSframe << " GPSsample: " << GPSsample << " GPSdiv: " << GPSdiv;

    }

    if(pps_offset < static_cast<long long>(1000)){
      TLOG(TLVL_WARN) << "NTB Timestamp: Trigger - PPS difference is less than 1us. pps offset: " << pps_offset << " NTP second: " << receivedNTPsecond << " t_trig: " << t_trig;
    }
    //TRACE(TFILLFRAG,"Trigger - PPS difference is %llu ns.",pps_offset); 
 
    prev_timestamp = new_timestamp;
    prevFrame = ntbheader->getFrame();
    //TLOG(TLVL_INFO) << "Last PPS time: " << GPSframe << " " << GPSsample << " " << GPSdiv ;
    //TLOG(TLVL_INFO) << "Trigger  time: " << tframe << " " << tsamp << " " << tdiv;
    //TLOG(TLVL_INFO) << "Corrected timestamp: " << new_timestamp;
    //TLOG(TLVL_INFO) << "Received NTP second: : " << receivedNTPsecond;
    //TLOG(TLVL_INFO) << "PPS OFFSET: " << pps_offset;
                     
    //to compare to PMT timestamps
    //auto now1 = std::chrono::high_resolution_clock::now();
    //auto ns_since_epoch1 = std::chrono::time_point_cast<std::chrono::nanoseconds>(now1).time_since_epoch();
    //auto ns1 = ns_since_epoch1.count();
    //TLOG(TLVL_INFO) << "Current time in nanoseconds in ntb (when getting corrected time): " << ns1;
                                               
    ntbmetadata_ = NevisTBFragmentMetadata(ntbheader->getTriggerNumber(), corrFrame, ntbheader->get2MHzSampleNumber(), ntbheader->getFrame());  
    //ntbmetadata_ = NevisTBFragmentMetadata(_this_event,ntbheader->getFrame(), ntbheader->get2MHzSampleNumber());
    frags.emplace_back( artdaq::Fragment::FragmentBytes(expected_size,
							ntbmetadata_.EventNumber(), //_this_event,//Sequence ID 
							pseudo_ntbfragment,
							detail::FragmentType::NevisTB,   //Fragment Type
							ntbmetadata_, 
							ntb_fragment_timestamp) );
 
    std::copy(CircularBufferNTB_.buffer.begin(),                                                                                                          
	      CircularBufferNTB_.buffer.begin()+(expected_size/sizeof(uint16_t)),                                                                         
	      (uint16_t*)(frags.back()->dataBegin()));     
    new_NTBbuffer_size = CircularBufferNTB_.Erase(expected_size/sizeof(uint16_t));                                                                        

    TRACE(TFILLFRAG,"Successfully erased %lu words. Buffer occupancy now %lu",
	  expected_size/sizeof(uint16_t),new_NTBbuffer_size);

    // bump the subrun
    if(EventsPerSubrun_ > 0 && _subrun_event_0 != _this_event && _this_event % EventsPerSubrun_== 0) {
      TRACE(TFILLFRAG, "Bumping artdaq subrun number from %u to %u. Last subrun spans events %i to %i.", current_subrun_,current_subrun_ + 1, _subrun_event_0, _this_event); 
      TLOG(TLVL_WARNING)<< "Bumping artdaq subrun number from" << current_subrun_<< " to "<<current_subrun_+1<<". Last subrun spans events "<<_subrun_event_0<<" to "<<_this_event<<".";
      _subrun_event_0 = _this_event;
      ++current_subrun_;
      artdaq::FragmentPtr endOfSubrunFrag(new artdaq::Fragment(static_cast<size_t>(ceil(sizeof(my_rank) / static_cast<double>(sizeof(artdaq::Fragment::value_type))))));
      endOfSubrunFrag->setSystemType(artdaq::Fragment::EndOfSubrunFragmentType);
      endOfSubrunFrag->setSequenceID(_this_event);
      *endOfSubrunFrag->dataBegin() = my_rank;
      frags.emplace_back(std::move(endOfSubrunFrag));
    }
    //    _this_event++;
    ++events_seen_;

    return true;                                                                                                                                          
                                                                                                                                                          
  } //End of FillNTBFragment                                                                                                                              

    
