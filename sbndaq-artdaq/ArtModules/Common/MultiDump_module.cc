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
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "artdaq-core/Data/Fragment.hh"
#include "artdaq-core/Data/ContainerFragment.hh"

//#include "art/Framework/Services/Optional/TFileService.h" //before art_root_io transition
#include "art_root_io/TFileService.h"
#include "TH1F.h"
#include "TNtuple.h"

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
      fhicl::Comment("window around CAEN TTS to keep WR timestamps.  integer values from 0 to 10^9")
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

  //--default values
  uint32_t nChannels;//    = 16;
  uint32_t Ttt_DownSamp;// =  4; 
 /* the waveforms are sampled at 500MHz sampling. The trigger timestamp is
                               * sampled 4 times slower than input channels*/

  TNtuple* nt_header;
  
  TH1F*    hEventCounter;
  TH1F*    hTriggerTimeTag;
  TH1F*    h_wvfm_ev0_ch0;

  TTree* fEventTree;
  int fRun;
  art::EventNumber_t fEvent;
  std::vector<uint64_t>  fTicksVec;
  std::vector< std::vector<uint16_t> >  fWvfmsVec;
  int fnstamps0;
  int fnstamps1;
  int fnstamps2;
  int fnstamps3;
  int fnstamps4;
  //  std::vector<uint64_t> 
  
  bool firstEvt = true;
  uint32_t TTS_ns;  // will be set to value in CAEN fragement header

  bool finclude_caen;
  int fShift; 
  bool finclude_wr;
  int fWindow;
	
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

}

void sbndaq::MultiDump::beginJob()
{
  art::ServiceHandle<art::TFileService> tfs; 
  nt_header       = tfs->make<TNtuple>("nt_header","Multi Header Ntuple","art_ev:caen_ev:caenv_ev_tts"); 
  /************************************************************************************************/
  hEventCounter   = tfs->make<TH1F>("hEventCounter","Event Counter Histogram",10000,0,10000);
  hTriggerTimeTag = tfs->make<TH1F>("hTriggerTimeTag","Trigger Time Tag Histogram",10,2000000000,4500000000);
  h_wvfm_ev0_ch0  = tfs->make<TH1F>("h_wvfm_ev0_ch0","Waveform",2000,0,2000);  
  /************************************************************************************************/
  //--make tree to store the channel waveform info:
  fEventTree = tfs->make<TTree>("events","waveform tree");
  fEventTree->Branch("fRun",&fRun,"fRun/I");
  fEventTree->Branch("fEvent",&fEvent,"fEvent/I");
  if (finclude_caen) {
    fEventTree->Branch("fTicksVec",&fTicksVec);
    fEventTree->Branch("fWvfmsVec",&fWvfmsVec);
  }
  if (finclude_wr) {
    fEventTree->Branch("nstamps0",&fnstamps0,"nstamps0/I");
    fEventTree->Branch("nstamps1",&fnstamps1,"nstamps1/I");
    fEventTree->Branch("nstamps2",&fnstamps2,"nstamps2/I");
    fEventTree->Branch("nstamps3",&fnstamps3,"nstamps3/I");
    fEventTree->Branch("nstamps4",&fnstamps4,"nstamps4/I");
    // add more variables to the tree here
    // fEventTree->Branch("",);
    // fEventTree->Branch("",);
    // fEventTree->Branch("",);
    // fEventTree->Branch("",);
  }

}

void sbndaq::MultiDump::endJob()
{
  std::cout << "Ending MultiDump...\n";
}


sbndaq::MultiDump::~MultiDump()
{
}


void sbndaq::MultiDump::analyze(const art::Event& evt)
{

  fRun = evt.run();
  fEvent = evt.event();
  std::cout << "Run " << fRun << " event " << fEvent << std::endl;

  //  Note that this code expects exactly 1 CAEN fragment per event
  uint32_t TTT_ns=0;  // will be set to value in CAEN fragement header

  std::vector<art::Handle<artdaq::Fragments>> fragmentHandles;
  evt.getManyByType(fragmentHandles);

  /************************************************************************************************/
  if (finclude_caen) { 
  for (auto handle : fragmentHandles) {
    if (!handle.isValid() || handle->size() == 0) continue;
    
    if (handle->front().type() == artdaq::Fragment::ContainerFragmentType) {
      //Container fragment                                                                                               
      for (auto cont : *handle) {
	artdaq::ContainerFragment contf(cont);
	if (contf.fragment_type()==sbndaq::detail::FragmentType::CAENV1730) {
	  std::cout << "    Found " << contf.block_count() << " CAEN Fragments in container " << std::endl;
	  fWvfmsVec.resize(16*contf.block_count());
	  for (size_t ii = 0; ii < contf.block_count(); ++ii)
	  	analyze_caen_fragment(*contf[ii].get());
	}
      } 
    }   
    else {      
      //normal fragment                
      if (handle->front().type()==sbndaq::detail::FragmentType::CAENV1730) {
	std::cout << "   found normal caen fragments " << handle->size() << std::endl;
	fWvfmsVec.resize(16*handle->size());
	for (auto frag : *handle)
	  analyze_caen_fragment(frag);
      }
    }
  } // loop over frag handles
  }  // if include caen

  /************************************************************************************************/
  // Expecting multiple WR fragments, so need to clear tree variables at the beginning of the event
  fnstamps0=0;   fnstamps1=0;   fnstamps2=0;   fnstamps3=0;   fnstamps4=0;
  
  if (finclude_wr) {
    for (auto handle : fragmentHandles) {
      if (!handle.isValid() || handle->size() == 0) continue;
      
      if (handle->front().type() == artdaq::Fragment::ContainerFragmentType) {
	//Container fragment                                                                                               
	for (auto cont : *handle) {
	  artdaq::ContainerFragment contf(cont);
	  if (contf.fragment_type()==sbndaq::detail::FragmentType::WhiteRabbit) {
	  std::cout << "    Found " << contf.block_count() << " WR Fragments in container " << std::endl;
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

  fEventTree->Fill();
  
}



void sbndaq::MultiDump::analyze_wr_fragment(artdaq::Fragment & frag)  {



      std::cout <<  "     timestamp is  " << frag.timestamp() << std::endl;
      std::cout <<  "     seq ID is " << frag.sequenceID() << std::endl;
  

      const WhiteRabbitEvent *event_ptr = reinterpret_cast<WhiteRabbitEvent const*>(frag.dataBeginBytes()); 
      timespec sysTime=event_ptr->systemTime;
      std::cout << "systime  " << sysTime.tv_sec << " " << sysTime.tv_nsec << std::endl;
      WhiteRabbitData fragdata=event_ptr->data;

      // each WR fragment has data from only one channel. The fragments are not always in time order
      std::cout << "WR: command " << fragdata.command << std::endl;
      std::cout << "WR: channel " << fragdata.channel << std::endl;
      std::cout << "WR: value " << fragdata.value << std::endl;
      std::cout << "WR: flags " << fragdata.flags << std::endl;
      std::cout << "WR: number of time stamps in this fragment " << fragdata.nstamp << std::endl;
      if (fragdata.channel==0)  fnstamps0+=fragdata.nstamp;
      if (fragdata.channel==1)	fnstamps1+=fragdata.nstamp;
      if (fragdata.channel==2)	fnstamps2+=fragdata.nstamp; 
      if (fragdata.channel==3)  fnstamps3+=fragdata.nstamp; 
      if (fragdata.channel==4)	fnstamps4+=fragdata.nstamp;

      for (int i=0;i<(int)fragdata.nstamp;++i) {

	uint diff = 0;
	uint this_time = fragdata.timeStamp[i].tv_nsec;
	if (TTS_ns>this_time) {
	  diff = TTS_ns-this_time;
	  if (diff>500000000) diff = 10000000000-diff;
	}
	else {
	  diff = this_time-TTS_ns;
	  if (diff>500000000) diff = 10000000000-diff;
	}

	// print out only timestamps within specified window of TTS
	if (diff<(uint)fWindow && fragdata.channel==1) 	
	  std::cout << " Event " << fEvent << " PMT" <<
	    " Timestamp " << i << "  : " << fragdata.timeStamp[i].tv_sec << " " << 
	    fragdata.timeStamp[i].tv_nsec << 
	    " TTS " << TTS_ns << 
	    " TTS diff " << diff << std::endl;
	if (diff<500000000 && fragdata.channel==2) 	
	  std::cout << " Event " << fEvent << " RWM" << 
	    " Timestamp " << i << "  : " << fragdata.timeStamp[i].tv_sec << " " << 
	    fragdata.timeStamp[i].tv_nsec << 
	    " TTS " << TTS_ns << 
	    " TTS diff " << diff << std::endl;

	// print out all timestamps
	// std::cout << "Timestamp " << i << "  : " << fragdata.timeStamp[i].tv_sec << " " << 
	//   printf("%09d",(int)fragdata.timeStamp[i].tv_nsec) << " TTS diff " << diff << std::endl; 

      }    
      std::cout << " ----------------------- " << std::endl;

  // }
  // }
}

void sbndaq::MultiDump::analyze_caen_fragment(artdaq::Fragment & frag)  {
  

      std::cout <<  "     timestamp is  " << frag.timestamp() << std::endl;
      std::cout <<  "     seq ID is " << frag.sequenceID() << std::endl;
  


    // for (size_t idx = 0; idx < rawFragHandle->size(); ++idx) { /*loop over the fragments*/
    //   //--use this fragment as a reference to the same data
    //   const auto& frag((*rawFragHandle)[idx]); 
      
      CAENV1730Fragment bb(frag);
      auto const* md = bb.Metadata();
      CAENV1730Event const* event_ptr = bb.Event();
      CAENV1730EventHeader header = event_ptr->Header;
      
      int fragId = static_cast<int>(frag.fragmentID()); 
      fragId-=fShift; 
      //
      std::cout << "\tFrom header, event counter is "  << header.eventCounter   << "\n";
      std::cout << "\tFrom header, triggerTimeTag is " << header.triggerTimeTag << "\n";
      std::cout << "\tFrom header, board id is "       << header.boardID       << "\n";
      std::cout << "\tFrom fragment, fragment id is "  << fragId << "\n";
      std::cout << "\tShift back, fragment id of "  << fShift << "\n";
      
      uint32_t t0 = header.triggerTimeTag;
      TTS_ns = t0*8;
      std::cout << "\n\tTriggerTimeTag in ns is " << 8*header.triggerTimeTag << "\n";  // 500 MHz is 2 ns per tick
      hEventCounter->Fill(header.eventCounter);
      hTriggerTimeTag->Fill(t0);
      nt_header->Fill(fEvent,header.eventCounter,t0);
      nChannels = md->nChannels;
      std::cout << "\tNumber of channels: " << nChannels << "\n";
      
      //--get the number of 32-bit words (quad_bytes) from the header
      uint32_t ev_size_quad_bytes = header.eventSize;
      std::cout << "Event size in quad bytes is: " << ev_size_quad_bytes << "\n";
      uint32_t evt_header_size_quad_bytes = sizeof(CAENV1730EventHeader)/sizeof(uint32_t);
      uint32_t data_size_double_bytes = 2*(ev_size_quad_bytes - evt_header_size_quad_bytes);
      uint32_t wfm_length = data_size_double_bytes/nChannels;
      //--note, needs to take into account channel mask
      std::cout << "Channel waveform length = " << wfm_length << "\n";
      
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
	//std::cout << "ch" << i_ch << " offset =" << ch_offset << std::endl;
	
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
    // } //--end loop fragments 
      

}

DEFINE_ART_MODULE(sbndaq::MultiDump)

