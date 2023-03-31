/////////////////////////////////////////////////////////////////////
// Class: ICARUSTriggerUDPDump_module.cc
// Module Type: analyzer
// File: ICARUSTriggerUDPDump_module.cc
// Description: Print out information about each event
/////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Utilities/ExceptionMessages.h"

#include "sbndaq-artdaq-core/Overlays/ICARUS/ICARUSTriggerUDPFragment.hh"
#include "sbndaq-artdaq-core/Overlays/ICARUS/ICARUSPMTGateFragment.hh"

#include "artdaq/DAQdata/Globals.hh"

#include "artdaq-core/Data/Fragment.hh"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <vector>
#include <iostream>

#ifdef TRACE_NAME
#undef TRACE_NAME
#endif

#define TRACE_NAME "ICARUSTriggerUDPDump_module"

namespace icarus {
  class ICARUSTriggerUDPDump;
}


class icarus::ICARUSTriggerUDPDump : public art::EDAnalyzer {
public:
  explicit ICARUSTriggerUDPDump(fhicl::ParameterSet const & pset);
  virtual ~ICARUSTriggerUDPDump();

  virtual void analyze(art::Event const & evt);

private:
  std::string raw_data_label_;
};

icarus::ICARUSTriggerUDPDump::ICARUSTriggerUDPDump(fhicl::ParameterSet const & pset)
  : EDAnalyzer(pset),
    raw_data_label_(pset.get<std::string>("raw_data_label"))
{
}

icarus::ICARUSTriggerUDPDump::~ICARUSTriggerUDPDump()
{
}

void icarus::ICARUSTriggerUDPDump::analyze(art::Event const & evt)
{
  art::EventNumber_t eventNumber = evt.event();

  // Look for fragments
 
  art::Handle< std::vector<artdaq::Fragment> > raw_data;
  evt.getByLabel(raw_data_label_, "ICARUSTriggerUDP", raw_data);
  
  if(!raw_data.isValid()) {
    TLOG(TLVL_INFO) << "Run " << evt.run() << ", subrun " << evt.subRun() << ", event " << eventNumber << " has zero ICARUSTriggerUDP Fragments in module ";
    return;
  }

  TLOG(TLVL_INFO) <<  "######################################################################";
  std::cout << "Run " << evt.run() << ", subrun " << evt.subRun()
		  << ", event " << eventNumber << " has " << raw_data->size()
	    << " ICARUSTriggerUDP fragment(s)." << std::endl;
  std::cout << raw_data->size() << std::endl;
  for(size_t idx=0; idx<raw_data->size(); ++idx){
    const auto& frag((*raw_data)[idx]);
    uint64_t artdaq_ts = frag.timestamp();
    ICARUSTriggerUDPFragment trigfrag(frag);
    //Print fragment 
    std::cout << " " << trigfrag << " artdaq TS: " << artdaq_ts << std::endl;
    //Parse data and dump similar information after parsing, test of offline decoding
    std::string data = trigfrag.GetDataString();
    char *buffer = const_cast<char*>(data.c_str());
    icarus::ICARUSTriggerInfo datastream_info = icarus::parse_ICARUSTriggerString(buffer);
    uint64_t wr_ts = datastream_info.getNanoseconds_since_UTC_epoch() + 2e9; //hardcode additional TAI->UTC offset for now
    long event_no = datastream_info.wr_event_no;
    long gate_id = datastream_info.gate_id;
    long gate_type = datastream_info.gate_type;
    bool isBNB = trigfrag.isBNB();
    bool isNuMI = trigfrag.isNuMI();
    uint64_t last_ts = trigfrag.getLastTimestamp();
    uint64_t last_ts_bnb = trigfrag.getLastTimestampBNB();
    uint64_t last_ts_numi = trigfrag.getLastTimestampNuMI();
    uint64_t ntp_time = trigfrag.getNTPTime();
    long delta_gates = trigfrag.getDeltaGates();
    long delta_gates_bnb = trigfrag.getDeltaGatesBNB();
    long delta_gates_numi = trigfrag.getDeltaGatesNuMI();
    
    std::cout << "Event No: " << event_no << std::endl;
    std::cout << "Timestamp: " << wr_ts << std::endl;
    std::cout << "Gate ID: " << gate_id << std::endl;
    std::cout << "Gate Type: " << gate_type << std::endl;
    std::cout << "isBNB: " << isBNB << std::endl;
    std::cout << "isNuMI: " << isNuMI << std::endl;
    std::cout << "Last Timestamp: " << last_ts << std::endl;
    std::cout << "Last Timestamp BNB: " << last_ts_bnb << std::endl;
    std::cout << "Last Timestamp NuMI: " << last_ts_numi << std::endl;
    std::cout << "NTP Time: " << ntp_time << std::endl;
    std::cout << "Delta Gates: " << delta_gates << std::endl;
    std::cout << "Delta Gates BNB: " << delta_gates_bnb << std::endl;
    std::cout << "Delta Gates NuMI: " << delta_gates_numi << std::endl;
  }

}										        

DEFINE_ART_MODULE(icarus::ICARUSTriggerUDPDump)
