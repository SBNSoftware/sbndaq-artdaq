////////////////////////////////////////////////////////////////////////
// Class:       BernCRTAnaV2
// Module Type: analyzer
// File:        BernCRTAna_module.cc
// Description: Makes a tree with waveform information.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"

#include "canvas/Utilities/Exception.h"

#include "sbndaq-artdaq-core/Overlays/Common/BernCRTFragmentV2.hh"
#include "artdaq-core/Data/Fragment.hh"
#include "artdaq-core/Data/ContainerFragment.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"

#include "art_root_io/TFileService.h"
#include "TNtuple.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <vector>
#include <iostream>

namespace sbndaq {
  class BernCRTAnaV2;
}

class sbndaq::BernCRTAnaV2 : public art::EDAnalyzer {

public:
  explicit BernCRTAnaV2(fhicl::ParameterSet const & pset); // explicit doesn't allow for copy initialization
  virtual ~BernCRTAnaV2();
  
  virtual void analyze(art::Event const & evt);
  

private:
  void analyze_fragment(artdaq::Fragment & frag);
  
  TTree * hits;

  uint16_t flags;
  uint16_t lostcpu;
  uint16_t lostfpga;
  uint32_t ts0;
  uint32_t ts1;
  uint16_t adc[32];
  uint32_t coinc;

  uint64_t  feb_hit_number          = 0; //hit counter for individual FEB, including hits lost in FEB or fragment generator        
  uint64_t  timestamp               = 0; //absolute timestamp
  uint64_t  last_accepted_timestamp = 0; //timestamp of previous accepted hit
  uint32_t  lost_hits               = 0; //number of lost hits from the previous one


//metadata
  uint8_t  mac5; //last 8 bits of FEB mac5 address
  uint64_t  run_start_time;
  uint64_t  this_poll_start;
  uint64_t  this_poll_end;
  uint64_t  last_poll_start;
  uint64_t  last_poll_end;
  int32_t   system_clock_deviation;
  uint32_t  feb_hits_in_poll;
  uint32_t  feb_hits_in_fragment;

//information from fragment header
  uint32_t  sequence_id;
};

//Define the constructor
sbndaq::BernCRTAnaV2::BernCRTAnaV2(fhicl::ParameterSet const & pset)
  : EDAnalyzer(pset)
{

  //this is how you setup/use the TFileService
  //I do this here in the constructor to setup the histogram just once
  //but you can call the TFileService and make new objects from anywhere
  art::ServiceHandle<art::TFileService> tfs; //pointer to a file named tfs

  hits = tfs->make<TTree>("hits", "FEB hits");

//event data
  hits->Branch("flags",         &flags,         "flags/s");
  hits->Branch("lostcpu",       &lostcpu,       "lostcpu/s");
  hits->Branch("lostfpga",      &lostfpga,      "lostfpga/s");
  hits->Branch("ts0",           &ts0,           "ts0/i");
  hits->Branch("ts1",           &ts1,           "ts1/i");
  hits->Branch("adc",           &adc,           "adc[32]/s");
  hits->Branch("coinc",         &coinc,         "coinc/i");
  hits->Branch("feb_hit_number",&feb_hit_number,"feb_hit_number/l");
  hits->Branch("timestamp",     &timestamp,     "timestamp/l");
  hits->Branch("last_accepted_timestamp",&last_accepted_timestamp, "last_accepted_timestamp/l");
  hits->Branch("lost_hits",     &lost_hits,     "lost_hits/s");

//metadata
  hits->Branch("mac5",                      &mac5,                        "mac5/b");
  hits->Branch("run_start_time",            &run_start_time,              "run_start_time/l");
  hits->Branch("this_poll_start",           &this_poll_start,             "this_poll_start/l");
  hits->Branch("this_poll_end",             &this_poll_end,               "this_poll_end/l");
  hits->Branch("last_poll_start",           &last_poll_start,             "last_poll_start/l");
  hits->Branch("last_poll_end",             &last_poll_end,               "last_poll_end/l");
  hits->Branch("system_clock_deviation",    &system_clock_deviation,      "system_clock_deviation/I");
  hits->Branch("feb_hits_in_poll",          &feb_hits_in_poll,            "feb_hits_in_poll/i");
  hits->Branch("feb_hits_in_fragment",      &feb_hits_in_fragment,        "feb_hits_in_fragment/i");

  hits->Branch("sequence_id",               &sequence_id,                 "sequence_id/i");
}

sbndaq::BernCRTAnaV2::~BernCRTAnaV2() {}

void sbndaq::BernCRTAnaV2::analyze_fragment(artdaq::Fragment & frag) {

  BernCRTFragmentV2 bern_fragment(frag);
//  TLOG(TLVL_INFO) << bern_fragment;

  sequence_id               = frag.sequenceID();

  //metadata
  const BernCRTFragmentMetadataV2* md = bern_fragment.metadata();
//  TLOG(TLVL_INFO)<<*md;

  mac5                      = md->MAC5();
  run_start_time            = md->run_start_time();
  this_poll_start           = md->this_poll_start();
  this_poll_end             = md->this_poll_end();
  last_poll_start           = md->last_poll_start();
  last_poll_end             = md->last_poll_end();
  system_clock_deviation    = md->system_clock_deviation();
  feb_hits_in_poll          = md->hits_in_poll();
  feb_hits_in_fragment      = md->hits_in_fragment();
  if (0) {
    std::cout <<   " mac5             "   <<    unsigned(mac5)                   << std::endl;
  std::cout << " run_start_time      "    <<  run_start_time              << std::endl;
  std::cout << " this_poll_start     "    << this_poll_start            << std::endl;
  std::cout << " this_poll_end        "   << this_poll_end              << std::endl;
  std::cout << "  last_poll_start      "   << last_poll_start            << std::endl;
  std::cout << "  last_poll_end          " << last_poll_end              << std::endl;
  std::cout << "  system_clock_deviation " << system_clock_deviation     << std::endl;
  std::cout << "  feb_hits_in_poll       " << feb_hits_in_poll           << std::endl;
  std::cout << "  feb_hits_in_fragment  "  << feb_hits_in_fragment       << std::endl;
  }

  for(unsigned int iHit = 0; iHit < feb_hits_in_fragment; iHit++) {

    BernCRTHitV2 const* bevt = bern_fragment.eventdata(iHit);

    //  TLOG(TLVL_INFO)<<*bevt;

    flags                   = bevt->flags;
    lostcpu                 = bevt->lostcpu;
    lostfpga                = bevt->lostfpga;
    ts0                     = bevt->ts0;
    ts1                     = bevt->ts1;
    coinc                   = bevt->coinc;
    
    feb_hit_number          = bevt->feb_hit_number;
    timestamp               = bevt->timestamp;
    last_accepted_timestamp = bevt->last_accepted_timestamp;
    lost_hits               = bevt->lost_hits;

    if (0) {
      std::cout << "  flags  "                << flags                   << std::endl;
      std::cout << "lostcpu         "        << lostcpu                 << std::endl;
      std::cout <<     "lostfpga    "<<     lostfpga     << std::endl;
      std::cout <<     "ts0         "<<     ts0          << std::endl;
      std::cout <<     "ts1                    "<<     ts1             << std::endl;        
      std::cout <<     "coinc                  "<<     coinc                   << std::endl;
      std::cout <<     "feb_hit_number         "<<     feb_hit_number          << std::endl;
      std::cout <<     "timestamp              "<<     timestamp               << std::endl;
      std::cout <<     "last_accepted_timestamp"<<     last_accepted_timestamp << std::endl;
      std::cout <<     "lost_hits              "<<     lost_hits               << std::endl;
    }

    // for(int ch=0; ch<32; ch++) adc[ch] = bevt->adc[ch];
    for(int ch=0; ch<32; ch++) {
      if (0) std::cout << "channel " << ch << " has adc value " << bevt->adc[ch] << std::endl;
      adc[ch] = bevt->adc[ch];
    }

    hits->Fill();
  }
}


void sbndaq::BernCRTAnaV2::analyze(art::Event const & evt)
{
  art::EventNumber_t eventNumber = evt.event();
//  TLOG(TLVL_INFO)<<" Processing event "<<eventNumber;

  std::vector<art::Handle<artdaq::Fragments>> fragmentHandles;

        fragmentHandles = evt.getMany<std::vector<artdaq::Fragment>>();

	for (auto handle : fragmentHandles) {
    if (!handle.isValid() || handle->size() == 0)
      continue;
    
    if (handle->front().type() == artdaq::Fragment::ContainerFragmentType) {
      //Container fragment
      for (auto cont : *handle) {
        artdaq::ContainerFragment contf(cont);
        if (contf.fragment_type() != sbndaq::detail::FragmentType::BERNCRTV2)
          continue;
        for (size_t ii = 0; ii < contf.block_count(); ++ii)
          analyze_fragment(*contf[ii].get());
      }
    }
    else {
      //normal fragment
      if (handle->front().type() != sbndaq::detail::FragmentType::BERNCRTV2) continue;
      for (auto frag : *handle) 
        analyze_fragment(frag);
    }
  }
} //analyze

DEFINE_ART_MODULE(sbndaq::BernCRTAnaV2)
//this is where the name is specified
