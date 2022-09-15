//-------------------------------------------------
//---------------------------------------

////////////////////////////////////////////////////////////////////////
// Class:       EventAna
// Module Type: analyzer
// File:        EventAna_module.cc
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
  class EventAna;
}

/**************************************************************************************************/

class sbndaq::EventAna : public art::EDAnalyzer {

public:
  struct Config {
    //--one atom for each parameter
    fhicl::Atom<bool> include_caen {
      fhicl::Name("include_caen"),
	fhicl::Comment("look for caen 1730 fragments true/false"),
	true
	};
    fhicl::Atom<bool> caen_keepwaveforms {
      fhicl::Name("caen_keepwaveforms"),
	fhicl::Comment("put waveforms into tree true/false"),
	false
	};
    fhicl::Atom<int> Shift {
      fhicl::Name("shift_fragment_id"),
	fhicl::Comment("Number to subtract to the fragment_id"),
	0
	};
    fhicl::Atom<bool> include_wr {
      fhicl::Name("include_wr"),
	fhicl::Comment("look for wr dio fragments true/false"),
	false
	};
    fhicl::Atom<int> window_wr {
      fhicl::Name("window_wr"),
	fhicl::Comment("window around CAEN TTT to keep WR timestamps.  integer values from 0 to 10^9"),
	10000	
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
  }; //--configuration
  using Parameters = art::EDAnalyzer::Table<Config>;

  explicit EventAna(Parameters const & pset);
  virtual ~EventAna();

  void analyze(const art::Event& evt) override;
  void beginJob() override;
  void endJob() override;

private:

  void analyze_caen_fragment(artdaq::Fragment & frag);
  void analyze_wr_fragment_dio(artdaq::Fragment & frag);
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

  TTree* events;
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
  std::vector<int> coinc;

  std::vector<int>  feb_hit_number          ; //hit counter for individual FEB, including hits lost in FEB or fragment generator
  std::vector<uint>  timestamp               ; //absolute timestamp
  std::vector<uint>  last_accepted_timestamp ; //timestamp of previous accepted hit
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


  bool finclude_caen;
  bool fcaen_keepwaveforms;
  int fShift;
  bool finclude_wr;
  int fWindow;
  bool finclude_berncrt;
  bool fcrt_keepall;
  bool fverbose;

}; //--class EventAna


sbndaq::EventAna::EventAna(EventAna::Parameters const& pset): art::EDAnalyzer(pset)
{
  finclude_caen = pset().include_caen();
  fcaen_keepwaveforms = pset().caen_keepwaveforms();
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
  fcrt_keepall = pset().crt_keepall();

}

void sbndaq::EventAna::beginJob()
{
  art::ServiceHandle<art::TFileService> tfs;
  nt_header       = tfs->make<TNtuple>("nt_header","Multi Header Ntuple","art_ev:caen_ev:caenv_ev_tts");
  /************************************************************************************************/
  hEventCounter   = tfs->make<TH1F>("hEventCounter","Event Counter Histogram",10000,0,10000);
  hTriggerTimeTag = tfs->make<TH1D>("hTriggerTimeTag","Trigger Time Tag Histogram",12500,0,125000000);
  h_wvfm_ev0_ch0  = tfs->make<TH1F>("h_wvfm_ev0_ch0","Waveform",2000,0,2000);
  /************************************************************************************************/
  //--make tree to store the channel waveform info:
  events = tfs->make<TTree>("events","waveform tree");
  events->Branch("fRun",&fRun,"fRun/I");
  events->Branch("fEvent",&fEvent,"fEvent/I");
  if (finclude_caen) {
    events->Branch("TTT_ns",&TTT_ns,"TTT_ns/I");
    if (fcaen_keepwaveforms) {
      events->Branch("fTicksVec",&fTicksVec);
      events->Branch("fWvfmsVec",&fWvfmsVec);
    }
    events->Branch("fPMT_ch0",&fPMT_ch0);
    events->Branch("fPMT_ch1",&fPMT_ch1);
    events->Branch("fPMT_ch2",&fPMT_ch2);
    events->Branch("fPMT_ch3",&fPMT_ch3);
    events->Branch("fPMT_ch4",&fPMT_ch4);
    events->Branch("fPMT_ch5",&fPMT_ch5);
    events->Branch("fPMT_ch6",&fPMT_ch6);
    events->Branch("fPMT_ch7",&fPMT_ch7);
    events->Branch("fPMT_ch8",&fPMT_ch8);
    events->Branch("fPMT_ch9",&fPMT_ch9);
    events->Branch("fPMT_ch10",&fPMT_ch10);
    events->Branch("fPMT_ch11",&fPMT_ch11);
    events->Branch("fPMT_ch12",&fPMT_ch12);
    events->Branch("fPMT_ch13",&fPMT_ch13);
    events->Branch("fPMT_ch14",&fPMT_ch14);
    events->Branch("fPMT_ch15",&fPMT_ch15);
  }
  if (finclude_wr) {
    events->Branch("fWR_ch0",&fWR_ch0);
    events->Branch("fWR_ch1",&fWR_ch1);
    events->Branch("fWR_ch2",&fWR_ch2);
    events->Branch("fWR_ch3",&fWR_ch3);
    events->Branch("fWR_ch4",&fWR_ch4);
  }
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
    if (fcrt_keepall) {
      events->Branch("lostcpu",       &lostcpu);
      events->Branch("lostfpga",      &lostfpga);
      events->Branch("feb_hit_number",&feb_hit_number);
      events->Branch("timestamp",     &timestamp);
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

}

void sbndaq::EventAna::endJob()
{
  if (fverbose)  std::cout << "Ending EventAna...\n";
}


sbndaq::EventAna::~EventAna()
{
}


void sbndaq::EventAna::analyze(const art::Event& evt)
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
  first_wr_ch0=0;
  first_wr_ch1=0;
  first_wr_ch2=0;
  first_wr_ch3=0;
  first_wr_ch4=0;
  /************************************************************************************************/

  mac5.clear(); flags.clear();   lostcpu.clear();   lostfpga.clear();   ts0.clear();   ts1.clear();
  adc0.clear();   adc1.clear();   adc2.clear();   adc3.clear();   adc4.clear();   adc5.clear();   adc6.clear();
  adc7.clear();   adc8.clear();   adc9.clear();   adc10.clear();   adc11.clear();   adc12.clear();   adc13.clear();
  adc14.clear();   adc15.clear();   adc16.clear();   adc17.clear();   adc18.clear();   adc19.clear();   adc20.clear();
  adc21.clear();   adc22.clear();   adc23.clear();   adc24.clear();   adc25.clear();   adc26.clear();   adc27.clear();
  adc28.clear();   adc29.clear();   adc30.clear();   adc31.clear();   coinc.clear();
  
  feb_hit_number.clear()          ;    timestamp.clear()               ;    last_accepted_timestamp.clear() ; 
  lost_hits.clear()               ;   run_start_time.clear();   this_poll_start.clear();   this_poll_end.clear();
  last_poll_start.clear();   last_poll_end.clear();    system_clock_deviation.clear();    feb_hits_in_poll.clear();
  feb_hits_in_fragment.clear();
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

    for (auto handle : fragmentHandles) {
      if (!handle.isValid() || handle->size() == 0) continue;

      if (handle->front().type() == artdaq::Fragment::ContainerFragmentType) {

        for (auto cont : *handle) {
          artdaq::ContainerFragment contf(cont);

          if (contf.fragment_type() == sbndaq::detail::FragmentType::CAENV1730 && finclude_caen) {
            if (fverbose)         std::cout << "    Found " << contf.block_count() << " CAEN Fragments in container " << std::endl;
            fWvfmsVec.resize(16*contf.block_count());
            for (size_t ii = 0; ii < contf.block_count(); ++ii)
              analyze_caen_fragment(*contf[ii].get());
          }

          else if (contf.fragment_type() == sbndaq::detail::FragmentType::WhiteRabbit && finclude_wr) {
            if (fverbose)         std::cout << "    Found " << contf.block_count() << " WR Fragments in container " << std::endl;
            for (size_t ii = 0; ii < contf.block_count(); ++ii)
              analyze_wr_fragment_dio(*contf[ii].get());
          }

          else if (contf.fragment_type() == sbndaq::detail::FragmentType::BERNCRTV2 && finclude_berncrt) {
            if (fverbose)         std::cout << "    Found " << contf.block_count() << " BERN Fragments in container " << std::endl;
            for (size_t ii = 0; ii < contf.block_count(); ++ii)
              analyze_bern_fragment(*contf[ii].get());
          }
        }
      }
      else {

        unsigned n_caen_frags(0), n_wr_frags(0), n_berncrt_frags(0);

        for (auto frag : *handle) {

          if (frag.type() == sbndaq::detail::FragmentType::CAENV1730 && finclude_caen)
            ++n_caen_frags;
          else if (frag.type() == sbndaq::detail::FragmentType::WhiteRabbit && finclude_wr)
            ++n_wr_frags;
          else if (frag.type() == sbndaq::detail::FragmentType::BERNCRTV2 && finclude_berncrt)
            ++n_berncrt_frags;
        }

        for (auto frag : *handle) {

          if (frag.type() == sbndaq::detail::FragmentType::CAENV1730 && finclude_caen) {
            fWvfmsVec.resize(16*n_caen_frags);
            analyze_caen_fragment(frag);
          }
          else if (frag.type() == sbndaq::detail::FragmentType::WhiteRabbit && finclude_wr)
            analyze_wr_fragment_dio(frag);
          else if (frag.type() == sbndaq::detail::FragmentType::BERNCRTV2 && finclude_berncrt)
            analyze_bern_fragment(frag);
        }

        if (fverbose > 0) {
          std::cout << "\n\tFound " << n_caen_frags << " normal CAEN fragments" << std::endl;
          std::cout << "\tFound " << n_wr_frags << " normal WR fragments" << std::endl;
          std::cout << "\tFound " << n_berncrt_frags << " normal BERNCRT fragments\n" << std::endl;
        }
      }
    } // loop over frag handles

    if (fverbose && finclude_wr)
      std::cout << " WR ch 0 " << fnstamps0 << " WR ch 1 " << fnstamps1 << " WR ch 2 " << fnstamps2 << " WR ch 3 "
                << fnstamps3 << " WR ch 4 " << fnstamps4 << std::endl;

    events->Fill();
}



void sbndaq::EventAna::analyze_wr_fragment_dio(artdaq::Fragment & frag)  {



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

void sbndaq::EventAna::analyze_caen_fragment(artdaq::Fragment & frag)  {
  
  
  if (fverbose) std::cout <<  "     timestamp is  " << frag.timestamp() << std::endl;
  if (fverbose) std::cout <<  "     seq ID is " << frag.sequenceID() << std::endl;
  
  
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

  int threshold[]= { 10000, 5000, 10000, 5000,10000, 5000, 10000, 5000,10000, 5000, 10000, 5000, 10000, 5000, 10000, 5000};

  // find leading edges in waveforms
  int toggle=0;
  int i_ch =0 ;
  auto this_value = fWvfmsVec[i_ch+nChannels*fragId][0];
  if (this_value>threshold[i_ch]) toggle=1;
  for(size_t i_t=1; i_t<wfm_length; ++i_t){
    this_value = fWvfmsVec[i_ch+nChannels*fragId][i_t];
    if (toggle==0 && this_value>threshold[i_ch]) {
      toggle=1;
      fPMT_ch0.emplace_back(i_t);
    }
    if (toggle==1 && this_value<threshold[i_ch]) toggle=0;
  }
  
  // find leading edges in waveforms
  i_ch = 1; toggle=0;
  this_value = fWvfmsVec[i_ch+nChannels*fragId][0];
  if (this_value>threshold[i_ch]) toggle=1;
  for(size_t i_t=1; i_t<wfm_length; ++i_t){
    this_value = fWvfmsVec[i_ch+nChannels*fragId][i_t];
    if (toggle==0 && this_value>threshold[i_ch]) {
      toggle=1;
      fPMT_ch1.emplace_back(i_t);
    }
    if (toggle==1 && this_value<threshold[i_ch]) toggle=0;
  }
  
  // find leading edges in waveforms
  i_ch = 2; toggle=0;
  this_value = fWvfmsVec[i_ch+nChannels*fragId][0];
  if (this_value>threshold[i_ch]) toggle=1;
  for(size_t i_t=1; i_t<wfm_length; ++i_t){
    this_value = fWvfmsVec[i_ch+nChannels*fragId][i_t];
    if (toggle==0 && this_value>threshold[i_ch]) {
      toggle=1;
      fPMT_ch2.emplace_back(i_t);
    }
    if (toggle==1 && this_value<threshold[i_ch]) toggle=0;
  }

  // find leading edges in waveforms
  i_ch = 3; toggle=0;
  this_value = fWvfmsVec[i_ch+nChannels*fragId][0];
  if (this_value>threshold[i_ch]) toggle=1;
  for(size_t i_t=1; i_t<wfm_length; ++i_t){
    this_value = fWvfmsVec[i_ch+nChannels*fragId][i_t];
    if (toggle==0 && this_value>threshold[i_ch]) {
      toggle=1;
      fPMT_ch3.emplace_back(i_t);
    }
    if (toggle==1 && this_value<threshold[i_ch]) toggle=0;
  }
  
  // find leading edges in waveforms
  i_ch = 4; toggle=0;
  this_value = fWvfmsVec[i_ch+nChannels*fragId][0];
  if (this_value>threshold[i_ch]) toggle=1;
  for(size_t i_t=1; i_t<wfm_length; ++i_t){
    this_value = fWvfmsVec[i_ch+nChannels*fragId][i_t];
    if (toggle==0 && this_value>threshold[i_ch]) {
      toggle=1;
      fPMT_ch4.emplace_back(i_t);
    }
    if (toggle==1 && this_value<threshold[i_ch]) toggle=0;
  }

  // find leading edges in waveforms
  i_ch = 5; toggle=0;
  this_value = fWvfmsVec[i_ch+nChannels*fragId][0];
  if (this_value>threshold[i_ch]) toggle=1;
  for(size_t i_t=1; i_t<wfm_length; ++i_t){
    this_value = fWvfmsVec[i_ch+nChannels*fragId][i_t];
    if (toggle==0 && this_value>threshold[i_ch]) {
      toggle=1;
      fPMT_ch5.emplace_back(i_t);
    }
    if (toggle==1 && this_value<threshold[i_ch]) toggle=0;
  }

  // find leading edges in waveforms
  i_ch = 6; toggle=0;
  this_value = fWvfmsVec[i_ch+nChannels*fragId][0];
  if (this_value>threshold[i_ch]) toggle=1;
  for(size_t i_t=1; i_t<wfm_length; ++i_t){
    this_value = fWvfmsVec[i_ch+nChannels*fragId][i_t];
    if (toggle==0 && this_value>threshold[i_ch]) {
      toggle=1;
      fPMT_ch6.emplace_back(i_t);
    }
    if (toggle==1 && this_value<threshold[i_ch]) toggle=0;
  }

  // find leading edges in waveforms
  i_ch = 7; toggle=0;
  this_value = fWvfmsVec[i_ch+nChannels*fragId][0];
  if (this_value>threshold[i_ch]) toggle=1;
  for(size_t i_t=1; i_t<wfm_length; ++i_t){
    this_value = fWvfmsVec[i_ch+nChannels*fragId][i_t];
    if (toggle==0 && this_value>threshold[i_ch]) {
      toggle=1;
      fPMT_ch7.emplace_back(i_t);
    }
    if (toggle==1 && this_value<threshold[i_ch]) toggle=0;
  }

  // find leading edges in waveforms
  i_ch = 8; toggle=0;
  this_value = fWvfmsVec[i_ch+nChannels*fragId][0];
  if (this_value>threshold[i_ch]) toggle=1;
  for(size_t i_t=1; i_t<wfm_length; ++i_t){
    this_value = fWvfmsVec[i_ch+nChannels*fragId][i_t];
    if (toggle==0 && this_value>threshold[i_ch]) {
      toggle=1;
      fPMT_ch8.emplace_back(i_t);
    }
    if (toggle==1 && this_value<threshold[i_ch]) toggle=0;
  }

  // find leading edges in waveforms
  i_ch = 9; toggle=0;
  this_value = fWvfmsVec[i_ch+nChannels*fragId][0];
  if (this_value>threshold[i_ch]) toggle=1;
  for(size_t i_t=1; i_t<wfm_length; ++i_t){
    this_value = fWvfmsVec[i_ch+nChannels*fragId][i_t];
    if (toggle==0 && this_value>threshold[i_ch]) {
      toggle=1;
      fPMT_ch9.emplace_back(i_t);
    }
    if (toggle==1 && this_value<threshold[i_ch]) toggle=0;
  }

  // find leading edges in waveforms
  i_ch = 10; toggle=0;
  this_value = fWvfmsVec[i_ch+nChannels*fragId][0];
  if (this_value>threshold[i_ch]) toggle=1;
  for(size_t i_t=1; i_t<wfm_length; ++i_t){
    this_value = fWvfmsVec[i_ch+nChannels*fragId][i_t];
    if (toggle==0 && this_value>threshold[i_ch]) {
      toggle=1;
      fPMT_ch10.emplace_back(i_t);
    }
    if (toggle==1 && this_value<threshold[i_ch]) toggle=0;
  }

  // find leading edges in waveforms
  i_ch = 11; toggle=0;
  this_value = fWvfmsVec[i_ch+nChannels*fragId][0];
  if (this_value>threshold[i_ch]) toggle=1;
  for(size_t i_t=1; i_t<wfm_length; ++i_t){
    this_value = fWvfmsVec[i_ch+nChannels*fragId][i_t];
    if (toggle==0 && this_value>threshold[i_ch]) {
      toggle=1;
      fPMT_ch11.emplace_back(i_t);
    }
    if (toggle==1 && this_value<threshold[i_ch]) toggle=0;
  }

  // find leading edges in waveforms
  i_ch = 12; toggle=0;
  this_value = fWvfmsVec[i_ch+nChannels*fragId][0];
  if (this_value>threshold[i_ch]) toggle=1;
  for(size_t i_t=1; i_t<wfm_length; ++i_t){
    this_value = fWvfmsVec[i_ch+nChannels*fragId][i_t];
    if (toggle==0 && this_value>threshold[i_ch]) {
      toggle=1;
      fPMT_ch12.emplace_back(i_t);
    }
    if (toggle==1 && this_value<threshold[i_ch]) toggle=0;
  }

  // find leading edges in waveforms
  i_ch = 13; toggle=0;
  this_value = fWvfmsVec[i_ch+nChannels*fragId][0];
  if (this_value>threshold[i_ch]) toggle=1;
  for(size_t i_t=1; i_t<wfm_length; ++i_t){
    this_value = fWvfmsVec[i_ch+nChannels*fragId][i_t];
    if (toggle==0 && this_value>threshold[i_ch]) {
      toggle=1;
      fPMT_ch13.emplace_back(i_t);
    }
    if (toggle==1 && this_value<threshold[i_ch]) toggle=0;
  }

  // find leading edges in waveforms
  i_ch = 14; toggle=0;
  this_value = fWvfmsVec[i_ch+nChannels*fragId][0];
  if (this_value>threshold[i_ch]) toggle=1;
  for(size_t i_t=1; i_t<wfm_length; ++i_t){
    this_value = fWvfmsVec[i_ch+nChannels*fragId][i_t];
    if (toggle==0 && this_value>threshold[i_ch]) {
      toggle=1;
      fPMT_ch14.emplace_back(i_t);
    }
    if (toggle==1 && this_value<threshold[i_ch]) toggle=0;
  }

  // find leading edges in waveforms
  i_ch = 15; toggle=0;
  this_value = fWvfmsVec[i_ch+nChannels*fragId][0];
  if (this_value>threshold[i_ch]) toggle=1;
  for(size_t i_t=1; i_t<wfm_length; ++i_t){
    this_value = fWvfmsVec[i_ch+nChannels*fragId][i_t];
    if (toggle==0 && this_value>threshold[i_ch]) {
      toggle=1;
      fPMT_ch15.emplace_back(i_t);
    }
    if (toggle==1 && this_value<threshold[i_ch]) toggle=0;
  }
  



}


void sbndaq::EventAna::analyze_bern_fragment(artdaq::Fragment & frag)  {

  BernCRTFragmentV2 bern_fragment(frag);
  const BernCRTFragmentMetadataV2* md = bern_fragment.metadata();
  for(unsigned int iHit = 0; iHit < md->hits_in_fragment(); iHit++) {
    BernCRTHitV2 const* bevt = bern_fragment.eventdata(iHit);
    
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
    }// if verbose
    
    
  }// end loop over fragments
  
}//analyze_bern_fragment



DEFINE_ART_MODULE(sbndaq::EventAna)
