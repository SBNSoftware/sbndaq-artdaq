//-------------------------------------------------
//---------------------------------------

////////////////////////////////////////////////////////////////////////
// Class:       CRTAframeReco
// Module Type: analyzer
// File:        CRTAframeReco_module.cc
// Description: Prints out information about each event.
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
// Authors: TODO add all the names/contacts
//
// Alexander Antonakis -> email: aantonakis@ucsb.edu
// Chris -> email: 
// Rohan -> email:
// Michelle Stancari -> email:
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"

#include "canvas/Utilities/Exception.h"

#include "sbndaq-artdaq-core/Overlays/Common/BernCRTFragmentV2.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "artdaq-core/Data/Fragment.hh"
#include "artdaq-core/Data/ContainerFragment.hh"

#include "art_root_io/TFileService.h"
#include "TH1F.h"
#include "TNtuple.h"
#include "TTree.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <vector>
#include <map>
#include <iostream>
#include <bitset>

// Aframe Utils directory added by Alex Antonakis to contain helper classes and functions for Aframe reconstruction
// Add ChannelMap class for initializing the geometry of your Aframe and calculating positions
#include "AFrameUtils/ChannelMap.h"
#include "AFrameUtils/helper_functions.h"
#include "AFrameUtils/Event.h"
#include "AFrameUtils/Cluster.h"
#include "AFrameUtils/Hit.h"
#include "AFrameUtils/AframeTrack.h"

namespace sbndaq {
  class CRTAframeReco;
}

/**************************************************************************************************/

class sbndaq::CRTAframeReco : public art::EDAnalyzer {

public:
  struct Config {
    //--one atom for each parameter

    // Try to add an Aframe config file for Channel Mapping
    fhicl::Atom<std::string> channel_map_config { 
      fhicl::Name("channel_map_config"),
	fhicl::Comment("This should give the path and file name for the channel mapping config file"),
	};
    fhicl::Atom<int> reco_mode { // Let this be an int since we might have more than two modes in the future
      fhicl::Name("reco_mode"),
	fhicl::Comment("Currently whether to use light balancing btw the SiPMs"),
	0 // Default is for the ChannelMap to place things at the strip center
	};
    fhicl::Atom<bool> include_berncrt {
      fhicl::Name("include_berncrt"),
	fhicl::Comment("look for bern CRT V2 fragments true/false"),
	false
	};
    fhicl::Atom<bool> crt_keepall {
      fhicl::Name("crt_keepall"),
	fhicl::Comment("put all crt fluff into tree true/false"),
	false
	};
    fhicl::Atom<bool> verbose {
      fhicl::Name("verbose"),
	fhicl::Comment("lots of text output if set to true"),
	false
	};
    fhicl::Atom<int> BeamTimeWindowStart {
      fhicl::Name("BeamTimeWindowStart"),
	fhicl::Comment("start of beam window cut on t1 timestamp (ns)"),
	324000
	};
    fhicl::Atom<int> BeamTimeWindowEnd {
      fhicl::Name("BeamTimeWindowEnd"),
	fhicl::Comment("end of beam window cut on t1 timestamp (ns)"),
	341000
	};
    fhicl::Atom<int> TimeCoinc {
      fhicl::Name("TimeCoinc"),
	fhicl::Comment("cut on time difference of two strip hits for CRT 3D reco (ns)"),
	100
	};
    fhicl::Atom<bool> MakeCRTHitTree {
      fhicl::Name("MakeCRTHitTree"),
	fhicl::Comment("if true make additional output tree"),
	false
	};
    fhicl::Atom<int> MakeRecoTree {
      fhicl::Name("MakeRecoTree"),
        fhicl::Comment("if true make an additional output tree for Aframe reconstruction"),
        true
        };
    fhicl::Atom<bool> MakeAnaTree {
      fhicl::Name("MakeAnaTree"),
	fhicl::Comment("if true make original ana tree"),
	true
	};
    fhicl::Atom<bool> MakeHistos {
      fhicl::Name("MakeHistos"),
	fhicl::Comment("if true make adc histos for each sipm"),
	false
	};
    fhicl::Sequence<int> LeftAframeHList {
      fhicl::Name("LeftAframeHList"),
	fhicl::Comment("list of horizontal modules on left side of Aframe")
	};
    fhicl::Sequence<int> LeftAframeVList {
      fhicl::Name("LeftAframeVList"),
	fhicl::Comment("list of vertical modules on left side of Aframe")
	};
    fhicl::Sequence<int> RightAframeHList {
      fhicl::Name("RightAframeHList"),
	fhicl::Comment("list of horizontal modules on right side of Aframe")
	};
    fhicl::Sequence<int> RightAframeVList {
      fhicl::Name("RightAframeVList"),
	fhicl::Comment("list of vertical modules on right side of Aframe")
	};

  }; //--configuration
  using Parameters = art::EDAnalyzer::Table<Config>;

  explicit CRTAframeReco(Parameters const & pset);
  virtual ~CRTAframeReco();

  void analyze(const art::Event& evt) override;
  void beginJob() override;
  void endJob() override;

private:

  bool checkList(int thisfeb,std::vector<int> feblist);
  void analyze_bern_fragment(artdaq::Fragment & frag);
  void CRTmaketree();
  void AframeMakeRecoTree();
  int getstrip_reco(BernCRTHitV2 const* bevt);
  int getstrip(int ihit);
  int adca,adcb;


  //--default values
  uint32_t nChannels;//    = 16;
  uint32_t Ttt_DownSamp;// =  4;
 /* the waveforms are sampled at 500MHz sampling. The trigger timestamp is
                               * sampled 4 times slower than input channels*/


  // Initialize the Channel Map for reconstruction  
  ChannelMap map;
  
  TNtuple* nt_header;
  int TTT_ns;

  TH1F*    hEventCounter;
  TH1D*    hTriggerTimeTag;
  TH1F*    h_wvfm_ev0_ch0;

  TTree* events;
  TTree* crthits;
  TTree* recotree;

  int fRun;
  art::EventNumber_t fEvent;
  
  // Add a cluster id for the new reco tree --> For a flattened tree. We could also try an object oriented tree with cluster objects 
  //int fCluster;
  // event object for the reco tree
  //Event* event = nullptr;

  //BernCRTV2 Information
 // std::map<uint64_t, std::vector<sbndaq::BernCRTHitV2 const*>> window;
  std::map<uint64_t, std::vector<int>> mac5_window; // have a map which is parallel to the window map to store mac5 addresses -> these are not in the BernCRTHitV2 objects
  std::map<uint64_t, std::vector<uint64_t>> window_timestamps;
  std::map<uint64_t, std::vector<int>> window_flags;
  
  std::map<uint64_t, std::vector<int>> window_adcA;
  std::map<uint64_t, std::vector<int>> window_adcB;
  std::map<uint64_t, std::vector<int>> window_strip_num;
  
  uint64_t t_low = 0;
  uint64_t t_high;
  bool fragcheck = false;
  std::vector<bool> is8fold;
  std::vector<int> flags;
  std::vector<int> lostcpu;
  std::vector<int> lostfpga;
  std::vector<uint> ts0;
  std::vector<uint> ts1;
  std::vector<int> adc0;
  std::vector<int> adc1;
  std::vector<int> adc2;
  std::vector<int> adc3;
  std::vector<int> adc4;
  std::vector<int> adc5;
  std::vector<int> adc6;
  std::vector<int> adc7;
  std::vector<int> adc8;
  std::vector<int> adc9;
  std::vector<int> adc10;
  std::vector<int> adc11;
  std::vector<int> adc12;
  std::vector<int> adc13;
  std::vector<int> adc14;
  std::vector<int> adc15;
  std::vector<int> adc16;
  std::vector<int> adc17;
  std::vector<int> adc18;
  std::vector<int> adc19;
  std::vector<int> adc20;
  std::vector<int> adc21;
  std::vector<int> adc22;
  std::vector<int> adc23;
  std::vector<int> adc24;
  std::vector<int> adc25;
  std::vector<int> adc26;
  std::vector<int> adc27;
  std::vector<int> adc28;
  std::vector<int> adc29;
  std::vector<int> adc30;
  std::vector<int> adc31;
  std::vector<int> maxadc;
  std::vector<int> coinc;

  std::vector<int>  feb_hit_number          ; //hit counter for individual FEB, including hits lost in FEB or fragment generator
  std::vector<uint64_t>  timestamp               ; //absolute timestamp
  std::vector<uint64_t>  last_accepted_timestamp ; //timestamp of previous accepted hit
  std::vector<int>  lost_hits               ; //number of lost hits from the previous one


  //metadata
  std::vector<int>  mac5; //last 8 bits of FEB mac5 address
  std::vector<uint>  run_start_time;
  std::vector<uint>  this_poll_start;
  std::vector<uint>  this_poll_end;
  std::vector<uint>  last_poll_start;
  std::vector<uint>  last_poll_end;
  std::vector<int>   system_clock_deviation;
  std::vector<int>  feb_hits_in_poll;
  std::vector<int>  feb_hits_in_fragment;

  //information from fragment header
  std::vector<int>  sequence_id;

  //crt hit information
  std::vector<int> febid_h;
  std::vector<int> strip_h;
  std::vector<int> ts0_h;
  std::vector<int> ts1_h;
  std::vector<int> adcA_h;
  std::vector<int> adcB_h;
  std::vector<int> febid_v;
  std::vector<int> strip_v;
  std::vector<int> ts0_v;
  std::vector<int> ts1_v;
  std::vector<int> adcA_v;
  std::vector<int> adcB_v;
  std::map<int,std::vector<int>> CrossingFEBs;
  std::vector<int> horiz;

  // new fcls for crt hit tree
  int fBeamTimeWindowStart; 
  int fBeamTimeWindowEnd;
  int fTimeCoinc;
  bool fMakeCRTHitTree;
  bool fMakeAnaTree;
  bool fMakeRecoTree;

  std::string fchannel_map_config;
  int freco_mode;
  bool finclude_berncrt;
  bool fcrt_keepall;
  bool fverbose;
  std::vector<int> fRightAframeVList;
  std::vector<int> fLeftAframeVList;
  std::vector<int> fRightAframeHList;
  std::vector<int> fLeftAframeHList;

  // Add elements of the Flat Reco Tree -> TODO Also Get the object oriented tree working
  std::vector<int> cluster_id;
  std::vector<int> hit_feb_h;
  std::vector<int> hit_feb_v;
  std::vector<uint64_t> hit_time;
  std::vector<uint64_t> hit_time_h;
  std::vector<uint64_t> hit_time_v;
  std::vector<float> hit_x; std::vector<float> hit_y; std::vector<float> hit_z;
  std::vector<float> hit_x_err; std::vector<float> hit_y_err; std::vector<float> hit_z_err;
  std::vector<int> hit_adcA_h; std::vector<int> hit_adcB_h; std::vector<int> hit_adcA_v; std::vector<int> hit_adcB_v;
  std::vector<float> x_vert; std::vector<float> y_horiz; std::vector<float> z_horiz;

}; //--class CRTAframeReco


sbndaq::CRTAframeReco::CRTAframeReco(CRTAframeReco::Parameters const& pset): art::EDAnalyzer(pset)
{
  // added a fcl parameter for the Channel Mapping
  fchannel_map_config = pset().channel_map_config();
  freco_mode = pset().reco_mode();
  fverbose = pset().verbose();
  finclude_berncrt = pset().include_berncrt();
  fcrt_keepall = pset().crt_keepall();

  fBeamTimeWindowStart = pset().BeamTimeWindowStart(); 
  fBeamTimeWindowEnd= pset().BeamTimeWindowEnd(); 
  fTimeCoinc= pset().TimeCoinc(); 
  fMakeCRTHitTree= pset().MakeCRTHitTree(); 
  fMakeAnaTree= pset().MakeAnaTree(); 
  fMakeRecoTree = pset().MakeRecoTree(); 

   //fMakeHistos=pset().MakeHistos(); 
  fRightAframeVList=pset().RightAframeVList();
  fLeftAframeVList=pset().LeftAframeVList();
  fRightAframeHList=pset().RightAframeHList();
  fLeftAframeHList=pset().LeftAframeHList();

}

void sbndaq::CRTAframeReco::beginJob()
{
  std::cout << "here" << std::endl;
  art::ServiceHandle<art::TFileService> tfs;
  nt_header       = tfs->make<TNtuple>("nt_header","Multi Header Ntuple","art_ev:caen_ev:caenv_ev_tts");
  /************************************************************************************************/
  hEventCounter   = tfs->make<TH1F>("hEventCounter","Event Counter Histogram",10000,0,10000);
  hTriggerTimeTag = tfs->make<TH1D>("hTriggerTimeTag","Trigger Time Tag Histogram",12500,0,125000000);
  h_wvfm_ev0_ch0  = tfs->make<TH1F>("h_wvfm_ev0_ch0","Waveform",2000,0,2000);
  /************************************************************************************************/

  // ChannelMap Class initialization 
  const std::string s(fchannel_map_config);
  //AFrame::ChannelMap map;
  map.initialize_config(s);
  map.CalculateParams(map.top_measure, map.bottom_measure);

  /************************************************************************************************/
  //crt_8fold = tfs->make<TTree>("crt_8fold", "waveform tree");
  //crt_8fold->Branch("window"             &map);

  //--make tree to store the channel waveform info:
  events = tfs->make<TTree>("events","waveform tree");
  events->Branch("fRun",&fRun,"fRun/I");
  events->Branch("fEvent",&fEvent,"fEvent/I");
  if (finclude_berncrt) {
    events->Branch("flags",         &flags);
    events->Branch("coinc",         &coinc);
    events->Branch("mac5",          &mac5);
    events->Branch("ts0",           &ts0);
    events->Branch("ts1",           &ts1);
    events->Branch("adc0",           &adc0);
    events->Branch("adc1",           &adc1);
    events->Branch("adc2",           &adc2);
    events->Branch("adc3",           &adc3);
    events->Branch("adc4",           &adc4);
    events->Branch("adc5",           &adc5);
    events->Branch("adc6",           &adc6);
    events->Branch("adc7",           &adc7);
    events->Branch("adc8",           &adc8);
    events->Branch("adc9",           &adc9);
    events->Branch("adc10",           &adc10);
    events->Branch("adc11",           &adc11);
    events->Branch("adc12",           &adc12);
    events->Branch("adc13",           &adc13);
    events->Branch("adc14",           &adc14);
    events->Branch("adc15",           &adc15);
    events->Branch("adc16",           &adc16);
    events->Branch("adc17",           &adc17);
    events->Branch("adc18",           &adc18);
    events->Branch("adc19",           &adc19);
    events->Branch("adc20",           &adc20);
    events->Branch("adc21",           &adc21);
    events->Branch("adc22",           &adc22);
    events->Branch("adc23",           &adc23);
    events->Branch("adc24",           &adc24);
    events->Branch("adc25",           &adc25);
    events->Branch("adc26",           &adc26);
    events->Branch("adc27",           &adc27);
    events->Branch("adc28",           &adc28);
    events->Branch("adc29",           &adc29);
    events->Branch("adc30",           &adc30);
    events->Branch("adc31",           &adc31);
    events->Branch("maxadc",         &maxadc);
    events->Branch("timestamp",   &timestamp);
    //events->Branch("window",         &window);

    if (fcrt_keepall) {
      events->Branch("lostcpu",       &lostcpu);
      events->Branch("lostfpga",      &lostfpga);
      events->Branch("feb_hit_number",&feb_hit_number);
      //events->Branch("timestamp",     &timestamp);
      events->Branch("last_accepted_timestamp",&last_accepted_timestamp);
      events->Branch("lost_hits",     &lost_hits);
      events->Branch("run_start_time",            &run_start_time);
      events->Branch("this_poll_start",           &this_poll_start);
      events->Branch("this_poll_end",             &this_poll_end);
      events->Branch("last_poll_start",           &last_poll_start);
      events->Branch("last_poll_end",             &last_poll_end);
      events->Branch("system_clock_deviation",    &system_clock_deviation);
      events->Branch("feb_hits_in_poll",          &feb_hits_in_poll);
      events->Branch("feb_hits_in_fragment",      &feb_hits_in_fragment);
      events->Branch("sequence_id",               &sequence_id);
    }
  }

  //--make tree to store the channel waveform info:
  if (fMakeCRTHitTree) {
    crthits = tfs->make<TTree>("crthits","beam muon hit tree");
    crthits->Branch("Run",&fRun,"fRun/I");
    crthits->Branch("Event",&fEvent,"fEvent/I");
    crthits->Branch("febid_h",&febid_h);
    crthits->Branch("strip_h",&strip_h);
    crthits->Branch("ts0_h",&ts0_h);
    crthits->Branch("ts1_h",&ts1_h);
    crthits->Branch("adcA_h",&adcA_h);
    crthits->Branch("adcB_h",&adcB_h);
    crthits->Branch("febid_v",&febid_v);
    crthits->Branch("strip_v",&strip_v);
    crthits->Branch("ts0_v",&ts0_v);
    crthits->Branch("ts1_v",&ts1_v);
    crthits->Branch("adcA_v",&adcA_v);
    crthits->Branch("adcB_v",&adcB_v);
    crthits->Branch("is8fold", &is8fold);
  }
  // Let's add a reco tree separately for now so we can leave Michelle's code alone and just cherry pick what we need
  if (fMakeRecoTree) {
    recotree = tfs->make<TTree>("reco_tree", "reco_tree");
    recotree->Branch("Run", &fRun, "fRun/I");
    recotree->Branch("Event", &fEvent, "fEvent/I");
    recotree->Branch("t_low", &t_low, "t_low/l");
    recotree->Branch("cluster_id", &cluster_id);  
    recotree->Branch("hit_feb_h", &hit_feb_h);  
    recotree->Branch("hit_feb_v", &hit_feb_v);  
    recotree->Branch("hit_time", &hit_time);  
    recotree->Branch("hit_time_h", &hit_time_h);  
    recotree->Branch("hit_time_v", &hit_time_v);
    recotree->Branch("hit_adcA_h", &hit_adcA_h); recotree->Branch("hit_adcB_h", &hit_adcB_h);
    recotree->Branch("hit_adcA_v", &hit_adcA_v); recotree->Branch("hit_adcB_v", &hit_adcB_v);
    recotree->Branch("hit_x", &hit_x); recotree->Branch("hit_y", &hit_y); recotree->Branch("hit_z", &hit_z); 
    recotree->Branch("hit_x_err", &hit_x_err); recotree->Branch("hit_y_err", &hit_y_err); recotree->Branch("hit_z_err", &hit_z_err); 
    recotree->Branch("x_vert", &x_vert); recotree->Branch("y_horiz", &y_horiz); recotree->Branch("z_horiz", &z_horiz); 

    // Eventually add object-oriented tree elements
    //recotree->Branch("Cluster", &fCluster, "fCluster/I");
    //recotree->Branch("event", &event);
  }

  // left Aframe
  horiz.clear();
  std::vector<int> match;  
  for (int i=0;i<(int)fRightAframeVList.size();i++) match.push_back(fRightAframeVList.at(i));
  for (int i=0;i<(int)fRightAframeHList.size();i++) { 
    int thisfebh=fRightAframeHList.at(i);
    //    std::cout << " horiz " << thisfebh << std::endl;
    CrossingFEBs.insert(std::make_pair(thisfebh,match));
    horiz.push_back(thisfebh);
  }
    match.clear(); 
  for (int i=0;i<(int)fRightAframeHList.size();i++) match.push_back(fRightAframeHList.at(i));
  for (int i=0;i<(int)fRightAframeVList.size();i++) { 
    int thisfebh=fRightAframeVList.at(i);
    CrossingFEBs.insert(std::make_pair(thisfebh,match));
  }
  // right Aframe#
  match.clear(); 
  for (int i=0;i<(int)fLeftAframeVList.size();i++) match.push_back(fLeftAframeVList.at(i));
  for (int i=0;i<(int)fLeftAframeHList.size();i++) { 
    int thisfebh=fLeftAframeHList.at(i);
    //    std::cout << " horiz2 " << thisfebh << std::endl;
    CrossingFEBs.insert(std::make_pair(thisfebh,match));
    horiz.push_back(thisfebh);
  }
    match.clear(); 
  for (int i=0;i<(int)fLeftAframeHList.size();i++) match.push_back(fLeftAframeHList.at(i));
  for (int i=0;i<(int)fLeftAframeVList.size();i++) { 
    int thisfebh=fLeftAframeVList.at(i);
    CrossingFEBs.insert(std::make_pair(thisfebh,match));
  }




}

bool sbndaq::CRTAframeReco::checkList(int thisfeb,std::vector<int> feblist)
{
  bool retval = false;
  for (int i=0;i<(int)feblist.size();++i) {
    if (thisfeb==feblist.at(i)) retval=true;
  }
  return(retval);
}


void sbndaq::CRTAframeReco::endJob()
{
  if (fverbose)  std::cout << "Ending CRTAframeReco...\n";
}


sbndaq::CRTAframeReco::~CRTAframeReco()
{

}


void sbndaq::CRTAframeReco::analyze(const art::Event& evt)
{
  // reset t_low for the next Event
  t_low = 0;
  fRun = evt.run();
  fEvent = evt.event();
  if (fverbose)   std::cout << "Run " << fRun << " event " << fEvent << std::endl;

  /************************************************************************************************/

  mac5.clear(); flags.clear();   lostcpu.clear();   lostfpga.clear();   ts0.clear();   ts1.clear(); 
  adc0.clear();   adc1.clear();   adc2.clear();   adc3.clear();   adc4.clear();   adc5.clear();   adc6.clear();
  adc7.clear();   adc8.clear();   adc9.clear();   adc10.clear();   adc11.clear();   adc12.clear();   adc13.clear();
  adc14.clear();   adc15.clear();   adc16.clear();   adc17.clear();   adc18.clear();   adc19.clear();   adc20.clear();
  adc21.clear();   adc22.clear();   adc23.clear();   adc24.clear();   adc25.clear();   adc26.clear();   adc27.clear();
  adc28.clear();   adc29.clear();   adc30.clear();   adc31.clear();   maxadc.clear();  coinc.clear();
  
  feb_hit_number.clear()          ;    timestamp.clear()               ;    last_accepted_timestamp.clear() ; 
  lost_hits.clear()               ;   run_start_time.clear();   this_poll_start.clear();   this_poll_end.clear();
  last_poll_start.clear();   last_poll_end.clear();    system_clock_deviation.clear();    feb_hits_in_poll.clear();
  feb_hits_in_fragment.clear(); sequence_id.clear(); 
  //window.clear(); 
  mac5_window.clear(); window_timestamps.clear();
  window_flags.clear(); window_adcA.clear(); window_adcB.clear(); window_strip_num.clear();
  /************************************************************************************************/


  //  Note that this code expects exactly 1 CAEN fragment per event
  TTT_ns=0;  // will be set to value in CAEN fragement header
  


  std::vector<art::Handle<artdaq::Fragments>> fragmentHandles;
  
#if ART_HEX_VERSION < 0x30900
  evt.getManyByType(fragmentHandles);
#else
  fragmentHandles = evt.getMany<std::vector<artdaq::Fragment>>();
#endif
  
  /************************************************************************************************/



  if (finclude_berncrt){

    std::vector<art::Handle<artdaq::Fragments>> fragmentHandles;
    
#if ART_HEX_VERSION < 0x30900
    evt.getManyByType(fragmentHandles);
#else
    fragmentHandles = evt.getMany<std::vector<artdaq::Fragment>>();
#endif
    
   /////////////////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////////////////
 
   // Duplicate Michelle's Code here to find t_low for this Art Event -> Not necessary !!!!!
   
   std::cout << "t_low for this Art Event after being reset: " << t_low << std::endl;
   /////////////////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////////////////

    for (auto handle : fragmentHandles) {
      if (!handle.isValid() || handle->size() == 0)
	continue;
      
      if (handle->front().type() == artdaq::Fragment::ContainerFragmentType) {
	//Container fragment
	
	for (auto cont : *handle) {
	    //int i = 0;
	  artdaq::ContainerFragment contf(cont);
	  if (contf.fragment_type() != sbndaq::detail::FragmentType::BERNCRTV2)
	    continue;
	    
	  for (size_t ii = 0; ii < contf.block_count(); ++ii){
	    analyze_bern_fragment(*contf[ii].get());
	    
	  }

	}
	    
      }	
      else {
        //normal fragment
        if (handle->front().type() != sbndaq::detail::FragmentType::BERNCRTV2) continue;
        for (auto frag : *handle)
          analyze_bern_fragment(frag);
      }
    }  //  loop over frag handles
  }// if include_berncrt

  // find t_low for this Art Event
  for (long unsigned int i = 0; i < timestamp.size(); ++i) {
    if (t_low == 0) { t_low = timestamp.at(i); }
    if (timestamp.at(i) < t_low) { t_low = timestamp.at(i); }
  }
  std::cout << "t_low for this Art Event: " << t_low << std::endl;

  std::cout << std::endl;  
  std::cout << "The number of Clusters in this event " << mac5_window.size() << std::endl;
  std::cout << std::endl;

  if (fMakeAnaTree) events->Fill();
  if (fMakeCRTHitTree) CRTmaketree();
  std::cout << "Filled Michelle's Trees" << std::endl;
  // Add a function for Aframe Reconstruction which fills the flattened tree
  if (fMakeRecoTree) AframeMakeRecoTree();
  std::cout << "Filled My Tree" << std::endl;

/*

  // Testing for Channel Mapping class with hardcoded values
  std::cout << "Testing Channel Map Class" << std::endl;
  int feb_v_test = 72; 
  int feb_h_test = 171;
  int strip_h_test = 5;
  int strip_v_test = 7;
  // put adc fake values of 50 -->  not needed in mode 0
  TVector3 horiz_pos = map.CalculatePosHoriz(feb_h_test, strip_h_test, 50, 50, 0); 
  TVector3 vert_pos = map.CalculatePosVert(feb_v_test, strip_v_test, 50, 50, 0);

  std::cout << "FEB_H " << feb_h_test << " strip " << strip_h_test << " pos: x " << horiz_pos.X() << " y "<< horiz_pos.Y() << " z " << horiz_pos.Z() << std::endl;
  std::cout << "FEB_V " << feb_v_test << " strip " << strip_v_test << " pos: x " << vert_pos.X() << " y " << vert_pos.Y() << " z " << vert_pos.Z() << std::endl;

  std::vector<float> errors = map.CalculateError(feb_v_test, feb_h_test);
  TVector3 hitpos = CalculateHitPos(errors, vert_pos, horiz_pos);

  std::cout << std::endl;
  std::cout << "Hit x: " << hitpos.X() << " y " << hitpos.Y() << " z " << hitpos.Z() << std::endl;
  std::cout << "Err x_v: " << errors.at(0) << " x_h " << errors.at(1) << " y_v " << errors.at(2) << " y_h " << errors.at(3) << " z_v " << errors.at(4) << " z_h " << errors.at(5)<< std::endl;

  Hit hit;
  hit.x_pos = hitpos.X(); hit.y_pos = hitpos.Y(); hit.z_pos = hitpos.Z();

  std::cout << "Reco Hit x " << hit.x_pos << " y " << hit.y_pos << " z " << hit.z_pos << std::endl;

*/

}



void sbndaq::CRTAframeReco::CRTmaketree()  {

  int hitcount = 0 ;
  int nhits = mac5.size();
  if (nhits<=0) { std::cout << " no crt hits in event " << fEvent << std::endl; return;}
  int thisFEB1,thisFEB2;
  int thistime1,thistime2;
  strip_h.clear(); febid_h.clear(); adcA_h.clear(); adcB_h.clear();
  strip_v.clear(); febid_v.clear(); adcA_v.clear(); adcB_v.clear();
  ts0_h.clear();   ts1_h.clear();
  ts0_v.clear();   ts1_v.clear();
  is8fold.clear();
  /************************************************************************************************/

  std::vector<int> match;
  for (int ih1=0;ih1<nhits-1;++ih1) {
    thisFEB1=mac5.at(ih1);
    // check that time is in beam window
    //    thistime1=ts1.at(ih1);
    thistime1=ts0.at(ih1)-TTT_ns;
    if (thistime1>1000000000) thistime1-=1000000000;
    else if (thistime1<-1000000000) thistime1+=1000000000;
    if (thistime1<fBeamTimeWindowEnd && thistime1>fBeamTimeWindowStart && flags.at(ih1)==3) {
    //if (thistime1<fBeamTimeWindowEnd && thistime1>fBeamTimeWindowStart && (!((flags.at(ih1) & 4) || (flags.at(ih1) & 8))) ) {
      std::map<int, std::vector<int>>::iterator it = CrossingFEBs.find(thisFEB1);
      if(it != CrossingFEBs.end()) {
      	match = it->second;
      	for (int ih2=ih1+1;ih2<nhits;++ih2) {
	  // use trigger time
	  thistime2=ts0.at(ih2)-TTT_ns;
	  if (thistime2>1000000000) thistime2-=1000000000;
	  else if (thistime2<-1000000000) thistime2+=1000000000;
	  // use beam spill
	  //    thistime2=ts1.at(ih2);
      	  if (thistime2<fBeamTimeWindowEnd && thistime2>fBeamTimeWindowStart && flags.at(ih2)==3) {
    	      thisFEB2=mac5.at(ih2);
      	    // are hits 100 ns apart or less in time?
      	    float tdiff = fabs(thistime1-thistime2);
	    bool a = true;
      	    if (tdiff>500000000) {tdiff-=1000000000; tdiff*=-1.0;}
      	    if (tdiff<fTimeCoinc) {
        	      // next check if they are from crossing strips
      	      if (checkList(thisFEB2,match)) {
      		      hitcount++;
		      if(hitcount>=4){
			is8fold.push_back(a);
		      }
		      
		      if (checkList(thisFEB1,horiz)) {
            		  strip_h.push_back(getstrip(ih1));
            		  febid_h.push_back(thisFEB1);  
			  adcA_h.push_back(adca); adcB_h.push_back(adcb);
            		  strip_v.push_back(getstrip(ih2));
            		  febid_v.push_back(thisFEB2); 
			  adcA_v.push_back(adca); adcB_v.push_back(adcb);
            		  ts0_h.push_back(ts0.at(ih1)); ts0_v.push_back(ts0.at(ih2));
            		  ts1_h.push_back(thistime1); ts1_v.push_back(thistime2);
						  
            		}
            		else {
            		  strip_h.push_back(getstrip(ih2));
            		  febid_h.push_back(thisFEB2);  
			  adcA_h.push_back(adca); adcB_h.push_back(adcb);
            		  strip_v.push_back(getstrip(ih1));
            		  febid_v.push_back(thisFEB1);  
			  adcA_v.push_back(adca); adcB_v.push_back(adcb);
            		  ts0_h.push_back(ts0.at(ih2)); ts0_v.push_back(ts0.at(ih1));
            		  ts1_h.push_back(thistime2); ts1_v.push_back(thistime1);
            		}
      	      } // if hits are from crossing strips
      	    } // if hits are at the same time
      	  } // if second hit is in beam window
      	}  // loop over hits after hit in outer loop	
      } //if feb is valid
    } // if hit time in beam window
  }//loop over hits
  crthits->Fill();
  
  //  if (hitcount>1) std::cout << "event " << fEvent << " has " << hitcount << " crt hits " << std::endl;
}

// Added by Alex Antonakis to take BernCRTHitV2 objects within time bins and pair vertical and horizontals to form reco hits and tracks 
void sbndaq::CRTAframeReco::AframeMakeRecoTree() 
{
  cluster_id.clear(); hit_feb_h.clear(); hit_feb_v.clear(); hit_time.clear(); hit_time_h.clear(); hit_time_v.clear();
  hit_x.clear(); hit_y.clear(); hit_z.clear();
  hit_x_err.clear(); hit_y_err.clear(); hit_z_err.clear();
  hit_adcA_v.clear(); hit_adcB_v.clear(); hit_adcA_h.clear(); hit_adcB_h.clear();
  x_vert.clear(); y_horiz.clear(); z_horiz.clear();
  //event->EventClear(); // clear the event before filling with new stuff
  //event->SetID(fEvent); // Set the Event ID where an event is a single readout
  //event->SetRun(fRun); // Set Run ID for this readout

  // Loop over the Clusters in time for a single event readout and form strip pairs, weighted hits, module/precision hits, and tracks
  //for (long unsigned int i = 0; i < window.size(); ++i) { 
  int num_bad = 0;
  int cluster_count = 0;
  for (const auto & [key, value] : mac5_window) {
  
    // Important: value holds a vector of BernCRTHitV2 objects associated with the Cluster ID: key
      
    //Cluster new_cluster;
    //new_cluster.SetID(key); // TODO Determine if we want the clusters to have the bin number as the ID or maybe just the order indexed from 0			
    //event->AddCluster(new_cluster);    
  
    int hitcount = 0 ;
    long unsigned int  nhits = value.size(); 
    //std::cout << "number of BernCRTHitV2s in this Cluster to loop over " << value.size() << std::endl;
    if (nhits<=0) { std::cout << " no crt hits in this Cluster associated with event: " << fEvent << std::endl; return;}
    int thisFEB1,thisFEB2;
    //uint64_t thistime1,thistime2;
    uint64_t thistime1;
    uint64_t thistime2;
    std::vector<int> match;
    
    for (long unsigned int ih1=0;ih1<nhits-1;++ih1) { // loop over the BernCRTHitV2 objects within this time bin and pair them
      thisFEB1 = mac5_window[key].at(ih1);
      //thistime1=value.at(ih1)->timestamp;
      thistime1=window_timestamps[key].at(ih1);
      //std::cout << "Flags at ih1 " << (int)value.at(ih1)->flags << std::endl; 
      //if ((int)value.at(ih1)->flags == 3) {
      if (window_flags[key].at(ih1) == 3) {
        std::map<int, std::vector<int>>::iterator it = CrossingFEBs.find(thisFEB1);
        if(it != CrossingFEBs.end()) {
      	  match = it->second;
      	  for (long unsigned int ih2=ih1+1;ih2<nhits;++ih2) {
	    // use trigger time
	    //thistime2=value.at(ih2)->timestamp;
            thistime2=window_timestamps[key].at(ih2);
      	    //if ((int)value.at(ih2)->flags == 3) {
      	    if (window_flags[key].at(ih2) == 3) {
    	      thisFEB2 = mac5_window[key].at(ih2);
      	      // are hits 100 ns apart or less in time?
      	      float tdiff = fabs(thistime1-thistime2);
      	      if (tdiff<fTimeCoinc) {
        	// next check if they are from crossing strips
      	        if (checkList(thisFEB2,match)) {
		  // Ok, We can make a reco hit now !!! 
		        if (checkList(thisFEB1,horiz)) {
			  //int strip_horiz = getstrip_reco(value.at(ih1)); 
			  //int adcA_horiz = adca; int adcB_horiz = adcb;  
			  //int strip_vert = getstrip_reco(value.at(ih2));
			  //int adcA_vert = adca; int adcB_vert = adcb;
                          // using new std::maps
                          int strip_horiz = window_strip_num[key].at(ih1);
                          int adcA_horiz = window_adcA[key].at(ih1);
                          int adcB_horiz = window_adcB[key].at(ih1);
                          int strip_vert = window_strip_num[key].at(ih2);
                          int adcA_vert = window_adcA[key].at(ih2);
                          int adcB_vert = window_adcB[key].at(ih2);
                          
  			  TVector3 horiz_pos; 
  			  TVector3 vert_pos;
                          if (freco_mode == 0) {
  			    horiz_pos = map.CalculatePosHoriz(thisFEB1, strip_horiz, 50, 50, 0); 
  			    vert_pos = map.CalculatePosVert(thisFEB2, strip_vert, 50, 50, 0);
                          }
                          else if (freco_mode == 1) { // Tanh weighting
  			    horiz_pos = map.CalculatePosHoriz(thisFEB1, strip_horiz, adcA_horiz, adcB_horiz, 1); 
  			    vert_pos = map.CalculatePosVert(thisFEB2, strip_vert, adcA_vert, adcB_vert, 1);
                          }
			  else {
                            std::cout << std::endl;
                            std::cout << "WARNING: Not a currently supported reco_mode !!!!!" << std::endl;
                            std::cout << std::endl;
			    return;
                          } 
 			  std::vector<float> errors = map.CalculateError(thisFEB2, thisFEB1);
  			  TVector3 hitpos = CalculateHitPos(errors, vert_pos, horiz_pos);
                          Hit hit; // let's make a weighted reco hit
  			  hit.x_pos = hitpos.X(); hit.y_pos = hitpos.Y(); hit.z_pos = hitpos.Z();
			  std::vector<float> error_tot = CalculateTotalError(errors);
			  hit.x_err = error_tot.at(0); hit.y_err = error_tot.at(1); hit.z_err = error_tot.at(2);
			  // Add info for Module hits
      		          hit.x_vert = vert_pos.X(); hit.y_horiz = horiz_pos.Y(); hit.z_horiz = horiz_pos.Z();
			  hit.time = (thistime1 + thistime2)/2; // TODO this might not work due to integer division !!!!!!!!
			  hit.time_h = thistime1; hit.time_v = thistime2;
 			  hit.feb_h = thisFEB1; hit.feb_v = thisFEB2;
			  hit.adcA_h = adcA_horiz; hit.adcB_h = adcB_horiz; hit.adcA_v = adcA_vert; hit.adcB_v = adcB_vert;
			  //event->GetCluster(cluster_count)->AddHit(hit);
			  //std::cout << "feb_h " << hit.feb_h << " feb_v " << hit.feb_v << std::endl;
			  //std::cout << "flags1 " << (int)value.at(ih2)->flags << " flags2 " << (int)value.at(ih2)->flags << std::endl; 
			  //std::cout << "hit_time_h: " << hit.time_h << " hit_time_v: " << hit.time_v << std::endl;
                          //std::cout << "thistime1   " << thistime1 <<  " thistime2   " << thistime2 << std::endl; 
 			  //if (thistime1 != thistime2) {
			  //  std::cout << std::endl;
			    //std::cout << "YES We have some different paired times !!!!!!" << std::endl;   			  
			  //  std::cout << std::endl;
			  //}
   			  if (thistime1 < t_low || thistime2 < t_low) {
 			    std::cout << std::endl;
  			    std::cout << std::endl;
 			    std::cout << "We formed a hit with small timestamps ? t_h: " << hit.time_h << " t_v: " << hit.time_v << std::endl;
			    std::cout << "From the other map t_1: " << window_timestamps[key].at(ih1) << " t_2: " << window_timestamps[key].at(ih2) << std::endl;
		            num_bad += 1;
  			    std::cout << std::endl;
 			    std::cout << std::endl;
			  }			  

			  // Flat tree elements
  			  cluster_id.push_back(cluster_count); hit_feb_h.push_back(hit.feb_h); hit_feb_v.push_back(hit.feb_v);
			  hit_time.push_back(hit.time - t_low); hit_time_h.push_back(hit.time_h - t_low); hit_time_v.push_back(hit.time_v - t_low); // subtract t_low
  			  hit_x.push_back(hit.x_pos); hit_y.push_back(hit.y_pos); hit_z.push_back(hit.z_pos);
  			  hit_x_err.push_back(hit.x_err); hit_y_err.push_back(hit.y_err); hit_z_err.push_back(hit.z_err);
			  hit_adcA_v.push_back(hit.adcA_v); hit_adcB_v.push_back(hit.adcB_v); hit_adcA_h.push_back(hit.adcA_h); hit_adcB_h.push_back(hit.adcB_h);  
			  x_vert.push_back(hit.x_vert); y_horiz.push_back(hit.y_horiz); z_horiz.push_back(hit.z_horiz);

                        }
                        else {
			  //int strip_horiz = getstrip_reco(value.at(ih2)); 
			  //int adcA_horiz = adca; int adcB_horiz = adcb;  
			  //int strip_vert = getstrip_reco(value.at(ih1));
			  //int adcA_vert = adca; int adcB_vert = adcb;   
			  // using new std::maps
                          int strip_horiz = window_strip_num[key].at(ih2);
                          int adcA_horiz = window_adcA[key].at(ih2);
                          int adcB_horiz = window_adcB[key].at(ih2);
                          int strip_vert = window_strip_num[key].at(ih1);
                          int adcA_vert = window_adcA[key].at(ih1);
                          int adcB_vert = window_adcB[key].at(ih1);
  			  
                          TVector3 horiz_pos; 
  			  TVector3 vert_pos;
                          if (freco_mode == 0) {
  			    horiz_pos = map.CalculatePosHoriz(thisFEB1, strip_horiz, 50, 50, 0); 
  			    vert_pos = map.CalculatePosVert(thisFEB2, strip_vert, 50, 50, 0);
                          }
                          else if (freco_mode == 1) { // Tanh weighting
  			    horiz_pos = map.CalculatePosHoriz(thisFEB1, strip_horiz, adcA_horiz, adcB_horiz, 1); 
  			    vert_pos = map.CalculatePosVert(thisFEB2, strip_vert, adcA_vert, adcB_vert, 1);
                          }
			  else {
                            std::cout << std::endl;
                            std::cout << "WARNING: Not a currently supported reco_mode !!!!!" << std::endl;
                            std::cout << std::endl;
			    return;
                          } 
 			  std::vector<float> errors = map.CalculateError(thisFEB1, thisFEB2);
  			  TVector3 hitpos = CalculateHitPos(errors, vert_pos, horiz_pos);
                          Hit hit; // let's make a weighted reco hit
  			  hit.x_pos = hitpos.X(); hit.y_pos = hitpos.Y(); hit.z_pos = hitpos.Z();
			  std::vector<float> error_tot = CalculateTotalError(errors);
			  hit.x_err = error_tot.at(0); hit.y_err = error_tot.at(1); hit.z_err = error_tot.at(2);
 			  // Add info for Module hits
      		          hit.x_vert = vert_pos.X(); hit.y_horiz = horiz_pos.Y(); hit.z_horiz = horiz_pos.Z();
			  hit.time = (thistime1 + thistime2)/2; // TODO this might not work due to integer division !!!!!!!!
			  hit.time_h = thistime2; hit.time_v = thistime1;
			  hit.feb_h = thisFEB2; hit.feb_v = thisFEB1;
			  hit.adcA_h = adcA_horiz; hit.adcB_h = adcB_horiz; hit.adcA_v = adcA_vert; hit.adcB_v = adcB_vert;
			  //event->GetCluster(cluster_count)->AddHit(hit);
			  //std::cout << "feb_h " << hit.feb_h << " feb_v " << hit.feb_v << std::endl;
			  //std::cout << "flags1 " << (int)value.at(ih2)->flags << " flags2 " << (int)value.at(ih2)->flags << std::endl; 
			  //std::cout << "hit_time_h: " << hit.time_h << " hit_time_v: " << hit.time_v << std::endl;
                          //std::cout << "thistime1   " << thistime1 <<  " thistime2   " << thistime2 << std::endl; 
 			  //if (thistime1 != thistime2) {
			  //  std::cout << std::endl;
			    //std::cout << "YES We have some different paired times !!!!!!" << std::endl;   			  
			  //  std::cout << std::endl;
			  //}
			  if (thistime1 < t_low || thistime2 < t_low) {
 			    std::cout << std::endl;
  			    std::cout << std::endl;
 			    std::cout << "We formed a hit with small timestamps ? t_h: " << hit.time_h << " t_v: " << hit.time_v << std::endl;
			    std::cout << "From the other map t_1: " << window_timestamps[key].at(ih1) << " t_2: " << window_timestamps[key].at(ih2) << std::endl;
			    num_bad += 1;
  			    std::cout << std::endl;
 			    std::cout << std::endl;
			  }			  
			
                          // Flat tree elements
  			  cluster_id.push_back(cluster_count); hit_feb_h.push_back(hit.feb_h); hit_feb_v.push_back(hit.feb_v);
			  hit_time.push_back(hit.time - t_low); hit_time_h.push_back(hit.time_h - t_low); hit_time_v.push_back(hit.time_v - t_low); // subtract t_low
  			  hit_x.push_back(hit.x_pos); hit_y.push_back(hit.y_pos); hit_z.push_back(hit.z_pos);
  			  hit_x_err.push_back(hit.x_err); hit_y_err.push_back(hit.y_err); hit_z_err.push_back(hit.z_err);
			  hit_adcA_v.push_back(hit.adcA_v); hit_adcB_v.push_back(hit.adcB_v); hit_adcA_h.push_back(hit.adcA_h); hit_adcB_h.push_back(hit.adcB_h);  
			  x_vert.push_back(hit.x_vert); y_horiz.push_back(hit.y_horiz); z_horiz.push_back(hit.z_horiz);
 			  				
			}
                 } // checkList(thisFEB2,match)-> Crossing Strips ?
              } // tdiff < timeCoinc
            } // if ih2 flags == 3              
          } // ih2 for loop
        } // if the feb is valid
      } // if ih1 flags == 3
    } // ih1 for loop
    // TODO Call the TrackMaker code here before clearing !!!!!
    cluster_count += 1; 
  } // Loop over clusters in the window
  // fill the object-oriented tree entries -> Each entry is all the stuff associated with one full readout
  recotree->Fill();
  std::cout << "number of bad timestamps for Event id: " << fEvent <<" --> num_bad: " << num_bad << std::endl;
  std::cout << std::endl;
  // Below code for object oriented eventually
  //for (int s = 0; s < event->GetClusterCount(); ++s) {
  //  event->GetCluster(s)->ClearHits();
  //  event->GetCluster(s)->ClearTracks();
  //}
  //event->EventClear();
}


// Added by Alex Antonakis to replace Michelle's implementation for the purpose of Aframe reconstruction
int sbndaq::CRTAframeReco::getstrip_reco(BernCRTHitV2 const* bevt)
{
  int thisadc, maxadc, maxchan;
  maxadc=bevt->adc[0]; maxchan=0; thisadc=bevt->adc[1];
  adca=maxadc; adcb=thisadc;
  int stripno=0; 
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=1;  }
  thisadc=bevt->adc[2];
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=2; stripno=1; adca=bevt->adc[2]; adcb=bevt->adc[3];}
  thisadc=bevt->adc[3];
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=3; stripno=1; adca=bevt->adc[2]; adcb=bevt->adc[3];}
  thisadc=bevt->adc[4];
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=4; stripno=2; adca=bevt->adc[4]; adcb=bevt->adc[5];}
  thisadc=bevt->adc[5];
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=5;  stripno=2; adca=bevt->adc[4]; adcb=bevt->adc[5];}
  thisadc=bevt->adc[6];
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=6; stripno=3; adca=bevt->adc[6]; adcb=bevt->adc[7];}
  thisadc=bevt->adc[7];
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=7;  stripno=3; adca=bevt->adc[6]; adcb=bevt->adc[7];}
  thisadc=bevt->adc[8];
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=8;  stripno=4; adca=bevt->adc[8]; adcb=bevt->adc[9];}
  thisadc=bevt->adc[9];
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=9;  stripno=4; adca=bevt->adc[8]; adcb=bevt->adc[9];}
  thisadc=bevt->adc[10];
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=10;  stripno=5; adca=bevt->adc[10]; adcb=bevt->adc[11];}
  thisadc=bevt->adc[11];
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=11;  stripno=5; adca=bevt->adc[10]; adcb=bevt->adc[11];}
  thisadc=bevt->adc[12];
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=12;  stripno=6; adca=bevt->adc[12]; adcb=bevt->adc[13];}
  thisadc=bevt->adc[13];
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=13;  stripno=6; adca=bevt->adc[12]; adcb=bevt->adc[13];}
  thisadc=bevt->adc[14];
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=14;  stripno=7; adca=bevt->adc[14]; adcb=bevt->adc[15];}
  thisadc=bevt->adc[15];
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=15;  stripno=7; adca=bevt->adc[14]; adcb=bevt->adc[15];}
  thisadc=bevt->adc[16];
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=16;  stripno=8; adca=bevt->adc[16]; adcb=bevt->adc[17];}
  thisadc=bevt->adc[17];
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=17;  stripno=8; adca=bevt->adc[16]; adcb=bevt->adc[17];}
  thisadc=bevt->adc[18];
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=18;  stripno=9; adca=bevt->adc[18]; adcb=bevt->adc[19];}
  thisadc=bevt->adc[19];
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=19;  stripno=9; adca=bevt->adc[18]; adcb=bevt->adc[19];}
  thisadc=bevt->adc[20];
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=20;  stripno=10; adca=bevt->adc[20]; adcb=bevt->adc[21];}
  thisadc=bevt->adc[21];
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=21;  stripno=10; adca=bevt->adc[20]; adcb=bevt->adc[21];}
  thisadc=bevt->adc[22];
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=22;  stripno=11; adca=bevt->adc[22]; adcb=bevt->adc[23];}
  thisadc=bevt->adc[23];
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=23;  stripno=11; adca=bevt->adc[22]; adcb=bevt->adc[23];}
  thisadc=bevt->adc[24];
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=24;  stripno=12; adca=bevt->adc[24]; adcb=bevt->adc[25];}
  thisadc=bevt->adc[25];
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=25;  stripno=12; adca=bevt->adc[24]; adcb=bevt->adc[25];}
  thisadc=bevt->adc[26];
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=26;  stripno=13; adca=bevt->adc[26]; adcb=bevt->adc[27];}
  thisadc=bevt->adc[27];
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=27;  stripno=13; adca=bevt->adc[26]; adcb=bevt->adc[27];}
  thisadc=bevt->adc[28];
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=28;  stripno=14; adca=bevt->adc[28]; adcb=bevt->adc[29];}
  thisadc=bevt->adc[29];
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=29;  stripno=14; adca=bevt->adc[28]; adcb=bevt->adc[29];}
  thisadc=bevt->adc[30];
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=30;  stripno=15; adca=bevt->adc[30]; adcb=bevt->adc[31];}
  thisadc=bevt->adc[31];
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=31;  stripno=15; adca=bevt->adc[30]; adcb=bevt->adc[31];}

  if (maxchan>32) std::cout << "maxchan is bogus" << std::endl;

  return(stripno);

}


int sbndaq::CRTAframeReco::getstrip(int ihit) 
{

  int thisadc, maxadc, maxchan;
  maxadc=adc0[ihit]; maxchan=0; thisadc=adc1[ihit]; 
  adca=maxadc; adcb=thisadc;
  int stripno=0; 
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=1;  }
  thisadc=adc2.at(ihit);
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=2; stripno=1; adca=adc2.at(ihit); adcb=adc3.at(ihit);}
  thisadc=adc3.at(ihit);
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=3; stripno=1; adca=adc2.at(ihit); adcb=adc3.at(ihit);}
  thisadc=adc4.at(ihit);
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=4; stripno=2; adca=adc4.at(ihit); adcb=adc5.at(ihit);}
  thisadc=adc5.at(ihit);
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=5;  stripno=2; adca=adc4.at(ihit); adcb=adc5.at(ihit);}
  thisadc=adc6.at(ihit);
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=6; stripno=3; adca=adc6.at(ihit); adcb=adc7.at(ihit);}
  thisadc=adc7.at(ihit);
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=7;  stripno=3; adca=adc6.at(ihit); adcb=adc7.at(ihit);}
  thisadc=adc8.at(ihit);
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=8;  stripno=4; adca=adc8.at(ihit); adcb=adc9.at(ihit);}
  thisadc=adc9.at(ihit);
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=9;  stripno=4; adca=adc8.at(ihit); adcb=adc9.at(ihit);}
  thisadc=adc10.at(ihit);
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=10;  stripno=5; adca=adc10.at(ihit); adcb=adc11.at(ihit);}
  thisadc=adc11.at(ihit);
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=11;  stripno=5; adca=adc10.at(ihit); adcb=adc11.at(ihit);}
  thisadc=adc12.at(ihit);
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=12;  stripno=6; adca=adc12.at(ihit); adcb=adc13.at(ihit);}
  thisadc=adc13.at(ihit);
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=13;  stripno=6; adca=adc12.at(ihit); adcb=adc13.at(ihit);}
  thisadc=adc14.at(ihit);
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=14;  stripno=7; adca=adc14.at(ihit); adcb=adc15.at(ihit);}
  thisadc=adc15.at(ihit);
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=15;  stripno=7; adca=adc14.at(ihit); adcb=adc15.at(ihit);}
  thisadc=adc16.at(ihit);
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=16;  stripno=8; adca=adc16.at(ihit); adcb=adc17.at(ihit);}
  thisadc=adc17.at(ihit);
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=17;  stripno=8; adca=adc16.at(ihit); adcb=adc17.at(ihit);}
  thisadc=adc18.at(ihit);
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=18;  stripno=9; adca=adc18.at(ihit); adcb=adc19.at(ihit);}
  thisadc=adc19.at(ihit);
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=19;  stripno=9; adca=adc18.at(ihit); adcb=adc19.at(ihit);}
  thisadc=adc20.at(ihit);
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=20;  stripno=10; adca=adc20.at(ihit); adcb=adc21.at(ihit);}
  thisadc=adc21.at(ihit);
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=21;  stripno=10; adca=adc20.at(ihit); adcb=adc21.at(ihit);}
  thisadc=adc22.at(ihit);
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=22;  stripno=11; adca=adc22.at(ihit); adcb=adc23.at(ihit);}
  thisadc=adc23.at(ihit);
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=23;  stripno=11; adca=adc22.at(ihit); adcb=adc23.at(ihit);}
  thisadc=adc24.at(ihit);
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=24;  stripno=12; adca=adc24.at(ihit); adcb=adc25.at(ihit);}
  thisadc=adc25.at(ihit);
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=25;  stripno=12; adca=adc24.at(ihit); adcb=adc25.at(ihit);}
  thisadc=adc26.at(ihit);
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=26;  stripno=13; adca=adc26.at(ihit); adcb=adc27.at(ihit);}
  thisadc=adc27.at(ihit);
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=27;  stripno=13; adca=adc26.at(ihit); adcb=adc27.at(ihit);}
  thisadc=adc28.at(ihit);
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=28;  stripno=14; adca=adc28.at(ihit); adcb=adc29.at(ihit);}
  thisadc=adc29.at(ihit);
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=29;  stripno=14; adca=adc28.at(ihit); adcb=adc29.at(ihit);}
  thisadc=adc30.at(ihit);
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=30;  stripno=15; adca=adc30.at(ihit); adcb=adc31.at(ihit);}
  thisadc=adc31.at(ihit);
  if (thisadc>maxadc) {maxadc=thisadc ; maxchan=31;  stripno=15; adca=adc30.at(ihit); adcb=adc31.at(ihit);}

  if (maxchan>32) std::cout << "maxchan is bogus" << std::endl;

  return(stripno);

}

void sbndaq::CRTAframeReco::analyze_bern_fragment(artdaq::Fragment & frag)  {
  uint64_t dt = 300; //300 ns
  
  BernCRTFragmentV2 bern_fragment(frag);
  const BernCRTFragmentMetadataV2* md = bern_fragment.metadata();
  //std::cout<<"hits in fragment: "<<md->hits_in_fragment()<<std::endl;
  for(unsigned int iHit = 0; iHit < md->hits_in_fragment(); iHit++) {
    BernCRTHitV2 const* bevt = bern_fragment.eventdata(iHit);
    
    timestamp.push_back(                 bevt->timestamp);
    
    uint64_t stamp = bevt->timestamp;
    //uint64_t t_low = 1692301830000000000;
    //uint64_t t_high = 1692305430000000000;
    //uint64_t bin = stamp*dt/(t_high-t_low);

    // TODO Check that this is exactly what we want !!!
    uint64_t bin = (stamp - t_low)/dt;
    
  
    //if(bin <= 1692301930000000000){
      //std::cout<<"bin: " << bin << " t_low: " << t_low << " stamp: " << stamp <<std::endl;
    //}

    if (stamp < t_low) {
      std::cout << std::endl;
      std::cout << "WARNING !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
      std::cout << "Smaller TimeStamp than t_low rest to 0" << stamp << std::endl;
      std::cout << std::endl;
    }
    if (stamp < 100) {
      std::cout << std::endl;
      std::cout << "Extra weird stamp " << stamp << std::endl;
      std::cout << std::endl;
    }

    //std::cout<<"mac5 " << (int)(md->MAC5()) << std::endl;
    
    
    //std::vector<sbndaq::BernCRTHitV2 const*> temp = window[bin];
    //window[bin].push_back(bevt);
    // push the mac5 address from the metadata for this BernCRTHitV2
    mac5_window[bin].push_back(md->MAC5());
    window_timestamps[bin].push_back(bevt->timestamp);  
    window_flags[bin].push_back((int)(bevt->flags));

    int strip_num = getstrip_reco(bevt);
    window_adcA[bin].push_back(adca);
    window_adcB[bin].push_back(adcb);
    window_strip_num[bin].push_back(strip_num);   

    //temp.push_back(bevt);
    //std::cout<<"pushed window"<<std::endl;
    
    if (fcrt_keepall) {
      //metadata
      sequence_id.push_back(frag.sequenceID());
      run_start_time.push_back(            md->run_start_time());
      this_poll_start.push_back(           md->this_poll_start());
      this_poll_end.push_back(             md->this_poll_end());
      last_poll_start.push_back(           md->last_poll_start());
      last_poll_end.push_back(             md->last_poll_end());
      system_clock_deviation.push_back(    md->system_clock_deviation());
      feb_hits_in_poll.push_back(          md->hits_in_poll());
      feb_hits_in_fragment.push_back(      md->hits_in_fragment());
      //event info
      lostcpu.push_back(                   bevt->lostcpu);
      lostfpga.push_back(                  bevt->lostfpga);
      feb_hit_number.push_back(            bevt->feb_hit_number);
      timestamp.push_back(                 bevt->timestamp);
      last_accepted_timestamp.push_back(   bevt->last_accepted_timestamp);
      lost_hits.push_back(                 bevt->lost_hits);
    }
    
    mac5.push_back(                      md->MAC5());
    flags.push_back(                     bevt->flags);
    ts0.push_back(                       bevt->ts0);
    ts1.push_back(                       bevt->ts1);
    coinc.push_back(                     bevt->coinc);    
    adc0.push_back(                      bevt->adc[0]);
    adc1.push_back(                      bevt->adc[1]);
    adc2.push_back(                      bevt->adc[2]);
    adc3.push_back(                      bevt->adc[3]);
    adc4.push_back(                      bevt->adc[4]);
    adc5.push_back(                      bevt->adc[5]);
    adc6.push_back(                      bevt->adc[6]);
    adc7.push_back(                      bevt->adc[7]);
    adc8.push_back(                      bevt->adc[8]);
    adc9.push_back(                      bevt->adc[9]);
    adc10.push_back(                      bevt->adc[10]);
    adc11.push_back(                      bevt->adc[11]);
    adc12.push_back(                      bevt->adc[12]);
    adc13.push_back(                      bevt->adc[13]);
    adc14.push_back(                      bevt->adc[14]);
    adc15.push_back(                      bevt->adc[15]);
    adc16.push_back(                      bevt->adc[16]);
    adc17.push_back(                      bevt->adc[17]);
    adc18.push_back(                      bevt->adc[18]);
    adc19.push_back(                      bevt->adc[19]); 
    adc20.push_back(                      bevt->adc[20]);
    adc21.push_back(                      bevt->adc[21]);
    adc22.push_back(                      bevt->adc[22]);
    adc23.push_back(                      bevt->adc[23]);
    adc24.push_back(                      bevt->adc[24]);
    adc25.push_back(                      bevt->adc[25]);
    adc26.push_back(                      bevt->adc[26]);
    adc27.push_back(                      bevt->adc[27]);
    adc28.push_back(                      bevt->adc[28]);
    adc29.push_back(                      bevt->adc[29]);
    adc30.push_back(                      bevt->adc[30]);
    adc31.push_back(                      bevt->adc[31]);
    maxadc.push_back(                     *std::max_element(bevt->adc,bevt->adc+32));
    
    if (fverbose) {
      std::cout <<   " mac5             "   <<    (int)(md->MAC5())                   << std::endl;
      std::cout << " run_start_time      "    <<  md->run_start_time()              << std::endl;
      std::cout << " this_poll_start     "    << md->this_poll_start()            << std::endl;
      std::cout << " this_poll_end        "   << md->this_poll_end()              << std::endl;
      std::cout << "  last_poll_start      "   << md->last_poll_start()            << std::endl;
      std::cout << "  last_poll_end          " << md->last_poll_end()              << std::endl;
      std::cout << "  system_clock_deviation " << md->system_clock_deviation()     << std::endl;
      std::cout << "  feb_hits_in_poll       " << md->hits_in_poll()           << std::endl;
      std::cout << "  feb_hits_in_fragment  "  << md->hits_in_fragment()       << std::endl;
      std::cout << "  flags  "                << (int)(bevt->flags)                   << std::endl;
      std::cout << "lostcpu         "        << bevt->lostcpu                 << std::endl;
      std::cout <<     "lostfpga    "<<     bevt->lostfpga     << std::endl;
      std::cout <<     "ts0         "<<     bevt->ts0          << std::endl;
      std::cout <<     "ts1                    "<<     bevt->ts1             << std::endl;
      std::cout <<     "coinc                  "<<     bevt->coinc                   << std::endl;
      std::cout <<     "feb_hit_number         "<<     bevt->feb_hit_number          << std::endl;
      std::cout <<     "timestamp              "<<     bevt->timestamp               << std::endl;
      std::cout <<     "last_accepted_timestamp"<<     bevt->last_accepted_timestamp << std::endl;
      std::cout <<     "lost_hits              "<<     bevt->lost_hits               << std::endl;
      

      for(int ch=0; ch<32; ch++) 
	std::cout << "channel " << ch << " has adc value " << bevt->adc[ch] << std::endl;
      std::cout << "max ADC value   " << maxadc.back() << std::endl;
    }// if verbose
    
    
  }// end loop over fragments
  
}//analyze_bern_fragment



DEFINE_ART_MODULE(sbndaq::CRTAframeReco)
