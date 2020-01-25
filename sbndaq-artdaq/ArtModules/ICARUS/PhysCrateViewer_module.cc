////////////////////////////////////////////////////////////////////////
// Class:       PhysCrateViewer
// Module Type: analyzer
// File:        PhysCrateViewer_module.cc
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

#include "TH2F.h"

#ifdef TRACE_NAME
#undef TRACE_NAME
#endif

#define TRACE_NAME "PhysCrateViewer_module"

namespace icarus {
  class PhysCrateViewer;
}

class icarus::PhysCrateViewer : public art::EDAnalyzer {
public:
  explicit PhysCrateViewer(fhicl::ParameterSet const & pset);
  virtual ~PhysCrateViewer();

  virtual void analyze(art::Event const & evt);

private:
  std::string raw_data_label_;
};


icarus::PhysCrateViewer::PhysCrateViewer(fhicl::ParameterSet const & pset)
  : EDAnalyzer(pset),
    raw_data_label_(pset.get<std::string>("raw_data_label"))
    
{
}

icarus::PhysCrateViewer::~PhysCrateViewer()
{
}

void icarus::PhysCrateViewer::analyze(art::Event const & evt)
{
  
  art::EventNumber_t eventNumber = evt.event();
  
  // ***********************
  // *** PhysCrate Fragments ***
  // ***********************
  
  // look for raw PhysCrate data
  
  art::Handle< std::vector<artdaq::Fragment> > raw_data;
  evt.getByLabel(raw_data_label_, "PHYSCRATEDATA", raw_data);

  // art::Handle< std::vector<artdaq::Fragment> > raw_stat;
  // evt.getByLabel(raw_data_label_, "PHYSCRATESTAT", raw_stat);
  
  if(!raw_data.isValid()){
    TLOG(TLVL_INFO) <<  "Run " << evt.run() << ", subrun " << evt.subRun()
              << ", event " << eventNumber << " has zero"
              << " PhysCrate fragments " << " in module " << raw_data_label_;    
    return;
  }
  /* if(!raw_stat.isValid()){
    TLOG(TLVL_INFO) <<  "Run " << evt.run() << ", subrun " << evt.subRun()
              << ", event " << eventNumber << " has zero"
              << " PhysCrateStat fragments " << " in module " << raw_data_label_;
    return;
  } */

  art::ServiceHandle<art::TFileService> tfs;
  std::vector<TH2F*> hist_vector;
  
  
  TLOG(TLVL_INFO) <<   "######################################################################";
  
  TLOG(TLVL_INFO) <<  "Run " << evt.run() << ", subrun " << evt.subRun()
	    << ", event " << eventNumber << " has " << raw_data->size()
	    << " PhysCrate fragment(s).";

  
  for (size_t idx = 0; idx < raw_data->size(); ++idx) {
    const auto& frag((*raw_data)[idx]);
    
    PhysCrateFragment bb(frag);
    for(size_t i_b=0; i_b < bb.nBoards(); ++i_b){
      std::stringstream ss_hist_title,ss_hist_name;
      ss_hist_title << "(Run,Event,Fragment,Board)=("
		    << evt.run() <<","
		    << eventNumber << ","
		    << idx << ","
		    << i_b << ");Channel;Time";
      ss_hist_name << "h_"
	//<< evt.run() <<"_"
		   << eventNumber << "_"
		   << idx << "_"
		   << i_b;

      TLOG(TLVL_INFO) << "going to create histogram " << ss_hist_name.str();

      hist_vector.push_back(tfs->make<TH2F>(ss_hist_name.str().c_str(),ss_hist_title.str().c_str(),
					    bb.nChannelsPerBoard(),0,bb.nChannelsPerBoard(),
					    bb.nSamplesPerChannel(),0,bb.nSamplesPerChannel()));

      TLOG(TLVL_INFO) << "Created histo. Total histos is now " << hist_vector.size();

      
      TLOG(TLVL_INFO) << "Printing board " << i_b+1 << " / " << bb.nBoards();
      TLOG(TLVL_INFO) << "\tData location is " << bb.DataTileHeaderLocation(i_b) << ".";
      TLOG(TLVL_INFO) << "\t Board (event,timestamp) = (0x" << std::hex << bb.BoardEventNumber(i_b) << ", 0x"
		<< bb.BoardTimeStamp(i_b) << ")" << std::dec ;
      
      TLOG(TLVL_INFO) <<  "\t(First data word is 0x"
		<< std::hex << *(bb.BoardData(i_b)) << std::dec << ")"  ;
      for(size_t i_t=0; i_t<bb.nSamplesPerChannel(); ++i_t){
	//TLOG(TLVL_INFO) <<  "Printing sample " << i_t+1 << " / " << bb.nSamplesPerChannel();
	for(size_t i_c=0; i_c<bb.nChannelsPerBoard(); ++i_c){
	  //TLOG(TLVL_INFO) << "\tPrinting channel " << i_c+1 << " / " << bb.nChannelsPerBoard();
	  auto adc_value = bb.adc_val(i_b,i_c,i_t);
	  
	  if(adc_value!=0) hist_vector.back()->SetBinContent(i_c+1,i_t+1,adc_value);
	}
      }
    }

  }
  
}

DEFINE_ART_MODULE(icarus::PhysCrateViewer)
