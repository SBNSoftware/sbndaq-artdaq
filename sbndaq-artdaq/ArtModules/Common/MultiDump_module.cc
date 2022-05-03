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
  std::vector<int> fPMT_chA;
  std::vector<int> fPMT_chB;
  std::vector<int> fPMT_chC;
  std::vector<int> fPMT_chD;
  std::vector<int> fPMT_chE;
  std::vector<int> fRWM;

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
  

  bool finclude_caen;
  int fShift; 
  bool finclude_wr;
  int fWindow;
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
    fEventTree->Branch("TTT",&TTT,"TTT/I");
    fEventTree->Branch("fTicksVec",&fTicksVec);
    fEventTree->Branch("fWvfmsVec",&fWvfmsVec);    
    fEventTree->Branch("fPMT_chA",&fPMT_chA);
    fEventTree->Branch("fPMT_chB",&fPMT_chB);
    fEventTree->Branch("fPMT_chC",&fPMT_chC);
    fEventTree->Branch("fPMT_chD",&fPMT_chD);
    fEventTree->Branch("fPMT_chE",&fPMT_chE);
    fEventTree->Branch("fRWM",&fRWM);
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
  fPMT_chA.clear();   fPMT_chB.clear();   fPMT_chC.clear();
  fPMT_chD.clear();   fPMT_chE.clear();   fRWM.clear();
  /************************************************************************************************/

  //  Note that this code expects exactly 1 CAEN fragment per event
  TTT_ns=0;  // will be set to value in CAEN fragement header

  first_wr_ch0=0;
  first_wr_ch1=0;
  first_wr_ch2=0;
  first_wr_ch3=0;
  first_wr_ch4=0;


  std::vector<art::Handle<artdaq::Fragments>> fragmentHandles;
  //evt.getManyByType(fragmentHandles);
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

  fEventTree->Fill();
  
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
  
  for (int i=0;i<(int)fragdata.nstamp;++i) {
    
    
    int diff = 0;
    uint this_time_sec = fragdata.timeStamp[i].tv_sec;
    uint this_time = fragdata.timeStamp[i].tv_nsec;
    // if (TTT_ns>this_time) {
    //    std::cout << this_time_sec << std::endl;
    diff = TTT_ns-this_time;
    if (diff>500000000) diff = 1000000000-diff;
    else if (diff<-500000000) diff = 1000000000+diff;
    // }
    // else {
    //   diff = this_time-TTT_ns;
    //   if (diff>500000000) diff = 1000000000-diff;
    // } 
    //	if (diff>500000000) std::cout<< "diff " << diff << "this_time " << this_time << " TTT_ns " << TTT_ns << std::endl;
    
    if (fabs(diff)<fWindow && fragdata.channel==1) 	 {
      if (first_wr_ch1==0) first_wr_ch1=fragdata.timeStamp[i].tv_sec;
      int secdiff=0;
      if (fragdata.timeStamp[i].tv_sec>first_wr_ch1) secdiff=this_time_sec-first_wr_ch1;
      fWR_ch1.emplace_back((int)this_time+1e9*secdiff); 
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
  int threshold=7800;
  int i_ch = 2; int toggle=0;
  auto this_value = fWvfmsVec[i_ch+nChannels*fragId][0];
  if (this_value<threshold) toggle=1;
  for(size_t i_t=1; i_t<wfm_length; ++i_t){     
    this_value = fWvfmsVec[i_ch+nChannels*fragId][i_t];
    if (toggle==0 && this_value<threshold) {
      toggle=1;
      fPMT_chA.emplace_back(i_t);
    }
    if (toggle==1 && this_value>threshold) toggle=0;
  }

  // find leading edges in waveforms
  threshold=7800;
  i_ch = 4; toggle=0;
  this_value = fWvfmsVec[i_ch+nChannels*fragId][0];
  if (this_value<threshold) toggle=1;
  for(size_t i_t=1; i_t<wfm_length; ++i_t){     
    this_value = fWvfmsVec[i_ch+nChannels*fragId][i_t];
    if (toggle==0 && this_value<threshold) {
      toggle=1;
      fPMT_chB.emplace_back(i_t);
    }
    if (toggle==1 && this_value>threshold) toggle=0;
  }

  // find leading edges in waveforms
  threshold=7800;
  i_ch = 8; toggle=0;
  this_value = fWvfmsVec[i_ch+nChannels*fragId][0];
  if (this_value<threshold) toggle=1;
  for(size_t i_t=1; i_t<wfm_length; ++i_t){     
    this_value = fWvfmsVec[i_ch+nChannels*fragId][i_t];
    if (toggle==0 && this_value<threshold) {
      toggle=1;
      fPMT_chC.emplace_back(i_t);
    }
    if (toggle==1 && this_value>threshold) toggle=0;
  }

  // find leading edges in waveforms
  threshold=7800;
  i_ch = 10; toggle=0;
  this_value = fWvfmsVec[i_ch+nChannels*fragId][0];
  if (this_value<threshold) toggle=1;
  for(size_t i_t=1; i_t<wfm_length; ++i_t){     
    this_value = fWvfmsVec[i_ch+nChannels*fragId][i_t];
    if (toggle==0 && this_value<threshold) {
      toggle=1;
      fPMT_chD.emplace_back(i_t);
    }
    if (toggle==1 && this_value>threshold) toggle=0;
  }

  // find leading edges in waveforms
  threshold=7800;
  i_ch = 12; toggle=0;
  this_value = fWvfmsVec[i_ch+nChannels*fragId][0];
  if (this_value<threshold) toggle=1;
  for(size_t i_t=1; i_t<wfm_length; ++i_t){     
    this_value = fWvfmsVec[i_ch+nChannels*fragId][i_t];
    if (toggle==0 && this_value<threshold) {
      toggle=1;
      fPMT_chE.emplace_back(i_t);
    }
    if (toggle==1 && this_value>threshold) toggle=0;
  }

  // find leading edges in waveforms
  threshold=7800;
  i_ch = 15; toggle=0;
  this_value = fWvfmsVec[i_ch+nChannels*fragId][0];
  if (this_value<threshold) toggle=1;
  for(size_t i_t=1; i_t<wfm_length; ++i_t){     
    this_value = fWvfmsVec[i_ch+nChannels*fragId][i_t];
    if (toggle==0 && this_value<threshold) {
      toggle=1;
      fRWM.emplace_back(i_t);
    }
    if (toggle==1 && this_value>threshold) toggle=0;
  }


  

}

DEFINE_ART_MODULE(sbndaq::MultiDump)

