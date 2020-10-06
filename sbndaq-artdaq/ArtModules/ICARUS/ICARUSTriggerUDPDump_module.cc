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

#include "sbndaq-artdaq/Generators/ICARUS/icarus-base/common.h"

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
  
  //art::Handle< std::vector<artdaq::Fragment> > pmt_gate;
  //evt.getByLabel(raw_data_label_, "ICARUSPMTGate", pmt_gate);
  
  if(!raw_data.isValid()) {
    TLOG(TLVL_INFO) << "Run " << evt.run() << ", subrun " << evt.subRun() << ", event " << eventNumber << " has zero ICARUSTriggerUDP Fragments in module ";
    return;
  }

  //if(!pmt_gate.isValid()) {
  //TLOG(TLVL_INFO) << "Run " << evt.run() << ", subrun " << evt.subRun() << ", event " << eventNumber << " has zero ICARUSPMTGate Fragments in module ";
    //return;
  //}

  TLOG(TLVL_INFO) <<  "######################################################################";
  std::cout << "Run " << evt.run() << ", subrun " << evt.subRun()
		  << ", event " << eventNumber << " has " << raw_data->size()
	    << " ICARUSTriggerUDP fragment(s)." << std::endl;
  std::cout << raw_data->size() << std::endl; 
  for(size_t idx=0; idx<raw_data->size(); ++idx){
    const auto& frag((*raw_data)[idx]);
    ICARUSTriggerUDPFragment trigfrag(frag);
    std::cout << " " << trigfrag << std::endl;
    int buffer_size = 23;
    for(int i = 0; i <= buffer_size; ++i)
    {
      if(i < buffer_size)
	std::cout << *((char*)frag.dataBeginBytes()+i*sizeof(char));
      if(i == buffer_size)
	std::cout << *((char*)frag.dataBeginBytes()+i*sizeof(char)) << std::endl;
      
    }
    //std::cout << "Name: " << trigfrag.Metadata()->getName() << std::endl;
    //std::cout << "Event Number: " << trigfrag.Metadata()->getEventNo() << std::endl;
    //std::cout << "Seconds: " << trigfrag.Metadata()->getSeconds() << std::endl;
    //std::cout << "Nanoseconds: " << trigfrag.Metadata()->getNanoSeconds() << std::endl;
  }

  //std::cout << "Run " << evt.run() << ", subrun " << evt.subRun()
  //	    << ", event " << eventNumber << " has " << pmt_gate->size()
  //        << " ICARUSPMTGate fragment(s)." << std::endl;
  //std::cout << pmt_gate->size() << std::endl;

  /*
  for(size_t idx=0; idx<pmt_gate->size(); ++idx){
    const auto& frag((*pmt_gate)[idx]);
    ICARUSPMTGateFragment pmtfrag(frag);
    //std::vector<int> data = pmtfrag.Metadata()->getNum();
    //for(unsigned int i=1; i<data.size(); ++i)
    //std::cout << "PMT pair " << i << ": " << data[i] << std::endl; 
  } 
  */
}										        

DEFINE_ART_MODULE(icarus::ICARUSTriggerUDPDump)
