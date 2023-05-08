////////////////////////////////////////////////////////////////////////
// Class:       CAENV1730WaveformAna
// Module Type: analyzer
// File:        CAENV1730WaveformAna_module.cc
// Description: Makes a tree with waveform information.
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
#include <numeric>

namespace sbndaq {
  class CAENV1730WaveformAna;
}

/*****/

class sbndaq::CAENV1730WaveformAna : public art::EDAnalyzer {

public:
  explicit CAENV1730WaveformAna(fhicl::ParameterSet const & pset); // explicit doesn't allow for copy initialization
  virtual ~CAENV1730WaveformAna();
  
  virtual void analyze(art::Event const & evt);
  
private:
  
  TNtuple* nt_header; //Ntuple header
  TTree* nt_wvfm;
  std::vector< std::vector<uint16_t> >  fWvfmsVec;
  
  int fArt_ev, fCaen_ev, fCh;
  double fCaen_ev_tts, fPed, fRMS, fTemp, fStampTime;
};

//Define the constructor
sbndaq::CAENV1730WaveformAna::CAENV1730WaveformAna(fhicl::ParameterSet const & pset)
  : EDAnalyzer(pset)
{
  art::ServiceHandle<art::TFileService> tfs; //pointer to a file named tfs
  nt_header = tfs->make<TNtuple>("nt_header","CAENV1730 Header Ntuple","art_ev:caen_ev:caen_ev_tts");
  //nt_wvfm = tfs->make<TNtuple>("nt_wvfm","Waveform information Ntuple","art_ev:caen_ev:caen_ev_tts:ch:ped:rms:temp:stamp_time");
  nt_wvfm = tfs->make<TTree>("nt_wvfm","Waveform information TTree");
  nt_wvfm->Branch("art_ev",&fArt_ev,"art_ev/I");
  nt_wvfm->Branch("caen_ev",&fCaen_ev,"caen_ev/I");
  nt_wvfm->Branch("caen_ev_tts",&fCaen_ev_tts,"caen_ev_tts/D");
  nt_wvfm->Branch("ch",&fCh,"ch/I");
  nt_wvfm->Branch("ped",&fPed,"ped/D");
  nt_wvfm->Branch("rms",&fRMS,"rms/D");
  nt_wvfm->Branch("temp",&fTemp,"temp/D");
  nt_wvfm->Branch("stamp_time",&fStampTime,"stamp_time/D");

}

sbndaq::CAENV1730WaveformAna::~CAENV1730WaveformAna()
{
}

void sbndaq::CAENV1730WaveformAna::analyze(art::Event const & evt)
{
  
  art::EventNumber_t eventNumber = evt.event();
  
  art::Handle< std::vector<artdaq::Fragment> > rawFragHandle; // it is a pointer to a vector of art fragments
  evt.getByLabel("daq","CAENV1730", rawFragHandle); // it says how many fragments are in an event
  
  if (!rawFragHandle.isValid()) return;
  
  std::cout << "######################################################################" << std::endl;
  std::cout << std::endl;
  
  std::cout << "Run " << evt.run() << ", subrun " << evt.subRun()
	    << ", event " << eventNumber << " has " << rawFragHandle->size()
	    << " fragment(s)." << std::endl;
  
  for (size_t idx = 0; idx < rawFragHandle->size(); ++idx) { // loop over the fragments of an event
    
    const auto& frag((*rawFragHandle)[idx]); // use this fragment as a refernce to the same data
    CAENV1730Fragment bb(frag);
    
    auto const* md = bb.Metadata();

    CAENV1730Event const* event_ptr = bb.Event();
    CAENV1730EventHeader header = event_ptr->Header;
    
    std::cout << "\tFrom header, event counter is " << header.eventCounter << std::endl;
    std::cout << "\tFrom header, triggerTimeTag is " << header.triggerTimeTag << std::endl;
    
    nt_header->Fill(eventNumber,header.eventCounter,header.triggerTimeTag);
    
    
    //get the number of 32-bit words from the header
    size_t const& ev_size(header.eventSize);
    
    size_t nChannels = md->nChannels; //fixme
    fWvfmsVec.resize(nChannels);

    const uint16_t* data_begin = reinterpret_cast<const uint16_t*>(frag.dataBeginBytes() 
								   + sizeof(CAENV1730EventHeader));
    std::cout<<" DATA BEGIN: "<<data_begin<<std::endl;
    //use that to get the number of 16-bit words for each channel
    size_t n_samples = (ev_size - sizeof(CAENV1730EventHeader)/sizeof(uint32_t))*2/nChannels;
    const uint16_t* data = reinterpret_cast<const uint16_t*>(frag.dataBeginBytes() + sizeof(CAENV1730EventHeader));
    std::cout<<"NCh:"<<nChannels<<" nSamples: "<<n_samples<<std::endl;

    const uint16_t* value_ptr =  data_begin;
    uint16_t value = 0;

    for(size_t i_ch=0; i_ch<nChannels; ++i_ch){
      fWvfmsVec[i_ch].resize(n_samples);
      
      //fill...
      size_t ch_offset = i_ch * n_samples;
      std::cout<<"WFANA "<< i_ch << " off="<<ch_offset<<std::endl;
      for (size_t i_t=0; i_t<n_samples; ++i_t){
	//std::cout<<"NNN "<<n_samples<<std::endl;
	/*if(i_t%2==0) {
	  fWvfmsVec[i_ch][i_t] = *(data+ch_offset+i_t+1);
	  if(i_t<15) std::cout<<*(data+ch_offset+i_t+1)<<std::endl;
	}
	else if(i_t%2==1) fWvfmsVec[i_ch][i_t] = *(data+ch_offset+i_t-1);*/
	value_ptr = data_begin + ch_offset + i_t; /*pointer arithmetic*/
	value = *(value_ptr);
	//std::cout<<"WFANA "<< i_ch << " off="<<ch_offset<<std::endl;
	fWvfmsVec[i_ch][i_t] = value;
      }

      //by here you have a vector<uint16_t> that is the waveform, in fWvfmsVec[i_ch]

      fArt_ev = eventNumber;
      fCaen_ev = header.eventCounter;
      fCaen_ev_tts = header.triggerTimeTag;
      fCh = i_ch;
      //get mean
      fPed = std::accumulate(fWvfmsVec[i_ch].begin(),fWvfmsVec[i_ch].end(),0.0) / fWvfmsVec[i_ch].size();
      
      //get rms
      fRMS =0.0;
      for(auto const& val : fWvfmsVec[i_ch])
	fRMS += (val-fPed)*(val-fPed);
      fRMS = std::sqrt(fRMS/fWvfmsVec[i_ch].size());
      
      fTemp = md->chTemps[i_ch];
      fStampTime = md->timeStampSec;
      
      std::cout<<"Temp Monitor..ch "<<i_ch<<":"<<fTemp<<" TS:"<<fStampTime<<std::endl;
 
      //nt_wvfm->Fill(eventNumber,header.eventCounter,header.triggerTimeTag,i_ch,wvfm_mean,wvfm_rms,md->chTemps[i_ch], ts);
      nt_wvfm->Fill();
    }
  }
  
}

DEFINE_ART_MODULE(sbndaq::CAENV1730WaveformAna)
//this is where the name is specified
