////////////////////////////////////////////////////////////////////////
// Class:       pmtTriggerProducer
// Plugin Type: producer (Unknown Unknown)
// File:        pmtTriggerProducer_module.cc
//
// Generated at Thu Feb 17 11:18:06 2022 by Patrick Green using cetskelgen
// from  version .
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "sbndaq-artdaq-core/Overlays/Common/CAENV1730Fragment.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "artdaq-core/Data/Fragment.hh"
#include "artdaq-core/Data/ContainerFragment.hh"

#include "sbndaq-artdaq-core/Obj/SBND/pmtTrigger.h"

#include <memory>
#include <iostream>

namespace sbndaq {
  class pmtTriggerProducer;
}


class sbndaq::pmtTriggerProducer : public art::EDProducer {
public:
  explicit pmtTriggerProducer(fhicl::ParameterSet const& p);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  pmtTriggerProducer(pmtTriggerProducer const&) = delete;
  pmtTriggerProducer(pmtTriggerProducer&&) = delete;
  pmtTriggerProducer& operator=(pmtTriggerProducer const&) = delete;
  pmtTriggerProducer& operator=(pmtTriggerProducer&&) = delete;

  // Required functions.
  void produce(art::Event& e) override;

private:

  // Declare member data here.

  // fhicl parameters
  bool fVerbose;

  // event information
  int fRun, fSubrun;
  art::EventNumber_t fEvent;

};


sbndaq::pmtTriggerProducer::pmtTriggerProducer(fhicl::ParameterSet const& p)
  : EDProducer{p},
  fVerbose(p.get<bool>("Verbose", false))
  // More initializers here.
{
  // Call appropriate produces<>() functions here.
  // Call appropriate consumes<>() for any products to be retrieved by this module.
}

void sbndaq::pmtTriggerProducer::produce(art::Event& e)
{
  // Implementation of required member function here.

  // event information
  fRun = e.run();
  fSubrun = e.subRun();
  fEvent = e.id().event();

  if (fVerbose) std::cout << "Processing Run: " << fRun << ", Subrun: " << fSubrun << ", Event: " << fEvent << std::endl;

}

DEFINE_ART_MODULE(sbndaq::pmtTriggerProducer)
