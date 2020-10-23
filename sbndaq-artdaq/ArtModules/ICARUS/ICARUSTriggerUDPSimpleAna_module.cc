////////////////////////////////////////////////////////////////////////                         
// Class:       ICARUSTriggerUDPSimpleAna                                             
// Module Type: analyzer                                                                         
// File:        ICARUSTriggerUDPSimpleAna_module.cc                                 
// Description: Prints out information about each event.                                         
////////////////////////////////////////////////////////////////////////  

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"

#include "art/Utilities/ExceptionMessages.h"
#include "sbndaq-artdaq-core/Overlays/ICARUS/ICARUSTriggerUDPFragment.hh"

#include "artdaq-core/Data/Fragment.hh"
#include "sbndaq-artdaq/Generators/ICARUS/icarus-base/common.h"

#include "art_root_io/TFileService.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <vector>
#include <iostream>
#include <sstream>

#include "TH1F.h"
#include "TNtuple.h"

#ifdef TRACE_NAME
#undef TRACE_NAME
#endif

#define TRACE_NAME "ICARUSTriggerUDPSimpleAna_module"

namespace icarus {
  class ICARUSTriggerUDPSimpleAna;
}

class icarus::ICARUSTriggerUDPSimpleAna : public art::EDAnalyzer {
public:
  explicit ICARUSTriggerUDPSimpleAna(fhicl::ParameterSet const & pset);
  virtual ~ICARUSTriggerUDPSimpleAna();
  
  virtual void analyze(art::Event const & evt);

private:
  std::string raw_data_label_;
  TNtuple* fTuple;
};

icarus::ICARUSTriggerUDPSimpleAna::ICARUSTriggerUDPSimpleAna(fhicl::ParameterSet const & pset) 
  : EDAnalyzer(pset), 
    raw_data_label_(pset.get<std::string>("raw_data_label"))
{
  art::ServiceHandle<art::TFileService> tfs;
  fTuple = tfs->make<TNtuple>("nt_trig", "Trigger Ntuple", "run:event:frag:eventno:sec:nsec");
  
}

icarus::ICARUSTriggerUDPSimpleAna::~ICARUSTriggerUDPSimpleAna()
{
  
}

void icarus::ICARUSTriggerUDPSimpleAna::analyze(art::Event const & evt)
{
  art::EventNumber_t eventNumber = evt.event();
  art::Handle< std::vector<artdaq::Fragment> > raw_data;
  evt.getByLabel(raw_data_label_, "ICARUSTriggerUDP", raw_data);

  if(!raw_data.isValid()){
    TLOG(TLVL_INFO) <<  "Run " << evt.run() << ", subrun " << evt.subRun()
		    << ", event " << eventNumber << " has zero"
		    << " ICARUSTriggerUDP fragments " << " in module " << raw_data_label_;
    return;
  }
  
  for(size_t idx = 0; idx < raw_data->size(); ++idx)
  {
    const auto& frag((*raw_data)[idx]);
    ICARUSTriggerUDPFragment trigfrag(frag);
    int eventno = trigfrag.getEventNo();
    int seconds = trigfrag.getSeconds();
    long nanoseconds = trigfrag.getNanoSeconds();
    fTuple->Fill(evt.run(),eventNumber, idx, eventno, seconds, nanoseconds);
  }
}

DEFINE_ART_MODULE(icarus::ICARUSTriggerUDPSimpleAna)
