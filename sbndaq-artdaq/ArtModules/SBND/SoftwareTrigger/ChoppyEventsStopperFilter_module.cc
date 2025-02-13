////////////////////////////////////////////////////////////////////////
// Class:       ChoppyEventsStopperFilter
// Plugin Type: filter (Unknown Unknown)
// File:        ChoppyEventsStopperFilter_module.cc
//
// Generated at Mon Aug 19 15:47:47 2024 by Sbnd using cetskelgen
// from cetlib version 3.18.02.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDFilter.h"
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

class ChoppyEventsStopperFilter;


class ChoppyEventsStopperFilter : public art::EDFilter {
public:
  explicit ChoppyEventsStopperFilter(fhicl::ParameterSet const& p);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  ChoppyEventsStopperFilter(ChoppyEventsStopperFilter const&) = delete;
  ChoppyEventsStopperFilter(ChoppyEventsStopperFilter&&) = delete;
  ChoppyEventsStopperFilter& operator=(ChoppyEventsStopperFilter const&) = delete;
  ChoppyEventsStopperFilter& operator=(ChoppyEventsStopperFilter&&) = delete;

  // Required functions.
  bool filter(art::Event& e) override;

private:

  // Declare member data here.
  bool fVerbose; //turn on/off print statements

};


ChoppyEventsStopperFilter::ChoppyEventsStopperFilter(fhicl::ParameterSet const& p)
  : EDFilter{p},
  fVerbose(p.get<bool>("Verbose", true))
{
  // Call appropriate produces<>() functions here.
  // Call appropriate consumes<>() for any products to be retrieved by this module.
}

bool ChoppyEventsStopperFilter::filter(art::Event& e)
{
  int fRun = e.run();
  art::EventNumber_t fEvent = e.event();

  if (fVerbose) {std::cout << "Run: " << fRun << ", Event: " << fEvent << std::endl;}

  //Throw an exception
  if (fEvent > 99) {
    TLOG(TLVL_ERROR) << "Event " << fEvent << ", throwing an exception.";
    //throw cet::exception("ChoppyEventsStopper") << "Event " << fEvent << ", throwing an exception." << std::endl;
    return false;
  }

  return true;
}

DEFINE_ART_MODULE(ChoppyEventsStopperFilter)
