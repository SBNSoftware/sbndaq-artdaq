////////////////////////////////////////////////////////////////////////
// Class:       PhysCrateDump
// Module Type: analyzer
// File:        PhysCrateDump_module.cc
// Description: Prints out information about each event.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"

#include "art/Utilities/ExceptionMessages.h"
#include "sbndaq-artdaq-core/Overlays/ICARUS/PhysCrateFragment.hh"
#include "sbndaq-artdaq-core/Overlays/ICARUS/PhysCrateStatFragment.hh"

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

#define TRACE_NAME "PhysCrateDump_module"


namespace icarus {
  class PhysCrateDump;
}

class icarus::PhysCrateDump : public art::EDAnalyzer {
public:
  explicit PhysCrateDump(fhicl::ParameterSet const & pset);
  virtual ~PhysCrateDump();

  virtual void analyze(art::Event const & evt);

private:
  std::string raw_data_label_;
  uint32_t num_adcs_to_show_;
};


icarus::PhysCrateDump::PhysCrateDump(fhicl::ParameterSet const & pset)
  : EDAnalyzer(pset),
    raw_data_label_(pset.get<std::string>("raw_data_label")),
    num_adcs_to_show_(pset.get<uint32_t>("num_adcs_to_show", 0))
    
{
}

icarus::PhysCrateDump::~PhysCrateDump()
{
}

void icarus::PhysCrateDump::analyze(art::Event const & evt)
{
    
  art::EventNumber_t eventNumber = evt.event();
  
  // ***********************
  // *** PhysCrate Fragments ***
  // ***********************
  
  // look for raw PhysCrate data
  
  art::Handle< std::vector<artdaq::Fragment> > raw_data;
  evt.getByLabel(raw_data_label_, "PHYSCRATEDATA", raw_data);

  art::Handle< std::vector<artdaq::Fragment> > raw_stat;
  evt.getByLabel(raw_data_label_, "PHYSCRATESTAT", raw_stat);
  
  if(!raw_data.isValid()){
    TLOG(TLVL_INFO) <<  "Run " << evt.run() << ", subrun " << evt.subRun()
              << ", event " << eventNumber << " has zero"
              << " PhysCrate fragments " << " in module " << raw_data_label_ ;
    return;
  }
  if(!raw_stat.isValid()){
    TLOG(TLVL_INFO) << "Run " << evt.run() << ", subrun " << evt.subRun()
              << ", event " << eventNumber << " has zero"
              << " PhysCrateStat fragments " << " in module " << raw_data_label_;
    return;
  }
  
  
  TLOG(TLVL_INFO) <<  "######################################################################";
  TLOG(TLVL_INFO) << "Run " << evt.run() << ", subrun " << evt.subRun()
	    << ", event " << eventNumber << " has " << raw_data->size()
	    << " PhysCrate fragment(s).";
  
  for (size_t idx = 0; idx < raw_data->size(); ++idx) {
    const auto& frag((*raw_data)[idx]);
    
    PhysCrateFragment bb(frag);
    
    //TLOG(TLVL_INFO) << "bb" << bb ;


    if (num_adcs_to_show_ > 0) {
      /*      
      if (num_adcs_to_show_ > bb.total_adc_values() ) {
	throw cet::exception("num_adcs_to_show is larger than total number of adcs in fragment");
      } else {

	TLOG(TLVL_INFO) << "First " << num_adcs_to_show_ 
		  << " ADC values in the fragment: " ;
      }
      */
      for (uint32_t i_adc = 0; i_adc < num_adcs_to_show_; ++i_adc) {
	TLOG(TLVL_INFO) <<  "" << std::hex << std::setfill('0') << std::setw(4) << *((uint16_t*)frag.dataBeginBytes() + i_adc);
	if(i_adc%8==7) TLOG(TLVL_INFO) <<  ".";
      }
      TLOG(TLVL_INFO) << " " << std::dec;
    }
  }

  for(size_t idx=0; idx<raw_stat->size(); ++idx){
    const auto& frag((*raw_stat)[idx]);    
    PhysCrateStatFragment bb(frag);
    TLOG(TLVL_INFO) << "" << bb ;
  }
}

DEFINE_ART_MODULE(icarus::PhysCrateDump)
