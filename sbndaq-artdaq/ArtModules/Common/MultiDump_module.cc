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

  fRun = evt.run();
  fEvent = evt.event();
  if (fverbose)   std::cout << "Run " << fRun << " event " << fEvent << std::endl;

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

  }else{// if include_berncrt

    fEventTree->Fill();

  }

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


  CAENV1730Fragment bb(frag);
  auto const* md = bb.Metadata();
  CAENV1730Event const* event_ptr = bb.Event();
  CAENV1730EventHeader header = event_ptr->Header;

  int fragId = static_cast<int>(frag.fragmentID());

  //
  if (fverbose)      std::cout << "\tFrom CAEN header, event counter is "  << header.eventCounter   << "\n";
  if (fverbose)      std::cout << "\tFrom CAEN header, triggerTimeTag is " << header.triggerTimeTag << "\n";
  if (fverbose)       std::cout << "\tFrom CAEN header, board id is "       << header.boardID       << "\n";
  if (fverbose)       std::cout << "\tFrom CAEN fragment, fragment id is "  << fragId << "\n";

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



DEFINE_ART_MODULE(sbndaq::MultiDump)
