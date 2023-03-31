//-------------------------------------------------
//---------------------------------------

////////////////////////////////////////////////////////////////////////
// Class:       IncompleteEventAna
// Module Type: analyzer
// File:        IncompleteEventAna_module.cc
// Description: Generates information on IDs of missing events
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"

#include "canvas/Utilities/Exception.h"

#include "artdaq-core/Data/Fragment.hh"

#include <vector>
#include <iostream>
#include <set>
#include <iterator>
#include <algorithm>


#include "sbndaq-artdaq-core/Trace/trace_defines.h"
#include "TRACE/tracemf3.h"

namespace sbndaq {
  class IncompleteEventAna;
}

/**************************************************************************************************/

class sbndaq::IncompleteEventAna : public art::EDAnalyzer {

public:
  
  struct Config {
    /*
    //--one atom for each parameter
    fhicl::Atom< std::vector<art::InputTag> > DataLabel {
      fhicl::Name("data_label"),
      fhicl::Comment("Tag for the input data product")
    };
    fhicl::Atom<int> Shift {
      fhicl::Name("shift_fragment_id"), 
      fhicl::Comment("Number to subtract to the fragment_id")
    };
    */
  }; //--configuration
  
  using Parameters = art::EDAnalyzer::Table<Config>;

  explicit IncompleteEventAna(Parameters const & pset);
  virtual ~IncompleteEventAna();

  void analyze(const art::Event& evt) override;
  void beginJob() override;
  void endJob() override;

private:

  std::set<artdaq::Fragment::fragment_id_t> fFragIDSet;

  std::set<artdaq::Fragment::fragment_id_t> fThisEvFragIDSet;

}; //--class IncompleteEventAna


sbndaq::IncompleteEventAna::IncompleteEventAna(IncompleteEventAna::Parameters const& pset): art::EDAnalyzer(pset)
{
  fFragIDSet.clear();
}

void sbndaq::IncompleteEventAna::beginJob()
{
}

void sbndaq::IncompleteEventAna::endJob()
{
}


sbndaq::IncompleteEventAna::~IncompleteEventAna()
{
}


void sbndaq::IncompleteEventAna::analyze(const art::Event& evt)
{

  auto run = evt.run();
  auto event = evt.event();

  fThisEvFragIDSet.clear();

  std::vector< art::Handle< std::vector<artdaq::Fragment> > > rawFragHandles;

#if ART_HEX_VERSION < 0x30900
        evt.getManyByType(rawFragHandles);
#else
        rawFragHandles = evt.getMany<std::vector<artdaq::Fragment>>();
#endif

  for(auto const& rawFragHandle : rawFragHandles){
    if(!rawFragHandle.isValid()) continue;

    auto const& rawFragVector(*rawFragHandle);
  
    for(auto const& frag : rawFragVector)
      fThisEvFragIDSet.insert(frag.fragmentID());

  }

  if(fThisEvFragIDSet.size()>fFragIDSet.size()){

    TLOG_INFO("IncompleteEventAna") 
      << "(Run,Ev)=(" << run << "," << event << "): "
      << "Updating reference FragIDSet from size "
      << fFragIDSet.size() << " to size "
      << fThisEvFragIDSet.size();

    fFragIDSet=fThisEvFragIDSet;
  }
  
  if(fFragIDSet.size()!=fThisEvFragIDSet.size()){

    TLOG_WARNING("IncompleteEventAna") 
      << "(Run,Ev)=(" << run << "," << event << "): "
      << "Incomplete Event. "
      << fThisEvFragIDSet.size()
      << " / "
      << fFragIDSet.size() << " fragments detected.";

    std::set<artdaq::Fragment::fragment_id_t> fragIDDiffSet;
    
    std::set_difference(fFragIDSet.begin(),fFragIDSet.end(),
			fThisEvFragIDSet.begin(),fThisEvFragIDSet.end(),
			std::inserter(fragIDDiffSet,fragIDDiffSet.begin()));
    
    /*
    //print message here on the missing fragments
    //maybe do a lookup table?
    */

    for(auto id : fragIDDiffSet)
      TLOG_WARNING("IncompleteEventAna") 
      << "(Run,Ev)=(" << run << "," << event << "): "
      << "Missing fragment id " << id;
    
  }

}

DEFINE_ART_MODULE(sbndaq::IncompleteEventAna)

