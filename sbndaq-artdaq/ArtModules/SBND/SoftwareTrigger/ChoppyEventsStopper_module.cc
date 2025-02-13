////////////////////////////////////////////////////////////////////////
// Class:       ChoppyEventsStopper
// Plugin Type: analyzer (Unknown Unknown)
// File:        ChoppyEventsStopper_module.cc
//
// Generated at Mon Aug 19 14:44:50 2024 by Sbnd (Erin Yandel) using cetskelgen
// from cetlib version 3.18.02.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "sbndaq-artdaq-core/Overlays/Common/BernCRTFragmentV2.hh"
#include "sbndaq-artdaq-core/Overlays/Common/CAENV1730Fragment.hh"
#include "sbndaq-artdaq-core/Overlays/Common/DummyFragment.hh"
#include "sbndaq-artdaq-core/Overlays/SBND/NevisTBFragment.hh"
#include "sbndaq-artdaq-core/Overlays/SBND/NevisTPCFragment.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "artdaq-core/Data/Fragment.hh"
#include "artdaq-core/Data/ContainerFragment.hh"
#include "artdaq/DAQdata/Globals.hh"

#include <memory>

using artdaq::MetricMode;

class ChoppyEventsStopper;


class ChoppyEventsStopper : public art::EDAnalyzer {
public:
  explicit ChoppyEventsStopper(fhicl::ParameterSet const& p);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  ChoppyEventsStopper(ChoppyEventsStopper const&) = delete;
  ChoppyEventsStopper(ChoppyEventsStopper&&) = delete;
  ChoppyEventsStopper& operator=(ChoppyEventsStopper const&) = delete;
  ChoppyEventsStopper& operator=(ChoppyEventsStopper&&) = delete;

  // Required functions.
  void analyze(art::Event const& e) override;

private:

  // Declare member data here.
  bool fVerbose; //turn on/off print statements

};


ChoppyEventsStopper::ChoppyEventsStopper(fhicl::ParameterSet const& p)
  : EDAnalyzer{p},
  fVerbose(p.get<bool>("Verbose", true))
{
  
}

void ChoppyEventsStopper::analyze(art::Event const& e)
{
  int fRun = e.run();
  art::EventNumber_t fEvent = e.event();

  if (fVerbose) {std::cout << "Run: " << fRun << ", Event: " << fEvent << std::endl;}

  //Throw an exception
  if (fEvent == 100) {
    TLOG(TLVL_ERROR) << "Event " << fEvent << ", throwing an exception.";
    throw cet::exception("ChoppyEventsStopper") << "Event " << fEvent << ", throwing an exception." << std::endl;
  }
}

DEFINE_ART_MODULE(ChoppyEventsStopper)
