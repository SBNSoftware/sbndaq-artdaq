////////////////////////////////////////////////////////////////////////
// Class:       CAENV1730Dump
// Module Type: analyzer
// File:        CAENV1730Dump_module.cc
// Description: Prints out information about each event.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"

#include "canvas/Utilities/Exception.h"

#include "sbndaq-artdaq-core/Overlays/Common/CAENV1730Fragment.hh"
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
  class CAENV1730Dump;
}

/**************************************************************************************************/

class sbndaq::CAENV1730Dump : public art::EDAnalyzer {

public:
  struct Config {
    //--one atom for each parameter
    fhicl::Atom<art::InputTag> DataLabel {
      fhicl::Name("data_label"),
      fhicl::Comment("Tag for the input data product")
    };
  }; //--configuration
  using Parameters = art::EDAnalyzer::Table<Config>;

  explicit CAENV1730Dump(Parameters const & pset);
  virtual ~CAENV1730Dump();

  void analyze(const art::Event& evt) override;
  void beginJob() override;
  void endJob() override;

private:

  //--default values
  uint32_t nChannels    = 16;
  uint32_t Ttt_DownSamp =  4; /* the trigger time resolution is 16ns when waveforms are sampled at
                               * 500MHz sampling. The trigger timestamp is thus
                               * sampled 4 times slower than input channels*/

  TNtuple* nt_header;
  
  TH1F*    hEventCounter;
  TH1F*    hTriggerTimeTag;
  TH1F*    h_wvfm_ev0_ch0;

  TTree* fEventTree;
  int fRun, fEvent, fFragID;
  size_t fNumBoards;
  std::vector<uint64_t>  fTicksVec;
  std::vector< std::vector<uint16_t> >  fWvfmsVec;
  
  bool firstEvt = true;
  art::InputTag fDataLabel;

}; //--class CAENV1730Dump


sbndaq::CAENV1730Dump::CAENV1730Dump(CAENV1730Dump::Parameters const& pset): art::EDAnalyzer(pset)
{
  fDataLabel = pset().DataLabel();
}

void sbndaq::CAENV1730Dump::beginJob()
{
  art::ServiceHandle<art::TFileService> tfs; 
  nt_header       = tfs->make<TNtuple>("nt_header","CAENV1730 Header Ntuple","art_ev:caen_ev:caenv_ev_tts"); 
  /************************************************************************************************/
  hEventCounter   = tfs->make<TH1F>("hEventCounter","Event Counter Histogram",10000,0,10000);
  hTriggerTimeTag = tfs->make<TH1F>("hTriggerTimeTag","Trigger Time Tag Histogram",10,2000000000,4500000000);
  h_wvfm_ev0_ch0  = tfs->make<TH1F>("h_wvfm_ev0_ch0","Waveform",2000,0,2000);  
  /************************************************************************************************/
  //--make tree to store the channel waveform info:
  fEventTree = tfs->make<TTree>("events","waveform tree");
  fEventTree->Branch("fRun",&fRun,"fRun/I");
  fEventTree->Branch("fEvent",&fEvent,"fEvent/I");
  fEventTree->Branch("fNumBoards",&fNumBoards,"fNumbBoards/I");
  fEventTree->Branch("fFragID",&fFragID);
  fEventTree->Branch("fTicksVec",&fTicksVec);
  fEventTree->Branch("fWvfmsVec",&fWvfmsVec);
}

void sbndaq::CAENV1730Dump::endJob()
{
  std::cout << "Ending CAENV1730Dump...\n";
}


sbndaq::CAENV1730Dump::~CAENV1730Dump()
{
}


void sbndaq::CAENV1730Dump::analyze(const art::Event& evt)
{

  fRun = evt.run();
  fEvent = evt.event();
    
  /************************************************************************************************/
  art::Handle< std::vector<artdaq::Fragment> > rawFragHandle;
  if ( !evt.getByLabel(fDataLabel, rawFragHandle) ) {
    std::cout << "Requested fragments with label : " << fDataLabel << "but none exist\n";
    return;
  }

  if (rawFragHandle.isValid()) {

    std::cout << "######################################################################\n";
    std::cout << "Run " << evt.run() << ", subrun " << evt.subRun()
              << ", event " << fEvent << " has " << rawFragHandle->size()
              << " fragment(s).\n";

    //bool firstEvt = true;
    fNumBoards = rawFragHandle->size();
    int fragID = -1, it = 0;
    for (auto const& frag: *rawFragHandle) { /*loop over the fragments*/

      //--Retrive the fragment id which distinguishes each V1730 board and is set in their
      //-- respective .fcl file
      fragID = static_cast<int>(frag.fragmentID());
      fFragID=fragID;
      //<--fFragID.push_back(fragID);
      std::cout << "Retrieved fragment id for fragment number " << (it+1) << "-of-" << fNumBoards
                << " is : " << fragID << std::endl;

      //--use this fragment as a reference to the same data
      CAENV1730Fragment bb(frag);
      auto const* md = bb.Metadata();
      CAENV1730Event const* event_ptr = bb.Event();

      CAENV1730EventHeader header = event_ptr->Header;

      std::cout << "\tFrom header, event counter is "  << header.eventCounter   << "\n";
      std::cout << "\tFrom header, triggerTimeTag is " << header.triggerTimeTag << "\n";

      uint32_t t0 = header.triggerTimeTag;
      hEventCounter->Fill(header.eventCounter);
      hTriggerTimeTag->Fill(t0);
      nt_header->Fill(fEvent,header.eventCounter,t0);
      nChannels = md->nChannels;
      std::cout << "\tNumber of channels: " << nChannels << "\n";
      fWvfmsVec.resize(nChannels);

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
        fWvfmsVec[i_ch].resize(wfm_length);
        ch_offset = (size_t)(i_ch * wfm_length);
        //std::cout << "ch" << i_ch << " offset =" << ch_offset << std::endl;

        //--loop over waveform samples
        for(size_t i_t=0; i_t<wfm_length; ++i_t){ 
          fTicksVec[i_t] = t0*Ttt_DownSamp + i_t;   /*timestamps, event level*/
          value_ptr = data_begin + ch_offset + i_t; /*pointer arithmetic*/
          value = *(value_ptr);

          if (i_ch == 0 && firstEvt) { 
            h_wvfm_ev0_ch0->SetBinContent(i_t,value);
            //std::cout << "ch" << std::to_string(i_ch) << "[" << i_t << "] = " << value <<  "= 0b" << std::bitset<16>(value) 
            //          << std::endl;
          }

          fWvfmsVec[i_ch][i_t] = value;
        } //--end loop samples
        firstEvt = false;
      } //--end loop channels
      ++it;
      fEventTree->Fill();
    } //--end loop fragments 
     
  //<--fEventTree->Fill();

  } //--valid fragments
}

DEFINE_ART_MODULE(sbndaq::CAENV1730Dump)
