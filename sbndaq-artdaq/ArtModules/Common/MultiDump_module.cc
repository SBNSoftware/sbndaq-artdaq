//-------------------------------------------------
//---------------------------------------

////////////////////////////////////////////////////////////////////////
// Class:       MultiDump
// Module Type: analyzer
// File:        MultiDump_module.cc
// Description: Prints out information about each event.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"

#include "canvas/Utilities/Exception.h"

#include "sbndaq-artdaq-core/Overlays/Common/CAENV1730Fragment.hh"
#include "sbndaq-artdaq-core/Overlays/Common/WhiteRabbitFragment.hh"
#include "sbndaq-artdaq-core/Overlays/Common/BernCRTFragmentV2.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "artdaq-core/Data/Fragment.hh"
#include "artdaq-core/Data/ContainerFragment.hh"

// Including the PTB fragment
#include "sbndaq-artdaq-core/Overlays/SBND/PTBFragment.hh"

//#include "art/Framework/Services/Optional/TFileService.h" //before art_root_io transition
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
#include <iostream>
#include <bitset>

namespace sbndaq {
  class MultiDump;
}

/**************************************************************************************************/

class sbndaq::MultiDump : public art::EDAnalyzer {

public:
  struct Config {
    //--one atom for each parameter
    fhicl::Atom<bool> include_caen {
      fhicl::Name("include_caen"),
      fhicl::Comment("look for caen 1730 fragments true/false")
    };
    fhicl::Atom<int> Shift {
      fhicl::Name("shift_fragment_id"),
      fhicl::Comment("Number to subtract to the fragment_id")
    };
    fhicl::Atom<bool> include_wr {
      fhicl::Name("include_wr"),
      fhicl::Comment("look for wr dio fragments true/false")
    };
    fhicl::Atom<int> window_wr {
      fhicl::Name("window_wr"),
      fhicl::Comment("window around CAEN TTT to keep WR timestamps.  integer values from 0 to 10^9")
    };
    fhicl::Atom<bool> include_berncrt {
      fhicl::Name("include_berncrt"),
      fhicl::Comment("look for bern CRT V2 fragments true/false")
    };
    fhicl::Atom<bool> verbose {
      fhicl::Name("verbose"),
      fhicl::Comment("lots of text output if set to true")
    };
// including one fcl paramenter for PTB on the Config structure
    fhicl::Atom<bool> include_ptb {
      fhicl::Name("include_ptb"),
      fhicl::Comment("look for ptb fragments")
    };
  }; //--configuration
  using Parameters = art::EDAnalyzer::Table<Config>;

  explicit MultiDump(Parameters const & pset);
  virtual ~MultiDump();

  void analyze(const art::Event& evt) override;
  void beginJob() override;
  void endJob() override;

private:

  void analyze_caen_fragment(artdaq::Fragment & frag);
  void analyze_wr_fragment(artdaq::Fragment & frag);
  void analyze_bern_fragment(artdaq::Fragment & frag);
  
// include ptb private classes
  void extract_triggers(artdaq::Fragment & frag);
  void reset_ptb_variables();

  //--default values
  uint32_t nChannels;//    = 16;
  uint32_t Ttt_DownSamp;// =  4;
 /* the waveforms are sampled at 500MHz sampling. The trigger timestamp is
                               * sampled 4 times slower than input channels*/

  TNtuple* nt_header;

  TH1F*    hEventCounter;
  TH1D*    hTriggerTimeTag;
  TH1F*    h_wvfm_ev0_ch0;

  TTree* fEventTree;
  int fRun;
  art::EventNumber_t fEvent;
  std::vector<uint64_t>  fTicksVec;
  std::vector< std::vector<uint16_t> >  fWvfmsVec;
  std::vector<int> fPMT_ch0;
  std::vector<int> fPMT_ch1;
  std::vector<int> fPMT_ch2;
  std::vector<int> fPMT_ch3;
  std::vector<int> fPMT_ch4;
  std::vector<int> fPMT_ch5;
  std::vector<int> fPMT_ch6;
  std::vector<int> fPMT_ch7;
  std::vector<int> fPMT_ch8;
  std::vector<int> fPMT_ch9;
  std::vector<int> fPMT_ch10;
  std::vector<int> fPMT_ch11;
  std::vector<int> fPMT_ch12;
  std::vector<int> fPMT_ch13;
  std::vector<int> fPMT_ch14;
  std::vector<int> fPMT_ch15;

  int fnstamps0;
  int fnstamps1;
  int fnstamps2;
  int fnstamps3;
  int fnstamps4;
  std::vector<float> fWR_ch0;
  std::vector<float> fWR_ch1;
  std::vector<float> fWR_ch2;
  std::vector<float> fWR_ch3;
  std::vector<float> fWR_ch4;
  bool firstEvt = true;
  int TTT;  // will be set to value in CAEN fragement header
  int TTT_ns;
  uint64_t caen_frag_ts;

  int first_wr_ch0;
  int first_wr_ch1;
  int first_wr_ch2;
  int first_wr_ch3;
  int first_wr_ch4;

  //BernCRTV2 Information

  uint16_t flags;
  uint16_t lostcpu;
  uint16_t lostfpga;
  uint32_t ts0;
  uint32_t ts1;
  uint16_t adc[32];
  uint32_t coinc;

  uint64_t  feb_hit_number          = 0; //hit counter for individual FEB, including hits lost in FEB or fragment generator
  uint64_t  timestamp               = 0; //absolute timestamp
  uint64_t  last_accepted_timestamp = 0; //timestamp of previous accepted hit
  uint32_t  lost_hits               = 0; //number of lost hits from the previous one


  //metadata
  uint8_t  mac5; //last 8 bits of FEB mac5 address
  uint64_t  run_start_time;
  uint64_t  this_poll_start;
  uint64_t  this_poll_end;
  uint64_t  last_poll_start;
  uint64_t  last_poll_end;
  int32_t   system_clock_deviation;
  uint32_t  feb_hits_in_poll;
  uint32_t  feb_hits_in_fragment;

  //information from fragment header
  uint32_t  sequence_id;


  bool finclude_caen;
  int fShift;
  bool finclude_wr;
  int fWindow;
  bool finclude_berncrt;
  bool fverbose;
// include ptb on fragment header
  bool finclude_ptb;

// including ptb information on the tree
  bool unknown_or_error_word; // flag to indicate the event has 
  int ts_word_count;
  int hlt_word_count;
  uint64_t ptb_frag_ts;
  std::vector<uint64_t> llt_trigger;
  std::vector<uint64_t> llt_ts;
  std::vector<uint64_t> hlt_trigger;
  std::vector<uint64_t> hlt_ts;
  std::vector<uint16_t> crt_status;
  std::vector<uint16_t> beam_status;
  std::vector<uint16_t> mtca_status;
  std::vector<uint16_t> nim_status;
  std::vector<uint32_t> auxpds_status;
  std::vector<uint64_t> chan_stat_ts;

}; //--class MultiDump


sbndaq::MultiDump::MultiDump(MultiDump::Parameters const& pset): art::EDAnalyzer(pset)
{
  finclude_caen = pset().include_caen();
  fShift = pset().Shift();
  finclude_wr = pset().include_wr();
  fWindow = pset().window_wr();
  if (fWindow<0 || fWindow>1000000000) {
    fWindow=1000000000;
    std::cout << "Bad value for fcl parameter window_wr=" << fWindow << "  setting to default value 1000000000 nsec = 1 sec"
	    << std::endl;
  }
  fverbose = pset().verbose();
  finclude_berncrt = pset().include_berncrt();
  // include ptb
  finclude_ptb = pset().include_ptb();
  /****************************************************************/
}

void sbndaq::MultiDump::beginJob()
{
  art::ServiceHandle<art::TFileService> tfs;
  nt_header       = tfs->make<TNtuple>("nt_header","Multi Header Ntuple","art_ev:caen_ev:caenv_ev_tts");
  /************************************************************************************************/
  hEventCounter   = tfs->make<TH1F>("hEventCounter","Event Counter Histogram",10000,0,10000);
  hTriggerTimeTag = tfs->make<TH1D>("hTriggerTimeTag","Trigger Time Tag Histogram",12500,0,125000000);
  h_wvfm_ev0_ch0  = tfs->make<TH1F>("h_wvfm_ev0_ch0","Waveform",2000,0,2000);
  /************************************************************************************************/
  //--make tree to store the channel waveform info:
  fEventTree = tfs->make<TTree>("events","waveform tree");
  fEventTree->Branch("fRun",&fRun,"fRun/I");
  fEventTree->Branch("fEvent",&fEvent,"fEvent/I");
  if (finclude_caen) {
    fEventTree->Branch("TTT_ns",&TTT_ns,"TTT_ns/I");
    fEventTree->Branch("caen_frag_ts",&caen_frag_ts,"caen_frag_ts/l");
    //fEventTree->Branch("fTicksVec",&fTicksVec);
    //fEventTree->Branch("fWvfmsVec",&fWvfmsVec);
    fEventTree->Branch("fPMT_ch0",&fPMT_ch0);
    fEventTree->Branch("fPMT_ch1",&fPMT_ch1);
    fEventTree->Branch("fPMT_ch2",&fPMT_ch2);
    fEventTree->Branch("fPMT_ch3",&fPMT_ch3);
    fEventTree->Branch("fPMT_ch4",&fPMT_ch4);
    fEventTree->Branch("fPMT_ch5",&fPMT_ch5);
    fEventTree->Branch("fPMT_ch6",&fPMT_ch6);
    fEventTree->Branch("fPMT_ch7",&fPMT_ch7);
    fEventTree->Branch("fPMT_ch8",&fPMT_ch8);
    fEventTree->Branch("fPMT_ch9",&fPMT_ch9);
    fEventTree->Branch("fPMT_ch10",&fPMT_ch10);
    fEventTree->Branch("fPMT_ch11",&fPMT_ch11);
    fEventTree->Branch("fPMT_ch12",&fPMT_ch12);
    fEventTree->Branch("fPMT_ch13",&fPMT_ch13);
    fEventTree->Branch("fPMT_ch14",&fPMT_ch14);
    fEventTree->Branch("fPMT_ch15",&fPMT_ch15);
  }
  if (finclude_wr) {
    // fEventTree->Branch("nstamps0",&fnstamps0,"nstamps0/I");
    // fEventTree->Branch("nstamps1",&fnstamps1,"nstamps1/I");
    // fEventTree->Branch("nstamps2",&fnstamps2,"nstamps2/I");
    // fEventTree->Branch("nstamps3",&fnstamps3,"nstamps3/I");
    // fEventTree->Branch("nstamps4",&fnstamps4,"nstamps4/I");
    fEventTree->Branch("fWR_ch0",&fWR_ch0);
    fEventTree->Branch("fWR_ch1",&fWR_ch1);
    fEventTree->Branch("fWR_ch2",&fWR_ch2);
    fEventTree->Branch("fWR_ch3",&fWR_ch3);
    fEventTree->Branch("fWR_ch4",&fWR_ch4);
  }
  if (finclude_berncrt) {
    fEventTree->Branch("flags",         &flags,         "flags/s");
    fEventTree->Branch("lostcpu",       &lostcpu,       "lostcpu/s");
    fEventTree->Branch("lostfpga",      &lostfpga,      "lostfpga/s");
    fEventTree->Branch("ts0",           &ts0,           "ts0/i");
    fEventTree->Branch("ts1",           &ts1,           "ts1/i");
    fEventTree->Branch("adc",           &adc,           "adc[32]/s");
    fEventTree->Branch("coinc",         &coinc,         "coinc/i");
    fEventTree->Branch("feb_hit_number",&feb_hit_number,"feb_hit_number/l");
    fEventTree->Branch("timestamp",     &timestamp,     "timestamp/l");
    fEventTree->Branch("last_accepted_timestamp",&last_accepted_timestamp, "last_accepted_timestamp/l");
    fEventTree->Branch("lost_hits",     &lost_hits,     "lost_hits/s");

  //metadata
    fEventTree->Branch("mac5",                      &mac5,                        "mac5/b");
    fEventTree->Branch("run_start_time",            &run_start_time,              "run_start_time/l");
    fEventTree->Branch("this_poll_start",           &this_poll_start,             "this_poll_start/l");
    fEventTree->Branch("this_poll_end",             &this_poll_end,               "this_poll_end/l");
    fEventTree->Branch("last_poll_start",           &last_poll_start,             "last_poll_start/l");
    fEventTree->Branch("last_poll_end",             &last_poll_end,               "last_poll_end/l");
    fEventTree->Branch("system_clock_deviation",    &system_clock_deviation,      "system_clock_deviation/I");
    fEventTree->Branch("feb_hits_in_poll",          &feb_hits_in_poll,            "feb_hits_in_poll/i");
    fEventTree->Branch("feb_hits_in_fragment",      &feb_hits_in_fragment,        "feb_hits_in_fragment/i");

    fEventTree->Branch("sequence_id",               &sequence_id,                 "sequence_id/i");
  }
  // include ptb branches
  if(finclude_ptb){
    fEventTree->Branch("unknown_or_error_word", &unknown_or_error_word);
    fEventTree->Branch("ts_word_count", &ts_word_count);
    fEventTree->Branch("hlt_word_count", &hlt_word_count);
    fEventTree->Branch("ptb_frag_ts", &ptb_frag_ts, "ptb_frag_ts/l");
    // Trigger words and TS
    fEventTree->Branch("hlt_trigger", &hlt_trigger);
    fEventTree->Branch("hlt_ts",      &hlt_ts);
    fEventTree->Branch("llt_trigger", &llt_trigger);
    fEventTree->Branch("llt_ts",      &llt_ts);
    // Channel status words & TS
    fEventTree->Branch("beam_status",   &beam_status);
    fEventTree->Branch("crt_status",    &crt_status);
    fEventTree->Branch("mtca_status",   &mtca_status);
    fEventTree->Branch("nim_status",    &nim_status);
    fEventTree->Branch("auxpds_status", &auxpds_status);
    fEventTree->Branch("chan_stat_ts",  &chan_stat_ts);
  }

}

void sbndaq::MultiDump::endJob()
{
  if (fverbose)  std::cout << "Ending MultiDump...\n";
}


sbndaq::MultiDump::~MultiDump()
{
}


void sbndaq::MultiDump::analyze(const art::Event& evt)
{
  // Reset PTB variables
  reset_ptb_variables();
  fRun = evt.run();
  fEvent = evt.event();
  std::cout << "Run " << fRun << " event " << fEvent << std::endl;

  /************************************************************************************************/
  // need to clear tree variables at the beginning of the event
  fnstamps0=0;   fnstamps1=0;   fnstamps2=0;   fnstamps3=0;   fnstamps4=0;
  fWR_ch0.clear();   fWR_ch1.clear();   fWR_ch2.clear();   fWR_ch3.clear();   fWR_ch4.clear();
  fPMT_ch0.clear();   fPMT_ch1.clear();   fPMT_ch2.clear(); fPMT_ch3.clear(); fPMT_ch4.clear();
  fPMT_ch5.clear();   fPMT_ch6.clear();   fPMT_ch7.clear(); fPMT_ch8.clear(); fPMT_ch9.clear();
  fPMT_ch10.clear();   fPMT_ch11.clear();   fPMT_ch12.clear(); fPMT_ch13.clear(); fPMT_ch14.clear();
  fPMT_ch15.clear();
  /************************************************************************************************/

  //  Note that this code expects exactly 1 CAEN fragment per event
  TTT_ns=0;  // will be set to value in CAEN fragement header
  caen_frag_ts = 0;

  first_wr_ch0=0;
  first_wr_ch1=0;
  first_wr_ch2=0;
  first_wr_ch3=0;
  first_wr_ch4=0;


  std::vector<art::Handle<artdaq::Fragments>> fragmentHandles;

  #if ART_HEX_VERSION < 0x30900
          evt.getManyByType(fragmentHandles);
  #else
          fragmentHandles = evt.getMany<std::vector<artdaq::Fragment>>();
  #endif

  /************************************************************************************************/
  if (finclude_caen) {
  for (auto handle : fragmentHandles) {
    if (!handle.isValid() || handle->size() == 0) continue;

    if (handle->front().type() == artdaq::Fragment::ContainerFragmentType) {
      //Container fragment
      for (auto cont : *handle) {
	artdaq::ContainerFragment contf(cont);
	if (contf.fragment_type()==sbndaq::detail::FragmentType::CAENV1730) {
  if (fverbose) 	  std::cout << "    Found " << contf.block_count() << " CAEN Fragments in container " << std::endl;
	  fWvfmsVec.resize(16*contf.block_count());
	  for (size_t ii = 0; ii < contf.block_count(); ++ii)
	  	analyze_caen_fragment(*contf[ii].get());
	}
      }
    }
    else {
      //normal fragment
      if (handle->front().type()==sbndaq::detail::FragmentType::CAENV1730) {
  if (fverbose) 	std::cout << "   found normal caen fragments " << handle->size() << std::endl;
	fWvfmsVec.resize(16*handle->size());
	for (auto frag : *handle)
	  analyze_caen_fragment(frag);
      }
    }
  } // loop over frag handles
  }  // if include caen


  if (finclude_wr) {
    for (auto handle : fragmentHandles) {
      if (!handle.isValid() || handle->size() == 0) continue;

      if (handle->front().type() == artdaq::Fragment::ContainerFragmentType) {
	//Container fragment
	for (auto cont : *handle) {
	  artdaq::ContainerFragment contf(cont);
	  if (contf.fragment_type()==sbndaq::detail::FragmentType::WhiteRabbit) {
  if (fverbose) 	  std::cout << "    Found " << contf.block_count() << " WR Fragments in container " << std::endl;
	  for (size_t ii = 0; ii < contf.block_count(); ++ii)
	    analyze_wr_fragment(*contf[ii].get());
	}
      }
    }
    else {
      //normal fragment
      if (handle->front().type()==sbndaq::detail::FragmentType::WhiteRabbit) {
	for (auto frag : *handle)
	  analyze_wr_fragment(frag);
      }
    }
  } // loop over frag handles
  /************************************************************************************************/

  std::cout << " WR ch 0 " << fnstamps0 << " WR ch 1 " << fnstamps1 << " WR ch 2 " << fnstamps2 << " WR ch 3 " <<
    fnstamps3 << " WR ch 4 " << fnstamps4 << std::endl;

  } // if (include_wr)


  if (finclude_berncrt){

    std::vector<art::Handle<artdaq::Fragments>> fragmentHandles;

  #if ART_HEX_VERSION < 0x30900
          evt.getManyByType(fragmentHandles);
  #else
          fragmentHandles = evt.getMany<std::vector<artdaq::Fragment>>();
  #endif

  	for (auto handle : fragmentHandles) {
      if (!handle.isValid() || handle->size() == 0)
        continue;

      if (handle->front().type() == artdaq::Fragment::ContainerFragmentType) {
        //Container fragment
        for (auto cont : *handle) {
          artdaq::ContainerFragment contf(cont);
          if (contf.fragment_type() != sbndaq::detail::FragmentType::BERNCRTV2)
            continue;
          for (size_t ii = 0; ii < contf.block_count(); ++ii)
            analyze_bern_fragment(*contf[ii].get());
        }
      }
      else {
        //normal fragment
        if (handle->front().type() != sbndaq::detail::FragmentType::BERNCRTV2) continue;
        for (auto frag : *handle)
          analyze_bern_fragment(frag);
      }
    }

  } // if include_berncrt


  /************************************************************************************************/
  // Save PTB data in tree
  if(finclude_ptb) {
    std::vector<art::Handle<artdaq::Fragments>> fragmentHandles;

    #if ART_HEX_VERSION < 0x30900
        evt.getManyByType(fragmentHandles);
    #else
        fragmentHandles = evt.getMany<std::vector<artdaq::Fragment>>();
    #endif

    for (auto handle : fragmentHandles) {
      if (!handle.isValid() || handle->size() == 0) continue;
                                                                                    
      if (handle->front().type() == artdaq::Fragment::ContainerFragmentType) {
        //Container fragment
        for (auto cont : *handle) {
          artdaq::ContainerFragment contf(cont);
          if (contf.fragment_type() != sbndaq::detail::FragmentType::PTB) continue;
          for (size_t ii = 0; ii < contf.block_count(); ++ii) {
            extract_triggers(*contf[ii].get());
          }
        }
      }
      else {
        //normal fragment
        if (handle->front().type() != sbndaq::detail::FragmentType::PTB) continue;
        for (auto frag : *handle) {
          extract_triggers(frag);
        }
      }
    }
    // Fill the ROOT file 
    fEventTree->Fill();
  } // if includes ptb

}


void sbndaq::MultiDump::analyze_wr_fragment(artdaq::Fragment & frag)  {



  if (fverbose)       std::cout <<  "     timestamp is  " << frag.timestamp() << std::endl;
  if (fverbose)       std::cout <<  "     seq ID is " << frag.sequenceID() << std::endl;


  const WhiteRabbitEvent *event_ptr = reinterpret_cast<WhiteRabbitEvent const*>(frag.dataBeginBytes());
  timespec sysTime=event_ptr->systemTime;
  if (fverbose)      std::cout << "systime  " << sysTime.tv_sec << " " << sysTime.tv_nsec << std::endl;
  WhiteRabbitData fragdata=event_ptr->data;

  // each WR fragment has data from only one channel. The fragments are not always in time order
  if (fverbose)       std::cout << "WR: command " << fragdata.command << std::endl;
  if (fverbose)       std::cout << "WR: channel " << fragdata.channel << std::endl;
  if (fverbose)       std::cout << "WR: value " << fragdata.value << std::endl;
  if (fverbose)       std::cout << "WR: flags " << fragdata.flags << std::endl;
  if (fverbose)       std::cout << "WR: number of time stamps in this fragment " << fragdata.nstamp << std::endl;
  if (fragdata.channel==0)  fnstamps0+=fragdata.nstamp;
  if (fragdata.channel==1)	fnstamps1+=fragdata.nstamp;
  if (fragdata.channel==2)	fnstamps2+=fragdata.nstamp;
  if (fragdata.channel==3)  fnstamps3+=fragdata.nstamp;
  if (fragdata.channel==4)	fnstamps4+=fragdata.nstamp;

  std::cout << "WR timestamp count " << fnstamps0 << " + " << fnstamps1 << " + " << fnstamps2 << " + " << fnstamps3 << " + " << fnstamps4 << std::endl;

  for (int i=0;i<(int)fragdata.nstamp;++i) {


    int diff = 0;
    uint this_time_sec = fragdata.timeStamp[i].tv_sec;
    uint this_time_ns = fragdata.timeStamp[i].tv_nsec;
    // if (TTT_ns>this_time) {
    //    std::cout << this_time_sec << std::endl;
    diff = TTT_ns-this_time_ns;
    if (diff>500000000) diff = 1000000000-diff;
    else if (diff<-500000000) diff = 1000000000+diff;
    // }
    // else {

    //   diff = this_time-TTT_ns;
    //   if (diff>500000000) diff = 1000000000-diff;
    // }
    //	if (diff>500000000) std::cout<< "diff " << diff << "this_time_ns " << this_time_ns << " TTT_ns " << TTT_ns << std::endl;

    if (fabs(diff)<fWindow && fragdata.channel==1) 	 {
      if (first_wr_ch1==0) first_wr_ch1=fragdata.timeStamp[i].tv_sec;
      int secdiff=0;
      if (fragdata.timeStamp[i].tv_sec>first_wr_ch1) secdiff=this_time_sec-first_wr_ch1;
      fWR_ch1.emplace_back((int)this_time_ns+1e9*secdiff);
      if (fverbose) 	  std::cout << " Event " << fEvent << " PMT" <<
			    " Timestamp " << i << "  : " << std::setw(16) << fragdata.timeStamp[i].tv_sec <<
			    " " << std::setw(9) << fragdata.timeStamp[i].tv_nsec <<
			    " TTT " << std::setw(9) << TTT_ns <<
			    " TTT diff  " << std::setw(9)  << diff << std::endl;
    }
    if (fabs(diff)< 50000000 && fragdata.channel==2) 	{
      if (first_wr_ch2==0) first_wr_ch2=fragdata.timeStamp[i].tv_sec;
      int secdiff=0;
      if (fragdata.timeStamp[i].tv_sec>first_wr_ch2) secdiff=fragdata.timeStamp[i].tv_sec-first_wr_ch2;
      fWR_ch2.emplace_back((int)fragdata.timeStamp[i].tv_nsec+1e9*secdiff);
      if (fverbose) 	  std::cout << " Event " << fEvent << " RWM" <<
			    " Timestamp " << i << "  : " << std::setw(16) << fragdata.timeStamp[i].tv_sec <<
			    " " << std::setw(9) << fragdata.timeStamp[i].tv_nsec <<
			    " TTT " << std::setw(9) << TTT_ns <<
			    " TTT diff  " << std::setw(9)  << diff << std::endl;
    }
    if (fragdata.channel==0 ) 	{
      if (first_wr_ch0==0) first_wr_ch0=fragdata.timeStamp[i].tv_sec;
      int secdiff=0;
      if (fragdata.timeStamp[i].tv_sec>first_wr_ch0) secdiff=fragdata.timeStamp[i].tv_sec-first_wr_ch0;
      fWR_ch0.emplace_back((int)fragdata.timeStamp[i].tv_nsec+1e9*secdiff);
      if (fverbose) 	  std::cout << " Event " << fEvent << " PPS" <<
			    " Timestamp " << i << "  : " << std::setw(16) << fragdata.timeStamp[i].tv_sec <<
			    " " << std::setw(9) << fragdata.timeStamp[i].tv_nsec <<
			    " TTT " << std::setw(9) << TTT_ns <<
			    " TTT diff  " << std::setw(9)  << diff << std::endl;
    }
    // if (diff<(uint)fWindow && fragdata.channel==3 )
    if ( fabs(diff)<5000000 && fragdata.channel==3) 	 {
      if (first_wr_ch3==0) first_wr_ch3=fragdata.timeStamp[i].tv_sec;
      int secdiff=0;
      if (fragdata.timeStamp[i].tv_sec>first_wr_ch3) secdiff=fragdata.timeStamp[i].tv_sec-first_wr_ch3;
      fWR_ch3.emplace_back((int)fragdata.timeStamp[i].tv_nsec+1e9*secdiff);
      if (fverbose) 	  std::cout << " Event " << fEvent << " TRIG" <<
			    " Timestamp " << i << "  : " << std::setw(16) << fragdata.timeStamp[i].tv_sec <<
			    " " << std::setw(10) << fragdata.timeStamp[i].tv_nsec <<
			    " TTT " << std::setw(10) << TTT_ns <<
			    " TTT diff  " << std::setw(10)  << diff << std::endl;
    }
  }
  std::cout << " ----------------------- " << std::endl;

}

void sbndaq::MultiDump::analyze_caen_fragment(artdaq::Fragment & frag)  {


  std::cout <<  "     timestamp is  " << frag.timestamp() << std::endl;
  std::cout <<  "     seq ID is " << frag.sequenceID() << std::endl;
  caen_frag_ts = frag.timestamp();

  CAENV1730Fragment bb(frag);
  auto const* md = bb.Metadata();
  CAENV1730Event const* event_ptr = bb.Event();
  CAENV1730EventHeader header = event_ptr->Header;

  int fragId = static_cast<int>(frag.fragmentID());
  fragId-=fShift;
  //
  if (fverbose)      std::cout << "\tFrom CAEN header, event counter is "  << header.eventCounter   << "\n";
  if (fverbose)      std::cout << "\tFrom CAEN header, triggerTimeTag is " << header.triggerTimeTag << "\n";
  if (fverbose)       std::cout << "\tFrom CAEN header, board id is "       << header.boardID       << "\n";
  if (fverbose)       std::cout << "\tFrom CAEN fragment, fragment id is "  << fragId << "\n";
  if (fverbose)       std::cout << "\tShift back, fragment id of "  << fShift << "\n";

  uint32_t t0 = header.triggerTimeTag;
  TTT = (int)t0;
  TTT_ns = t0*8;
  if (fverbose)       std::cout << "\n\tTriggerTimeTag in ns is " << TTT_ns << "\n";  // 500 MHz is 2 ns per tick
  hEventCounter->Fill(header.eventCounter);
  hTriggerTimeTag->Fill((int)t0);
  nt_header->Fill(fEvent,header.eventCounter,t0);
  nChannels = md->nChannels;
  if (fverbose)       std::cout << "\tNumber of channels: " << nChannels << "\n";

  //--get the number of 32-bit words (quad_bytes) from the header
  uint32_t ev_size_quad_bytes = header.eventSize;
  if (fverbose)       std::cout << "Event size in quad bytes is: " << ev_size_quad_bytes << "\n";
  uint32_t evt_header_size_quad_bytes = sizeof(CAENV1730EventHeader)/sizeof(uint32_t);
  uint32_t data_size_double_bytes = 2*(ev_size_quad_bytes - evt_header_size_quad_bytes);
  uint32_t wfm_length = data_size_double_bytes/nChannels;
  //--note, needs to take into account channel mask
  if (fverbose) std::cout << "Channel waveform length = " << wfm_length << "\n";

  //--store the tick value for each acquisition
  fTicksVec.resize(wfm_length);

  const uint16_t* data_begin = reinterpret_cast<const uint16_t*>(frag.dataBeginBytes()
								 + sizeof(CAENV1730EventHeader));

  const uint16_t* value_ptr =  data_begin;
  uint16_t value = 0;
  size_t ch_offset = 0;
  //--loop over channels
  for (size_t i_ch=0; i_ch<nChannels; ++i_ch){
    fWvfmsVec[i_ch+nChannels*fragId].resize(wfm_length);
    ch_offset = (size_t)(i_ch * wfm_length);

    //--loop over waveform samples
    for(size_t i_t=0; i_t<wfm_length; ++i_t){
      fTicksVec[i_t] = t0*Ttt_DownSamp + i_t;   /*timestamps, event level*/
      value_ptr = data_begin + ch_offset + i_t; /*pointer arithmetic*/
      value = *(value_ptr);
      if (i_ch == 0 && firstEvt) {
	h_wvfm_ev0_ch0->SetBinContent(i_t,value);
      }
      fWvfmsVec[i_ch+nChannels*fragId][i_t] = value;
    } //--end loop samples
    firstEvt = false;
  } //--end loop channels


  // find leading edges in waveforms
  int threshold=5000;//7800;
  int i_ch = 0; int toggle=0;
  auto this_value = fWvfmsVec[i_ch+nChannels*fragId][0];
  if (this_value<threshold) toggle=1;
  for(size_t i_t=1; i_t<wfm_length; ++i_t){
    this_value = fWvfmsVec[i_ch+nChannels*fragId][i_t];
    if (toggle==0 && this_value>threshold) {
      toggle=1;
      fPMT_ch0.emplace_back(i_t);
    }
    if (toggle==1 && this_value<threshold) toggle=0;
  }

  // find leading edges in waveforms
  //threshold=7800;
  i_ch = 1; toggle=0;
  this_value = fWvfmsVec[i_ch+nChannels*fragId][0];
  if (this_value>threshold) toggle=1;
  for(size_t i_t=1; i_t<wfm_length; ++i_t){
    this_value = fWvfmsVec[i_ch+nChannels*fragId][i_t];
    if (toggle==0 && this_value>threshold) {
      toggle=1;
      fPMT_ch1.emplace_back(i_t);
    }
    if (toggle==1 && this_value<threshold) toggle=0;
  }

  // find leading edges in waveforms
  //threshold=7000;
  i_ch = 2; toggle=0;
  this_value = fWvfmsVec[i_ch+nChannels*fragId][0];
  if (this_value>threshold) toggle=1;
  for(size_t i_t=1; i_t<wfm_length; ++i_t){
    this_value = fWvfmsVec[i_ch+nChannels*fragId][i_t];
    if (toggle==0 && this_value>threshold) {
      toggle=1;
      fPMT_ch2.emplace_back(i_t);
    }
    if (toggle==1 && this_value<threshold) toggle=0;
  }

  // find leading edges in waveforms
  //threshold=7800;
  i_ch = 3; toggle=0;
  this_value = fWvfmsVec[i_ch+nChannels*fragId][0];
  if (this_value>threshold) toggle=1;
  for(size_t i_t=1; i_t<wfm_length; ++i_t){
    this_value = fWvfmsVec[i_ch+nChannels*fragId][i_t];
    if (toggle==0 && this_value>threshold) {
      toggle=1;
      fPMT_ch3.emplace_back(i_t);
    }
    if (toggle==1 && this_value<threshold) toggle=0;
  }

  // find leading edges in waveforms
  //threshold=7000;
  i_ch = 4; toggle=0;
  this_value = fWvfmsVec[i_ch+nChannels*fragId][0];
  if (this_value>threshold) toggle=1;
  for(size_t i_t=1; i_t<wfm_length; ++i_t){
    this_value = fWvfmsVec[i_ch+nChannels*fragId][i_t];
    if (toggle==0 && this_value>threshold) {
      toggle=1;
      fPMT_ch4.emplace_back(i_t);
    }
    if (toggle==1 && this_value<threshold) toggle=0;
  }

  // find leading edges in waveforms
  //threshold=7000;
  i_ch = 5; toggle=0;
  this_value = fWvfmsVec[i_ch+nChannels*fragId][0];
  if (this_value>threshold) toggle=1;
  for(size_t i_t=1; i_t<wfm_length; ++i_t){
    this_value = fWvfmsVec[i_ch+nChannels*fragId][i_t];
    if (toggle==0 && this_value>threshold) {
      toggle=1;
      fPMT_ch5.emplace_back(i_t);
    }
    if (toggle==1 && this_value<threshold) toggle=0;
  }

  // find leading edges in waveforms
  //threshold=7800;
  i_ch = 6; toggle=0;
  this_value = fWvfmsVec[i_ch+nChannels*fragId][0];
  if (this_value>threshold) toggle=1;
  for(size_t i_t=1; i_t<wfm_length; ++i_t){
    this_value = fWvfmsVec[i_ch+nChannels*fragId][i_t];
    if (toggle==0 && this_value>threshold) {
      toggle=1;
      fPMT_ch6.emplace_back(i_t);
    }
    if (toggle==1 && this_value<threshold) toggle=0;
  }

  // find leading edges in waveforms
  //threshold=7800;
  i_ch = 7; toggle=0;
  this_value = fWvfmsVec[i_ch+nChannels*fragId][0];
  if (this_value>threshold) toggle=1;
  for(size_t i_t=1; i_t<wfm_length; ++i_t){
    this_value = fWvfmsVec[i_ch+nChannels*fragId][i_t];
    if (toggle==0 && this_value>threshold) {
      toggle=1;
      fPMT_ch7.emplace_back(i_t);
    }
    if (toggle==1 && this_value<threshold) toggle=0;
  }

  // find leading edges in waveforms
  //threshold=7800;
  i_ch = 8; toggle=0;
  this_value = fWvfmsVec[i_ch+nChannels*fragId][0];
  if (this_value>threshold) toggle=1;
  for(size_t i_t=1; i_t<wfm_length; ++i_t){
    this_value = fWvfmsVec[i_ch+nChannels*fragId][i_t];
    if (toggle==0 && this_value>threshold) {
      toggle=1;
      fPMT_ch8.emplace_back(i_t);
    }
    if (toggle==1 && this_value<threshold) toggle=0;
  }

  // find leading edges in waveforms
  //threshold=7800;
  i_ch = 9; toggle=0;
  this_value = fWvfmsVec[i_ch+nChannels*fragId][0];
  if (this_value>threshold) toggle=1;
  for(size_t i_t=1; i_t<wfm_length; ++i_t){
    this_value = fWvfmsVec[i_ch+nChannels*fragId][i_t];
    if (toggle==0 && this_value>threshold) {
      toggle=1;
      fPMT_ch9.emplace_back(i_t);
    }
    if (toggle==1 && this_value<threshold) toggle=0;
  }

  // find leading edges in waveforms
  //threshold=7800;
  i_ch = 10; toggle=0;
  this_value = fWvfmsVec[i_ch+nChannels*fragId][0];
  if (this_value>threshold) toggle=1;
  for(size_t i_t=1; i_t<wfm_length; ++i_t){
    this_value = fWvfmsVec[i_ch+nChannels*fragId][i_t];
    if (toggle==0 && this_value>threshold) {
      toggle=1;
      fPMT_ch10.emplace_back(i_t);
    }
    if (toggle==1 && this_value<threshold) toggle=0;
  }

  // find leading edges in waveforms
  //threshold=7800;
  i_ch = 11; toggle=0;
  this_value = fWvfmsVec[i_ch+nChannels*fragId][0];
  if (this_value>threshold) toggle=1;
  for(size_t i_t=1; i_t<wfm_length; ++i_t){
    this_value = fWvfmsVec[i_ch+nChannels*fragId][i_t];
    if (toggle==0 && this_value>threshold) {
      toggle=1;
      fPMT_ch11.emplace_back(i_t);
    }
    if (toggle==1 && this_value<threshold) toggle=0;
  }

  // find leading edges in waveforms
  //threshold=7800;
  i_ch = 12; toggle=0;
  this_value = fWvfmsVec[i_ch+nChannels*fragId][0];
  if (this_value>threshold) toggle=1;
  for(size_t i_t=1; i_t<wfm_length; ++i_t){
    this_value = fWvfmsVec[i_ch+nChannels*fragId][i_t];
    if (toggle==0 && this_value>threshold) {
      toggle=1;
      fPMT_ch12.emplace_back(i_t);
    }
    if (toggle==1 && this_value<threshold) toggle=0;
  }

  // find leading edges in waveforms
  //threshold=7800;
  i_ch = 13; toggle=0;
  this_value = fWvfmsVec[i_ch+nChannels*fragId][0];
  if (this_value>threshold) toggle=1;
  for(size_t i_t=1; i_t<wfm_length; ++i_t){
    this_value = fWvfmsVec[i_ch+nChannels*fragId][i_t];
    if (toggle==0 && this_value>threshold) {
      toggle=1;
      fPMT_ch13.emplace_back(i_t);
    }
    if (toggle==1 && this_value<threshold) toggle=0;
  }

  // find leading edges in waveforms
  //threshold=7800;
  i_ch = 14; toggle=0;
  this_value = fWvfmsVec[i_ch+nChannels*fragId][0];
  if (this_value>threshold) toggle=1;
  for(size_t i_t=1; i_t<wfm_length; ++i_t){
    this_value = fWvfmsVec[i_ch+nChannels*fragId][i_t];
    if (toggle==0 && this_value>threshold) {
      toggle=1;
      fPMT_ch14.emplace_back(i_t);
    }
    if (toggle==1 && this_value<threshold) toggle=0;
  }

  // find leading edges in waveforms
  //threshold=7800;
  i_ch = 15; toggle=0;
  this_value = fWvfmsVec[i_ch+nChannels*fragId][0];
  if (this_value>threshold) toggle=1;
  for(size_t i_t=1; i_t<wfm_length; ++i_t){
    this_value = fWvfmsVec[i_ch+nChannels*fragId][i_t];
    if (toggle==0 && this_value>threshold) {
      toggle=1;
      fPMT_ch15.emplace_back(i_t);
    }
    if (toggle==1 && this_value<threshold) toggle=0;
  }




}


void sbndaq::MultiDump::analyze_bern_fragment(artdaq::Fragment & frag)  {

  BernCRTFragmentV2 bern_fragment(frag);
//  TLOG(TLVL_INFO) << bern_fragment;

  sequence_id = frag.sequenceID();

  //metadata
  const BernCRTFragmentMetadataV2* md = bern_fragment.metadata();
//  TLOG(TLVL_INFO)<<*md;

  mac5                      = md->MAC5();
  run_start_time            = md->run_start_time();
  this_poll_start           = md->this_poll_start();
  this_poll_end             = md->this_poll_end();
  last_poll_start           = md->last_poll_start();
  last_poll_end             = md->last_poll_end();
  system_clock_deviation    = md->system_clock_deviation();
  feb_hits_in_poll          = md->hits_in_poll();
  feb_hits_in_fragment      = md->hits_in_fragment();
  if (0) {
    std::cout <<   " mac5             "   <<    unsigned(mac5)                   << std::endl;
  std::cout << " run_start_time      "    <<  run_start_time              << std::endl;
  std::cout << " this_poll_start     "    << this_poll_start            << std::endl;
  std::cout << " this_poll_end        "   << this_poll_end              << std::endl;
  std::cout << "  last_poll_start      "   << last_poll_start            << std::endl;
  std::cout << "  last_poll_end          " << last_poll_end              << std::endl;
  std::cout << "  system_clock_deviation " << system_clock_deviation     << std::endl;
  std::cout << "  feb_hits_in_poll       " << feb_hits_in_poll           << std::endl;
  std::cout << "  feb_hits_in_fragment  "  << feb_hits_in_fragment       << std::endl;
  }

  for(unsigned int iHit = 0; iHit < feb_hits_in_fragment; iHit++) {

    BernCRTHitV2 const* bevt = bern_fragment.eventdata(iHit);

    //  TLOG(TLVL_INFO)<<*bevt;

    flags                   = bevt->flags;
    lostcpu                 = bevt->lostcpu;
    lostfpga                = bevt->lostfpga;
    ts0                     = bevt->ts0;
    ts1                     = bevt->ts1;
    //    coinc                   = bevt->coinc;

    feb_hit_number          = bevt->feb_hit_number;
    timestamp               = bevt->timestamp;
    last_accepted_timestamp = bevt->last_accepted_timestamp;
    lost_hits               = bevt->lost_hits;

    if (0) {
      std::cout << "  flags  "                << flags                   << std::endl;
      std::cout << "lostcpu         "        << lostcpu                 << std::endl;
      std::cout <<     "lostfpga    "<<     lostfpga     << std::endl;
      std::cout <<     "ts0         "<<     ts0          << std::endl;
      std::cout <<     "ts1                    "<<     ts1             << std::endl;
      //     std::cout <<     "coinc                  "<<     coinc                   << std::endl;
      std::cout <<     "feb_hit_number         "<<     feb_hit_number          << std::endl;
      std::cout <<     "timestamp              "<<     timestamp               << std::endl;
      std::cout <<     "last_accepted_timestamp"<<     last_accepted_timestamp << std::endl;
      std::cout <<     "lost_hits              "<<     lost_hits               << std::endl;
    }

    // for(int ch=0; ch<32; ch++) adc[ch] = bevt->adc[ch];
    for(int ch=0; ch<32; ch++) {
      if (0) std::cout << "channel " << ch << " has adc value " << bevt->adc[ch] << std::endl;
      adc[ch] = bevt->adc[ch];
    }

    fEventTree->Fill();

  }

}//analyze_bern_fragment


// Extract the PTB words/data from the artDAQ fragments
void sbndaq::MultiDump::extract_triggers(artdaq::Fragment & frag) {

  // Construct PTB fragment overlay class giving us access to all the helpful decoder functions
  CTBFragment ptb_fragment(frag);

  std::cout << "PTB Fragment ID: " << frag.sequenceID() << " TS: " << frag.timestamp() 
            << " Containing " << ptb_fragment.NWords() << " words" << std::endl;

  /*********************
  *
  * Note: Below for the Timestamp conversion,
  *       The PTB TS is in UTC seconds since the Unix epoch in 50MHz clock ticks.
  *       This means to recover seconds since the Unix epoch use: sec_since_epoch = TS / 50e6 
  *       and of course nanosec_since_epoch = (TS / 50e6) * 1e9
  *
  * Note: The `CTBFragment` constructor grabs the chunk of memory the artDAQ fragment occupies.
  *       Since we know a priori the number of PTB words (from the TCP header) and the size (128b for all words)
  *       we can loop over the memory addresses, casting each 128b into the correct word type given by each word's `type`.
  *
  * There are 5 word types with the following bit arrangement:
  *       Feeback        = | 3b Word Type | 61b Payload | 64b Timestamp | (here Payload is split into code, source, payload1, payload2)
  *       LLT, HLT       = | 3b Word Type | 61b Payload | 64b Timestamp |
  *       Channel Status = | 3b Word Type | 64b Payload | 61b Timestamp | (Larger Payload to fit all input channels)
  *        
  * Word Type:
  *       - 0x0 = Feedback/Error Word -> Errors from the firmware, should abort the run
  *       - 0x1 = Low Level Trigger (LLT) -> Holds a record of any asserted LLTs
  *       - 0x2 = High Level Trigger (HLT) -> Holds a record of any asserted HLTs
  *       - 0x3 = Channel status -> Holds a record of any asserted inputs
  *       - 0x7 = Timestamp Word -> No payload just a timestamp, these are periodic
  *
  * Note: Payload AND'd with mask the size of the expected number of bits just
  *       to make sure we don't get any unexpected garbage bits since we aren't using
  *       the full bits of the variable type e.g. uint64_t, unint16_t..
  *
  **********************/

  // Loop through all the PTB words in the fragment, casting to 
  // one of the 5 word types. The 3 Msb hold the word type
  for ( size_t i = 0; i < ptb_fragment.NWords(); i++ ) {
    if (fverbose) std::cout << "PTB Word type [" << ptb_fragment.Word(i)->word_type << "]" << std::endl;
    if (fverbose) std::cout << "PTB word type: " << ptb_fragment.Word(i)->word_type << std::endl;
    switch ( ptb_fragment.Word(i)->word_type ) {
      case 0x0 : // Feedback (errors) Word
        // Only get this word if something goes wrong at the firmware level requires expert knowledge 
        // to interpret. The appearance of this word should have crashed the run.
        unknown_or_error_word = true;
        std::cout << "Feedback Word! Code: " << ptb_fragment.Feedback(i)->code 
                  << " Source: "             << ptb_fragment.Feedback(i)->source
                  << " Payload: "            <<  ptb_fragment.Feedback(i)->payload 
                  << " Timestamp: "          << ptb_fragment.TimeStamp(i) << std::endl; 
        break;
      case 0x1 : // LL Trigger
        if (fverbose) std::cout << "LLT Payload: " << ptb_fragment.Trigger(i)->trigger_word << std::endl;
        llt_trigger.emplace_back( ptb_fragment.Trigger(i)->trigger_word & 0x1FFFFFFFFFFFFFFF ); // bit map of asserted LLTs 
        llt_ts.emplace_back( (ptb_fragment.TimeStamp(i) / 50e6) * 1e9 ); // Timestamp of the word 
        break;
      case 0x2 : // HL Trigger
        if (fverbose) std::cout << "HLT Payload: " << ptb_fragment.Trigger(i)->trigger_word << std::endl;
        hlt_trigger.emplace_back( ptb_fragment.Trigger(i)->trigger_word & 0x1FFFFFFFFFFFFFFF );
        hlt_ts.emplace_back( (ptb_fragment.TimeStamp(i) / 50e6) * 1e9 ); 
        ptb_frag_ts = frag.timestamp();
        hlt_word_count++;
        break;
      case 0x3 : // Channel Status
        // Each PTB input gets a bit map e.g. CRT has 14 inputs and is 14b 
        // (1 is channel asserted 0 otherwise)
        // TODO add MTCA and NIM channel status words
        auxpds_status.emplace_back( ptb_fragment.ChStatus(i)->pds & 0x3FF );
        crt_status.emplace_back( ptb_fragment.ChStatus(i)->crt & 0x3FFF );
        beam_status.emplace_back( ptb_fragment.ChStatus(i)->beam & 0x3 );
        chan_stat_ts.emplace_back( (ptb_fragment.TimeStamp(i) / 50e6) * 1e9 );
        break;
      case 0x7 : // Timestamp Word
        // We don't care about this word, it only has a TS and is sent periodically.
        ptb_fragment.TimeStamp(i);
        ts_word_count++;
        break;
      default : // Unknown, should never happen!
        unknown_or_error_word = true;
        std::cout << "Unknown PTB word type = " << ptb_fragment.Word(i)->word_type << std::endl;
    }
  }

}  // extract trigger fragments for the PTB

void sbndaq::MultiDump::reset_ptb_variables() {

  // Initialize/reset the variables
  unknown_or_error_word = false;
  ts_word_count = 0;
  hlt_word_count = 0;
  ptb_frag_ts = 0;
  llt_trigger.clear();
  llt_ts.clear();
  hlt_trigger.clear();
  hlt_ts.clear();
  crt_status.clear();
  beam_status.clear();
  mtca_status.clear();
  nim_status.clear();
  auxpds_status.clear();
  chan_stat_ts.clear();

} // reset for ptb parameters


DEFINE_ART_MODULE(sbndaq::MultiDump)
