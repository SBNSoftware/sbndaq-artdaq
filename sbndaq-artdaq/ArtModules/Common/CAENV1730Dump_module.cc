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

#include "art/Framework/Services/Optional/TFileService.h"
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
  class CAENV1730Dump;
}

/*****/

class sbndaq::CAENV1730Dump : public art::EDAnalyzer {
// sbndaq::CAENV1730Dump means that in the namespace sbndaq I choose the type CAENV1730Dump
// if I had also 'class Ciao;' I could do sbndaq::Ciao

	public:
 	 explicit CAENV1730Dump(fhicl::ParameterSet const & pset); // explicit doesn't allow for copy initialization
  	 virtual ~CAENV1730Dump();

  	 virtual void analyze(art::Event const & evt);

	private:

	  TH1F*    hEventCounter;
          TH1F*    hTriggerTimeTag;
	  
          TNtuple* nt_header; //Ntuple header
          //TNtuple* nt_header;

};





//Define the constructor
sbndaq::CAENV1730Dump::CAENV1730Dump(fhicl::ParameterSet const & pset)
    : EDAnalyzer(pset) //EDAnalyzer (which is a data member of some class) is initialized with pset
{
  art::ServiceHandle<art::TFileService> tfs; //pointer to a file named tfs

  hEventCounter = tfs->make<TH1F>("hEventCounter","Event Counter Histogram",10000,0,10000);
  //initialize the private element  


  // modified by froccati Aug 10th 2017

  hTriggerTimeTag = tfs->make<TH1F>("hTriggerTimeTag","Trigger Time Tag Histogram",10,2000000000,4500000000);

  /**************************************/

  nt_header = tfs->make<TNtuple>("nt_header","CAENV1730 Header Ntuple","art_ev:caen_ev:caenv_ev_tts"); //(tRIGGERtIMEsTAMP) things in "" are just labels!
  //initialize the private element (an ntuple is like a vector somehow..)
}





sbndaq::CAENV1730Dump::~CAENV1730Dump()
{
}





void sbndaq::CAENV1730Dump::analyze(art::Event const & evt)
{
  art::EventNumber_t eventNumber = evt.event();
  
  //modified by froccati Aug 10th 2017

  //	art::Timestamp eventTimestamp = evt.time(); // now I say event because event=fragment at the time of this modification

  /****************************************************/
  art::Handle< std::vector<artdaq::Fragment> > rawFragHandle; // it is a pointer to a vector of art fragments
  evt.getByLabel("daq","CAENV1730", rawFragHandle); // it says how many fragments are in an event

  if (rawFragHandle.isValid()) {

    std::cout << "######################################################################" << std::endl;
    std::cout << std::endl;
   
    std::cout << "Run " << evt.run() << ", subrun " << evt.subRun()
              << ", event " << eventNumber << " has " << rawFragHandle->size()
              << " fragment(s)." << std::endl;

    for (size_t idx = 0; idx < rawFragHandle->size(); ++idx) { // loop over the fragments of an event
	//std::cout << "idx = " << idx <<std::endl;
      const auto& frag((*rawFragHandle)[idx]); // use this fragment as a refernce to the same data
	// I could write artdaq::Fragment instead of auto
      CAENV1730Fragment bb(frag);

	// modified by froccati Aug 10th 2017

        //std::cout << "Timestamp of the " << idx << "th fragment of this event: " << (bb.Event()->Header).triggerTimeTag << std::endl;
        /*****************************************/

      //CAENV1730FragmentMetadata const* metadata_ptr = bb.Metadata();
      CAENV1730Event const* event_ptr = bb.Event();

      CAENV1730EventHeader header = event_ptr->Header;

      std::cout << "\tFrom header, event counter is " << header.eventCounter << std::endl;
      std::cout << "\tFrom header, triggerTimeTag is " << header.triggerTimeTag << std::endl;
     
      hEventCounter->Fill(header.eventCounter);
      hTriggerTimeTag->Fill(header.triggerTimeTag);
      
      nt_header->Fill(eventNumber,header.eventCounter,header.triggerTimeTag);// ,eventTimestamp,header.triggerTimeTag);  // I added header.triggerTimeTag


	 //only doing this for the first event, 
      if(eventNumber!=0) continue;

      //get the number of 32-bit words from the header
      uint32_t ev_size = header.eventSize;
      std::cout << "Event size is " << ev_size << std::endl;
      
      //use that to get the number of 16-bit words for each channel
      uint32_t ch_size = 2*(ev_size - sizeof(CAENV1730EventHeader)/sizeof(uint32_t))/16; //fixme
      // isn't sizeof(CAENV1730EventHeader)/sizeof(uint32_t) == 4? why is this the channel size?
      std::cout << "Channel size in event is " << ch_size << std::endl;
      //note, needs to take into account channel mask

      const uint16_t* data = reinterpret_cast<const uint16_t*>(frag.dataBeginBytes() + sizeof(CAENV1730EventHeader));
      //what is this? does it mean that I change the type of frag.dataBeginBytes() to const uint16_t* ?

      //create histogram, only for ch0 for now
      art::ServiceHandle<art::TFileService> tfs;
      TH1F* h_wvfm_ev0_ch0;
      h_wvfm_ev0_ch0 = tfs->make<TH1F>("h_wvfm_ev0_ch0","Waveform",ch_size,0,ch_size);

      for(size_t i_t=0; i_t<ch_size; ++i_t){ // why is there this if?
	if(i_t%2==0) {
	  h_wvfm_ev0_ch0->SetBinContent(i_t,*(data+i_t+1)); // what is this operator *?
	  std::cout << "\t\tTick " << i_t << " " << *(data+i_t+1) << std::endl;
	}
	else if(i_t%2==1){ 
	  h_wvfm_ev0_ch0->SetBinContent(i_t,*(data+i_t-1));
	  std::cout << "\t\tTick " << i_t << " " << *(data+i_t-1) << std::endl;
	}
      }
	
	
	
	
    }
  }

}

DEFINE_ART_MODULE(sbndaq::CAENV1730Dump)
//this is where the name is specified
