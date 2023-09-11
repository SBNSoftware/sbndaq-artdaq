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

sbndaq::BernCRT_GeneratorBase::BernCRT_GeneratorBase(fhicl::ParameterSet const & ps)
  :
  CommandableFragmentGenerator(ps),
  ps_(ps)
{
  TLOG(TLVL_INFO)<<"constructor called";
  Initialize();
  TLOG(TLVL_INFO)<<"constructor completed";
}

/*---------------------------------------------------------------------*/

void sbndaq::BernCRT_GeneratorBase::Initialize() {

  TLOG(TLVL_INFO)<<"called";

  //reset last poll times
  sequence_id_ = 0;

  //read parameters from FHiCL file
  std::vector<uint16_t> fragment_ids = ps_.get< std::vector<uint16_t> >("fragment_ids");

  feb_restart_period_         = 1e9 * ps_.get<uint32_t>("feb_restart_period_s");
  initial_delay_ns_           = 1e9 * ps_.get<uint32_t>("initial_delay_s");
  fragment_period_            = 1e6 * ps_.get<uint16_t>("fragment_period_ms");

  uint32_t FEBBufferCapacity_ = ps_.get<uint32_t>("FEBBufferCapacity");
  feb_poll_period_            = 1e6 * ps_.get<uint32_t>("feb_poll_ms");

  max_time_with_no_data_ns_   = 1'000'000UL * ps_.get<uint64_t>("max_time_with_no_data_ms", 1000); //set to 0 to disable warning message
  max_tolerable_t0_           = ps_.get<uint32_t>("max_tolerable_t0_", 1'000'100'000); //set to 1<<30 (or more) to disable error message

  // new fcl parameter for using older firmware versions.
  // older firmware for which the coinc word is not in the FEB hit structure is used
  const std::string flagString = ps_.get<std::string>("firmware_flag", "ICARUS");
  if(flagString == "ICARUS") {
    FirmwareFlag = ICARUS;
  }
  else if(flagString == "SBND") {
    FirmwareFlag = SBND;
  }
  else {
    throw cet::exception(std::string(TRACE_NAME)+"::"+__func__+" \""+flagString+"\" isn't a valid value for firmware_flag fhicl parameter");
  }



  //Initialize buffers and calculate MAC5 addresses (last 8 bits)
  for( auto id : fragment_ids ) {
    uint8_t MAC5 = id & 0xff; //last 8 bits of fragment ID are last 8 bits of FEB MAC5
    MAC5s_.push_back(MAC5);
    FEBs_[MAC5] = FEB_t(FEBBufferCapacity_, id);
  }
  

  TLOG(TLVL_INFO)<<"completed ... starting GetData worker thread.";
  share::ThreadFunctor functor = std::bind(&BernCRT_GeneratorBase::GetData,this);
  auto worker_functor = share::WorkerThreadFunctorUPtr(new share::WorkerThreadFunctor(functor,"GetDataWorkerThread"));
  auto getData_worker = share::WorkerThread::createWorkerThread(worker_functor);
  GetData_thread_.swap(getData_worker);
} //Initialize

/*-----------------------------------------------------------------------*/


void sbndaq::BernCRT_GeneratorBase::start() {
  TLOG(TLVL_INFO)<<"called";

  run_start_time = std::chrono::system_clock::now().time_since_epoch().count();
  TLOG(TLVL_DEBUG)<<"Run start time: " << sbndaq::BernCRTFragment::print_timestamp(run_start_time);
  
  for(size_t iMAC5=0; iMAC5<MAC5s_.size(); ++iMAC5){
    const uint8_t& MAC5 = MAC5s_[iMAC5];
    FEBs_[MAC5].metadata.set_run_start_time(run_start_time);
  }
  
  //calibrate offset between system and steady clock
  steady_clock_offset = std::chrono::system_clock::now().time_since_epoch().count() - std::chrono::steady_clock::now().time_since_epoch().count();

  ConfigureStart();
  GetData_thread_->start();

  TLOG(TLVL_INFO)<<"completed";
} //start

/*-----------------------------------------------------------------------*/


void sbndaq::BernCRT_GeneratorBase::stop() {
  TLOG(TLVL_INFO)<<"called";
  GetData_thread_->stop();
  ConfigureStop();
  TLOG(TLVL_INFO)<<"completed";
} //stop

/*-----------------------------------------------------------------------*/


void sbndaq::BernCRT_GeneratorBase::stopNoMutex() {
TLOG(TLVL_INFO)<<"called";
  GetData_thread_->stop();
  ConfigureStop();
  TLOG(TLVL_INFO)<<"completed";
} //stopNoMutex

/*-----------------------------------------------------------------------*/


void sbndaq::BernCRT_GeneratorBase::Cleanup(){
  TLOG(TLVL_INFO)<<"called";
  TLOG(TLVL_INFO)<<"completed";
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

  TLOG(TLVL_DEBUG+1)<<"called";

  //std::string id_str = GetFEBIDString(id);
  //metricMan->sendMetric("BufferOccupancy_"+id_str,buffer_size,"events",5,true,"BernCRTGenerator");
  //metricMan->sendMetric("BufferOccupancyPercent_"+id_str,
  //                         ((float)(buffer_size) / (float)(FEBBufferCapacity_))*100.,
  //                         "%",5,true,"BernCRTGenerator");    
} //UpdateBufferOccupancyMetrics


/*-----------------------------------------------------------------------*/

bool sbndaq::BernCRT_GeneratorBase::GetData() {

  TLOG(TLVL_DEBUG+2) <<"called";

  unsigned long total_hits = GetFEBData(); //read data FEB and fill circular buffer
  
  if(metricMan != nullptr) metricMan->sendMetric("total_hits_per_poll", total_hits, "CRT hits per poll", 11, artdaq::MetricMode::Average);

  return true;
} //GetData

/*-----------------------------------------------------------------------*/


void sbndaq::BernCRT_GeneratorBase::FillFragment(uint64_t const& feb_id,
                                                    artdaq::FragmentPtrs & frags) {

  TLOG(TLVL_DEBUG+1) << "(feb_id=" << feb_id << ") called with starting size of fragments: " << frags.size() << std::endl;

  FEB_t & feb = FEBs_[feb_id];

  size_t buffer_end = feb.buffer.size();

  TLOG(TLVL_DEBUG+5) << "(feb_id=" << feb_id << ") Current size of the FEB buffer: " << buffer_end << " fragments";
  if(metricMan != nullptr) metricMan->sendMetric("max_feb_buffer_size", buffer_end, "CRT hits", 11, artdaq::MetricMode::Maximum);

  //workaround: avoid processing hits at the beginning of the run, to prevent CRT from accumulating lot's of data before TPCs are ready
  static bool discard_data = true;
  if(discard_data) {
    if(std::chrono::system_clock::now().time_since_epoch().count() - run_start_time > initial_delay_ns_) {
      discard_data = false;
      if(initial_delay_ns_) {
        TLOG(TLVL_INFO) <<"CRT begins to send data after initial delay of "<< (initial_delay_ns_/1000000000) <<" seconds";
      }
    }
  }

  if(!discard_data) {
    //loop over all the CRTHit events in our buffer (for this FEB)
    for(size_t i_e=0; i_e<buffer_end; ++i_e) {
      const std::vector<BernCRTHitV2> & data     = feb.buffer[i_e].hits;
      const BernCRTFragmentMetadataV2 & metadata = feb.buffer[i_e].metadata;
      const uint64_t & fragment_timestamp        = feb.buffer[i_e].fragment_timestamp;

      if(i_e == 0) { //send metrics only once for each FillFragment call
        if(metricMan != nullptr) {
          metricMan->sendMetric(
              std::string("feb_hit_rate_Hz_")+std::to_string(feb.fragment_id & 0xff),
              metadata.hits_in_poll() * 1e9 / (metadata.this_poll_end() - metadata.last_poll_end()),
              "CRT hit rate", 11, artdaq::MetricMode::Average);
          metricMan->sendMetric(
              std::string("feb_poll_time_ms_")+std::to_string(feb.fragment_id & 0xff),
              (metadata.this_poll_end() - metadata.this_poll_start()) * 1e6,
              "CRT poll time", 11, artdaq::MetricMode::Average);
          metricMan->sendMetric(
              std::string("feb_poll_period_ms_")+std::to_string(feb.fragment_id & 0xff),
              (metadata.this_poll_end() - metadata.last_poll_end()) * 1e6,
              "CRT poll period", 11, artdaq::MetricMode::Average);
        }
      }

      //create our new fragment on the end of the frags vector
      frags.emplace_back( artdaq::Fragment::FragmentBytes(
            sizeof(BernCRTHitV2)*metadata.hits_in_fragment(), //payload_size
            sequence_id_++,
            feb.fragment_id,
            sbndaq::detail::FragmentType::BERNCRTV2,
            metadata,
            fragment_timestamp
            ) );

      //copy the BernCRTEvent into the fragment we just created
      memcpy(frags.back()->dataBeginBytes(), data.data(), sizeof(BernCRTHitV2)*metadata.hits_in_fragment());
    } //loop over events in feb buffer
  }

  //delete from the buffer all the events we've just put into frags
  size_t new_buffer_size = EraseFromFEBBuffer(feb, buffer_end);

  //update 
  std::string id_str = GetFEBIDString(feb_id);
  metricMan->sendMetric("FragmentsBuilt_"+id_str,buffer_end,"events/s",11,artdaq::MetricMode::Rate);
  UpdateBufferOccupancyMetrics(feb_id,new_buffer_size);

  TLOG(TLVL_DEBUG+1) <<"(feb_id=" << feb_id << ") ending size of frags is " << frags.size();
} //FillFragment

/*-----------------------------------------------------------------------*/

size_t sbndaq::BernCRT_GeneratorBase::EraseFromFEBBuffer(FEB_t & feb, size_t const& nevents) {
  TLOG(TLVL_DEBUG+1) <<"called";
  std::unique_lock<std::mutex> lock(*(feb.mutexptr));
  TLOG(TLVL_DEBUG+1) <<"Buffer size before erasing the events: " << std::setw(3) << feb.buffer.size() << " events";
  feb.buffer.erase_begin(nevents);
  TLOG(TLVL_DEBUG+1) <<"Buffer size after erasing the events: " << std::setw(4) << feb.buffer.size() << " events";
  return feb.buffer.size();
} //EraseFromFEBBuffer

/*-----------------------------------------------------------------------*/


void sbndaq::BernCRT_GeneratorBase::SendMetadataMetrics(BernCRTFragmentMetadataV2 const& /*m*/) {

  TLOG(TLVL_DEBUG)<<"called";

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

  TLOG(TLVL_DEBUG+1) <<"called with frags.size = " << frags.size();

  //initialise variables
  static auto t_start = std::chrono::steady_clock::now();
  static auto t_end = std::chrono::steady_clock::now();
  
  t_start = std::chrono::steady_clock::now();
  if(metricMan != nullptr) metricMan->sendMetric("time_outside_getNext_ms",
     artdaq::TimeUtils::GetElapsedTimeMilliseconds(t_end, t_start),
     "CRT performance", 11, artdaq::MetricMode::Maximum);

  if (should_stop()) {
    return false;
  }

  for(auto const& MAC5 : MAC5s_) {
    FillFragment(MAC5, frags);
  }

  TLOG(TLVL_DEBUG+1) <<"completed with frags.size = " << frags.size();
  t_end = std::chrono::steady_clock::now();

  if(metricMan != nullptr) metricMan->sendMetric("getNext_execution_time_ms",
     artdaq::TimeUtils::GetElapsedTimeMilliseconds(t_start, t_end),
     "CRT performance", 11, artdaq::MetricMode::Maximum);


  if(frags.size()>0)
    TLOG(TLVL_DEBUG+1)
		     << " : Send fragment with type " << (int)(frags.back()->type())
		     << " (" << frags.back()->typeString() << "):  "
		     << " (id,seq,timestamp)=(" << frags.back()->fragmentID() << ","<<frags.back()->sequenceID()<< "," << frags.back()->timestamp();
  

  return true;
} //getNext_

/*-----------------------------------------------------------------------*/

// The following macro is defined in artdaq's GeneratorMacros.hh header
//DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(demo::BernCRT) 
