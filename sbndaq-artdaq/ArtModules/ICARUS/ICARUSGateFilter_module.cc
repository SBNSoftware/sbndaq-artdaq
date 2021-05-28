/////////////////////////////////////////////////////////////////////
// Class: ICARUSGateFilter_module.cc
// Module Type: analyzer
// File: ICARUSGateFilter_module.cc
// Description: Filter if Gate Matches
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

#define TRACE_NAME "ICARUSGateFilter_module"

namespace icarus {
  class ICARUSGateFilter;
}


class icarus::ICARUSGateFilter : public art::EDFilter {
public:
  explicit ICARUSGateFilter(fhicl::ParameterSet const & pset);
  virtual ~ICARUSGateFilter();

  virtual void filter(art::Event const & evt);

private:
  std::string raw_data_label_;
  int gate_type_;
  
};

icarus::ICARUSGateFilter::ICARUSGateFilter(fhicl::ParameterSet const & pset)
  : EDAnalyzer(pset),
    raw_data_label_(pset.get<std::string>("raw_data_label")),
    gate_type_(pset.get<int>("gate_type"))
{
}

icarus::ICARUSGateFilter::~ICARUSGateFilter()
{
}

void icarus::ICARUSGateFilter::filter(art::Event const & evt)
{
  art::EventNumber_t eventNumber = evt.event();

  // Look for fragments
 
  art::Handle< std::vector<artdaq::Fragment> > raw_data;
  evt.getByLabel(raw_data_label_, "ICARUSTriggerUDP", raw_data);
  
  if(!raw_data.isValid()) {
    TLOG(TLVL_INFO) << "Run " << evt.run() << ", subrun " << evt.subRun() << ", event " << eventNumber << " has zero ICARUSTriggerUDP Fragments in module ";
    return false;
  }

  for(auto const& frag : *raw_data){
    ICARUSTriggerUDPFragment trigfrag(frag);

    if(trigfrag.getGateType()==gate_type_){
      TLOG(TLVL_DEBUG) << "Event " << eventNumber << " has gate type " 
		       << trigfrag.getGateType() << "==" << gate_type_ << " and passes filter.";
      return true;
    }
    else{
      TLOG(TLVL_DEBUG) << "Event " << eventNumber << " has gate type " 
		       << trigfrag.getGateType() << "!=" << gate_type_ << " and passes filter.";
      return false;
    }
    
  }
}

DEFINE_ART_MODULE(icarus::ICARUSGateFilter)
