////////////////////////////////////////////////////////////////////////
// Class:       TPCMetricAnalyzer
// Plugin Type: analyzer
// File:        TPCMetricAnalyzer_module.cc
//
// Nupur Oza & Daisy Kalra
// February 2025
////////////////////////////////////////////////////////////////////////

#define TRACE_NAME "TPCMetric"

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

using artdaq::MetricMode;

namespace sbndaq {
  class TPCMetricAnalyzer;
}

class sbndaq::TPCMetricAnalyzer : public art::EDAnalyzer {
public:
  explicit TPCMetricAnalyzer(fhicl::ParameterSet const& p);
  virtual ~TPCMetricAnalyzer();

  void analyze(const art::Event& evt) override;

private:
  void readNTB(const artdaq::Fragment& frag);
  void readTPC(const artdaq::Fragment& frag);
  std::string fTagTPC;
  std::string fTagNTB;

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

  int N_Bad_DFEMFrame = 0;
  int N_Bad_DNTBFEMSample = 0;

};

sbndaq::TPCMetricAnalyzer::TPCMetricAnalyzer(fhicl::ParameterSet const& p)
  : art::EDAnalyzer{p}

{
  fTagTPC = p.get<std::string>("tpc_data_label","daq:NEVISTPC");
  fTagNTB = p.get<std::string>("ntb_data_label","daq:NEVISTB");
}


sbndaq::TPCMetricAnalyzer::~TPCMetricAnalyzer()
{
}

void sbndaq::TPCMetricAnalyzer::analyze(const art::Event& evt)
{

  auto daq_handleTPC = evt.getHandle<artdaq::Fragments>(fTagTPC);
  auto daq_handleNTB = evt.getHandle<artdaq::Fragments>(fTagNTB);


  if ( daq_handleTPC.isValid() and daq_handleNTB.isValid()  ) {
    //TLOG(TLVL_DEBUG+1)     << "********NTB fragment SIZE: "<< daq_handleNTB->size();
    //std::cout << "********SIZE: "<< daq_handleNTB->size() << std::endl;
    for (auto const &rawfragntb: *daq_handleNTB) {
      readNTB(rawfragntb);
    }
    
    //std::cout << "********SIZE: "<< daq_handleTPC->size() << std::endl;
    //TLOG(TLVL_DEBUG+1)     << "********TPC fragment SIZE: "<< daq_handleTPC->size();
    for (auto const &rawfragtpc: *daq_handleTPC) {
      readTPC(rawfragtpc);

      // Check 3: Is TPC trigger frame number - TPC packet frame number = +1 ?        
      int D_FEMFrameNo = tpc_trigframeno - tpc_packframeno;

      if (D_FEMFrameNo != 1){
        N_Bad_DFEMFrame += 1;
        
        // Add more information to log files if counter increments
        TLOG(TLVL_INFO)<< "TPC READOUT ERROR: For Event " << tpc_eventno << ", the FEM trigger - packet frame number is " << D_FEMFrameNo << " for TPC Crate " << tpc_crate << " FEM " << tpc_slot << "and for NTB frame & sample number " << ntb_frameno << " & " << ntb_sampleno << " and TPC trigger frame & sample number " << tpc_trigframeno << " & " << tpc_sampleno << ".";
      }
      
      // Check 4: Is TPC FEM absolute sample number expected with respect to NTB frame number?
      int ntb_abssampleno = ntb_frameno * 1144 + ntb_sampleno;
      int tpc_abssampleno = tpc_trigframeno * 1144 + tpc_sampleno;
      
      int D_NTBFEMSampleNo = tpc_abssampleno - ntb_abssampleno - 1144; // Subtract the frame size

      if (ntb_sampleno == 0){
        if (D_NTBFEMSampleNo != 1 && D_NTBFEMSampleNo != 2){
          N_Bad_DNTBFEMSample += 1;
          TLOG(TLVL_INFO)<< "TPC READOUT ERROR: For Event " << ntb_eventno << ", the TPC - NTB absolute sample number is " << D_NTBFEMSampleNo << " for TPC Crate " << tpc_crate << " FEM " << tpc_slot << " and boundary NTB sample number 0.";
        }
      } else if (ntb_sampleno == 1141){
        if (D_NTBFEMSampleNo != 0 && D_NTBFEMSampleNo != 1 && D_NTBFEMSampleNo != 4){
          N_Bad_DNTBFEMSample += 1;
          TLOG(TLVL_INFO)<< "TPC READOUT ERROR: For Event " << ntb_eventno << ", the TPC - NTB absolute sample number is " << D_NTBFEMSampleNo << " for TPC Crate " << tpc_crate << " FEM " << tpc_slot << " and boundary NTB sample number 1141.";
        }
      } else if (ntb_sampleno == 1142){
        if (D_NTBFEMSampleNo != 3 && D_NTBFEMSampleNo != 4){
          N_Bad_DNTBFEMSample += 1;
          TLOG(TLVL_INFO)<< "TPC READOUT ERROR: For Event " << ntb_eventno << ", the TPC - NTB absolute sample number is " << D_NTBFEMSampleNo << " for TPC Crate " << tpc_crate << " FEM " << tpc_slot << " and boundary NTB sample number 1142.";
        }
      } else if (ntb_sampleno == 1143){
        if (D_NTBFEMSampleNo != 2 && D_NTBFEMSampleNo != 3){
          N_Bad_DNTBFEMSample += 1;
          TLOG(TLVL_INFO)<< "TPC READOUT ERROR: For Event " << ntb_eventno << ", the TPC - NTB absolute sample number is " << D_NTBFEMSampleNo << " for TPC Crate " << tpc_crate << " FEM " << tpc_slot << " and boundary NTB sample number 1143.";
        }
      } else {
        if (D_NTBFEMSampleNo != 0 && D_NTBFEMSampleNo != 1){
          N_Bad_DNTBFEMSample += 1;
          TLOG(TLVL_INFO)<< "TPC READOUT ERROR: For Event " << ntb_eventno << ", the TPC - NTB absolute sample number is " << D_NTBFEMSampleNo << " for TPC Crate " << tpc_crate << " FEM " << tpc_slot << " and NTB sample number" << ntb_sampleno << ".";
        }
      }
    }
  }
  
  if(metricMan != nullptr) {
    //send flag metrics
    metricMan->sendMetric(
        "FEM_trig-pack_not1_Count",
        N_Bad_DFEMFrame,
        "fem_badframediff_count", 11, artdaq::MetricMode::LastPoint);
    metricMan->sendMetric(
        "NTBFEM_Async_Count",
        N_Bad_DNTBFEMSample,
        "ntbfem_badsamplediff_count", 11, artdaq::MetricMode::LastPoint);  
  }

};


void sbndaq::TPCMetricAnalyzer::readNTB(const artdaq::Fragment &frag)
{
  sbndaq::NevisTBFragment tbfrag(frag);
  const auto ntbheader = *tbfrag.header();
  ntb_eventno = ntbheader.getTriggerNumber();
  ntb_frameno = ntbheader.getFrame();
  ntb_sampleno = ntbheader.sample;
  //std::cout << "NTB information: Event, Frame, Sample: " << ntb_eventno << ", " << ntb_frameno << ", " << ntb_sampleno << std::endl; 

};

void sbndaq::TPCMetricAnalyzer::readTPC(const artdaq::Fragment &frag)
{
  sbndaq::NevisTPCFragment tpcfrag(frag);
  const auto tpcheader = *tpcfrag.header();                                                                                                                                                                                                            
  tpc_crate = (frag.fragmentID() >> 8) & 0xF;
  tpc_slot = tpcheader.getSlot();
  tpc_eventno = tpcheader.getEventNum();
  tpc_packframeno = tpcheader.getFrameNum();
  tpc_sampleno = tpcheader.get2mhzSample();
  tpc_trigframeno = tpcheader.getTrigFrame();
  //std::cout << "TPC information: Crate, Slot, Frame: " << tpc_crate << ", " << tpc_slot << ", " << tpc_packframeno << std::endl;
  //std::cout << "TPC information: Event, PacketFrame, Sample, TrigFrame:  "  << tpc_eventno << ", " << tpc_packframeno << ", " << tpc_sampleno << ", " << tpc_trigframeno << std::endl;

};


DEFINE_ART_MODULE(sbndaq::TPCMetricAnalyzer)

