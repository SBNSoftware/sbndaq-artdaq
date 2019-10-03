////////////////////////////////////////////////////////////////////////
// Class:       BernCRTZMQAna
// Module Type: analyzer
// File:        BernCRTZMQAna_module.cc
// Description: Makes a tree with waveform information.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"

#include "canvas/Utilities/Exception.h"

#include "sbndaq-artdaq-core/Overlays/Common/BernCRTZMQFragment.hh"
#include "artdaq-core/Data/Fragment.hh"

//#include "art/Framework/Services/Optional/TFileService.h"
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

namespace sbndaq {
  class BernCRTZMQAna;
}

/*****/

class sbndaq::BernCRTZMQAna : public art::EDAnalyzer {

public:
  explicit BernCRTZMQAna(fhicl::ParameterSet const & pset); // explicit doesn't allow for copy initialization
  virtual ~BernCRTZMQAna();
  
  virtual void analyze(art::Event const & evt);
  

private:
  
  //sample histogram
  TH1F* fSampleHist;
  TH1F* TS0;

  TTree * events;

   uint8_t mac5;
   uint16_t flags;
   uint16_t lostcpu;
   uint16_t lostfpga;
   uint32_t ts0;
   uint32_t ts1;
   //const uint16_t * adc;
   uint16_t adc[32];
   uint32_t coinc;
  
};

//Define the constructor
sbndaq::BernCRTZMQAna::BernCRTZMQAna(fhicl::ParameterSet const & pset)
  : EDAnalyzer(pset)
{

  //this is how you setup/use the TFileService
  //I do this here in the constructor to setup the histogram just once
  //but you can call the TFileService and make new objects from anywhere
  art::ServiceHandle<art::TFileService> tfs; //pointer to a file named tfs

  //make the histogram
  //the arguments are the same as what would get passed into ROOT
  fSampleHist = tfs->make<TH1F>("hSampleHist","Sample Hist Title; x axis (units); y axis (units)",100,-50,50);
  TS0 = tfs->make<TH1F>("hSampleHist_TS0","TS0 distribution; time [ns]; counts",1000,1e9-500,1e9+500);

  events = tfs->make<TTree>("events", "FEB events");

  events->Branch("mac5",        &mac5,  "mac5/b");
  events->Branch("flags",       &flags, "flags/s");
  events->Branch("lostcpu",     &lostcpu,       "lostcpu/s");
  events->Branch("lostfpga",    &lostcpu,       "lostfpga/s");
  events->Branch("ts0",         &ts0,           "ts0/i");
  events->Branch("ts1",         &ts1,           "ts1/i");
  events->Branch("adc",         &adc,           "adc[32]/s");
  events->Branch("coinc",       &coinc,         "coinc/i");

}

sbndaq::BernCRTZMQAna::~BernCRTZMQAna()
{
}

void sbndaq::BernCRTZMQAna::analyze(art::Event const & evt)
{


  //can get the art event number
  art::EventNumber_t eventNumber = evt.event();
  
  //we get a 'handle' to the fragments in the event
  //this will act like a pointer to a vector of artdaq fragments
  art::Handle< std::vector<artdaq::Fragment> > rawFragHandle;
  
  //we fill the handle by getting the right data according to the label
  //the module label will be 'daq', while the instance label (second argument) matches the type of fragment
  evt.getByLabel("daq","BERNCRTZMQ", rawFragHandle);

  //this checks to make sure it's ok
  if (!rawFragHandle.isValid()) return;


  //define an ofstream to read the data out in a file
  //
  //std::ofstream outputFile("Timestamps.txt",std::ios::app);
  
  std::cout << "######################################################################" << std::endl;
  std::cout << std::endl;  
  std::cout << "Run " << evt.run() << ", subrun " << evt.subRun()
	    << ", event " << eventNumber << " has " << rawFragHandle->size()
	    << " CRT fragment(s)." << std::endl;
  
  for (size_t idx = 0; idx < rawFragHandle->size(); ++idx) { // loop over the fragments of an event
    
    const auto& frag((*rawFragHandle)[idx]); // use this fragment as a reference to the same data

    //this applies the 'overlay' to the fragment, to tell it to treat it like a BernCRTZMQ fragment
    BernCRTZMQFragment bb(frag);
    
    //gets a pointer to the metadata from the fragment
    //type is BernCRTZMQFragmentMetadata*
    auto const* md = bb.metadata();
    std::cout << *md << std::endl;

    //gets the number of CRT events (hits) inside this fragment
    size_t n_events = md->n_events();

    //loop over the events in the fragment ...
    for(size_t i_e=0; i_e<n_events; ++i_e){

      //grab the pointer to the event
      BernCRTZMQEvent const* evt = bb.eventdata(i_e);
      
      //we can print this
      std::cout << *evt << std::endl;

      //write all timestamps in an outuputfile
      //outputFile << evt->Time_TS0() << std::endl;


      //let's fill our sample hist with the Time_TS0()-1e9 if 
      //it's a GPS reference pulse
      if(evt->IsReference_TS0()){
	fSampleHist->Fill(evt->Time_TS0()-1e9);
        //fSampleHist_TS0 -> Fill(evt->Time_TS0());
       
      }

      TS0 -> Fill(evt->Time_TS0());

      mac5 = evt->MAC5();
      flags = evt->flags;
      lostcpu = evt->lostcpu;
      lostfpga = evt->lostfpga;
      ts0 = evt->Time_TS0();
      ts1 = evt->Time_TS1();
      coinc = evt->coinc;

      //adc = evt->ADC();
      for(int ch=0; ch<32; ch++) adc[ch] = evt->ADC(ch);

      events->Fill();

    }//end loop over events in a fragment
  

  }//end loop over fragments

  //just close the output stream
  //outputFile.close();

}

DEFINE_ART_MODULE(sbndaq::BernCRTZMQAna)
//this is where the name is specified
