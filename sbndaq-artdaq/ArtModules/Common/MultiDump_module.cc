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
#include "artdaq-core/Data/Fragment.hh"

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
    fhicl::Atom<art::InputTag> DataLabelPMT {
      fhicl::Name("data_label_caen_pmt"),
      fhicl::Comment("Tag for the input data product for CAEN 1730")
    };
    fhicl::Atom<bool> include_caen {
      fhicl::Name("include_caen"),
      fhicl::Comment("look for caen 1730 fragments true/false")
    };
    fhicl::Atom<int> Shift {
      fhicl::Name("shift_fragment_id"), 
      fhicl::Comment("Number to subtract to the fragment_id")
    };
    fhicl::Atom<art::InputTag> DataLabelWR {
      fhicl::Name("data_label_wr"),
      fhicl::Comment("Tag for the input data product for WR DIO")
    };
    fhicl::Atom<bool> include_wr {
      fhicl::Name("include_wr"),
      fhicl::Comment("look for wr dio fragments true/false")
    };
  }; //--configuration
  using Parameters = art::EDAnalyzer::Table<Config>;

  explicit MultiDump(Parameters const & pset);
  virtual ~MultiDump();

  void analyze(const art::Event& evt) override;
  void beginJob() override;
  void endJob() override;

private:

  //--default values
  uint32_t nChannels;//    = 16;
  uint32_t Ttt_DownSamp;// =  4; 
 /* the trigger time resolution is 16ns when waveforms are sampled at
                               * 500MHz sampling. The trigger timestamp is thus
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
  int fnstamps;
  std::vector<uint64_t> 
  
  bool firstEvt = true;
  art::InputTag fDataLabelPMT;
  bool finclude_caen;
  int fShift; 
  art::InputTag fDataLabelWR;
  bool finclude_wr;
	

}; //--class MultiDump


sbndaq::MultiDump::MultiDump(MultiDump::Parameters const& pset): art::EDAnalyzer(pset)
{
  fDataLabelPMT = pset().DataLabelPMT();
  finclude_caen = pset().include_caen();
  fShift = pset().Shift();
  fDataLabelWR = pset().DataLabelWR();
  finclude_wr = pset().include_wr();
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
    fEventTree->Branch("nstamps",&fnstamps);
    fEventTree->Branch("",);
    fEventTree->Branch("",);
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
    
  /************************************************************************************************/
  art::Handle< std::vector<artdaq::Fragment> > rawFragHandle;
  //evt.getByLabel("daq","CAENV1730", rawFragHandle); 
  //evt.getByLabel(fDataLabel,"CAENV1730", rawFragHandle); 
  //<--std::vector<art::Ptr<artdaq::Fragment>> Frags;

  if (finclude_caen) {
  if ( !evt.getByLabel(fDataLabelPMT, rawFragHandle) ) {
//    art::fill_ptr_vector(Frags,rawFragHandle);
//  }
//  else {
    std::cout << "Requested fragments with label : " << fDataLabelPMT << "but none exist\n";
    return;
  }

  if (rawFragHandle.isValid()) {

    std::cout << "######################################################################\n";
    std::cout << "Run " << evt.run() << ", subrun " << evt.subRun()
              << ", event " << fEvent << " has " << rawFragHandle->size()
              << " fragment(s).\n";

    

    fWvfmsVec.resize(16*rawFragHandle->size());

    //bool firstEvt = true;
    for (size_t idx = 0; idx < rawFragHandle->size(); ++idx) { /*loop over the fragments*/
      //--use this fragment as a reference to the same data
      const auto& frag((*rawFragHandle)[idx]); 
      CAENV1730Fragment bb(frag);
      auto const* md = bb.Metadata();
      CAENV1730Event const* event_ptr = bb.Event();
      CAENV1730EventHeader header = event_ptr->Header;
      int fragId = static_cast<int>(frag.fragmentID()); 
      fragId-=fShift; 
      std::cout << "\tFrom header, event counter is "  << header.eventCounter   << "\n";
      std::cout << "\tFrom header, triggerTimeTag is " << header.triggerTimeTag << "\n";
      std::cout << "\tFrom header, board id is "       << header.boardID       << "\n";
      std::cout << "\tFrom fragment, fragment id is "  << fragId << "\n";
      std::cout << "\tShift back, fragment id of"  << fShift << "\n";

      uint32_t t0 = header.triggerTimeTag;
      hEventCounter->Fill(header.eventCounter);
      hTriggerTimeTag->Fill(t0);
      nt_header->Fill(fEvent,header.eventCounter,t0);
      nChannels = md->nChannels;
      std::cout << "\tNumber of channels: " << nChannels << "\n";
      //fWvfmsVec.resize(nChannels*(idx+1));

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
        //dumpouttext<<fEvent<<"\t"<<idx<<"\t"<<i_ch<<std::endl;
        for(size_t i_t=0; i_t<wfm_length; ++i_t){ 
          fTicksVec[i_t] = t0*Ttt_DownSamp + i_t;   /*timestamps, event level*/
          value_ptr = data_begin + ch_offset + i_t; /*pointer arithmetic*/
          value = *(value_ptr);

          if (i_ch == 0 && firstEvt) { 
            h_wvfm_ev0_ch0->SetBinContent(i_t,value);
            //std::cout << "ch" << std::to_string(i_ch) << "[" << i_t << "] = " << value <<  "= 0b" << std::bitset<16>(value) 
            //          << std::endl;
          }

          fWvfmsVec[i_ch+nChannels*fragId][i_t] = value;

          //dumpouttext<<value<<"\t";


        } //--end loop samples
        firstEvt = false;
      } //--end loop channels
    } //--end loop fragments 

  } //   if (rawFragHandle.isValid()) {

  } // if (finclude_caen)

  // now fetch white rabbit fragments for this event
  if (finclude_wr) {

  if ( !evt.getByLabel(fDataLabelWR, rawFragHandle) ) {
    std::cout << "Requested fragments with label : " << fDataLabelWR << " but none exist\n";
    return;
  }

  if (rawFragHandle.isValid()) {

    std::cout << "######################################################################\n";
    std::cout << "Run " << evt.run() << ", subrun " << evt.subRun()
              << ", event " << fEvent << " has " << rawFragHandle->size()
              << " fragment(s).\n";

    /*    
    std::cout << "WRevent size " << sizeof(WhiteRabbitEvent) << std::endl;
    std::cout << "WRdata size " << sizeof(WhiteRabbitData) << std::endl;
      std::cout << sizeof(bb.command) << std::endl;
      std::cout << sizeof(bb.channel) << std::endl;
      std::cout << sizeof(bb.value) << std::endl;
      std::cout << sizeof(bb.flags) << std::endl;
      std::cout << sizeof(bb.nstamp) << std::endl;
    */

    for (size_t idx = 0; idx < rawFragHandle->size(); ++idx) { /*loop over the fragments*/

      const auto& frag((*rawFragHandle)[idx]); 
      const WhiteRabbitEvent *event_ptr = reinterpret_cast<WhiteRabbitEvent const*>(frag.dataBeginBytes()); 
      timespec sysTime=event_ptr->systemTime;
      std::cout << "systime  " << sysTime.tv_sec << " " << sysTime.tv_nsec << std::endl;
      WhiteRabbitData fragdata=event_ptr->data;

      std::cout << "1: command " << fragdata.command << std::endl;
      std::cout << "1: channel " << fragdata.channel << std::endl;
      std::cout << "1: value " << fragdata.value << std::endl;
      std::cout << "1: flags " << fragdata.flags << std::endl;
      std::cout << "1: number of time stamps in this fragment " << fragdata.nstamp << std::endl;
      fnstamps=fragdata.nstamp;
      if (fnstamps>16) fnstamps=16;
      for (int i=0;i<fnstamps;++i) {
      	std::cout << "Timestamp " << i << "  : " << fragdata.timeStamp[i].tv_sec << " " << fragdata.timeStamp[i].tv_nsec << std::endl;
	//      	std::cout << "Timestamp " << i << "  : " << difftime(sysTime.tv_sec,fragdata.timeStamp[i].tv_sec) << " " << fragdata.timeStamp[i].tv_nsec << std::endl;
      }
    

  }
  }
     
  fEventTree->Fill();

  } //--valid fragments
}

DEFINE_ART_MODULE(sbndaq::MultiDump)

