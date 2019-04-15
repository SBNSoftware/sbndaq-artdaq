////////////////////////////////////////////////////////////////////////
// Class:       PhysCrateSimpleAna
// Module Type: analyzer
// File:        PhysCrateSimpleAna_module.cc
// Description: Prints out information about each event.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"

#include "art/Utilities/ExceptionMessages.h"
#include "sbndaq-artdaq-core/Overlays/ICARUS/PhysCrateFragment.hh"
#include "sbndaq-artdaq-core/Overlays/ICARUS/PhysCrateStatFragment.hh"

#include "artdaq-core/Data/Fragment.hh"
#include "icarus-artdaq-base/common.h"


#include "art/Framework/Services/Optional/TFileService.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <vector>
#include <iostream>
#include <sstream>

#include "TH2F.h"
#include "TNtuple.h"

#ifdef TRACE_NAME
#undef TRACE_NAME
#endif

#define TRACE_NAME "PhysCrateSimpleAna_module"

namespace icarus {
  class PhysCrateSimpleAna;
}

class icarus::PhysCrateSimpleAna : public art::EDAnalyzer {
public:
  explicit PhysCrateSimpleAna(fhicl::ParameterSet const & pset);
  virtual ~PhysCrateSimpleAna();

  virtual void analyze(art::Event const & evt);

private:
  std::string raw_data_label_;

  TNtuple* fChTuple;
};


icarus::PhysCrateSimpleAna::PhysCrateSimpleAna(fhicl::ParameterSet const & pset)
  : EDAnalyzer(pset),
    raw_data_label_(pset.get<std::string>("raw_data_label"))
    
{
  
  art::ServiceHandle<art::TFileService> tfs;
  fChTuple = tfs->make<TNtuple>("nt_ch","Channel Ntuple","run:event:frag:board:ch:ped:rms:max:min");
  
}

icarus::PhysCrateSimpleAna::~PhysCrateSimpleAna()
{
}

void icarus::PhysCrateSimpleAna::analyze(art::Event const & evt)
{
  
  art::EventNumber_t eventNumber = evt.event();
  
  art::Handle< std::vector<artdaq::Fragment> > raw_data;
  evt.getByLabel(raw_data_label_, "PHYSCRATEDATA", raw_data);

  if(!raw_data.isValid()){
    TLOG(TLVL_INFO) <<  "Run " << evt.run() << ", subrun " << evt.subRun()
              << ", event " << eventNumber << " has zero"
              << " PhysCrate fragments " << " in module " << raw_data_label_;    
    return;
  }
  
  for (size_t idx = 0; idx < raw_data->size(); ++idx) {

    const auto& frag((*raw_data)[idx]);
    
    PhysCrateFragment bb(frag);

    for(size_t i_b=0; i_b < bb.nBoards(); ++i_b){

      for(size_t i_c=0; i_c<bb.nChannelsPerBoard(); ++i_c){

	float ped=0,rms=0,max=-999,min=99999999;
	
	for(size_t i_t=0; i_t<bb.nSamplesPerChannel(); ++i_t){

	  
	  auto adc_value = bb.adc_val(i_b,i_c,i_t);
	  if(adc_value==0) continue;

	  ped += adc_value;
	  if(max<adc_value) max = adc_value;
	  if(min>adc_value) min = adc_value;

	}

	ped = ped / bb.nSamplesPerChannel();

	for(size_t i_t=0; i_t<bb.nSamplesPerChannel(); ++i_t){	  
	  auto adc_value = bb.adc_val(i_b,i_c,i_t);
	  if(adc_value==0) continue;

	  rms += (ped - adc_value)*(ped - adc_value);
	}

	rms = rms / bb.nSamplesPerChannel();
	rms = std::sqrt(rms);

	fChTuple->Fill(evt.run(),eventNumber,idx,i_b,i_c,
		       ped,rms,max,min);

      }
    }

  }
  
}

DEFINE_ART_MODULE(icarus::PhysCrateSimpleAna)
