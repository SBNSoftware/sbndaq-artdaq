#include "sbndaq-artdaq/Generators/Common/BernCRTZMQ_GeneratorBase.hh"

//#include "art/Utilities/Exception.h"
#include "cetlib_except/exception.h"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "fhiclcpp/ParameterSet.h"
//#include "artdaq-core/Utilities/SimpleLookupPolicy.h"
#include "artdaq/DAQdata/Globals.hh"

#include <fstream>
#include <iomanip>
#include <iterator>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <sys/time.h>
#include <sys/timeb.h>

#define TRACE_NAME "BernCRTZMQ_GeneratorBase"

sbndaq::BernCRTZMQ_GeneratorBase::BernCRTZMQ_GeneratorBase(fhicl::ParameterSet const & ps)
  :
  CommandableFragmentGenerator(ps),
  ps_(ps)
{
  TLOG(TLVL_INFO)<<"BernCRTZMQ_GeneratorBase constructor called";  
  Initialize();
  TLOG(TLVL_INFO)<<"BernCRTZMQ_GeneratorBase constructor completed";
}


/*---------------------------------------------------------------------*/

bool be_verbose_section = true;


void sbndaq::BernCRTZMQ_GeneratorBase::Initialize() {

  if(be_verbose_section){
    std::cout << std::endl;
    std::cout << "---------------------------" << std::endl;
    std::cout << "SECTION 1 - INITIALIZE" << std::endl;
    std::cout << "---------------------------" << std::endl;  
    std::cout << std::endl;
  }

  TLOG(TLVL_INFO)<<"BernCRTZMQ_GeneratorBase::Initialize() called";

  RunNumber_ = 0;
  FEBIDs_ = ps_.get< std::vector<uint64_t> >("FEBIDs");

  //new variable added by me (see the header file)
  FragmentCounter_ = 0;
  GPSCounter_= 0;
  event_in_clock = 0;
  GPS_time = 0;

  FEBBufferCapacity_ = ps_.get<uint32_t>("FEBBufferCapacity");
  ZMQBufferCapacity_ = ps_.get<uint32_t>("ZMQBufferCapacity");

  MaxTimeDiffs_   = ps_.get< std::vector<uint32_t> >("MaxTimeDiffs",std::vector<uint32_t>(FEBIDs_.size()));

  if(MaxTimeDiffs_.size()!=FEBIDs_.size()){
    if(MaxTimeDiffs_.size()==1){
      auto size = MaxTimeDiffs_.at(0);
      MaxTimeDiffs_ = std::vector<uint32_t>(FEBIDs_.size(),size);
    }
    else{
      throw cet::exception("BernCRTZMQ_GeneratorBase::Initialize")
        << "MaxTimeDiffs must be same size as ZMQIDs in config!";
    }
  }

  throttle_usecs_ = ps_.get<size_t>("throttle_usecs");
  throttle_usecs_check_ = ps_.get<size_t>("throttle_usecs_check");

  if (throttle_usecs_ > 0 && (throttle_usecs_check_ >= throttle_usecs_ ||
        throttle_usecs_ % throttle_usecs_check_ != 0) ) {
    throw cet::exception("Error in BernCRTZMQ: disallowed combination of throttle_usecs and throttle_usecs_check (see BernCRTZMQ.hh for rules)");
  }

  TLOG(TLVL_INFO)<<"BernCRTZMQ_GeneratorBase::Initialize() completed";



  for( size_t i_id=0; i_id<FEBIDs_.size(); ++i_id){
    auto const& id = FEBIDs_[i_id];
    FEBBuffers_[id] = FEBBuffer_t(FEBBufferCapacity_,MaxTimeDiffs_[i_id],id);
  }
  ZMQBufferUPtr.reset(new BernCRTZMQEvent[ZMQBufferCapacity_]);

  TLOG(TLVL_DEBUG)<<"\tMade %lu ZMQBuffers",FEBIDs_.size();

  TLOG(TLVL_INFO)<<"BernCRTZMQ_GeneratorBase::Initialize() ... starting GetData worker thread.";
  share::ThreadFunctor functor = std::bind(&BernCRTZMQ_GeneratorBase::GetData,this);
  auto worker_functor = share::WorkerThreadFunctorUPtr(new share::WorkerThreadFunctor(functor,"GetDataWorkerThread"));
  auto getData_worker = share::WorkerThread::createWorkerThread(worker_functor);
  GetData_thread_.swap(getData_worker);

  SeqIDMinimumSec_ = 0;
} //Initialize

/*-----------------------------------------------------------------------*/


void sbndaq::BernCRTZMQ_GeneratorBase::start() {

if(be_verbose_section){
std::cout << std::endl;
std::cout << "---------------------------" << std::endl;
std::cout << "SECTION 2 - START" << "\n";
std::cout << "---------------------------" << std::endl;
std::cout << std::endl;
}
  TLOG(TLVL_INFO)<<"BernCRTZMQ_GeneratorBase::start() called";

  std::cout << std::endl << "--- START ---" << std::endl;
  
  //get the time of the day when the run starts
  timeval start_time; gettimeofday(&start_time,NULL); //TODO: use different way to measure time. chrono perhaps
  start_time_metadata_s = start_time.tv_sec;
  start_time_metadata_ns = start_time.tv_usec*1000;
  std::cout<<"start_time [s]: " << start_time.tv_sec << "\n";
  std::cout<<"start_time [ns]: " << start_time.tv_usec*1000 << "\n" << "\n" ;

  RunNumber_ = run_number();
  current_subrun_ = 0;

  for(auto & buf : FEBBuffers_)
    buf.second.Init();
  
  ConfigureStart();
  GetData_thread_->start();

  TLOG(TLVL_INFO)<<"BernCRTZMQ_GeneratorBase::start() completed";
} //start

/*-----------------------------------------------------------------------*/


void sbndaq::BernCRTZMQ_GeneratorBase::stop() {

if(be_verbose_section){
std::cout << std::endl;
std::cout << "---------------------------" << std::endl;
std::cout << "SECTION 3 - STOP" << "\n";
std::cout << "---------------------------" << std::endl;
std::cout << std::endl;
}
  TLOG(TLVL_INFO)<<"BernCRTZMQ_GeneratorBase::stop() called";
  GetData_thread_->stop();
  ConfigureStop();
  TLOG(TLVL_INFO)<<"BernCRTZMQ_GeneratorBase::stop() completed";
} //stop

/*-----------------------------------------------------------------------*/


void sbndaq::BernCRTZMQ_GeneratorBase::stopNoMutex() {

if(be_verbose_section){
std::cout << std::endl;
std::cout << "---------------------------" << std::endl;
std::cout << "SECTION 4 - stopNoMutex" << "\n";
std::cout << "---------------------------" << std::endl;
std::cout << std::endl;
}

  TLOG(TLVL_INFO)<<"BernCRTZMQ_GeneratorBase::stopNoMutex() called";
  GetData_thread_->stop();
  ConfigureStop();
  TLOG(TLVL_INFO)<<"BernCRTZMQ_GeneratorBase::stopNoMutex() completed";
} //stopNoMutex

/*-----------------------------------------------------------------------*/


void sbndaq::BernCRTZMQ_GeneratorBase::Cleanup(){

if(be_verbose_section){
std::cout << std::endl;
std::cout << "---------------------------" << std::endl;
std::cout << "SECTION 5 - CLEANUP" << "\n";
std::cout << "---------------------------" << std::endl;
std::cout << std::endl;
}
  TLOG(TLVL_INFO)<<"BernCRTZMQ_GeneratorBase::Cleanup() called";
  TLOG(TLVL_INFO)<<"BernCRTZMQ_GeneratorBase::Cleanup() completed";
} //Cleanup

/*-----------------------------------------------------------------------*/


sbndaq::BernCRTZMQ_GeneratorBase::~BernCRTZMQ_GeneratorBase(){

if(be_verbose_section){
std::cout << std::endl;
std::cout << "---------------------------" << std::endl;
std::cout << "SECTION 6 - BernCRTZMQ_GeneratorBase Destructor" << "\n";
std::cout << "---------------------------" << std::endl;
std::cout << std::endl;
}
  TLOG(TLVL_INFO)<<"BernCRTZMQ_GeneratorBase destructor called";  
  Cleanup();
  TLOG(TLVL_INFO)<<"BernCRTZMQ_GeneratorBase destructor completed"; 
} //destructor

/*-----------------------------------------------------------------------*/


std::string sbndaq::BernCRTZMQ_GeneratorBase::GetFEBIDString(uint64_t const& id) const{

if(be_verbose_section){
std::cout << std::endl;
std::cout << "---------------------------" << std::endl;
std::cout << "SECTION 7 - GetFEBIDString" << "\n";
std::cout << "---------------------------" << std::endl;
std::cout << std::endl;
}
  std::stringstream ss_id;
  ss_id << "0x" << std::hex << std::setw(12) << std::setfill('0') << (id & 0xffffffffffff);
  //std::cout << "ss_id.str: " << ss_id.str() << std::endl;
  return ss_id.str();
} //GetFEBIDString

/*-----------------------------------------------------------------------*/


void sbndaq::BernCRTZMQ_GeneratorBase::UpdateBufferOccupancyMetrics(uint64_t const& /*id*/,
								    size_t const& ) const { //buffer_size) const {
/* No point to spam the screen with a function that does nothing (uncomment this if you add content)
if(be_verbose_section){
std::cout << std::endl;
std::cout << "---------------------------" << std::endl;
std::cout << "SECTION 8 - UpdateBufferOccupancyMetrics" << "\n";
std::cout << "---------------------------" << std::endl;
std::cout << std::endl;
}
*/

  //std::string id_str = GetFEBIDString(id);
  //metricMan->sendMetric("BufferOccupancy_"+id_str,buffer_size,"events",5,true,"BernCRTZMQGenerator");    
  //metricMan->sendMetric("BufferOccupancyPercent_"+id_str,
  //			 ((float)(buffer_size) / (float)(FEBBufferCapacity_))*100.,
  //			 "%",5,true,"BernCRTZMQGenerator");    
} //UpdateBufferOccupancyMetrics

/*-----------------------------------------------------------------------*/


size_t sbndaq::BernCRTZMQ_GeneratorBase::InsertIntoFEBBuffer(FEBBuffer_t & b,
							      size_t begin_index,
							      size_t nevents,
                                                              size_t /*total_events*/){
  if(be_verbose_section){
    std::cout << std::endl;
    std::cout << "---------------------------" << std::endl;
    std::cout << "SECTION 9 - InsertIntoFEBBuffer" << "\n";
    std::cout << "---------------------------" << std::endl;
    std::cout << std::endl;
  }

  TLOG(TLVL_DEBUG)<<__func__<<": FEB ID "<<(b.id & 0xff)<<". Current buffer size "<<b.buffer.size()<<" / "<<b.buffer.capacity()<<". Want to add "<<nevents<<" events.";

  //wait for available capacity...
  for(int i=0; (b.buffer.capacity()-b.buffer.size()) < nevents; i++) { std::cout<<(i?".":"no available capacity to save the events in FEBBuffers (I will be printing dots while waiting)!"); usleep(10); }
  std::cout << "\nCheck the buffer after waiting for capacity: " << std::endl;	
  std::cout << "Buffer Capacity: " << std::setw(5) << b.buffer.capacity() << std::endl;
  std::cout << "Buffer Size: " << std::setw(6) << b.buffer.size() <<  std::endl;


  //obtain the lock
  std::unique_lock<std::mutex> lock(*(b.mutexptr));


  // ------TRACE----------//
  TLOG(TLVL_DEBUG)<<"FEB ID "<<(b.id & 0xff)
                  <<". Current buffer size "<<b.buffer.size()
                  <<" with T0 in range ["<<b.buffer.front().Time_TS0()
                  <<", "<<b.buffer.back().Time_TS0()<<"].";
  TLOG(TLVL_DEBUG)<<"Want to add "<<nevents
                  <<" events with T0 in range ["<<ZMQBufferUPtr[begin_index].Time_TS0()
                  <<", "<<ZMQBufferUPtr[begin_index+nevents-1].Time_TS0()<<"].";

  TLOG(TLVL_DEBUG)<<"Before sort, here's contents of buffer:";
  TLOG(TLVL_DEBUG)<<"============================================";
  for(size_t i_e=0; i_e<b.buffer.size(); ++i_e)
    TLOG(TLVL_DEBUG)<<"\t\t "<<i_e
      <<" : MAC5="<<b.buffer.at(i_e).MAC5()
      <<" TS0="<<b.buffer.at(i_e).Time_TS0()
      <<" IsReference_TS0="<<b.buffer.at(i_e).IsReference_TS0();
  TLOG(TLVL_DEBUG)<<"--------------------------------------------";
  for(size_t i_e=begin_index; i_e<begin_index+nevents; ++i_e)
    TLOG(TLVL_DEBUG)<<"\t\t "<<i_e
      <<" : MAC5="<<ZMQBufferUPtr[i_e].MAC5()
      <<" TS0="<<ZMQBufferUPtr[i_e].Time_TS0()
      <<" IsReference_TS0="<<ZMQBufferUPtr[i_e].IsReference_TS0();
  TLOG(TLVL_DEBUG)<<"============================================";
  // ---------------------//

  //some timestamp
  //here I define the times since the last poll has started and finished
  //timeval time_last_poll_start;//,time_last_poll_finished;
  //time_last_poll_start.tv_sec;//, time_last_poll_finished.tv_sec = 0;
  //std::cout << "time last poll start : " << time_last_poll_start.tv_sec << std::endl << "\n";

  timeval time_poll_start; gettimeofday(&time_poll_start,NULL); //TODO: work on this!


  //for the moment, to delete after

  time_poll_start_metadata_s = time_poll_start.tv_sec;
  time_poll_start_metadata_ns = time_poll_start.tv_usec*1000;

  //time_last_poll_start_metadata_s = time_poll_start.tv_sec;
  //time_last_poll_start_metadata_ns = time_poll_start.tv_usec*1000;

  std::cout<<"time_poll_start [s] " << time_poll_start.tv_sec << std::endl;
  std::cout<<"time_poll_start [ns] " << time_poll_start.tv_usec*1000 << std::endl;

  TLOG(TLVL_DEBUG)<<"Last event looks like \n "<<ZMQBufferUPtr[nevents/*total_events-1*/].c_str();

  //BernCRTZMQEvent* last_event_ptr = &(ZMQBufferUPtr[begin_index]);
  //BernCRTZMQEvent* this_event_ptr = last_event_ptr;

  b.buffer.insert(b.buffer.end(), &(ZMQBufferUPtr[begin_index]), &(ZMQBufferUPtr[nevents+begin_index]));

  std::cout << "\nInserted into buffer on FEB " << (b.id & 0xff) << ": " << nevents << " events." << std::endl;


  //Time the poll has finished

  //timeb time_poll_finished = *((timeb*)((char*)(ZMQBufferUPtr[total_events-1].adc)+sizeof(int)+sizeof(struct timeb)));

  timeval time_poll_finished; gettimeofday(&time_poll_finished,NULL);

  std::cout<<"time_poll_finish [s] " << time_poll_finished.tv_sec << std::endl;
  std::cout<<"time_poll_finish [ns] " << time_poll_finished.tv_usec*1000 << std::endl << "\n";

  time_poll_finish_metadata_s = time_poll_finished.tv_sec ;
  time_poll_finish_metadata_ns = time_poll_finished.tv_usec*1000;


  //let's print out the content of the buffer and count the event in the feb

  for(size_t i_e=0; i_e<b.buffer.size(); ++i_e){
    //std::cout << i_e << ". Timestamp " << b.buffer.at(i_e).Time_TS0();
    //std::cout << std::setw(10) << " --- IsRefTS0? " <<b.buffer.at(i_e).IsReference_TS0() << std::endl;
    feb_event_count = i_e+1;

    time_poll_start_store_sec.push_back(time_poll_start_metadata_s/*time_poll_start.tv_sec*/);
    time_poll_start_store_nanosec.push_back(time_poll_start_metadata_ns/*time_poll_start.tv_usec*1000*/);
    time_poll_finish_store_sec.push_back(time_poll_finish_metadata_s/*time_poll_finished.tv_sec*/);
    time_poll_finish_store_nanosec.push_back(time_poll_finish_metadata_ns/*time_poll_finished.tv_usec*1000*/);

    /*std::cout << "start_" << i_e << " " << time_poll_start_store_sec[i_e] << " fin_" << i_e << " " << 	time_poll_finish_store_sec[i_e] << " [s] " << "\n";
      std::cout << "start_" << i_e << " " << time_poll_start_store_nanosec[i_e] << " fin_" << i_e << " " << time_poll_finish_store_nanosec[i_e] << " [ns] " << "\n";*/
  }


  for(size_t i_e=0; i_e<b.buffer.size(); ++i_e){
    std::cout << "start_" << i_e << " " << time_poll_start_store_sec[i_e] << " fin_" << i_e << " " << 	time_poll_finish_store_sec[i_e] << " [s] " << "\n";
    std::cout << "start_" << i_e << " " << time_poll_start_store_nanosec[i_e] << " fin_" << i_e << " " << time_poll_finish_store_nanosec[i_e] << " [ns] " << "\n";
  }


  std::cout << "\n";


  /*for(unsigned i=0; i< b.buffer.size();i++){

    std::cout << "start_" << i << " " << time_poll_start_store_sec[i] << " fin_" << i << " " << time_poll_finish_store_sec[i] << " [s] ""\n";
    std::cout << "start_" << i << " " << time_poll_start_store_nanosec[i] << " fin_" << i << " " << time_poll_finish_store_nanosec[i] << " [ns] " "\n";

    }*/

  std::cout << "feb_event_count: " << feb_event_count << std::endl;
  std::cout << std::endl;
  return b.buffer.size();

  /*
  //ok, now, we need to do a few things:
  //(1) loop through our new events, and see if things are in order (time1)
  //(2) if they aren't, check if it's a reference or overflow condition
  //(3) if not, we're gonna stop right there and drop the remaining events.
  //(4) insert good events onto the list

  TLOG(TLVL_DEBUG)<<"FEB ID 0x%lx. Current buffer size %lu with times [%u,%u].",b.id,b.buffer.size(),
  b.buffer.front().Time_TS0(),
  b.buffer.back().Time_TS0());
  TLOG(TLVL_DEBUG)<<"Want to add %lu events with times [%u,%u].",nevents,
  ZMQBufferUPtr[begin_index].Time_TS0(),
  ZMQBufferUPtr[begin_index+nevents-1].Time_TS0());

  TLOG(TLVL_DEBUG)<<"Before sort, here's contents of buffer:");
  TLOG(TLVL_DEBUG)<<"============================================");
  for(size_t i_e=0; i_e<b.buffer.size(); ++i_e)
  TLOG(TLVL_DEBUG)<<"\t\t %lu : %u %d",i_e,b.buffer.at(i_e).Time_TS0(),b.buffer.at(i_e).IsReference_TS0());
  TLOG(TLVL_DEBUG)<<"--------------------------------------------");
  for(size_t i_e=begin_index; i_e<begin_index+nevents; ++i_e)
  TLOG(TLVL_DEBUG)<<"\t\t %lu : %u %d",i_e,ZMQBufferUPtr[i_e].Time_TS0(),ZMQBufferUPtr[i_e].IsReference_TS0());
  TLOG(TLVL_DEBUG)<<"============================================");

  BernCRTZMQEvent* last_event_ptr = &(ZMQBufferUPtr[begin_index]);
  BernCRTZMQEvent* this_event_ptr = last_event_ptr;

  size_t good_events=1;
  while(good_events<nevents){
    //AA: here someone (Federico?) apparently tries to filter out junk time values (a.k.a. "spikes") from CAEN FEBs. I really hope CAEN finally fixes this issue and we won't need to uncomment these lines
    std::cout << "we are in the while cycle because good_events < nevents " << std::endl;
    std::cout << "good_events " << good_events << " <= " << nevents << " nevents"<< std::endl;
    this_event_ptr = &(ZMQBufferUPtr[good_events+begin_index]);

    //if times not in order ...
    if(this_event_ptr->Time_TS0() <= last_event_ptr->Time_TS0())
    {	std::cout << "\tif times not in order if cycle" << std::endl;
      std::cout << "this_event_ptr "<< this_event_ptr->Time_TS0() << " <= " << "last_event_ptr " <<last_event_ptr->Time_TS0() << std::endl;
      // ... and the current is not an overflow or prev is not reference
      // then we need to break out of this.
      if( !(last_event_ptr->IsReference_TS0() || this_event_ptr->IsOverflow_TS0()))
         {std::cout<<"1st.we are going to break the cycle\n";break;}
     }

    //if time difference is too large
    else
    if( (this_event_ptr->Time_TS0()-last_event_ptr->Time_TS0())>b.max_time_diff )
      {std::cout<<"2nd.we are going to break the cycle\n";
    std::cout<<"b.max_time_diff " << b.max_time_diff << std::endl;
    std::cout<<"this_event_ptr-last_event_ptr " << this_event_ptr->Time_TS0()-last_event_ptr->Time_TS0() << std::endl;

    break;}


  last_event_ptr = this_event_ptr;
  ++good_events;

  }

  //note, the order here is important. the buffer with events needs to be last, as that's
  //what is used later for the filling process to determing number of events. 
  //determining number of events is an unlocked procedure

  timeb time_poll_finished = *((timeb*)((char*)(ZMQBufferUPtr[total_events-1].adc)+sizeof(int)+sizeof(struct timeb)));

  TLOG(TLVL_DEBUG)<<"Last event looks like \n %s",ZMQBufferUPtr[total_events-1].c_str());
  TLOG(TLVL_DEBUG)<<"Time is %ld, %hu",time_poll_finished.time,time_poll_finished.millitm);

  timenow.tv_sec = time_poll_finished.time;
  timenow.tv_usec = time_poll_finished.millitm*1000;

  if(b.last_timenow.tv_sec==0){
    timeb time_poll_started = *((timeb*)((char*)(ZMQBufferUPtr[total_events-1].adc)+sizeof(int)));
    b.last_timenow.tv_sec = time_poll_started.time;
    b.last_timenow.tv_usec = time_poll_started.millitm*1000;
  }

  b.timebuffer.insert(b.timebuffer.end(),good_events,std::make_pair(b.last_timenow,timenow));
  b.droppedbuffer.insert(b.droppedbuffer.end(),good_events-1,0);
  b.droppedbuffer.insert(b.droppedbuffer.end(),1,nevents-good_events);
  b.buffer.insert(b.buffer.end(),&(ZMQBufferUPtr[begin_index]),&(ZMQBufferUPtr[good_events+begin_index])); //!!

  b.last_timenow = timenow;

  TLOG(TLVL_DEBUG)<<"After insert, here's contents of buffer:");
  TLOG(TLVL_DEBUG)<<"============================================");
  for(size_t i_e=0; i_e<b.buffer.size(); ++i_e)
    TLOG(TLVL_DEBUG)<<"\t\t %lu : %u",i_e,b.buffer.at(i_e).Time_TS0());
  if(good_events!=nevents)
    TLOG(TLVL_DEBUG)<<"\tWE DROPPED %lu EVENTS.",nevents-good_events);
    TLOG(TLVL_DEBUG)<<"============================================");

  std::cout << "Inserted into buffer on FEB " << (b.id & 0xff) << " " << good_events << " events." << std::endl;

  return b.buffer.size();*/
} //InsertIntoFEBBuffer

/*-----------------------------------------------------------------------*/


size_t sbndaq::BernCRTZMQ_GeneratorBase::EraseFromFEBBuffer(FEBBuffer_t & b, size_t const& nevents){

  if(be_verbose_section){
    std::cout << std::endl;
    std::cout << "---------------------------" << std::endl;
    std::cout << "SECTION 10 - EraseFromFEBBuffer" << "\n";
    std::cout << "---------------------------" << std::endl;
    std::cout << std::endl;
  }

  std::cout << "\n---EraseFromFEBBuffer---\n" ;

  std::cout << "Buffer Size before erasing the events: " << std::setw(3) << b.buffer.size() << " events" << std::endl;

  std::unique_lock<std::mutex> lock(*(b.mutexptr));
  //b.droppedbuffer.erase_begin(nevents); //std::cout << b.droppedbuffer.erase_begin(nevents) << std::endl;
  //b.timebuffer.erase_begin(nevents);    //std::cout << b.timebuffer.erase_begin(nevents) << std::endl;
  //b.correctedtimebuffer.erase_begin(nevents); // std::cout << b.correctedtimebuffer.erase_begin(nevents) << std::endl;
  b.buffer.erase_begin(nevents); //std::cout << b.buffer.erase_begin(nevents) << std::endl;
  std::cout << "Buffer Size after erasing the events: " << std::setw(4) << b.buffer.size() << " events" << std::endl << std::endl;
  return b.buffer.size();
} //EraseFromFEBBuffer

/*-----------------------------------------------------------------------*/

bool sbndaq::BernCRTZMQ_GeneratorBase::GetData() {

  bool be_verbose = false;

  if(be_verbose_section){
    std::cout << std::endl;
    std::cout << "---------------------------" << std::endl;
    std::cout << "SECTION 11 - GETDATA" << "\n";
    std::cout << "---------------------------" << std::endl;
    std::cout << std::endl;
  }

  TLOG(TLVL_INFO)<<"BernCRTZMQ_GeneratorBase::GetData() called";

  if( GetDataSetup()!=1 ) return false; //TODO do we need GetDataSetup and this check at all?

  const size_t data_size = GetZMQData(); //read zmq data from febdrv and fill ZMQ buffer

  //simple check of data size validity
  if(data_size % sizeof(BernCRTZMQEvent)) {
    TLOG(TLVL_ERROR)<<__func__<<": received data of "<<data_size<<" bytes cannot be divided into "<<sizeof(BernCRTZMQEvent)<<" chunks of BernCRTZMQEvent. Possible mismatch of febdrv version and FEB firmware.";
    throw cet::exception(std::string(TRACE_NAME) + __func__ + ": received data of " + std::to_string(data_size) + " bytes cannot be divided into " + std::to_string(sizeof(BernCRTZMQEvent)) + " chunks of BernCRTZMQEvent. Possible mismatch of febdrv version and FEB firmware.");
  }
  else if(data_size == 0) {
    TLOG(TLVL_ERROR)<<"BernCRTZMQ::GetData() failed. Stopping.";
    throw cet::exception("BernCRTZMQ::GetData() failed. Stopping");
    //TODO: note that we quit without turning off HV, but we can't turn off HV if zmq context is stopped, which is the reason of the failure here
  }

  size_t total_events = data_size/sizeof(BernCRTZMQEvent);
  TLOG(TLVL_DEBUG)<<__func__<<": "<<total_events<<" total events";

  size_t this_n_events=0; //number of events in given FEB
  uint64_t prev_mac = (FEBIDs_[0] & 0xffffffffffffff00) + ZMQBufferUPtr[0].MAC5();

  TLOG(TLVL_DEBUG)<<"\tBernCRTZMQ::GetData() start sorting with mac="<<prev_mac;

  for(size_t i_e = 0; i_e < total_events; i_e++) {
    //loop over events in ZMQBufferUPtr

    auto const& this_event = ZMQBufferUPtr[i_e];

    if(be_verbose){
      std::cout << "\n\t\ttotal_events " << total_events << std::endl;
      std::cout << "\n\t\tthis_event " << this_event << std::endl;
      std::cout << "\t\tIsOverflow_TS0() " << this_event.IsOverflow_TS0() << std::endl;
      std::cout << "\t\tIsOverflow_TS1() " << this_event.IsOverflow_TS1() << std::endl;
      std::cout << "\t\tIsReference_TS0() " << this_event.IsReference_TS0() << std::endl;
      std::cout << "\t\tIsReference_TS1() " << this_event.IsReference_TS1() << std::endl;
      std::cout << "\t\tTS0() " << this_event.Time_TS0() << std::endl;
      std::cout << "\t\tTS1() " << this_event.Time_TS1() << std::endl;
    }

    if((prev_mac&0xff)!=this_event.MAC5()){ //TODO: understand the logic behind this

      TLOG(TLVL_DEBUG)<<"\tBernCRTZMQ::GetData() found new MAC ("<<this_event.MAC5()
                      <<")! prev_mac="<<(prev_mac&0xff)
                      <<", iterator="<<i_e
                      <<" this_events="<<this_n_events;

      //insert group of events from a single FEB (distinct mac) to a dedicated FEBBuffer
      size_t new_buffer_size = InsertIntoFEBBuffer(FEBBuffers_[prev_mac], i_e-this_n_events, this_n_events, total_events);

      TLOG(TLVL_DEBUG)<<"\tBernCRTZMQ::GetData() ... id="<<FEBBuffers_[prev_mac].id
                      <<", n_events="<<this_n_events
                      <<", buffer_size="<<FEBBuffers_[prev_mac].buffer.size();

      //auto id_str = GetFEBIDString(prev_mac);
      //metricMan->sendMetric("EventsAdded_"+id_str,this_n_events,"events",5,true,"BernCRTZMQGenerator");
      UpdateBufferOccupancyMetrics(prev_mac, new_buffer_size); //TODO: this function does nothing as of now!

      this_n_events=0;
    }

    prev_mac = (prev_mac & 0xffffffffffffff00) + this_event.MAC5();
    ++this_n_events;
  }


  //metricMan->sendMetric("TotalEventsAdded",total_events-1,"events",5,true,"BernCRTZMQGenerator");

  if(total_events>0) return true;
  return false;
} //GetData

/*-----------------------------------------------------------------------*/


bool sbndaq::BernCRTZMQ_GeneratorBase::FillFragment(uint64_t const& feb_id,
						    artdaq::FragmentPtrs & frags)
{


  if(be_verbose_section){
    std::cout << std::endl;
    std::cout << "---------------------------" << std::endl;
    std::cout << "SECTION 12 - FILL FRAGMENT" << "\n";
    std::cout << "---------------------------" << std::endl;
    std::cout << std::endl;
  }

  TLOG(TLVL_INFO)<<"BernCRTZMQ::FillFragment(id="<<feb_id<<",frags) called";

  std::cout << "\n---FillFragment---\n";

  std::cout << "\nSTARTING SIZE OF FRAGS IN FILLFRAGMENT IS " << frags.size() << std::endl;

  auto & feb = (FEBBuffers_[feb_id]);

  /*if(!clear_buffer && feb.buffer.size()<3) {
    TLOG(TLVL_INFO)<<"BernCRTZMQ::FillFragment() completed, buffer (mostly) empty.");
    return false;
    }*/

  std::string id_str = GetFEBIDString(feb_id);

  size_t buffer_end = feb.buffer.size(); 

  std::cout << "\nCurrent size of the Buffer: " << buffer_end << " events" << std::endl;
  if(metricMan != nullptr) metricMan->sendMetric("feb_buffer_size", buffer_end, "CRT hits", 5, artdaq::MetricMode::Average);

  TLOG(TLVL_INFO)<<"BernCRTZMQ::FillFragment() : Fragment Searching. Total events in buffer="<<buffer_end;

  int time_correction; 

  //find GPS pulse and count wraparounds
  //size_t i_gps=buffer_end;
  //size_t n_wraparounds=0;  
  //uint32_t last_time =0;  
  //size_t i_e;
  //uint32_t frag_begin_time = 0;
  //uint32_t frag_end_time = 0; 
  //size_t event_in_clock = 0;  
  //BernCRTZMQEvent event_before;
  //BernCRTZMQEvent event_GPS;

  //loop over all the CRTHit events in our buffer (for this FEB)
  for(size_t i_e=0; i_e<buffer_end; ++i_e){

    //get this event!
    auto const& this_event = feb.buffer[i_e];
    //	 std::cout << this_event << std::endl; //the same as in GETDATA section
    //	 std::cout << "TS0 of the event: " << this_event.Time_TS0() << " [ns]" <<std::endl;


    //if it is a reference pulse, let's count it!
    //start to define the beginning and ending time of the fragment as well

    if(this_event.IsReference_TS0()){
      ++GPSCounter_;
      event_in_clock = 0;  //just a counter over the events within a clock of the FEB
      GPS_time += 1e9; //this_event.Time_TS0(); //time past from the very beginning
      //event_GPS = this_event;
      //frag_begin_time = GPS_time;//this_event.Time_TS0();//event_GPS.Time_TS0();
      //frag_end_time = GPS_time;//this_event.Time_TS0();//event_GPS.Time_TS0();
    } 
    else{
      ++event_in_clock;
      //frag_begin_time = GPS_time;//feb.buffer[i_e-event_in_clock].Time_TS0();
      //frag_end_time = GPS_time + this_event.Time_TS0();//feb.buffer[i_e-event_in_clock].Time_TS0() + this_event.Time_TS0();		
    }



    //if reference pulse, let's make a metric!
    if(this_event.IsReference_TS0()){
      TLOG(TLVL_INFO)<<"BernCRTZMQ::FillFragment() Found reference pulse at i_e="<<i_e<<", time="<<this_event.Time_TS0();
      if(metricMan != nullptr) metricMan->sendMetric("GPS_Ref_Time", (long int)this_event.Time_TS0() - 1e9, "ns", 5, artdaq::MetricMode::LastPoint);
      //i_gps=i_e;
    }


    timeval fragment_fill_time; gettimeofday(&fragment_fill_time,NULL);
    fragment_fill_time_metadata_s = fragment_fill_time.tv_sec;
    fragment_fill_time_metadata_ns = fragment_fill_time.tv_usec*1000;

    //std::cout<<"fragment_fill_time [s] " << fragment_fill_time_metadata_s/*fragment_fill_time.tv_sec*/ << std::endl;
    //std::cout<<"fragment_fill_time [micros] " << fragment_fill_time.tv_usec << std::endl << "\n";


    /*    
          BernCRTZMQFragmentMetadata metadata(frag_begin_time_s,frag_begin_time_ns,
          frag_end_time_s,frag_end_time_ns,
          time_correction,time_offset,
          RunNumber_,
          seq_id,
          feb_id);
     */

    if(time_poll_start_store_nanosec[i_e+1]!=time_poll_start_store_nanosec[i_e]){
      time_last_poll_start_metadata_s = time_poll_start_store_sec[i_e];
      time_last_poll_start_metadata_ns = time_poll_start_store_nanosec[i_e];
      time_last_poll_finish_metadata_s = time_poll_finish_store_sec[i_e];
      time_last_poll_finish_metadata_ns = time_poll_finish_store_nanosec[i_e];
    }


    //create metadata!
    BernCRTZMQFragmentMetadata metadata(start_time_metadata_s,start_time_metadata_ns,
        time_poll_start_store_sec[i_e],time_poll_start_store_nanosec[i_e],
        time_poll_finish_store_sec[i_e],time_poll_finish_store_nanosec[i_e],
        time_last_poll_start_metadata_s,time_last_poll_start_metadata_ns,
        time_last_poll_finish_metadata_s,time_last_poll_finish_metadata_ns,
        fragment_fill_time_metadata_s, fragment_fill_time_metadata_ns,
        feb_event_count,GPSCounter_,0,FragmentCounter_++);

    //increment n_events in metadata by 1 (to tell it we have one CRT Hit event!)
    metadata.inc_Events();
    std::cout << metadata << std::endl;
    std::cout << "event " << i_e << std::endl;
    //std::cout << "\nEventCounter metadata: " << metadata.n_events() << std::endl;
    //std::cout << "\nFragmentCounter metadata: " << metadata.sequence_number() << std::endl;
    //std::cout << "TimeStart[ns]: " << metadata.time_start_nanosec() << std::endl;
    //std::cout << "TimeEnd[ns]: " << metadata.time_end_nanosec() << std::endl;
    /*if(this_event.flags==5){
      ++GPSCounter_;
      std::cout << "\nGPSCounter: " << GPSCounter_ << std::endl;
      }else{std::cout << "This is not a GPS count" << std::endl; ++GPSCounter_; }*/




    //create our new fragment on the end of the frags vector
    frags.emplace_back( artdaq::Fragment::FragmentBytes(sizeof(BernCRTZMQEvent),  
          metadata.sequence_number(),feb_id,
          sbndaq::detail::FragmentType::BERNCRTZMQ, metadata, 
          0//metadata.time_end_nanosec() //timestamp! to be filled!!!!
          ) );

    //copy the BernCRTZMQEvent into the fragment we just created
    //  std::cout << "\n--Copy events into the fragment--\n";
    std::copy(feb.buffer.begin()+i_e,feb.buffer.begin()+i_e+1,(BernCRTZMQEvent*)(frags.back()->dataBegin()));


  }

  std::cout << "\nwe've got " << GPSCounter_ << " GPS-PPS" << std::endl;
  std::cout << "event_in_clock " << event_in_clock << std::endl;



  //delete from the buffer all the events we've just put into frags
  TLOG(TLVL_DEBUG)<<"BernCRTZMQ::FillFragment() : Buffer size before erase = "<<feb.buffer.size();
  size_t new_buffer_size = EraseFromFEBBuffer(feb,buffer_end);
  TLOG(TLVL_DEBUG)<<"BernCRTZMQ::FillFragment() : Buffer size after erase = "<<feb.buffer.size();

  //update 
  metricMan->sendMetric("FragmentsBuilt_"+id_str,buffer_end,"events/s",5,artdaq::MetricMode::Rate);
  UpdateBufferOccupancyMetrics(feb_id,new_buffer_size);

  std::cout << "ENDING SIZE OF FRAGS IN FILLFRAGMENT IS " << frags.size() << std::endl;

  time_poll_start_store_sec.clear();
  time_poll_start_store_nanosec.clear();
  time_poll_finish_store_sec.clear();
  time_poll_finish_store_nanosec.clear();

  if(!time_poll_start_store_sec.empty()){std::cout << "time vector not empty" << "\n";}

  time_poll_start_store_sec.push_back(time_poll_start_metadata_s/*time_poll_start.tv_sec*/);
  time_poll_start_store_nanosec.push_back(time_poll_start_metadata_ns/*time_poll_start.tv_usec*1000*/);
  time_poll_finish_store_sec.push_back(time_poll_finish_metadata_s/*time_poll_finished.tv_sec*/);
  time_poll_finish_store_nanosec.push_back(time_poll_finish_metadata_ns/*time_poll_finished.tv_usec*1000*/);



  return false;

  /*

     if(this_event.IsReference_TS0()){
     TLOG(TLVL_INFO)<<"BernCRTZMQ::FillFragment() Found reference pulse at i_e=%lu, time=%u",
     i_e,this_event.Time_TS0());
     if(metricMan != nullptr) metricMan->sendMetric("GPS_Ref_Time", (long int)this_event.Time_TS0() - 1e9, "ns", 5, artdaq::MetricMode::LastPoint);
     i_gps=i_e;
     }
     else if(this_event.Time_TS0()<last_time && this_event.IsOverflow_TS0()){
     n_wraparounds++;
     TLOG(TLVL_INFO)<<"BernCRTZMQ::FillFragment() Found wraparound pulse at i_e=%lu, time=%u (last_time=%u), n=%lu",
     i_e,this_event.Time_TS0(),last_time,n_wraparounds);
     }
     feb.correctedtimebuffer[i_e] = (this_event.Time_TS0()+(uint64_t)n_wraparounds*0x40000000);
     last_time = this_event.Time_TS0();

     std::cout << "last_time " << last_time << std::endl;


     if(i_gps!=buffer_end) break;
     }

  if(i_gps==buffer_end){
    TLOG(TLVL_INFO)<<"BernCRTZMQ::FillFragment() completed, buffer not empty, but waiting for GPS pulse.");
    return false;
  }
    
  //determine how much time passed. should be close to corrected time.
  //then determine the correction.
  uint32_t elapsed_secs = (feb.correctedtimebuffer[i_gps]/1e9);
  if(feb.correctedtimebuffer[i_gps]%1000000000 > 500000000)
    elapsed_secs+=1;
  time_correction = (int)(elapsed_secs*1000000000) - (int)(feb.correctedtimebuffer[i_gps]);
  
  TLOG(TLVL_INFO)<<"BernCRTZMQ::FillFragment() : time correction is %d, with elapsed_sec=%u and corrected_time=%lu",
	time_correction,elapsed_secs,feb.correctedtimebuffer[i_gps]);

  //ok, so now, determine the nearest second for the last event (closest to one second), based on ntp time
  //get the usecs from the timeval
  auto gps_timeval_pair = feb.timebuffer.at(i_gps);
  
  TLOG(TLVL_INFO)<<"BernCRTZMQ::FillFragment() : GPS time was between (%ld sec, %ld usec) and (%ld sec, %ld usec)",
	gps_timeval_pair.first.tv_sec,gps_timeval_pair.first.tv_usec,
	gps_timeval_pair.second.tv_sec,gps_timeval_pair.second.tv_usec);

  //report remainder as a metric to watch for
 // std::string id_str = GetFEBIDString(feb_id);	REMEMBER TO UNCOMMENT

  ////metricMan->sendMetric("BoundaryTimeRemainder_"+id_str,(float)(gps_timeval.tv_usec),"microseconds",false,"BernCRTZMQGenerator");
  
  //round the boundary time to the nearest second.
  //ROUND DOWN!!
  //if(gps_timeval.tv_usec > 5e5)
  //gps_timeval.tv_sec+=1;
  //gps_timeval.tv_usec = 0;

  uint32_t first_time_sec = gps_timeval_pair.first.tv_sec;

  if( (gps_timeval_pair.second.tv_sec-gps_timeval_pair.first.tv_sec)==(elapsed_secs-1) )
    first_time_sec = first_time_sec -1;

  uint32_t frag_begin_time_s = first_time_sec;
  uint32_t frag_begin_time_ns = 0;
  uint32_t frag_end_time_s = first_time_sec;
  uint32_t frag_end_time_ns = SequenceTimeWindowSize_;
  
  if(SeqIDMinimumSec_==0){
    SeqIDMinimumSec_ = frag_begin_time_s - 10;
  }

  i_e=0;
  last_time=0;
  uint64_t time_offset=0;
  size_t i_b=0;
  while(frag_end_time_s < gps_timeval_pair.second.tv_sec){

    if(frag_end_time_ns>=1000000000){
      frag_end_time_ns = frag_end_time_ns%1000000000;
      frag_end_time_s+=1;
    }

    if(frag_begin_time_ns>=1000000000){
      frag_begin_time_ns = frag_begin_time_ns%1000000000;
      frag_begin_time_s+=1;
    }

    //ms since Jan 1 2015
    uint32_t seq_id = (frag_begin_time_s-SeqIDMinimumSec_)*1000 + (frag_begin_time_ns/1000000);

    //make metadata object
	std::cout << std::endl << "METADETA:" << std::endl;
	std::cout << "frag_begin_time_s " << frag_begin_time_s << std::endl;
     	std::cout << "frag_begin_time_ns " << frag_begin_time_ns << std::endl;
	std::cout << "frag_end_time_s " << frag_end_time_s << std::endl;
	std::cout << "frag_end_time_ns " << frag_end_time_ns << std::endl;
	std::cout << "time_correction " << time_correction << std::endl;
	std::cout << "time_offset " << time_offset << std::endl;
	std::cout << "RunNumber_ " << RunNumber_ << std::endl;
	std::cout << "seq_id " << seq_id << std::endl;
	std::cout << "feb_id " << feb_id << std::endl;

    BernCRTZMQFragmentMetadata metadata(frag_begin_time_s,frag_begin_time_ns,
				     frag_end_time_s,frag_end_time_ns,
				     time_correction,time_offset,
				     RunNumber_,
				     seq_id,
				     feb_id);

    double time_corr_factor = 1.0e9 / (1000000000 - time_correction);
    
    TLOG(TLVL_INFO)<<"BernCRTZMQ::FillFragment() : looking for events in frag %u,%u (%u ms)",
	  frag_begin_time_s,frag_begin_time_ns,seq_id);
    
    while(i_e<i_gps+1){
      
      auto const& this_corrected_time = feb.correctedtimebuffer[i_e];
      auto const& this_event = feb.buffer[i_e];
      
      TLOG(TLVL_INFO)<<"BernCRTZMQ::FillFragment() : event %lu, time=%u, corrected_time=%lf",
	    i_e,this_event.Time_TS0(),(double)this_corrected_time*time_corr_factor);
      
      if( (double)this_corrected_time*time_corr_factor > (frag_end_time_s-first_time_sec)*1e9+frag_end_time_ns )
	break;


      if(this_event.Time_TS0()<last_time)
	time_offset += 0x40000000;

      
      if(this_event.lostcpu > feb.overwritten_counter)
	metadata.increment(this_event.lostfpga,this_event.lostcpu-feb.overwritten_counter,feb.droppedbuffer[i_e]);
      else
	metadata.increment(this_event.lostfpga,0,feb.droppedbuffer[i_e]);
      feb.overwritten_counter = this_event.lostcpu;
      
      //metadata.increment(this_event.lostfpga,this_event.lostcpu,feb.droppedbuffer[i_e]);

      ++i_e;
    }
    
    //great, now add the fragment on the end.`<
    frags.emplace_back( artdaq::Fragment::FragmentBytes(metadata.n_events()*sizeof(BernCRTZMQEvent),  
							metadata.sequence_number(),feb_id,
							sbndaq::detail::FragmentType::BERNCRTZMQ, metadata) );
    std::copy(feb.buffer.begin()+i_b,feb.buffer.begin()+i_e,(BernCRTZMQEvent*)(frags.back()->dataBegin()));

	//for (auto i = frags.begin(); i != frags.end(); ++i)
    //std::cout << i << ' ';
	//std::cout << "fragment back " << frags.back()->dataBegin() << std::endl;

    std::cout << std::endl << "frags.size " << frags.size() << std::endl << std::endl;
	

    TLOG(TLVL_DEBUG)<<"BernCRTZMQ::FillFragment() : Fragment created. Type %d. First event in fragment: %s",
	  sbndaq::detail::FragmentType::BERNCRTZMQ,
	  ((BernCRTZMQEvent*)(frags.back()->dataBegin()))->c_str() );
    
    TLOG(TLVL_INFO)<<"BernCRTZMQ::FillFragment() : Fragment created. Events=%u. Metadata : %s",
	  metadata.n_events(),metadata.c_str());
        
    SendMetadataMetrics(metadata);


    i_b=i_e; //new beginning...
    frag_begin_time_ns += SequenceTimeWindowSize_;
    frag_end_time_ns += SequenceTimeWindowSize_;
  }

  TLOG(TLVL_DEBUG)<<"BernCRTZMQ::FillFragment() : Buffer size before erase = %lu",feb.buffer.size());
  size_t new_buffer_size = EraseFromFEBBuffer(feb,i_gps+1);
  TLOG(TLVL_DEBUG)<<"BernCRTZMQ::FillFragment() : Buffer size after erase = %lu",feb.buffer.size());

  //metricMan->sendMetric("FragmentsBuilt_"+id_str,1.0,"events",5,true,"BernCRTZMQGenerator");
  UpdateBufferOccupancyMetrics(feb_id,new_buffer_size);

  //return true;

  return false;
  */

} //FillFragment

/*-----------------------------------------------------------------------*/


void sbndaq::BernCRTZMQ_GeneratorBase::SendMetadataMetrics(BernCRTZMQFragmentMetadata const& /*m*/) {

if(be_verbose_section){
std::cout << std::endl;
std::cout << "---------------------------" << std::endl;
std::cout << "SECTION 13 - SendMetadataMetrics" << "\n";
std::cout << "---------------------------" << std::endl;
std::cout << std::endl;
}

//  std::string id_str = GetFEBIDString(m.feb_id());



  //metricMan->sendMetric("FragmentLastTime_"+id_str,(uint64_t)(m.time_end_seconds()*1000000000+m.time_end_nanosec()),"ns",5,false,"BernCRTZMQGenerator");
  //metricMan->sendMetric("EventsInFragment_"+id_str,(float)(m.n_events()),"events",5,true,"BernCRTZMQGenerator");
  //metricMan->sendMetric("MissedEvents_"+id_str,     (float)(m.missed_events()),     "events",5,true,"BernCRTZMQGenerator");
  //metricMan->sendMetric("OverwrittenEvents_"+id_str,(float)(m.overwritten_events()),"events",5,true,"BernCRTZMQGenerator");
  //float eff=1.0;
  //if((m.n_events()+m.missed_events()+m.overwritten_events())!=0)
  //eff = (float)(m.n_events()) / (float)(m.n_events()+m.missed_events()+m.overwritten_events());
  
  //metricMan->sendMetric("Efficiency_"+id_str,eff*100.,"%",5,true,"BernCRTZMQGenerator");
}

/*-----------------------------------------------------------------------*/


bool sbndaq::BernCRTZMQ_GeneratorBase::getNext_(artdaq::FragmentPtrs & frags) {

if(be_verbose_section){
std::cout << std::endl;
std::cout << "---------------------------" << std::endl;
std::cout << "SECTION 14 - getNext" << "\n";
std::cout << "---------------------------" << std::endl;
std::cout << std::endl;
}


  std::cout << "\n---Calling getNext_---" << std::endl;

  TLOG(TLVL_INFO)<<"BernCRTZMQ::getNext_(frags) called";

  std::cout << std::endl << "STARTING SIZE OF frags.size IN GETNEXT_ IS " << frags.size() << std::endl << std::endl;
  
  //throttling...
  if (throttle_usecs_ > 0) {
    size_t nchecks = throttle_usecs_ / throttle_usecs_check_;
    
    for (size_t i_c = 0; i_c < nchecks; ++i_c) {
      usleep( throttle_usecs_check_ );
      
      if (should_stop()) {
	return false;
      }
    }
  } else {
    if (should_stop()) {
      return false;
    }
  }
  
  for(auto const& id : FEBIDs_){
    while(true){
      if(!FillFragment(id,frags)) break;
    }
  }

  TLOG(TLVL_INFO)<<"BernCRTZMQ::getNext_(frags) completed";

  std::cout << "ENDING SIZE OF frags.size IN GETNEXT_ IS " << frags.size() << std::endl;
  //std::cout << "---PRINT OUT FRAGMENT IN GETNEXT_---" << std::endl  << std::endl;

 /* if(frags.size()!=0){
    BernCRTZMQFragment bb(*frags.back());
    std::cout << *(bb.eventdata(0)) << std::endl;
  }*/


  if(frags.size()!=0) TLOG(TLVL_DEBUG)<<"BernCRTZMQ::geNext_() : last fragment is: "<<(BernCRTZMQFragment(*frags.back())).c_str();

  return true;

} //getNext_

/*-----------------------------------------------------------------------*/

// The following macro is defined in artdaq's GeneratorMacros.hh header
//DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(demo::BernCRTZMQ) 
