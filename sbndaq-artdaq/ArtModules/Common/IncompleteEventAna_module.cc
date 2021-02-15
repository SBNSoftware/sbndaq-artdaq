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
  bool fFirstEvt;

  std::set<artdaq::Fragment::fragment_id_t> fThisEvFragIDSet;

}; //--class IncompleteEventAna


sbndaq::IncompleteEventAna::IncompleteEventAna(IncompleteEventAna::Parameters const& pset): art::EDAnalyzer(pset)
{
  fFragIDSet.clear();
  fFirstEvt = true;
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

  fThisEvFragIDSet.clear();

  std::vector< art::Handle< std::vector<artdaq::Fragment> > > rawFragHandles;

  evt.getManyByType(rawFragHandles);

  for(auto const& rawFragHandle : rawFragHandles){
    if(!rawFragHandle.isValid()) continue;

    auto const& rawFragVector(*rawFragHandle);
  
    for(auto const& frag : rawFragVector)
      fThisEvFragIDSet.insert(frag.fragmentID());

  }

  if(fFirstEvt){
    fFragIDSet = fThisEvFragIDSet;
    fFirstEvt = false;
  }

  if(fThisEvFragIDSet.size()>fFragIDSet.size()){

    /*
    //print a message here to say we are updating?
    */

    fFragIDSet=fThisEvFragIDSet;
  }
  
  if(fFragIDSet.size()!=fThisEvFragIDSet.size()){

    std::set<artdaq::Fragment::fragment_id_t> fragIDDiffSet;
    
    std::set_difference(fFragIDSet.begin(),fFragIDSet.end(),
			fThisEvFragIDSet.begin(),fThisEvFragIDSet.end(),
			std::inserter(fragIDDiffSet,fragIDDiffSet.begin()));
    
    /*
    //print message here on the missing fragments
    //maybe do a lookup table?
    */

    for(auto id : fragIDDiffSet){
      std::cout << "\t\t\tMissing fragment id " << id << std::endl;
    }
  }

}

DEFINE_ART_MODULE(sbndaq::IncompleteEventAna)

