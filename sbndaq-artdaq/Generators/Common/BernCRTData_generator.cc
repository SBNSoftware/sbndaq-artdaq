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

#define id "BernCRT"
#define TRACE_NAME "BernCRTData"


sbndaq::BernCRTData::BernCRTData(fhicl::ParameterSet const & ps)
  :
  BernCRT_GeneratorBase(ps) {
  TLOG(TLVL_INFO) << __func__ <<"() constructor called";

  std::string ethernet_port = ps_.get<std::string>("ethernet_port");

  if( ! febdrv.Init(ethernet_port) ) {
    TLOG(TLVL_ERROR) <<  __func__ << "() Failed to initialise febdrv on interface \"" << ethernet_port << "\"!";
    throw cet::exception( std::string(TRACE_NAME) +"::" + __func__ + "() Failed to initialise febdrv on interface \"" + ethernet_port + "\"!");
  }
  
  //compare detected list of FEBs with that declared in FHiCL file
  auto hardware_macs = febdrv.GetMACs();
  VerifyMACConfiguration(hardware_macs);


  for(unsigned int iFEB = 0; iFEB < nFEBs(); iFEB++) {
    TLOG(TLVL_DEBUG) << __func__ << " Reading configuration for MAC5 #" << std::to_string(iFEB) << ": " << std::to_string(MAC5s_[iFEB]);
    feb_configuration[MAC5s_[iFEB]] = sbndaq::BernCRTFEBConfiguration(ps_, iFEB); //create configuration object
  }

  TLOG(TLVL_INFO) << __func__ << "() constructor completed";
} //constructor

sbndaq::BernCRTData::~BernCRTData() {
  TLOG(TLVL_INFO) << __func__ <<  "() called";

  Cleanup();
  TLOG(TLVL_INFO) << __func__ << "() completed";
} //destructor

void sbndaq::BernCRTData::ConfigureStart() {
  TLOG(TLVL_INFO) << __func__ << "() called";
  
  //make sure the HV and DAQ are off before we start to send the configuration to the board
  febdrv.biasOFF();
  
  for(unsigned int iFEB = 0; iFEB < nFEBs(); iFEB++) {
    feb_send_bitstreams(MAC5s_[iFEB]); //send PROBE and SC configuration to FEB
    if(feb_configuration[MAC5s_[iFEB]].GetHVOnPermission()) febdrv.biasON(MAC5s_[iFEB]); //turn on SiPM HV (if FHiCL file allows it)
  }
  StartFebdrv(); //start data taking mode for all boards

  TLOG(TLVL_INFO) << __func__ << "() completed";
} //ConfigureStart

void sbndaq::BernCRTData::ConfigureStop() {
  TLOG(TLVL_INFO) << __func__ << "() called";

  febdrv.biasOFF();

  TLOG(TLVL_INFO) << __func__ << "() completed";
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
    TLOG(TLVL_ERROR) <<  __func__ << "() Could not find FEB " << mac5 << " in MAC5s!";
    throw cet::exception( std::string(TRACE_NAME) +"::"+ __func__ + " Could not find FEB " + std::to_string(mac5) + " in MAC5s!");
  }

  if(mac5==255) {
    TLOG(TLVL_ERROR) <<  __func__ << "() Bitstreams cannot be sent to mac5 = 255!";
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
  TLOG(TLVL_DEBUG)<< __func__<<"() (re)starting febdrv";
  if(! febdrv.startDAQ() ) {
    TLOG(TLVL_ERROR) <<  __func__ << "() Failed to (re)start DAQ";
    throw cet::exception( std::string(TRACE_NAME) +"::"+ __func__ + " Failed to (re)start DAQ!");
  }

  last_restart_time = std::chrono::system_clock::now();
}

void sbndaq::BernCRTData::VerifyMACConfiguration(const std::vector<uint8_t> &  hardware_macs) {
  /**
   * Check if the FEB list defined in FHiCL file agrees with FEBs detected by febdrv
   */

  //check number of FEBs
  bool configuration_ok = (hardware_macs.size() == nFEBs());

  //check FEB MAC addresses
  //here we rely on both hardware_macs and MAC5s_ being sorted already
  for(unsigned int i = 0; i < nFEBs() && configuration_ok; i++) {
    configuration_ok = (hardware_macs[i] == MAC5s_[i]);
  }

  //print error message and crash
  if(!configuration_ok) {
    std::string error_message = __func__;
    error_message += "() List of "+std::to_string(MAC5s_.size())+ " FEBs declared in the FHiCL file (dec):";
    for(auto m : MAC5s_) error_message += " " + std::to_string(m);
    error_message += " doesn't match list of " + std::to_string(hardware_macs.size()) + " detected FEBs:";
    for(auto h : hardware_macs) error_message += " " + std::to_string(h);
    TLOG(TLVL_ERROR) << error_message;
    throw cet::exception( std::string(TRACE_NAME) + " " + error_message );
  }
} //VerifyMACConfiguration

uint64_t sbndaq::BernCRTData::GetTimeSinceLastRestart() {
  return (std::chrono::system_clock::now() -  last_restart_time).count();
}

/*---------------BERN CRT FEB DATA-------------------------------------*/
size_t sbndaq::BernCRTData::GetFEBData() {
  /**
   * Reads data from FEB
   */
  
  TLOG(TLVL_DEBUG) << __func__ << "() called";
  
  // Sleep until the next poll comes
  int now = std::chrono::system_clock::now().time_since_epoch().count() % feb_poll_period_;
  usleep((feb_poll_period_ - now)/1000);

  size_t data_size=0;
  
  size_t events = 0;
  
  for(auto mac : MAC5s_) {
    FEB_t & feb = FEBs_[mac];
    
    uint64_t poll_start = std::chrono::system_clock::now().time_since_epoch().count();
    int32_t system_clock_deviation = poll_start - steady_clock_offset - std::chrono::steady_clock::now().time_since_epoch().count();
    
    febdrv.pollfeb(mac);
    
    uint64_t poll_end = std::chrono::system_clock::now().time_since_epoch().count();
    
    feb.metadata.update_poll_time(poll_start, poll_end);
    feb.metadata.set_clock_deviation(system_clock_deviation);
    
    std::unique_lock<std::mutex> lock(*(feb.mutexptr));
    
    unsigned int feb_read_events = 0;
    uint64_t last_poll_event_number = feb.metadata.feb_event_number();
    
    while(true) {
      //loop over events received via ethernet and push into circular buffer
      int numbytes = febdrv.GetData();
      if(numbytes<=0) break;
      
      int datalen = numbytes-18;
      
      
      TLOG(TLVL_DEBUG)<<__func__<<"()  datalen = "<<datalen;
        
      data_size += datalen;
      
      for(int jj=0; jj<datalen; ) { // jj is incremented in processSingleEvent
        feb_read_events++;
        febdrv.processSingleEvent(jj, feb.event);
        feb.metadata.increment_feb_events(1 + feb.event.lostcpu + feb.event.lostfpga);
        feb.buffer.push_back(std::make_pair(feb.event, feb.metadata));
      }
    }
    
    events += feb_read_events;
    
    //only at the end of the poll we know how many events we have in the poll
    //loop over the poll again and update metadata
    for(unsigned int i = feb.buffer.size() - feb_read_events; i < feb.buffer.size(); i++) {
      feb.buffer[i].second.set_feb_events_per_poll(feb.metadata.feb_event_number() - last_poll_event_number);
      //TODO perhaps we would like to save number of lost events as well?
    }

//the commented code below doesn't work: to be investigated
/*    if(metricMan != nullptr) metricMan->sendMetric(
      std::string("feb_hit_rate_Hz_")+std::to_string(feb.fragment_id & 0xff),
      feb.metadata.feb_events_per_poll() * 1e9 / (feb.metadata.this_poll_end() - feb.metadata.last_poll_end()),
      "CRT rate", 5, artdaq::MetricMode::Average);
    if(metricMan != nullptr) metricMan->sendMetric(
      std::string("feb_read_rate_Hz_")+std::to_string(feb.fragment_id & 0xff),
      feb_read_events * 1e9 / (feb.metadata.this_poll_end() - feb.metadata.last_poll_end()),
      "CRT rate", 5, artdaq::MetricMode::Average); */
    
  } //loop over FEBs
  
  //workaround for spike issue: periodically restart febdrv
  if(feb_restart_period_) {
    if(GetTimeSinceLastRestart() > feb_restart_period_) {
      StartFebdrv();
    }
  }
  
  TLOG(TLVL_DEBUG) << __func__ << "() read " << std::to_string(events) << " events of size of " << std::to_string(data_size);

  return events;
} //GetFEBData

DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(sbndaq::BernCRTData) 
