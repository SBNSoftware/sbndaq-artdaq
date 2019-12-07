#include "sbndaq-artdaq/Generators/Common/BernCRTZMQ_GeneratorBase.hh"

#include "cetlib_except/exception.h"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "fhiclcpp/ParameterSet.h"
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

//TRACE level for messages sent once per event
#define TLVL_SPECIAL 11


sbndaq::BernCRTZMQ_GeneratorBase::BernCRTZMQ_GeneratorBase(fhicl::ParameterSet const & ps)
  :
  CommandableFragmentGenerator(ps),
  ps_(ps)
{
  TLOG(TLVL_INFO)<<"constructor called";  
  Initialize();
  TLOG(TLVL_INFO)<<"constructor completed";
}

/*---------------------------------------------------------------------*/

void sbndaq::BernCRTZMQ_GeneratorBase::Initialize() {

  TLOG(TLVL_INFO)<<__func__<<" called";

  RunNumber_ = 0;
  FEBIDs_ = ps_.get< std::vector<uint8_t> >("FEBIDs");

  //new variable added by me (see the header file)
  //TODO possibly all this need to be removed
  FragmentCounter_ = 0;
  GPSCounter_= 0;
  event_in_clock = 0;
  GPS_time = 0;

  //reset last poll value
  last_poll_start = 0;
  last_poll_end = 0;

  FEBBufferCapacity_ = ps_.get<uint32_t>("FEBBufferCapacity");
  ZMQBufferCapacity_ = ps_.get<uint32_t>("ZMQBufferCapacity");

  MaxTimeDiffs_ = ps_.get< std::vector<uint32_t> >("MaxTimeDiffs",std::vector<uint32_t>(FEBIDs_.size()));

  if(MaxTimeDiffs_.size() != FEBIDs_.size()) { //check FHiCL file validity
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
        throttle_usecs_ % throttle_usecs_check_ != 0) ) { //check FHiCL file validity
    throw cet::exception("Error in BernCRTZMQ: disallowed combination of throttle_usecs and throttle_usecs_check (see BernCRTZMQ.hh for rules)");
  }

  //Initialize buffers
  for(size_t i_id=0; i_id<FEBIDs_.size(); ++i_id){
    auto const& id = FEBIDs_[i_id];
    FEBBuffers_[id] = FEBBuffer_t(FEBBufferCapacity_,MaxTimeDiffs_[i_id],id);
  }
  ZMQBufferUPtr.reset(new BernCRTZMQEvent[ZMQBufferCapacity_]);

  TLOG(TLVL_DEBUG) << "\tMade %lu ZMQBuffers",FEBIDs_.size();

  TLOG(TLVL_INFO)<<"BernCRTZMQ_GeneratorBase::Initialize() completed";

  TLOG(TLVL_INFO)<<"BernCRTZMQ_GeneratorBase::Initialize() ... starting GetData worker thread.";
  share::ThreadFunctor functor = std::bind(&BernCRTZMQ_GeneratorBase::GetData,this);
  auto worker_functor = share::WorkerThreadFunctorUPtr(new share::WorkerThreadFunctor(functor,"GetDataWorkerThread"));
  auto getData_worker = share::WorkerThread::createWorkerThread(worker_functor);
  GetData_thread_.swap(getData_worker);

  SeqIDMinimumSec_ = 0;
} //Initialize

/*-----------------------------------------------------------------------*/


void sbndaq::BernCRTZMQ_GeneratorBase::start() {
  TLOG(TLVL_INFO)<<__func__<<" called";

  start_time_metadata = std::chrono::system_clock::now().time_since_epoch().count();
  TLOG(TLVL_DEBUG)<<__func__<<" Run start time: " << sbndaq::BernCRTZMQFragment::print_timestamp(start_time_metadata);

  RunNumber_ = run_number();
  current_subrun_ = 0;

  for(auto & buf : FEBBuffers_)
    buf.second.Init();
  
  ConfigureStart();
  GetData_thread_->start();

  TLOG(TLVL_INFO)<<__func__<<" completed";
} //start

/*-----------------------------------------------------------------------*/


void sbndaq::BernCRTZMQ_GeneratorBase::stop() {
  TLOG(TLVL_INFO)<<__func__<<" called";
  GetData_thread_->stop();
  ConfigureStop();
  TLOG(TLVL_INFO)<<__func__<<" completed";
} //stop

/*-----------------------------------------------------------------------*/


void sbndaq::BernCRTZMQ_GeneratorBase::stopNoMutex() {
TLOG(TLVL_INFO)<<__func__<<" called";
  GetData_thread_->stop();
  ConfigureStop();
  TLOG(TLVL_INFO)<<__func__<<" completed";
} //stopNoMutex

/*-----------------------------------------------------------------------*/


void sbndaq::BernCRTZMQ_GeneratorBase::Cleanup(){
  TLOG(TLVL_INFO)<<__func__<<" called";
  TLOG(TLVL_INFO)<<__func__<<" completed";
} //Cleanup

/*-----------------------------------------------------------------------*/


sbndaq::BernCRTZMQ_GeneratorBase::~BernCRTZMQ_GeneratorBase(){
  TLOG(TLVL_INFO)<<" destructor called";  
  Cleanup();
  TLOG(TLVL_INFO)<<" destructor completed"; 
} //destructor

/*-----------------------------------------------------------------------*/


std::string sbndaq::BernCRTZMQ_GeneratorBase::GetFEBIDString(uint64_t const& id) const{
  std::stringstream ss_id;
  ss_id << "0x" << std::hex <<id;
  return ss_id.str();
} //GetFEBIDString

/*-----------------------------------------------------------------------*/


void sbndaq::BernCRTZMQ_GeneratorBase::UpdateBufferOccupancyMetrics(uint64_t const& /*id*/,
								    size_t const& ) const { //buffer_size) const {

  TLOG(TLVL_DEBUG)<<__func__<<" called";

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
                                                              size_t /* */){ //TODO perhaps we can get rid of this?

  TLOG(TLVL_DEBUG) << __func__ << ": FEB ID " << b.id
    << ". Current buffer size " << b.buffer.size() << " / " << b.buffer.capacity()
    << ". Want to add " << nevents << " events.";

  //wait for available capacity...
  for(int i=0; (b.buffer.capacity()-b.buffer.size()) < nevents; i++) {
    //TODO make use of circular buffer and overwrite old events if buffer is full
    if(i%100000 == 0) TLOG(TLVL_WARNING)<<"No available capacity to save the events in FEBBuffers!";
    usleep(10);
  }

  //obtain the lock
  std::unique_lock<std::mutex> lock(*(b.mutexptr));

  TLOG(TLVL_DEBUG) << "FEB ID " << b.id
    << ". Current FEB buffer size " << b.buffer.size()
    << " with T0 in range [" << sbndaq::BernCRTZMQFragment::print_timestamp(b.buffer.front().Time_TS0())
    << ", " << sbndaq::BernCRTZMQFragment::print_timestamp(b.buffer.back().Time_TS0()) << "].";
  TLOG(TLVL_DEBUG) << "Want to add " << nevents
    << " events with T0 in range [" << sbndaq::BernCRTZMQFragment::print_timestamp(ZMQBufferUPtr[begin_index].Time_TS0())
    << ", " << sbndaq::BernCRTZMQFragment::print_timestamp(ZMQBufferUPtr[begin_index+nevents-1].Time_TS0()) << "].";


  //Insert events into FEBBuffer, the most important line of the function
  b.buffer.insert(b.buffer.end(), &(ZMQBufferUPtr[begin_index]), &(ZMQBufferUPtr[nevents+begin_index]));
  BernCRTZMQFragmentMetadata metadata(
      start_time_metadata,
      this_poll_start,
      this_poll_end,
      last_poll_start,
      last_poll_end,
      nevents,
      7, //GPSCounter_, (temporary placeholder, a random number obtained using a fair dice)
      0, //events_in_data_packet
      FragmentCounter_); //sequence number
  metadata.inc_Events(); //increase number of events in fragment to 1

  for(size_t i_e=0; i_e<nevents; ++i_e) {
    metadata.inc_SequenceNumber();
    b.metadata_buffer.push_back(metadata);
  }
  FragmentCounter_ += nevents;

  feb_event_count = b.buffer.size();

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
  TLOG(TLVL_SPECIAL) << "we are in the while cycle because good_events < nevents " << std::endl;
  TLOG(TLVL_SPECIAL) << "good_events " << good_events << " <= " << nevents << " nevents"<< std::endl;
  this_event_ptr = &(ZMQBufferUPtr[good_events+begin_index]);

  //if times not in order ...
  if(this_event_ptr->Time_TS0() <= last_event_ptr->Time_TS0())
  {	TLOG(TLVL_SPECIAL) << "\tif times not in order if cycle" << std::endl;
  TLOG(TLVL_SPECIAL) << "this_event_ptr "<< this_event_ptr->Time_TS0() << " <= " << "last_event_ptr " <<last_event_ptr->Time_TS0() << std::endl;
  // ... and the current is not an overflow or prev is not reference
  // then we need to break out of this.
  if( !(last_event_ptr->IsReference_TS0() || this_event_ptr->IsOverflow_TS0()))
  {TLOG(TLVL_SPECIAL)<<"1st.we are going to break the cycle\n";break;}
  }

  //if time difference is too large
  else
  if( (this_event_ptr->Time_TS0()-last_event_ptr->Time_TS0())>b.max_time_diff ) {
  TLOG(TLVL_SPECIAL)<<"2nd.we are going to break the cycle\n";
  TLOG(TLVL_SPECIAL)<<"b.max_time_diff " << b.max_time_diff << std::endl;
  TLOG(TLVL_SPECIAL)<<"this_event_ptr-last_event_ptr " << this_event_ptr->Time_TS0()-last_event_ptr->Time_TS0() << std::endl;

  break;
  }

  last_event_ptr = this_event_ptr;
  ++good_events;

  }

  //note, the order here is important. the buffer with events needs to be last, as that's
  //what is used later for the filling process to determing number of events. 
  //determining number of events is an unlocked procedure

  //AA: This is an attempt to fill timebuffers (variables not filled otherwise in the uncommented code above)

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

  TLOG(TLVL_SPECIAL) << "Inserted into buffer on FEB " << (b.id & 0xff) << " " << good_events << " events." << std::endl;

  return b.buffer.size();*/
} //InsertIntoFEBBuffer

/*-----------------------------------------------------------------------*/


size_t sbndaq::BernCRTZMQ_GeneratorBase::EraseFromFEBBuffer(FEBBuffer_t & b, size_t const& nevents){

  TLOG(TLVL_DEBUG) << "Buffer Size before erasing the events: " << std::setw(3) << b.buffer.size() << " events";

  std::unique_lock<std::mutex> lock(*(b.mutexptr));
  b.buffer.erase_begin(nevents);
  b.metadata_buffer.erase_begin(nevents);
  TLOG(TLVL_DEBUG) << "Buffer Size after erasing the events: " << std::setw(4) << b.buffer.size() << " events";
  return b.buffer.size();
} //EraseFromFEBBuffer

/*-----------------------------------------------------------------------*/

bool sbndaq::BernCRTZMQ_GeneratorBase::GetData() {

  TLOG(TLVL_DEBUG) << "BernCRTZMQ_GeneratorBase::GetData() called";

  if( GetDataSetup()!=1 ) return false; //TODO do we need GetDataSetup and this check at all?

  const size_t data_size = GetZMQData(); //read zmq data from febdrv and fill ZMQ buffer

  //simple check of data size validity
  if(data_size % sizeof(BernCRTZMQEvent)) {
    TLOG(TLVL_ERROR)<<__func__<<": received data of "<<data_size<<" bytes cannot be divided into "<<sizeof(BernCRTZMQEvent)<<" chunks of BernCRTZMQEvent. Possible mismatch of febdrv version and FEB firmware.";
    throw cet::exception(std::string(TRACE_NAME) + __func__ + ": received data of " + std::to_string(data_size) + " bytes cannot be divided into " + std::to_string(sizeof(BernCRTZMQEvent)) + " chunks of BernCRTZMQEvent. Possible mismatch of febdrv version and FEB firmware.");
  }
  else if(data_size == 0) {
    TLOG(TLVL_ERROR)<<"BernCRTZMQ::GetData() Obtained data size of 0. Stopping.";
    throw cet::exception("BernCRTZMQ::GetData() Obtained data size of 0. Stopping");
    //TODO: note that we quit without turning off HV, but we can't turn off HV if zmq context is stopped, which is the reason of the failure here
  }

  size_t total_events = data_size/sizeof(BernCRTZMQEvent); //number of events in all FEBs, including the last event containing the timing information
  TLOG(TLVL_DEBUG)<<__func__<<": "<<(total_events-1)<<"+1 events ";

  size_t this_n_events=0; //number of events in given FEB

  //NOTE: value in ZMQBuffer is 16 bit. We just assume it will always be less than 0xff
  //      In particular the special last event in ZMQ buffer has "mac5" field set to 0x0xffff
  uint16_t prev_mac = ZMQBufferUPtr[0].MAC5();

  //Get the special last event and read the time information from it
  //NOTE: The code below assumes we have only one poll in a single buffer.
  //      I'm not sure if this will be true if we decrease the poll length.
  //TODO: Test it at short poll times, and allow for several polls in a zmq paquet
  TLOG(TLVL_DEBUG)<<"Reading information from the last zeromq event";
  /*
   * AA: I apologize for the ugly pointer arithmetics below. The situation
   *     is that the structure sent by the present version of febdrv (see
   *     function senddata) does not follow the documentation. Perhaps all
   *     this should be reviewed and fixed, but for now I want to get it
   *     running properly.
   *     TODO: change it to use the same structure here and in febdrv
   */
  uint8_t* last_event_ptr = (uint8_t*)&(ZMQBufferUPtr[total_events-1].adc[0]);
  uint32_t n_events;
  timeb    poll_time_start;
  timeb    poll_time_end; 
  memcpy(&n_events,        last_event_ptr, sizeof(uint32_t)); last_event_ptr += sizeof(uint32_t);
  memcpy(&poll_time_start, last_event_ptr, sizeof(timeb));    last_event_ptr += sizeof(timeb);
  memcpy(&poll_time_end,   last_event_ptr, sizeof(timeb));
  
  //convert to ns since epoch
  this_poll_start = poll_time_start.time * 1000*1000*1000 + poll_time_start.millitm * 1000*1000;
  this_poll_end   = poll_time_end  .time * 1000*1000*1000 + poll_time_end  .millitm * 1000*1000;

  if(last_poll_start == 0) { //fix the poll timestamp for the very first poll, other wise the numbers won't make any sense (they still may make no sense, though...)
    last_poll_start = this_poll_start - 300*1000*1000;//TODO dummy value... move to fhicl file?
    last_poll_end   = this_poll_end   - 300*1000*1000;//TODO dummy value... move to fhicl file?
  }

  TLOG(TLVL_DEBUG)<<"Number of events is "<<n_events;
  TLOG(TLVL_DEBUG)<<"this_poll_start  " << sbndaq::BernCRTZMQFragment::print_timestamp(this_poll_start);
  TLOG(TLVL_DEBUG)<<"this_poll_end    " << sbndaq::BernCRTZMQFragment::print_timestamp(this_poll_end);
  TLOG(TLVL_DEBUG)<<"last_poll_start  " << sbndaq::BernCRTZMQFragment::print_timestamp(last_poll_start);
  TLOG(TLVL_DEBUG)<<"last_poll_end    " << sbndaq::BernCRTZMQFragment::print_timestamp(last_poll_end);

  if(n_events != total_events -1) {
    TLOG(TLVL_DEBUG)<<"BernCRTZMQ::"<<__func__<<" Data corruption! Number of events reported in the last zmq event: "<<n_events<<" differs from what you expect from the packet size: "<<(total_events-1);
    throw cet::exception("BernCRTZMQ::GetData() Data corruption! Number of events reported in the last zmq event differs from what you expect from the packet size ");
  }

  TLOG(TLVL_DEBUG)<<"\tBernCRTZMQ::GetData() start sorting with mac="<<prev_mac;

  for(size_t i_e = 0; i_e < total_events; i_e++) { //loop over events in ZMQBufferUPtr

    auto const& this_event = ZMQBufferUPtr[i_e];

    if(prev_mac != this_event.MAC5()){
      /*
       * febdrv zeromq packet looks as follows:
       * - events for first MAC5
       * - events for second MAC5
       * ...
       * - events for the last MAC5
       * - additional event with timing information (which has MAC5=0xffff)
       *
       * Whenever we see a new MAC5 and we know the previous one is complete and we can insert it into the buffer.
       * The timing information event isn't inserted into buffer directly, but is accessed directly by InsertIntoFEBBuffer
       *
       * TODO: note that the above assumes the zmq packet contains a single poll only! If we start polling more often
       * the code may not work properly -> write something more robust.
       */

      TLOG(TLVL_DEBUG)<<"BernCRTZMQ::GetData() found new MAC ("<<this_event.MAC5()
                      <<")! prev_mac="<<prev_mac
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

    prev_mac = this_event.MAC5();
    ++this_n_events;
  }

  last_poll_start = this_poll_start;
  last_poll_end = this_poll_end;

  //metricMan->sendMetric("TotalEventsAdded",total_events-1,"events",5,true,"BernCRTZMQGenerator");

  return true;
} //GetData

/*-----------------------------------------------------------------------*/


bool sbndaq::BernCRTZMQ_GeneratorBase::FillFragment(uint64_t const& feb_id,
						    artdaq::FragmentPtrs & frags) {

  TLOG(TLVL_DEBUG) << __func__<<"(feb_id=" << feb_id << ") called";

  TLOG(TLVL_DEBUG) <<__func__<< " STARTING SIZE OF FRAGS IN FILL FRAGMENT IS " << frags.size() << std::endl;

  auto & feb = (FEBBuffers_[feb_id]);

  size_t buffer_end = feb.buffer.size(); 
  size_t metadata_buffer_end = feb.metadata_buffer.size();

  if(buffer_end != metadata_buffer_end) {
    //This error would suggest either there is a bug in function filling the buffers
    TLOG(TLVL_ERROR) <<__func__<<": FEB buffer size: "<<buffer_end<<" ≠ metadata buffer size: "<< metadata_buffer_end;
    throw cet::exception("BernCRTZMQ_GeneratorBase::FillFragment: Error: FEB buffer size differs from metadata buffer size!");
  }

  TLOG(TLVL_DEBUG) <<__func__ << " Current size of the FEB buffer: " << buffer_end << " events";
  if(metricMan != nullptr) metricMan->sendMetric("feb_buffer_size", buffer_end, "CRT hits", 5, artdaq::MetricMode::Average);

  //loop over all the CRTHit events in our buffer (for this FEB)
  for(size_t i_e=0; i_e<buffer_end; ++i_e) {
    auto const& this_event = feb.buffer[i_e];
    auto const& metadata = feb.metadata_buffer[i_e];

    //assign timestamp to the event
    int ts0  = this_event.ts0;

    //add PPS cable offset modulo 1s
    ts0 = (ts0 + feb_configuration[this_event.mac5].GetPPSOffset()) % (1000*1000*1000);
    if(ts0 < 0) ts0 += 1000*1000*1000;

    uint64_t mean_poll_time = metadata.last_poll_start()/2 + metadata.this_poll_end()/2;
    int mean_poll_time_ns = mean_poll_time % (1000*1000*1000); 
    
    uint64_t timestamp = 0;

    if(ts0 - mean_poll_time_ns < -500*1000*1000) {
      timestamp = mean_poll_time - mean_poll_time_ns + ts0 + 1000*1000*1000;
    }
    else if(ts0 - mean_poll_time_ns > 500*1000*1000) {
      timestamp = mean_poll_time - mean_poll_time_ns + ts0 - 1000*1000*1000;
    }
    else {
      timestamp = mean_poll_time - mean_poll_time_ns + ts0;
    } 

    TLOG(TLVL_SPECIAL)<<__func__ << " Event: " << i_e<<"\n"<<__fu metadata;
    TLOG(TLVL_SPECIAL)<<__func__ << " Timestamp:       "<<sbndaq::BernCRTZMQFragment::print_timestamp(timestamp);

    //create our new fragment on the end of the frags vector
    frags.emplace_back( artdaq::Fragment::FragmentBytes(
          sizeof(BernCRTZMQEvent),  
          metadata.sequence_number(),
          feb_id,
          sbndaq::detail::FragmentType::BERNCRTZMQ,
          metadata,
          timestamp
          ) ); 

    //copy the BernCRTZMQEvent into the fragment we just created
    std::copy(feb.buffer.begin()+i_e,
              feb.buffer.begin()+i_e+1,
              (BernCRTZMQEvent*)(frags.back()->dataBegin())); 

  } //loop over events in feb buffer

  //delete from the buffer all the events we've just put into frags
  TLOG(TLVL_DEBUG)<<__func__<<": FEB Buffer size before erase = "<<feb.buffer.size();
  size_t new_buffer_size = EraseFromFEBBuffer(feb,buffer_end);
  TLOG(TLVL_DEBUG)<<__func__": FEB Buffer size after erase = "<<feb.buffer.size();

  //update 
  std::string id_str = GetFEBIDString(feb_id);
  metricMan->sendMetric("FragmentsBuilt_"+id_str,buffer_end,"events/s",5,artdaq::MetricMode::Rate);
  UpdateBufferOccupancyMetrics(feb_id,new_buffer_size);

  TLOG(TLVL_DEBUG) <<__func__<< " ENDING SIZE OF FRAGS IN FILLFRAGMENT IS " << frags.size();

  return false;

  /*
    if(this_event.IsReference_TS0()) {
      TLOG(TLVL_INFO)<<"BernCRTZMQ::FillFragment() Found reference pulse at i_e=%lu, time=%u",
      i_e,this_event.Time_TS0());
      if(metricMan != nullptr) metricMan->sendMetric("GPS_Ref_Time", (long int)this_event.Time_TS0() - 1e9, "ns", 5, artdaq::MetricMode::LastPoint);
      i_gps=i_e;
    }
    else if(this_event.Time_TS0()<last_time && this_event.IsOverflow_TS0()) {
      n_wraparounds++;
      TLOG(TLVL_INFO)<<"BernCRTZMQ::FillFragment() Found wraparound pulse at i_e=%lu, time=%u (last_time=%u), n=%lu",
      i_e,this_event.Time_TS0(),last_time,n_wraparounds);
    }
    feb.correctedtimebuffer[i_e] = (this_event.Time_TS0()+(uint64_t)n_wraparounds*0x40000000);
    last_time = this_event.Time_TS0();

    TLOG(TLVL_SPECIAL) << "last_time " << last_time << std::endl;


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
	TLOG(TLVL_SPECIAL) << std::endl << "METADETA:" << std::endl;
	TLOG(TLVL_SPECIAL) << "frag_begin_time_s " << frag_begin_time_s << std::endl;
     	TLOG(TLVL_SPECIAL) << "frag_begin_time_ns " << frag_begin_time_ns << std::endl;
	TLOG(TLVL_SPECIAL) << "frag_end_time_s " << frag_end_time_s << std::endl;
	TLOG(TLVL_SPECIAL) << "frag_end_time_ns " << frag_end_time_ns << std::endl;
	TLOG(TLVL_SPECIAL) << "time_correction " << time_correction << std::endl;
	TLOG(TLVL_SPECIAL) << "time_offset " << time_offset << std::endl;
	TLOG(TLVL_SPECIAL) << "RunNumber_ " << RunNumber_ << std::endl;
	TLOG(TLVL_SPECIAL) << "seq_id " << seq_id << std::endl;
	TLOG(TLVL_SPECIAL) << "feb_id " << feb_id << std::endl;

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
    //TLOG(TLVL_SPECIAL) << i << ' ';
	//TLOG(TLVL_SPECIAL) << "fragment back " << frags.back()->dataBegin() << std::endl;

    TLOG(TLVL_SPECIAL) << std::endl << "frags.size " << frags.size() << std::endl << std::endl;
	

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

  TLOG(TLVL_DEBUG)<<__func__<<" called";

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

  TLOG(TLVL_DEBUG) <<__func__<< " called with frags.size = " << frags.size();
  
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
      if(!FillFragment(id, frags)) break;
    }
  }

  TLOG(TLVL_DEBUG) <<__func__<< ": completed with frags.size = " << frags.size();

  return true;
} //getNext_

/*-----------------------------------------------------------------------*/

// The following macro is defined in artdaq's GeneratorMacros.hh header
//DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(demo::BernCRTZMQ) 
