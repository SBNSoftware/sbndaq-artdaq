#include "sbndaq-artdaq/Generators/Common/BernCRT_GeneratorBase.hh"

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

#define TRACE_NAME "BernCRT_GeneratorBase"

//TRACE level for messages sent once per event
#define TLVL_SPECIAL 11


sbndaq::BernCRT_GeneratorBase::BernCRT_GeneratorBase(fhicl::ParameterSet const & ps)
  :
  CommandableFragmentGenerator(ps),
  ps_(ps)
{
  TLOG(TLVL_INFO)<<__func__<<"() constructor called";
  Initialize();
  TLOG(TLVL_INFO)<<__func__<<"() constructor completed";
}

/*---------------------------------------------------------------------*/

void sbndaq::BernCRT_GeneratorBase::Initialize() {

  TLOG(TLVL_INFO)<<__func__<<"() called";

  //reset last poll times
  sequence_id_ = 0;

  //read parameters from FHiCL file
  std::vector<uint16_t> fragment_ids = ps_.get< std::vector<uint16_t> >("fragment_ids");
  std::sort(fragment_ids.begin(), fragment_ids.end());

  omit_out_of_order_events_   = ps_.get<bool>("omit_out_of_order_events");
  omit_out_of_sync_events_    = ps_.get<bool>("omit_out_of_sync_events");
  out_of_sync_tolerance_ns_   = 1000000 * ps_.get<uint32_t>("out_of_sync_tolerance_ms");
  feb_restart_period_         = 1e9 * ps_.get<uint32_t>("feb_restart_period_s");
  initial_delay_ns_           = 1e9 * ps_.get<uint32_t>("initial_delay_s");

  uint32_t FEBBufferCapacity_ = ps_.get<uint32_t>("FEBBufferCapacity");
  feb_poll_period_            = 1e6 * ps_.get<uint32_t>("feb_poll_ms");



  //Initialize buffers and calculate MAC5 addresses (last 8 bits)
  for( auto id : fragment_ids ) {
    uint8_t MAC5 = id & 0xff; //last 8 bits of fragment ID are last 8 bits of FEB MAC5
    MAC5s_.push_back(MAC5);
    FEBs_[MAC5] = FEB_t(FEBBufferCapacity_, id);
  }
  

  TLOG(TLVL_INFO)<<__func__<<"() completed ... starting GetData worker thread.";
  share::ThreadFunctor functor = std::bind(&BernCRT_GeneratorBase::GetData,this);
  auto worker_functor = share::WorkerThreadFunctorUPtr(new share::WorkerThreadFunctor(functor,"GetDataWorkerThread"));
  auto getData_worker = share::WorkerThread::createWorkerThread(worker_functor);
  GetData_thread_.swap(getData_worker);
} //Initialize

/*-----------------------------------------------------------------------*/


void sbndaq::BernCRT_GeneratorBase::start() {
  TLOG(TLVL_INFO)<<__func__<<"() called";

  run_start_time = std::chrono::system_clock::now().time_since_epoch().count();
  TLOG(TLVL_DEBUG)<<__func__<<"() Run start time: " << sbndaq::BernCRTFragment::print_timestamp(run_start_time);
  
  for(size_t iMAC5=0; iMAC5<MAC5s_.size(); ++iMAC5){
    const uint8_t& MAC5 = MAC5s_[iMAC5];
    FEBs_[MAC5].metadata.set_run_start_time(run_start_time);
  }
  
  //calibrate offset between system and steady clock
  steady_clock_offset = std::chrono::system_clock::now().time_since_epoch().count() - std::chrono::steady_clock::now().time_since_epoch().count();

  ConfigureStart();
  GetData_thread_->start();

  TLOG(TLVL_INFO)<<__func__<<"() completed";
} //start

/*-----------------------------------------------------------------------*/


void sbndaq::BernCRT_GeneratorBase::stop() {
  TLOG(TLVL_INFO)<<__func__<<"() called";
  GetData_thread_->stop();
  ConfigureStop();
  TLOG(TLVL_INFO)<<__func__<<"() completed";
} //stop

/*-----------------------------------------------------------------------*/


void sbndaq::BernCRT_GeneratorBase::stopNoMutex() {
TLOG(TLVL_INFO)<<__func__<<"() called";
  GetData_thread_->stop();
  ConfigureStop();
  TLOG(TLVL_INFO)<<__func__<<"() completed";
} //stopNoMutex

/*-----------------------------------------------------------------------*/


void sbndaq::BernCRT_GeneratorBase::Cleanup(){
  TLOG(TLVL_INFO)<<__func__<<"() called";
  TLOG(TLVL_INFO)<<__func__<<"() completed";
} //Cleanup

/*-----------------------------------------------------------------------*/


sbndaq::BernCRT_GeneratorBase::~BernCRT_GeneratorBase(){
  TLOG(TLVL_INFO)<<" destructor called";  
  Cleanup();
  TLOG(TLVL_INFO)<<" destructor completed"; 
} //destructor

/*-----------------------------------------------------------------------*/


std::string sbndaq::BernCRT_GeneratorBase::GetFEBIDString(uint64_t const& id) const{
  std::stringstream ss_id;
  ss_id << "0x" << std::hex <<id;
  return ss_id.str();
} //GetFEBIDString

/*-----------------------------------------------------------------------*/


void sbndaq::BernCRT_GeneratorBase::UpdateBufferOccupancyMetrics(uint64_t const& /*id*/,
                                                                    size_t const& ) const { //buffer_size) const {

  TLOG(TLVL_DEBUG)<<__func__<<"() called";

  //std::string id_str = GetFEBIDString(id);
  //metricMan->sendMetric("BufferOccupancy_"+id_str,buffer_size,"events",5,true,"BernCRTGenerator");
  //metricMan->sendMetric("BufferOccupancyPercent_"+id_str,
  //                         ((float)(buffer_size) / (float)(FEBBufferCapacity_))*100.,
  //                         "%",5,true,"BernCRTGenerator");    
} //UpdateBufferOccupancyMetrics


/*-----------------------------------------------------------------------*/

bool sbndaq::BernCRT_GeneratorBase::GetData() {

  TLOG(TLVL_DEBUG) <<__func__<< "() called";

  unsigned long total_events = GetFEBData(); //read data FEB and fill circular buffer

//   metricMan->sendMetric("TotalEventsAdded",total_events,"events",5,true,"BernCRTGenerator");

  return true;
} //GetData

/*-----------------------------------------------------------------------*/

bool sbndaq::BernCRT_GeneratorBase::OmitHit(uint64_t const & timestamp, BernCRTFragmentMetadata & metadata, FEB_t & feb, uint64_t const& feb_id) {
  /**
   * Check if given CRT hit should be omitted, i.e. not sent to DAQ
   * Perhaps we don't need this code at all - to be determined during testing
   * This function returns true if a hit should be omitted
   */
  if(omit_out_of_sync_events_) {
    /**
     * Omit events in which T0 value from FEB seems out of sync with server clock.
     * This is a method to alleviate problem of "spikes".  Otherwise it should not
     * be needed.  Under normal conditions server clock can be off by up to 500ms
     * before the data starts to be corrupted.
     */
    if(    timestamp < metadata.last_poll_start() - out_of_sync_tolerance_ns_
        || timestamp > metadata.this_poll_end()   + out_of_sync_tolerance_ns_) {
      TLOG(TLVL_WARNING)<<__func__ <<"(feb_id=" << feb_id << ") Event: \n"<< metadata;
      TLOG(TLVL_WARNING)<<__func__ <<"(feb_id=" << feb_id << ") Timestamp:       "<<sbndaq::BernCRTFragment::print_timestamp(timestamp);
      TLOG(TLVL_WARNING) <<__func__<<"(feb_id=" << feb_id << ") Omitted FEB timestamp out of sync with server clock";
      return true;
    }
  }

  if(omit_out_of_order_events_) {
    /**
     * Omit events with nonmonotonically growing timestamp
     * This is a method to alleviate problem of "spikes"
     * Out of order events may appear due to corruption of FEB data
     * or due to large desynchronisation of the server clock w.r.t. FEB PPS
     */
    if(timestamp <= feb.last_accepted_timestamp) {
      TLOG(TLVL_WARNING)<<__func__<<"(feb_id=" << feb_id << ") Event: \n"<< metadata;
      TLOG(TLVL_WARNING)<<__func__<<"(feb_id=" << feb_id << ") Timestamp:       "<<sbndaq::BernCRTFragment::print_timestamp(timestamp);
      TLOG(TLVL_WARNING)<<__func__<<"(feb_id=" << feb_id << ") Omitted out of order timestamp: "<<sbndaq::BernCRTFragment::print_timestamp(timestamp) <<" ≤ "<<sbndaq::BernCRTFragment::print_timestamp(feb.last_accepted_timestamp);
      return true;
    }
  }
  return false;
}


uint64_t sbndaq::BernCRT_GeneratorBase::CalculateTimestamp(BernCRTEvent const& data, BernCRTFragmentMetadata & metadata) {
  /**
   * Calculate timestamp based on nanosecond from FEB and poll times measured by server
   * see: https://sbn-docdb.fnal.gov/cgi-bin/private/DisplayMeeting?sessionid=7783
   */
  int ts0  = data.ts0;

  //add PPS cable length offset modulo 1s
  ts0 = (ts0 + feb_configuration[data.mac5].GetPPSOffset()) % (1000*1000*1000);
  if(ts0 < 0) ts0 += 1000*1000*1000; //just in case the cable offset is negative (should be positive normally)

  uint64_t mean_poll_time = metadata.last_poll_start()/2 + metadata.this_poll_end()/2;
  int mean_poll_time_ns = mean_poll_time % (1000*1000*1000); 

  if(ts0 - mean_poll_time_ns < -500*1000*1000)
    return mean_poll_time - mean_poll_time_ns + ts0 + 1000*1000*1000;
  else if(ts0 - mean_poll_time_ns > 500*1000*1000)
    return mean_poll_time - mean_poll_time_ns + ts0 - 1000*1000*1000;
  else
    return mean_poll_time - mean_poll_time_ns + ts0;
}


void sbndaq::BernCRT_GeneratorBase::FillFragment(uint64_t const& feb_id,
                                                    artdaq::FragmentPtrs & frags) {

  TLOG(TLVL_DEBUG) << __func__<<"(feb_id=" << feb_id << ") called with starting size of fragments: " << frags.size() << std::endl;

  FEB_t & feb = FEBs_[feb_id];

  size_t buffer_end = feb.buffer.size();

  TLOG(TLVL_DEBUG) <<__func__ << "(feb_id=" << feb_id << ") Current size of the FEB buffer: " << buffer_end << " events";
  if(metricMan != nullptr) metricMan->sendMetric("max_feb_buffer_size", buffer_end, "CRT hits", 5, artdaq::MetricMode::Maximum);

  //workaround: avoid processing hits at the beginning of the run, to prevent CRT from accumulating lot's of data before TPCs are ready
  static bool discard_data = true;
  if(discard_data) {
    if(std::chrono::system_clock::now().time_since_epoch().count() - run_start_time > initial_delay_ns_) {
      discard_data = false;
      if(initial_delay_ns_) {
        TLOG(TLVL_INFO) <<__func__ << "() CRT begins to send data after initial delay of "<< (initial_delay_ns_/1000000000) <<" seconds";
      }
    }
  }

  if(!discard_data) {
    //loop over all the CRTHit events in our buffer (for this FEB)
    for(size_t i_e=0; i_e<buffer_end; ++i_e) {
      BernCRTEvent const& data = feb.buffer[i_e].first;
      BernCRTFragmentMetadata & metadata = feb.buffer[i_e].second;

      if(i_e == 0)
        if(metricMan != nullptr) metricMan->sendMetric(
            std::string("feb_hit_rate_Hz_")+std::to_string(feb.fragment_id & 0xff),
            metadata.feb_events_per_poll() * 1e9 / (metadata.this_poll_end() - metadata.last_poll_end()),
            "CRT rate", 5, artdaq::MetricMode::Average);

      const uint64_t timestamp = CalculateTimestamp(data, metadata);

      if(OmitHit(timestamp, metadata, feb, feb_id)) continue;

      metadata.set_omitted_events(metadata.feb_event_number() - feb.last_accepted_event_number - 1);
      metadata.set_last_accepted_timestamp(feb.last_accepted_timestamp);

      feb.last_accepted_timestamp = timestamp;
      feb.last_accepted_event_number = metadata.feb_event_number();

      TLOG(TLVL_SPECIAL)<<__func__ << "(feb_id=" << feb_id << ") Event: " << i_e<<"\n"<< metadata;
      TLOG(TLVL_SPECIAL)<<__func__ << "(feb_id=" << feb_id << ") Timestamp:       "<<sbndaq::BernCRTFragment::print_timestamp(timestamp);

      //create our new fragment on the end of the frags vector
      frags.emplace_back( artdaq::Fragment::FragmentBytes(
            sizeof(BernCRTEvent), //payload_size
            sequence_id_++,
            feb.fragment_id,
            sbndaq::detail::FragmentType::BERNCRT,
            metadata,
            timestamp
            ) );

      //copy the BernCRTEvent into the fragment we just created
      memcpy(frags.back()->dataBeginBytes(), &data, sizeof(BernCRTEvent));
    } //loop over events in feb buffer
  }

  //delete from the buffer all the events we've just put into frags
  size_t new_buffer_size = EraseFromFEBBuffer(feb, buffer_end);

  //update 
  std::string id_str = GetFEBIDString(feb_id);
  metricMan->sendMetric("FragmentsBuilt_"+id_str,buffer_end,"events/s",5,artdaq::MetricMode::Rate);
  UpdateBufferOccupancyMetrics(feb_id,new_buffer_size);

  TLOG(TLVL_DEBUG) <<__func__<< "(feb_id=" << feb_id << ") ending size of frags is " << frags.size();
} //FillFragment

/*-----------------------------------------------------------------------*/

size_t sbndaq::BernCRT_GeneratorBase::EraseFromFEBBuffer(FEB_t & feb, size_t const& nevents) {
  TLOG(TLVL_DEBUG) <<__func__<< "() called";
  std::unique_lock<std::mutex> lock(*(feb.mutexptr));
  TLOG(TLVL_DEBUG) <<__func__<< "() Buffer size before erasing the events: " << std::setw(3) << feb.buffer.size() << " events";
  feb.buffer.erase_begin(nevents);
  TLOG(TLVL_DEBUG) <<__func__<< "() Buffer size after erasing the events: " << std::setw(4) << feb.buffer.size() << " events";
  return feb.buffer.size();
} //EraseFromFEBBuffer

/*-----------------------------------------------------------------------*/


void sbndaq::BernCRT_GeneratorBase::SendMetadataMetrics(BernCRTFragmentMetadata const& /*m*/) {

  TLOG(TLVL_DEBUG)<<__func__<<"() called";

//  std::string id_str = GetFEBIDString(m.feb_id());

  //metricMan->sendMetric("FragmentLastTime_"+id_str,(uint64_t)(m.time_end_seconds()*1000000000+m.time_end_nanosec()),"ns",5,false,"BernCRTGenerator");
  //metricMan->sendMetric("EventsInFragment_"+id_str,(float)(m.n_events()),"events",5,true,"BernCRTGenerator");
  //metricMan->sendMetric("MissedEvents_"+id_str,     (float)(m.missed_events()),     "events",5,true,"BernCRTGenerator");
  //metricMan->sendMetric("OverwrittenEvents_"+id_str,(float)(m.overwritten_events()),"events",5,true,"BernCRTGenerator");
  //float eff=1.0;
  //if((m.n_events()+m.missed_events()+m.overwritten_events())!=0)
  //eff = (float)(m.n_events()) / (float)(m.n_events()+m.missed_events()+m.overwritten_events());
  
  //metricMan->sendMetric("Efficiency_"+id_str,eff*100.,"%",5,true,"BernCRTGenerator");
}

/*-----------------------------------------------------------------------*/


bool sbndaq::BernCRT_GeneratorBase::getNext_(artdaq::FragmentPtrs & frags) {

  TLOG(TLVL_DEBUG) <<__func__<< " called with frags.size = " << frags.size();

  //initialise variables
  static auto t_start = std::chrono::steady_clock::now();
  static auto t_end = std::chrono::steady_clock::now();
  
  t_start = std::chrono::steady_clock::now();
  if(metricMan != nullptr) metricMan->sendMetric("time_outside_getNext_ms",
     artdaq::TimeUtils::GetElapsedTimeMilliseconds(t_end, t_start),
     "CRT performance", 5, artdaq::MetricMode::Maximum);

  if (should_stop()) {
    return false;
  }

  for(auto const& MAC5 : MAC5s_) {
    FillFragment(MAC5, frags);
  }

  TLOG(TLVL_DEBUG) <<__func__<< ": completed with frags.size = " << frags.size();
  t_end = std::chrono::steady_clock::now();

  if(metricMan != nullptr) metricMan->sendMetric("getNext_execution_time_ms",
     artdaq::TimeUtils::GetElapsedTimeMilliseconds(t_start, t_end),
     "CRT performance", 5, artdaq::MetricMode::Maximum);


  if(frags.size()>0)
    TLOG(TLVL_DEBUG) << __func__ 
		     << " : Send fragment with type " << frags.back()->type() 
		     << " (" << frags.back()->typeString() << "):  "
		     << " (id,seq,timestamp)=(" << frags.back()->fragmentID() << ","<<frags.back()->sequenceID()<< "," << frags.back()->timestamp();
  

  return true;
} //getNext_

/*-----------------------------------------------------------------------*/

// The following macro is defined in artdaq's GeneratorMacros.hh header
//DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(demo::BernCRT) 
