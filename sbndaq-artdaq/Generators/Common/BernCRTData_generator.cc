#include "sbndaq-artdaq/Generators/Common/BernCRTData.hh"
#include "artdaq/Generators/GeneratorMacros.hh"

#include <iomanip>
#include <iterator>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include "febdrv.hh"

#include "artdaq/DAQdata/Globals.hh"
#define id "BernCRT"
#define TRACE_NAME "BernCRTData"


sbndaq::BernCRTData::BernCRTData(fhicl::ParameterSet const & ps)
  :
  BernCRT_GeneratorBase(ps) {
  TLOG(TLVL_INFO) << "constructor called";

  std::string ethernet_port = ps_.get<std::string>("ethernet_port");

  if( ! febdrv.Init(ethernet_port) ) {
    TLOG(TLVL_ERROR) <<  "Failed to initialise febdrv on interface \"" << ethernet_port << "\"!";
    throw cet::exception( std::string(TRACE_NAME) +"::" + __func__ + "() Failed to initialise febdrv on interface \"" + ethernet_port + "\"!");
  }

  //compare detected list of FEBs with that declared in FHiCL file
  auto hardware_macs = febdrv.GetMACs();
  VerifyMACConfiguration(hardware_macs);


  for(const uint8_t& mac5 : MAC5s_) {
    TLOG(TLVL_DEBUG) << "Reading configuration for MAC5 " << std::to_string(mac5);
    feb_configuration[mac5] = sbndaq::BernCRTFEBConfiguration(ps_, mac5); //create configuration object

    TLOG(TLVL_INFO)
      <<"Read configuration for CRT FEB "<<(int)(mac5)
      <<" PPS offset: "<<feb_configuration[mac5].GetPPSOffset()
      <<" Turn on HV: "<<(feb_configuration[mac5].GetHVOnPermission()?"yes":"no");
  }

  TLOG(TLVL_INFO) << "constructor completed";
} //constructor

sbndaq::BernCRTData::~BernCRTData() {
  TLOG(TLVL_INFO) << "called";

  Cleanup();
  TLOG(TLVL_INFO) << "completed";
} //destructor

void sbndaq::BernCRTData::ConfigureStart() {
  TLOG(TLVL_INFO) << "called";
  
  //make sure the HV and DAQ are off before we start to send the configuration to the board
  //switch off bias voltage one by one (rather than sending signal to all boards at once
  //using MAC 255) so that if it fails, the error message will specify which board failed 
  for(unsigned int iFEB = 0; iFEB < nFEBs(); iFEB++) {
    febdrv.biasOFF(MAC5s_[iFEB]);
  }
 
  for(unsigned int iFEB = 0; iFEB < nFEBs(); iFEB++) {
    feb_send_bitstreams(MAC5s_[iFEB]); //send PROBE and SC configuration to FEB
    if(feb_configuration[MAC5s_[iFEB]].GetHVOnPermission()) febdrv.biasON(MAC5s_[iFEB]); //turn on SiPM HV (if FHiCL file allows it)
  }
  StartFebdrv(); //start data taking mode for all boards

  TLOG(TLVL_INFO) << "completed";
} //ConfigureStart

void sbndaq::BernCRTData::ConfigureStop() {
  TLOG(TLVL_INFO) << "called";

  febdrv.biasOFF();

  TLOG(TLVL_INFO) << "completed";
} //ConfigureStop

void sbndaq::BernCRTData::feb_send_bitstreams(uint8_t mac5) {
  /**
   * Sends configuration bitstream
   * Arguments:
   * mac5 of the board, note one cannot use 255 address to broadcast to all boards
   * Bitstreams should be read from (?)
   * - Probe_bitStream - monitoring configuration. It consists of 224 bits, typically all '0' for normal operation 
   * - SlowControl_bitStream - actual configuration to be sent to the board, 1144 bits
   */

  if(feb_configuration.find(mac5) == feb_configuration.end()) {
    TLOG(TLVL_ERROR) <<  "Could not find FEB " << mac5 << " in MAC5s!";
    throw cet::exception( std::string(TRACE_NAME) +"::"+ __func__ + " Could not find FEB " + std::to_string(mac5) + " in MAC5s!");
  }

  if(mac5==255) {
    TLOG(TLVL_ERROR) <<  "Bitstreams cannot be sent to mac5 = 255!";
    return;
  }
  
  bool status = febdrv.sendconfig(
    mac5,
    feb_configuration[mac5].GetSlowControlBitStream(),
    feb_configuration[mac5].GetSlowControlBitStreamNBytes(),
    feb_configuration[mac5].GetProbeBitStream(),
    feb_configuration[mac5].GetProbeBitStreamNBytes()
  );
  
  if(!status)
    throw cet::exception( std::string(TRACE_NAME) + __func__ + "() Failed to send configuration bitstreams");

} //feb_send_bitstreams

void sbndaq::BernCRTData::StartFebdrv() {
  TLOG(TLVL_DEBUG)<< "(re)starting febdrv";
  if(! febdrv.startDAQ() ) {
    TLOG(TLVL_ERROR) <<  "Failed to (re)start DAQ";
    throw cet::exception( std::string(TRACE_NAME) +"::"+ __func__ + " Failed to (re)start DAQ!");
  }

  last_restart_time = std::chrono::system_clock::now();
}

void sbndaq::BernCRTData::VerifyMACConfiguration(std::vector<uint8_t> hardware_macs) {
  /**
   * Check if the FEB list defined in FHiCL file agrees with FEBs detected by febdrv
   */

  //check number of FEBs
  bool configuration_ok = (hardware_macs.size() == nFEBs());

  //check FEB MAC addresses
  //sort both arrays
  std::sort(hardware_macs.begin(), hardware_macs.end());
  auto mac5s = MAC5s_;
  std::sort(mac5s.begin(), mac5s.end());
  for(unsigned int i = 0; i < nFEBs() && configuration_ok; i++) {
    configuration_ok = (hardware_macs[i] == mac5s[i]);
  }

  //print error message and crash
  if(!configuration_ok) {
    std::string error_message = __func__;
    error_message += "() List of "+std::to_string(mac5s.size())+ " FEBs declared in the FHiCL file (dec):";
    for(auto m : mac5s) error_message += " " + std::to_string(m);
    error_message += " doesn't match list of " + std::to_string(hardware_macs.size()) + " detected FEBs:";
    for(auto h : hardware_macs) error_message += " " + std::to_string(h);
    TLOG(TLVL_ERROR) << error_message;
    throw cet::exception( std::string(TRACE_NAME) + " " + error_message );
  }
} //VerifyMACConfiguration

uint64_t sbndaq::BernCRTData::GetTimeSinceLastRestart() {
  return (std::chrono::system_clock::now() -  last_restart_time).count();
}

void sbndaq::BernCRTData::CalculateTimestamp(BernCRTHitV2& hit, BernCRTFragmentMetadataV2& metadata) {
  /**
   * Calculate timestamp based on nanosecond from FEB and poll times measured by server
   * see: https://sbn-docdb.fnal.gov/cgi-bin/private/DisplayMeeting?sessionid=7783
   */
  int32_t ts0  = hit.ts0; //must be signed int

  //add PPS cable length offset modulo 1s
  ts0 = (ts0 + feb_configuration[metadata.MAC5()].GetPPSOffset()) % (1000*1000*1000);
  if(ts0 < 0) ts0 += 1000*1000*1000; //just in case the cable offset is negative (should be positive normally)

  uint64_t mean_poll_time = metadata.last_poll_start()/2 + metadata.this_poll_end()/2;
  int mean_poll_time_ns = mean_poll_time % (1000*1000*1000); 
  
  hit.timestamp = mean_poll_time - mean_poll_time_ns + ts0
    + (ts0 - mean_poll_time_ns < -500*1000*1000) * 1000*1000*1000
    - (ts0 - mean_poll_time_ns >  500*1000*1000) * 1000*1000*1000;
}


    
/*---------------BERN CRT FEB DATA-------------------------------------*/
size_t sbndaq::BernCRTData::GetFEBData() {
  /**
   * Reads data from FEB
   */
  
  TLOG(TLVL_DEBUG+2) << "called";

  //measure time of function execution
  static auto t_start = std::chrono::steady_clock::now();
  static auto t_end = std::chrono::steady_clock::now();

  t_start = std::chrono::steady_clock::now();
  if(metricMan != nullptr) metricMan->sendMetric("time_outside_GetFEBData_ms",
      artdaq::TimeUtils::GetElapsedTimeMilliseconds(t_end, t_start),
      "CRT performance", 5, artdaq::MetricMode::Maximum);
  
  // Sleep until the time for next poll comes
  int now = std::chrono::system_clock::now().time_since_epoch().count() % feb_poll_period_;
  usleep((feb_poll_period_ - now)/1000);

  const auto t_wait = std::chrono::steady_clock::now();
  if(metricMan != nullptr) metricMan->sendMetric("time_waiting_for_poll_ms",
      artdaq::TimeUtils::GetElapsedTimeMilliseconds(t_start, t_wait),
      "CRT performance", 5, artdaq::MetricMode::Maximum);


  size_t hit_count_all_febs = 0;
  
  for(auto mac : MAC5s_) {
    FEB_t & feb = FEBs_[mac];
    feb.hits.clear();

    uint64_t poll_start = std::chrono::system_clock::now().time_since_epoch().count();
    int32_t system_clock_deviation = poll_start - steady_clock_offset - std::chrono::steady_clock::now().time_since_epoch().count();
    feb.metadata.set_clock_deviation(system_clock_deviation);
    
    febdrv.pollfeb(mac);
    
    
    while(true) {
      //loop over hits received via ethernet and push into circular buffer
      int numbytes = febdrv.GetData(FirmwareFlag);
      if(numbytes<=0) break;
      
      int datalen = numbytes-18;
      
      TLOG(TLVL_DEBUG+2)<<" datalen = "<<datalen;
        
      //loop over bytes of the data
      for(int jj = 0; jj < datalen; ) { // jj is incremented in processSingleHit
        BernCRTHitV2 hit;

	      febdrv.processSingleHit(jj, hit, FirmwareFlag); //read next hit
        feb.hits.push_back(hit);
      }
    }

    uint64_t poll_end = std::chrono::system_clock::now().time_since_epoch().count();
    
    feb.metadata.update_poll_time(poll_start, poll_end);

    const double last_poll_hit_rate = (feb.last_poll_total_lostfpga + feb.last_poll_total_read_hits + 0.001) / (feb.last_poll_total_read_hits + 0.001);
    unsigned int feb_total_lostfpga = 0;
    uint64_t last_poll_hit_number = feb.hit_number;

    uint32_t max_t0_in_poll = 0;

    for(auto & hit : feb.hits) {
      CalculateTimestamp(hit, feb.metadata);
      max_t0_in_poll = std::max(max_t0_in_poll, hit.ts0);

      //compute hit number, including lost hits
      feb_total_lostfpga += feb.last_lostfpga;
      hit.lost_hits = //hits lost since the last measured hit
          hit.lostcpu * last_poll_hit_rate //estimate lostfpga during lostcpu (not counted by FEB)
        + feb.last_lostfpga; //Lost fpga contains hits omitted *after* the trigger event in given hit, this is why we use the field from the previous hit here
      feb.last_lostfpga = hit.lostfpga;
      hit.feb_hit_number = (feb.hit_number += 1 + hit.lost_hits);

      hit.last_accepted_timestamp = feb.last_accepted_timestamp;
      feb.last_accepted_timestamp = hit.timestamp;

      if(max_time_with_no_data_ns_ && hit.last_accepted_timestamp > 1) {
        /**
         * Display warning if the time between hits drops below certain value
         */
        if(hit.timestamp - hit.last_accepted_timestamp > max_time_with_no_data_ns_) {
          TLOG(TLVL_WARNING) 
            <<"FEB "<<(int)mac
            <<" time between consecutive timestamps: "
            <<sbndaq::BernCRTFragment::print_timestamp(hit.timestamp - hit.last_accepted_timestamp)
            <<" > max_time_with_no_data_ms (" <<(max_time_with_no_data_ns_/1'000'000)<<" ms)"
            <<" this hit: "
            <<sbndaq::BernCRTFragment::print_timestamp(hit.timestamp)
            <<" previous hit: "
            <<sbndaq::BernCRTFragment::print_timestamp(hit.last_accepted_timestamp);
        }
      }
    } //loop over hits in given FEB

    if(max_time_with_no_data_ns_ && feb.hits.size() == 0) {
      /**
       * Display warning if FEB measured no hits during this poll
       */
      if(feb.last_accepted_timestamp > 1) {
        TLOG(TLVL_WARNING)
          <<"FEB "<<(int)mac
          <<" sent no hits during this poll. "
          <<" Timestamp of the last hit: "
          <<sbndaq::BernCRTFragment::print_timestamp(feb.last_accepted_timestamp)
          <<" ("
          <<sbndaq::BernCRTFragment::print_timestamp(poll_end - feb.last_accepted_timestamp)
          <<" ago)";
      }
      else {
        TLOG(TLVL_WARNING)
          <<"FEB "<<(int)mac
          <<" sent no hits since the beginning of the run";
      }
    }
    
    if(max_t0_in_poll > max_tolerable_t0_) {
      TLOG(TLVL_ERROR)
        <<"During this poll in FEB "<<(int)mac
        <<" the maximum registered t0 counter value was "
        <<sbndaq::BernCRTFragment::print_timestamp(max_t0_in_poll)
        <<" which suggests the FEB does not receive Pulse Per Second (PPS) signal";
    }

    hit_count_all_febs += feb.hits.size();
    feb.last_poll_total_read_hits = feb.hits.size();
    feb.last_poll_total_lostfpga = feb_total_lostfpga;

    feb.metadata.set_hits_in_poll(feb.hit_number - last_poll_hit_number);


    std::unique_lock<std::mutex> lock(*(feb.mutexptr));

    //split hits vector into fragments in windows of size fragment_period
    //note these are not the same windows as those defined by window pull mode
    if(!feb.hits.empty()) { //form fragments only if there are any hits
      int start_index = 0;
      uint64_t start_timestamp = feb.hits[0].timestamp / fragment_period_;
      for(unsigned int i = 0; i <= feb.hits.size(); i++) {
        if(i == feb.hits.size()  ||  feb.hits[i].timestamp / fragment_period_ != start_timestamp) {
          feb.metadata.set_hits_in_fragment(i - start_index);
          std::vector<BernCRTHitV2> fragment_hits(feb.hits.cbegin() + start_index, feb.hits.cbegin() + i);
          const uint64_t fragment_timestamp = (start_timestamp + 0.5) * fragment_period_;
          feb.buffer.push_back({fragment_hits, feb.metadata, fragment_timestamp});

          TLOG(TLVL_DEBUG+5) << "pushing " << std::to_string(feb.metadata.hits_in_fragment()) << " hits into a fragment for MAC "<<std::to_string(mac)<<" at "<< sbndaq::BernCRTFragment::print_timestamp(fragment_timestamp);

          if(i < feb.hits.size()) {
            start_index = i;
            start_timestamp = feb.hits[i].timestamp / fragment_period_;
          }
        }
      } //loop over buffer
    } //add fragments to fragment buffer
  } //loop over FEBs
  
  //workaround for spike issue: periodically restart febdrv
  if(feb_restart_period_) {
    if(GetTimeSinceLastRestart() > feb_restart_period_) {
      StartFebdrv();
    }
  }

  t_end = std::chrono::steady_clock::now();

  if(metricMan != nullptr) metricMan->sendMetric("time_reading_FEBs_ms",
      artdaq::TimeUtils::GetElapsedTimeMilliseconds(t_wait, t_end),
      "CRT performance", 5, artdaq::MetricMode::Maximum);

  if(metricMan != nullptr) metricMan->sendMetric("GetFEBData_execution_time_ms",
      artdaq::TimeUtils::GetElapsedTimeMilliseconds(t_start, t_end),
      "CRT performance", 5, artdaq::MetricMode::Maximum);
  
  TLOG(TLVL_DEBUG+2) << "read " << std::to_string(hit_count_all_febs) << " hits";

  return hit_count_all_febs;
} //GetFEBData

DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(sbndaq::BernCRTData) 
