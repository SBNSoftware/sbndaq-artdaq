/////////////////////////////////////////////////////////////////////
// Class: ICARUSGateFilter_module.cc
// Module Type: analyzer
// File: ICARUSGateFilter_module.cc
// Description: Filter if Gate Matches
/////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Utilities/ExceptionMessages.h"

#include "sbndaq-artdaq-core/Overlays/ICARUS/ICARUSTriggerV2Fragment.hh"
#include "sbndaq-artdaq-core/Overlays/ICARUS/ICARUSPMTGateFragment.hh"


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

  virtual bool filter(art::Event & evt) override;

private:
  std::string raw_data_label_;
  int gate_type_;
  int trigger_type_;
  int trig_location_;
};

icarus::ICARUSGateFilter::ICARUSGateFilter(fhicl::ParameterSet const & pset)
  : EDFilter(pset),
    raw_data_label_(pset.get<std::string>("raw_data_label")),
    gate_type_(pset.get<int>("gate_type")),
    trigger_type_(pset.get<int>("trigger_type",-1)),
    trig_location_(pset.get<int>("trigger_location",-1))
{
}

icarus::ICARUSGateFilter::~ICARUSGateFilter()
{
}

bool icarus::ICARUSGateFilter::filter(art::Event & evt)
{
  art::EventNumber_t eventNumber = evt.event();

  // Look for fragments
 
  art::Handle< std::vector<artdaq::Fragment> > raw_data;
  evt.getByLabel(raw_data_label_, "ICARUSTriggerV2", raw_data);
  
  if(!raw_data.isValid()) {
    TLOG(TLVL_INFO) << "Run " << evt.run() << ", subrun " << evt.subRun() << ", event " << eventNumber << " has zero ICARUSTriggerV2 Fragments in module ";
    return false;
  }

  for(auto const& frag : *raw_data){
    ICARUSTriggerV2Fragment trigfrag(frag);
    //To document the current (5/3/22) mapping:
    //Gate Type: 1 = BNB, 2 = NuMI, 3 = OffbeamBNB, 4 = OffbeamNuMI, 5 = Calib
    //Trigger Type: 0 = Majority, 1 = Minbias
    //Trigger Source (Location): 1 = East, 2 = West, 7 = Both, 0 = Unknown

    if((trigfrag.getGateType()==gate_type_ || gate_type_==-1) 
       && (trigfrag.getTriggerType()==trigger_type_ || trigger_type_==-1) 
       && (trigfrag.getTriggerSource()==trig_location_ || trig_location_==-1)){
      TLOG(TLVL_INFO) << "Event " << eventNumber << " has gate type " 
		       << trigfrag.getGateType() << "==" << gate_type_  
	               << " and trigger type " << trigfrag.getTriggerType() << "==" << trigger_type_ 
		       << " and trigger source " << trigfrag.getTriggerSource() << "==" << trig_location_ 
		       << "  and passes filter.";
      return true;
    }
    else{
      TLOG(TLVL_INFO) << "Event " << eventNumber << " has gate type " 
		       << trigfrag.getGateType() << "!=" << gate_type_
		       << " or trigger type " << trigfrag.getTriggerType() << "!=" << trigger_type_
                       << " or trigger source " << trigfrag.getTriggerSource() << "!=" << trig_location_
		       << " and fails filter.";
      return false;
    }
    
  }

  return false;
}

DEFINE_ART_MODULE(icarus::ICARUSGateFilter)
