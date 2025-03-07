////////////////////////////////////////////////////////////////////////
// Class:       TPCMetricAnalyzer
// Plugin Type: analyzer
// File:        TPCMetricAnalyzer_module.cc
//
// Nupur Oza & Daisy Kalra
// February 2025
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
#include "artdaq-core/Data/Fragment.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "artdaq-core/Data/ContainerFragment.hh"
#include "artdaq/DAQdata/Globals.hh"
#include "art_root_io/TFileService.h"
#include "art_root_io/TFileDirectory.h"


#include "sbndaq-artdaq-core/Overlays/SBND/NevisTPCFragment.hh"
#include "sbndaq-artdaq-core/Overlays/SBND/NevisTBFragment.hh"
#include "sbndaq-artdaq-core/Overlays/SBND/NevisTB_dataFormat.hh"
#include "sbndaq-artdaq-core/Overlays/SBND/NevisTPC/NevisTPCTypes.hh"
#include "sbndaq-artdaq-core/Overlays/SBND/NevisTPC/NevisTPCUtilities.hh"

#include "sbndaq-artdaq-core/Obj/SBND/TPCmetric.hh"

namespace sbndaq {
  class TPCMetricAnalyzer;
}

class sbndaq::TPCMetricAnalyzer : public art::EDAnalyzer {
public:
  explicit TPCMetricAnalyzer(fhicl::ParameterSet const& p);
  virtual ~TPCMetricAnalyzer();

  void analyze(const art::Event& evt) override;
  void beginJob() override;
  void endJob() override;

  /*  // Plugins should not be copied or assigned.
  TPCMetricAnalyzer(TPCMetricAnalyzer const&) = delete;
  TPCMetricAnalyzer(TPCMetricAnalyzer&&) = delete;
  TPCMetricAnalyzer& operator=(TPCMetricAnalyzer const&) = delete;
  TPCMetricAnalyzer& operator=(TPCMetricAnalyzer&&) = delete;

  // Required functions.
  void analyze(art::Event& evt) override;
  */

private:
  void readNTB(const artdaq::Fragment& frag);
  void readTPC(const artdaq::Fragment& frag);
  std::string fTagTPC;
  std::string fTagNTB;
  bool fVerbose;

  // NTB Data
  int ntb_eventno;
  int ntb_frameno;
  int ntb_sampleno;
  
  // TPC Data
  int tpc_eventno;
  int tpc_packframeno;
  int tpc_trigframeno;
  int tpc_slot;
  int tpc_crate;
  int tpc_sampleno;

  // Previous values for consistency checks
  int prev_eventno;
  int prev_frameno;
};


sbndaq::TPCMetricAnalyzer::TPCMetricAnalyzer(fhicl::ParameterSet const& p)
  : art::EDAnalyzer{p}

//  _tag(param.get<std::string>("raw_data_label", "daq"),param.get<std::string>("fragment_type_label", "NEVISTPC")),
// _tag1(param.get<std::string>("raw_data_label", "daq"),param.get<std::string>("fragment_type_label", "NEVISTPC")),
{
  fTagTPC = p.get<std::string>("tpc_data_label","daq:NEVISTPC");
  fTagNTB = p.get<std::string>("ntb_data_label","daq:NEVISTB");
  fVerbose = (p.get<bool>("Verbose", false));
    //  consumes<artdaq::Fragments>(fTagTPC);
    //consumes<artdaq::Fragments>(fTagNTB);
  //produces< sbndaq::TPCmetric >("");

  //consumes<artdaq::Fragments>(fContainerTag);
   }
 /*  ntb_eventno(0),
  ntb_frameno(0),
  ntb_sampleno(0),
  tpc_eventno(0),
  tpc_packframeno(0),
  tpc_trigframeno(0),
  tpc_slot(0),
  tpc_crate(0),
  tpc_sampleno(0),
  prev_eventno(0),
  prev_frameno(0)
  */

  //{}

void sbndaq::TPCMetricAnalyzer::beginJob()
{
  //  art::ServiceHandle<art::TFileService> tfs;
  std::cout << "Beginning EventAna...\n";

}

void sbndaq::TPCMetricAnalyzer::endJob()
{
   std::cout << "Ending EventAna...\n";
}

sbndaq::TPCMetricAnalyzer::~TPCMetricAnalyzer()
{
}



void sbndaq::TPCMetricAnalyzer::analyze(const art::Event& evt)
{

  auto daq_handleTPC = evt.getHandle<artdaq::Fragments>(fTagTPC);
  auto daq_handleNTB = evt.getHandle<artdaq::Fragments>(fTagNTB);
  std::cout << "here"<< std::endl;

  if ( daq_handleTPC.isValid() and daq_handleNTB.isValid()  ) {
    std::cout << "********SIZE: "<< daq_handleTPC->size() << std::endl;
    for (auto const &rawfragtpc: *daq_handleTPC) {
      readTPC(rawfragtpc);
    } 
    std::cout << "********SIZE: "<< daq_handleNTB->size() << std::endl;
    for (auto const &rawfragntb: *daq_handleNTB) {
      readNTB(rawfragntb);
    }

 }
  //    for (auto const &rawfrag: *daq_handle) {
  else{
    std::cout << "Not valid handle" << std::endl;
  }


  
  /*
  std::vector<art::Handle<artdaq::Fragments>> fragmentHandles = evt.getMany<std::vector<artdaq::Fragment>>();
  
  // loop over fragments
  //  for (auto &cont : *handle)
  for (auto handle : fragmentHandles) {
    if (!handle.isValid() || handle->size() == 0) continue;
    
    for (const auto& frag : *handle) {
      if (frag.type() == artdaq::Fragment::ContainerFragmentType) {
        // Handle container fragment
        artdaq::ContainerFragment contf(frag);
        if (contf.fragment_type() == sbndaq::detail::FragmentType::NEVISTB) {
          for (size_t ii = 0; ii < contf.block_count(); ++ii) {
            readNTB(*contf[ii].get());
          }
        }
        else if (contf.fragment_type() == sbndaq::detail::FragmentType::NEVISTPC) {
          for (size_t ii = 0; ii < contf.block_count(); ++ii) {
            readTPC(*contf[ii].get());
          }
      } else {
        // Handle individual fragments
        if (frag.type() == sbndaq::detail::FragmentType::NEVISTB) {
          readNTB(frag);
        } else if (frag.type() == sbndaq::detail::FragmentType::NEVISTPC) {
          readTPC(frag);
          }
        }
      }
    }
    } */
  
/*
  // === Perform Calculations ===
  // Check 1: Is NTB trigger number increasing by 1?
  int d_eventno = ntb_eventno - prev_eventno;
  prev_eventno = ntb_eventno;
  
  // Check 2: Is NTB frame number increasing?
  int d_frameno = ntb_frameno - prev_frameno;
  prev_frameno = ntb_frameno;
  
  // Check 3: Is TPC packet frame number - TPC trigger frame number = +1 or 2?
  int frame_diff = tpc_packframeno - tpc_trigframeno;
  
  // Check 4: Is TPC FEM absolute sample number expected with respect to NTB frame number?
  */

    
};



void sbndaq::TPCMetricAnalyzer::readNTB(const artdaq::Fragment &frag)
{
  sbndaq::NevisTBFragment tbfrag(frag);
  const auto ntbheader = *tbfrag.header();
  //ntbheader is an object, and threfore using . operator 
  ntb_eventno = ntbheader.getTriggerNumber();
  ntb_frameno = ntbheader.getFrame();
  ntb_sampleno = ntbheader.sample;
  std::cout << "NTB information: Event, Frame, Sample: " << ntb_eventno << " , " << ntb_frameno << " , " << ntb_sampleno << std::endl; 

};

void sbndaq::TPCMetricAnalyzer::readTPC(const artdaq::Fragment &frag)
{
  sbndaq::NevisTPCFragment tpcfrag(frag);
  
  const auto tpcheader = *tpcfrag.header();
  //tpcheader is an object, and threfore using . operator                                                                                                                                                                                                                 
  //   tpc_crate = (tpcfrag.fragmentID() >> 8) & 0xF; // need to fix this
  tpc_slot = tpcheader.getSlot();
  tpc_eventno = tpcheader.getEventNum();
  tpc_packframeno = tpcheader.getFrameNum();
  tpc_sampleno = tpcheader.get2mhzSample();
  tpc_trigframeno = tpcheader.getTrigFrame();
  std::cout << "TPC information: Event , PacketFrame , Sample , TrigFrame:  "  << tpc_eventno << " , " << tpc_packframeno << " , " << tpc_sampleno << " , " << tpc_trigframeno << std::endl;

};


DEFINE_ART_MODULE(sbndaq::TPCMetricAnalyzer)
