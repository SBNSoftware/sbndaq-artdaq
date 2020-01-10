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

  FEBIDs_ = ps_.get< std::vector<uint8_t> >("FEBIDs");

  //new variable added by me (see the header file)
  //TODO possibly all this need to be removed
  FragmentCounter_ = 0;

  //reset last poll value
  last_poll_start = 0;
  last_poll_end = 0;

  uint32_t FEBBufferCapacity_ = ps_.get<uint32_t>("FEBBufferCapacity");
  ZMQBufferCapacity_ = ps_.get<uint32_t>("ZMQBufferCapacity");

  throttle_usecs_ = ps_.get<size_t>("throttle_usecs");
  throttle_usecs_check_ = ps_.get<size_t>("throttle_usecs_check");

  if (throttle_usecs_ > 0 && (throttle_usecs_check_ >= throttle_usecs_ ||
        throttle_usecs_ % throttle_usecs_check_ != 0) ) { //check FHiCL file validity
    throw cet::exception("Error in BernCRTZMQ: disallowed combination of throttle_usecs and throttle_usecs_check (see BernCRTZMQ.hh for rules)");
  }

  //Initialize buffers
  for(size_t i_id=0; i_id<FEBIDs_.size(); ++i_id){
    auto const& id = FEBIDs_[i_id];
    FEBBuffers_[id] = FEBBuffer_t(FEBBufferCapacity_,id);
  }
  ZMQBufferUPtr.reset(new BernCRTZMQEvent[ZMQBufferCapacity_]);

  TLOG(TLVL_DEBUG)<< __func__ << " Created ZMQBuffer of size of "<<ZMQBufferCapacity_;

  TLOG(TLVL_INFO)<<__func__<<" completed ... starting GetData worker thread.";
  share::ThreadFunctor functor = std::bind(&BernCRTZMQ_GeneratorBase::GetData,this);
  auto worker_functor = share::WorkerThreadFunctorUPtr(new share::WorkerThreadFunctor(functor,"GetDataWorkerThread"));
  auto getData_worker = share::WorkerThread::createWorkerThread(worker_functor);
  GetData_thread_.swap(getData_worker);
} //Initialize

/*-----------------------------------------------------------------------*/


void sbndaq::BernCRTZMQ_GeneratorBase::start() {
  TLOG(TLVL_INFO)<<__func__<<" called";

  run_start_time = std::chrono::system_clock::now().time_since_epoch().count();
  TLOG(TLVL_DEBUG)<<__func__<<" Run start time: " << sbndaq::BernCRTZMQFragment::print_timestamp(run_start_time);

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
    << " with T0 in range [" << sbndaq::BernCRTZMQFragment::print_timestamp(b.buffer.front().first.Time_TS0())
    << ", " << sbndaq::BernCRTZMQFragment::print_timestamp(b.buffer.back().first.Time_TS0()) << "].";
  TLOG(TLVL_DEBUG) << "Want to add " << nevents
    << " events with T0 in range [" << sbndaq::BernCRTZMQFragment::print_timestamp(ZMQBufferUPtr[begin_index].Time_TS0())
    << ", " << sbndaq::BernCRTZMQFragment::print_timestamp(ZMQBufferUPtr[begin_index+nevents-1].Time_TS0()) << "].";

  //prepare metadata object
  BernCRTZMQFragmentMetadata metadata(
      run_start_time,
      this_poll_start,
      this_poll_end,
      last_poll_start,
      last_poll_end,
      system_clock_deviation,
      nevents, //_feb_event_count
      0, //events_in_zmq_packet
      FragmentCounter_); //sequence number
  metadata.inc_Events(); //there is one event in a fragment

  //Insert events into FEBBuffer
  for(size_t i_e=0; i_e<nevents; ++i_e) {
    metadata.inc_SequenceNumber();
    b.buffer.push_back(std::make_pair(ZMQBufferUPtr[begin_index + i_e], metadata));
  }
  FragmentCounter_ += nevents;

  return b.buffer.size();
} //InsertIntoFEBBuffer

/*-----------------------------------------------------------------------*/


size_t sbndaq::BernCRTZMQ_GeneratorBase::EraseFromFEBBuffer(FEBBuffer_t & b, size_t const& nevents){

  TLOG(TLVL_DEBUG) <<__func__<< "Buffer size before erasing the events: " << std::setw(3) << b.buffer.size() << " events";

  std::unique_lock<std::mutex> lock(*(b.mutexptr));
  b.buffer.erase_begin(nevents);
//  b.metadata_buffer.erase_begin(nevents);
  TLOG(TLVL_DEBUG) <<__func__<< "Buffer size after erasing the events: " << std::setw(4) << b.buffer.size() << " events";
  return b.buffer.size();
} //EraseFromFEBBuffer

/*-----------------------------------------------------------------------*/

bool sbndaq::BernCRTZMQ_GeneratorBase::GetData() {

  TLOG(TLVL_DEBUG) <<__func__<< "() called";

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
  //TODO: Test it at short poll times, and allow for several polls in a zmq paquet
  TLOG(TLVL_DEBUG)<<"Reading information from the last zeromq event";
  sbndaq::BernCRTZMQEventUnion last_event;
  last_event.event = ZMQBufferUPtr[total_events-1];
  
  //sanity checks
  if(
      last_event.last_event.mac5 != 0xffff
      || last_event.last_event.flags != 0xffff
      || last_event.last_event.magic_number0 != 0xaa55aa55
      || last_event.last_event.magic_number1 != 0xaa55aa55) {
    TLOG(TLVL_ERROR) << __func__ <<" Data corruption! Check of control fields in the last event failed!";
    //TODO: crash DAQ if this happens, print the values
  }

  uint32_t n_events = last_event.last_event.n_events;
  
  this_poll_start = last_event.last_event.poll_time_start;
  this_poll_end   = last_event.last_event.poll_time_end;
  int32_t poll_start_deviation = last_event.last_event.poll_start_deviation;
  int32_t poll_end_deviation   = last_event.last_event.poll_end_deviation;

  //calculate maximum deviation, to be stored in metadata
  if(abs(poll_start_deviation) > abs(poll_end_deviation))
    system_clock_deviation = poll_start_deviation;
  else
    system_clock_deviation = poll_end_deviation;

  
  if(last_poll_start == 0) { //fix the poll timestamp for the very first poll to attempt to estimate the timestamps
    last_poll_start = this_poll_start - 300*1000*1000;//TODO dummy value... move to fhicl file?
    last_poll_end   = this_poll_end   - 300*1000*1000;//TODO dummy value... move to fhicl file?
  }

  TLOG(TLVL_DEBUG)<<"Number of events is "<<n_events;
  TLOG(TLVL_DEBUG)<<"this_poll_start  " << sbndaq::BernCRTZMQFragment::print_timestamp(this_poll_start) <<" (deviation from steady clock "<<poll_start_deviation<<" ns)";
  TLOG(TLVL_DEBUG)<<"this_poll_end    " << sbndaq::BernCRTZMQFragment::print_timestamp(this_poll_end) <<" (deviation from steady clock "<<poll_end_deviation<<" ns)";
  TLOG(TLVL_DEBUG)<<"last_poll_start  " << sbndaq::BernCRTZMQFragment::print_timestamp(last_poll_start);
  TLOG(TLVL_DEBUG)<<"last_poll_end    " << sbndaq::BernCRTZMQFragment::print_timestamp(last_poll_end);

  if(n_events != total_events -1) {
    TLOG(TLVL_DEBUG)<<"BernCRTZMQ::"<<__func__<<" Data corruption! Number of events reported in the last zmq event: "<<n_events<<" differs from what you expect from the packet size: "<<(total_events-1);
    throw cet::exception("BernCRTZMQ::GetData() Data corruption! Number of events reported in the last zmq event differs from what you expect from the packet size ");
  }

  TLOG(TLVL_DEBUG)<<__func__<<" start sorting with mac="<<prev_mac;

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
       */

      TLOG(TLVL_DEBUG)<<__func__<<": found new MAC ("<<this_event.MAC5()
                      <<")! prev_mac="<<prev_mac
                      <<", iterator="<<i_e
                      <<" this_events="<<this_n_events;

      //Verify if a buffer is created for given mac address (in case we receive incorrect mac address in the data)
      if (FEBBuffers_.find(prev_mac) == FEBBuffers_.end()) {
        TLOG(TLVL_ERROR)<<TRACE_NAME<<"::"<<__func__<<" Data corruption! Unexpected MAC address received in the data: "<<prev_mac;
//        throw cet::exception(std::string(TRACE_NAME)+"::"+__func__+" Data corruption! Unexpected MAC address received in the data: "+std::to_string(prev_mac));
      }
      else {
        //temporary fix, don't fill data for unexpected MAC addresses
        //TODO: crash if data is corrupted
        
        //insert group of events from a single FEB (distinct mac) to a dedicated FEBBuffer
        size_t new_buffer_size = InsertIntoFEBBuffer(FEBBuffers_[prev_mac], i_e-this_n_events, this_n_events, total_events);

        TLOG(TLVL_DEBUG)<<__func__<<": ... id="<<FEBBuffers_[prev_mac].id
          <<", n_events="<<this_n_events
          <<", buffer_size="<<FEBBuffers_[prev_mac].buffer.size();

        //auto id_str = GetFEBIDString(prev_mac);
        //metricMan->sendMetric("EventsAdded_"+id_str,this_n_events,"events",5,true,"BernCRTZMQGenerator");
        UpdateBufferOccupancyMetrics(prev_mac, new_buffer_size); //TODO: this function does nothing as of now!
      }
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

  FEBBuffer_t & feb = (FEBBuffers_[feb_id]);

  size_t buffer_end = feb.buffer.size(); 

  TLOG(TLVL_DEBUG) <<__func__ << " Current size of the FEB buffer: " << buffer_end << " events";
  if(metricMan != nullptr) metricMan->sendMetric("feb_buffer_size", buffer_end, "CRT hits", 5, artdaq::MetricMode::Average);

  //loop over all the CRTHit events in our buffer (for this FEB)
  for(size_t i_e=0; i_e<buffer_end; ++i_e) {
    BernCRTZMQEvent const& data = feb.buffer[i_e].first;
    BernCRTZMQFragmentMetadata const& metadata = feb.buffer[i_e].second;

    //assign timestamp to the event
    int ts0  = data.ts0;

    //add PPS cable offset modulo 1s
    ts0 = (ts0 + feb_configuration[data.mac5].GetPPSOffset()) % (1000*1000*1000);
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

    TLOG(TLVL_SPECIAL)<<__func__ << " Event: " << i_e<<"\n"<< metadata;
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
    memcpy(frags.back()->dataBeginBytes(), &data, sizeof(BernCRTZMQEvent));

  } //loop over events in feb buffer

  //delete from the buffer all the events we've just put into frags
  size_t new_buffer_size = EraseFromFEBBuffer(feb,buffer_end);

  //update 
  std::string id_str = GetFEBIDString(feb_id);
  metricMan->sendMetric("FragmentsBuilt_"+id_str,buffer_end,"events/s",5,artdaq::MetricMode::Rate);
  UpdateBufferOccupancyMetrics(feb_id,new_buffer_size);

  TLOG(TLVL_DEBUG) <<__func__<< " ENDING SIZE OF FRAGS IN FILLFRAGMENT IS " << frags.size();

  return false;
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
