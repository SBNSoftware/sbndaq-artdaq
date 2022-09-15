////////////////////////////////////////////////////////////////////////
// Class:       NeutrinoStreamFilter
// Plugin Type: filter (Unknown Unknown)
// File:        NeutrinoStreamFilter_module.cc
//
// Generated at Tue Mar 29 13:45:55 2022 by Erin Yandel using cetskelgen
// from  version .
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "sbndaq-artdaq-core/Overlays/Common/BernCRTFragmentV2.hh"
#include "sbndaq-artdaq-core/Overlays/Common/CAENV1730Fragment.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "artdaq-core/Data/Fragment.hh"
#include "artdaq-core/Data/ContainerFragment.hh"
#include "artdaq/DAQdata/Globals.hh"

//#include "sbndaq-artdaq-core/Obj/SBND/pmtSoftwareTrigger.hh"
#include "sbndaq-artdaq-core/Obj/SBND/CRTmetric.hh"
//#include "sbnobj/SBND/Trigger/CRTmetric.hh"

#include <memory>

using artdaq::MetricMode;

class NeutrinoStreamFilter;


class NeutrinoStreamFilter : public art::EDFilter {
public:
  explicit NeutrinoStreamFilter(fhicl::ParameterSet const& pset);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  NeutrinoStreamFilter(NeutrinoStreamFilter const&) = delete;
  NeutrinoStreamFilter(NeutrinoStreamFilter&&) = delete;
  NeutrinoStreamFilter& operator=(NeutrinoStreamFilter const&) = delete;
  NeutrinoStreamFilter& operator=(NeutrinoStreamFilter&&) = delete;

  // Required functions.
  bool filter(art::Event& e) override;

private:

  // Declare member data here.
  // fhicl paramters
  std::string fCRTInputModuleName; //CRT Metrics Module Name
  std::string fPMTInputModuleName; //PMT Metrics Module Name
  int fNumCRTPlanes; //Total number of CRT planes expected (should be 7)
  std::vector<int> fExcludeCRTPlanes; //which CRT planes to allow to have hits in the beam window (window is defined by CRT metrics producer)
  int fNPMTHits;
  bool fVerbose;

};


NeutrinoStreamFilter::NeutrinoStreamFilter(fhicl::ParameterSet const& pset)
  : EDFilter{pset},
  fCRTInputModuleName(pset.get<std::string>("CRTInputModule", "crttriggerproducer")),
  fPMTInputModuleName(pset.get<std::string>("PMTInputModule", "pmttriggerproducer")),
  fNumCRTPlanes(pset.get<int>("NumCRTPlanes", 7)),
  fExcludeCRTPlanes(pset.get<std::vector<int>>("ExcludeCRTPlanes", {2})),
  fNPMTHits(pset.get<int>("NPMTHits", 0)),
  fVerbose(pset.get<bool>("Verbose", true))
{
  // Call appropriate produces<>() functions here.
  // Call appropriate consumes<>() for any products to be retrieved by this module.
}

bool NeutrinoStreamFilter::filter(art::Event& e)
{
  int fRun = e.run();
  art::EventNumber_t fEvent = e.event();

  if (fVerbose) {std::cout << "Run: " << fRun << ", Event: " << fEvent << std::endl;}

  // read crt metric information from event
  art::Handle< sbndaq::CRTmetric > crtHandle;
  e.getByLabel(fCRTInputModuleName, crtHandle);
  if(!crtHandle.isValid()) {
    TLOG(TLVL_ERROR) << "NeutrinoStreamFilter: could not find crt information from producer, check CRT input module name specified is correct.";
    return false;
  }

  if(sizeof(crtHandle->hitsperplane)/sizeof(crtHandle->hitsperplane[0]) != fNumCRTPlanes){
    TLOG(TLVL_ERROR) << "NeutrinoStreamFilter: Number of CRT planes does not match! Check NumCRTPlanes in fcl.";
    return false;
  }

  std::vector<int> hitsperplane;
  for (int ip=0;ip<fNumCRTPlanes;++ip)  { hitsperplane.emplace_back(-1); }
  for (int ip=0;ip<fNumCRTPlanes;++ip)  { hitsperplane[ip] = crtHandle->hitsperplane[ip]; }

  for (int ip=0;ip<fNumCRTPlanes;++ip)  {
    if (std::find(fExcludeCRTPlanes.begin(), fExcludeCRTPlanes.end(), ip) != fExcludeCRTPlanes.end()) continue;

    if (hitsperplane[ip] !=0){ return false; }
  }

  if (fVerbose) {
    TLOG(TLVL_INFO) << "NeutrinoStreamFilter: Selected Event "<< fEvent;
  }

  if(metricMan != nullptr) {
      //send flag metrics
      metricMan->sendMetric(
          "Neutrino_Stream_Rate", 1,
          "events/s", 5, artdaq::MetricMode::Rate);
  }

  return true;


}

DEFINE_ART_MODULE(NeutrinoStreamFilter)
