////////////////////////////////////////////////////////////////////////
// Class:       MetricProducer
// Plugin Type: producer (Unknown Unknown)
// File:        MetricProducer_module.cc
//
// Michelle Stancari
// August 2022
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

#include "sbndaq-artdaq-core/Overlays/Common/BernCRTFragmentV2.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "artdaq-core/Data/Fragment.hh"
#include "artdaq-core/Data/ContainerFragment.hh"
#include "artdaq/DAQdata/Globals.hh"

#include "sbndaq-artdaq-core/Obj/SBND/CRTmetric.hh"

#include <memory>
#include <iostream>

using artdaq::MetricMode;

namespace sbndaq {
  class MetricProducer;
}

class sbndaq::MetricProducer : public art::EDProducer {
public:
  explicit MetricProducer(fhicl::ParameterSet const& p);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  MetricProducer(MetricProducer const&) = delete;
  MetricProducer(MetricProducer&&) = delete;
  MetricProducer& operator=(MetricProducer const&) = delete;
  MetricProducer& operator=(MetricProducer&&) = delete;

  // Required functions.
  void produce(art::Event& evt) override;

private:

  // fhicl parameters
  art::Persistable is_persistable_;
  int fBeamWindowStart;
  int fBeamWindowEnd;
  bool fVerbose;

  // event information
  int fRun;
  art::EventNumber_t fEvent;

  //metric variables
  int hitsperplane[7];
  std::vector<uint32_t> ts1s;

  //graphana metric
  int num_t1_resets;


  void analyze_crt_fragment(artdaq::Fragment & frag);

};


sbndaq::MetricProducer::MetricProducer(fhicl::ParameterSet const& p)
  : EDProducer{p},
  is_persistable_(p.get<bool>("is_persistable", true) ? art::Persistable::Yes : art::Persistable::No),
  fBeamWindowStart(p.get<int>("BeamWindowStart",320000)),
  fBeamWindowEnd(p.get<int>("BeamWindowEnd",350000)),
  fVerbose(p.get<bool>("Verbose",false))
  {
  // Call appropriate produces<>() functions here.
  produces< sbndaq::CRTmetric >("", is_persistable_);

  // Call appropriate consumes<>() for any products to be retrieved by this module.
}


void sbndaq::MetricProducer::produce(art::Event& evt)
{

  // load event information
  int fRun = evt.run();
  art::EventNumber_t fEvent = evt.event();

  if (fVerbose) std::cout << "Processing: Run " << fRun << ", Event " << fEvent << std::endl;

  // object to store required trigger information in
  std::unique_ptr<sbndaq::CRTmetric> CRTMetricInfo = std::make_unique<sbndaq::CRTmetric>();

  // clear variables at the beginning of the event
  // move this to constructor??
  for (int ip=0;ip<7;++ip)  { CRTMetricInfo->hitsperplane[ip]=0; hitsperplane[ip]=0;}

  // get fragment handles
  std::vector<art::Handle<artdaq::Fragments>> fragmentHandles = evt.getMany<std::vector<artdaq::Fragment>>();

  num_t1_resets = 0;

  // loop over fragment handles
  for (auto handle : fragmentHandles) {
    if (!handle.isValid() || handle->size() == 0) continue;

    if (handle->front().type() == artdaq::Fragment::ContainerFragmentType) {
      // container fragment
      for (auto cont : *handle) {
        artdaq::ContainerFragment contf(cont);
        if (contf.fragment_type() != sbndaq::detail::FragmentType::BERNCRTV2) continue;
	if (fVerbose)     std::cout << "    Found " << contf.block_count() << " CRT Fragments in container " << std::endl;
	for (size_t ii = 0; ii < contf.block_count(); ++ii) analyze_crt_fragment(*contf[ii].get());
      }
    }
    else {
      // normal fragment
      if (handle->front().type()!=sbndaq::detail::FragmentType::BERNCRTV2) continue;
      if (fVerbose)   std::cout << "   found CRT fragments " << handle->size() << std::endl;
      for (auto frag : *handle)	analyze_crt_fragment(frag);
    }
  } // loop over frag handles

  if(metricMan != nullptr) {
      //send flag metrics
      metricMan->sendMetric(
          "T1_resets_per_event",
          num_t1_resets,
          "CRT T1 resets per event", 11, artdaq::MetricMode::LastPoint);

      for (int i=0;i<7;++i){
        metricMan->sendMetric(
            std::string("CRT_hits_beam_plane_")+std::to_string(i),
            hitsperplane[i],
            "CRT hits in beam window per plane per event", 11, artdaq::MetricMode::LastPoint);
      }

      if (fVerbose) {
        TLOG(TLVL_INFO) << "T1 resets per event: "<< num_t1_resets;
      }

  }



  for (int i=0;i<7;++i) {CRTMetricInfo->hitsperplane[i] = hitsperplane[i];}

  if (fVerbose) {
    std::cout << "CRT hit count during beam spill ";
    for (int i=0;i<7;++i) std::cout << i << " " << CRTMetricInfo->hitsperplane[i] ;
    std::cout << std::endl;

    TLOG(TLVL_INFO) << "CRT hit count during beam spill "
      << "  " << 0 << ": " << CRTMetricInfo->hitsperplane[0]
      << "  " << 1 << ": " << CRTMetricInfo->hitsperplane[1]
      << "  " << 2 << ": " << CRTMetricInfo->hitsperplane[2]
      << "  " << 3 << ": " << CRTMetricInfo->hitsperplane[3]
      << "  " << 4 << ": " << CRTMetricInfo->hitsperplane[4]
      << "  " << 5 << ": " << CRTMetricInfo->hitsperplane[5]
      << "  " << 6 << ": " << CRTMetricInfo->hitsperplane[6];
  }

  // add to event
  evt.put(std::move(CRTMetricInfo));

  //clear
  ts1s.clear();
  ts1s.shrink_to_fit();

}



void sbndaq::MetricProducer::analyze_crt_fragment(artdaq::Fragment & frag)
{


  sbndaq::BernCRTFragmentV2 bern_fragment(frag);

  // use  fragment ID to get plane information
  const sbndaq::BernCRTFragmentMetadataV2* md = bern_fragment.metadata();
  //frag.sequenceID()
  auto thisone = frag.fragmentID();  uint plane = (thisone & 0x0700) >> 8;
  if (plane>7) {std::cout << "bad plane value " << plane << std::endl; plane=0;}

  for(unsigned int iHit = 0; iHit < md->hits_in_fragment(); iHit++) {
    sbndaq::BernCRTHitV2 const* bevt = bern_fragment.eventdata(iHit);
    // require that this is data and not clock reset (0xC), and that the ts1 time is valid (0x2)
    auto thisflag = bevt->flags;
    if (thisflag & 0b1000){num_t1_resets++;}
    if (thisflag & 0x2 && !(thisflag & 0xC) ) {
      // check ts1 for beam window
      auto thistime=bevt->ts1;
      if ((int)thistime>fBeamWindowStart && (int)thistime<fBeamWindowEnd) {
	hitsperplane[plane]++;
	ts1s.push_back(thistime);
	if(metricMan != nullptr) {
	  //send t1 to grafana
	  metricMan->sendMetric(
				"CRT_T1_in_beam",
				(double)thistime,
				"CRT T1 times for hits in beam", 12, artdaq::MetricMode::LastPoint);
	}
      }
      //CRTMetricInfo->hitsperplane[plane]++;
    }
  }



}

// -------------------------------------------------

// -------------------------------------------------

DEFINE_ART_MODULE(sbndaq::MetricProducer)
