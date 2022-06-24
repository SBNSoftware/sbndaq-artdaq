////////////////////////////////////////////////////////////////////////
// Class:       PTBDump
// Module Type: analyzer
// File:        PTBDump_module.cc
// Description: Makes a tree with waveform information.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"

#include "canvas/Utilities/Exception.h"

#include "sbndaq-artdaq-core/Overlays/SBND/PTBFragment.hh"
#include "artdaq-core/Data/Fragment.hh"
#include "artdaq-core/Data/ContainerFragment.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"

#include "art_root_io/TFileService.h"
#include "TNtuple.h"
#include "TTree.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <vector>
#include <iostream>

namespace sbndaq {
  class PTBDump;
}

class sbndaq::PTBDump : public art::EDAnalyzer {

public:
  explicit PTBDump(fhicl::ParameterSet const & pset); // explicit doesn't allow for copy initialization
  virtual ~PTBDump();
  
  virtual void analyze(art::Event const & evt);
  

private:

  void analyze_fragment(artdaq::Fragment & frag);
  void extract_triggers(artdaq::Fragment & frag);
  void reset();
  
  TTree *outtree;
  
  // TODO make fcl parameter
  bool print_frags = true;
  bool save_triggers = true;

  int event_number;
  int run_number;

  bool unknown_word;
  unsigned int wtype;
  uint64_t tstamp;
  uint64_t payload;
  uint16_t crt_status;
  uint16_t beam_status;
  uint16_t mtca_status;
  uint16_t nim_status;
  uint32_t auxpds_status;

  //information from fragment header
  uint32_t  sequence_id;

};

//Define the constructor
sbndaq::PTBDump::PTBDump(fhicl::ParameterSet const & pset)
  : EDAnalyzer(pset) {

  print_frags = pset.get<bool>("DumpFrags");
  save_triggers = pset.get<bool>("SaveTriggers");

  art::ServiceHandle<art::TFileService> tfs;
  outtree = tfs->make<TTree>("data","PTB output data");

  outtree->Branch("run_number",    &run_number);
  outtree->Branch("event_number",  &event_number);
  outtree->Branch("word_type",     &wtype);
  outtree->Branch("timestamp",     &tstamp, "timestamp/l");
  outtree->Branch("payload",       &payload, "payload/l");
  outtree->Branch("beam_status",   &beam_status);
  outtree->Branch("crt_status",    &crt_status);
  outtree->Branch("mtca_status",   &mtca_status);
  outtree->Branch("nim_status",    &nim_status);
  outtree->Branch("auxpds_status", &auxpds_status); 

}

sbndaq::PTBDump::~PTBDump() {}

void sbndaq::PTBDump::analyze_fragment(artdaq::Fragment & frag) {

  CTBFragment f(frag);

  for ( unsigned int i = 0; i < f.NWords() ; ++i ) {

    if ( f.Word(i)->word_type == 0x2 ) { // HLT
      std::cout << "HL Trigger word " << std::hex << f.Trigger(i) -> word_type
          << ", payload: "  << f.Trigger(i) -> trigger_word << std::dec
          << ", TS: " << f.TimeStamp(i) << std::endl ;
    }
    else if ( f.Word(i)->word_type == 0x1 ) { // LLT
      std::cout << "LL Trigger word " << std::hex << f.Trigger(i) -> word_type
          << ", payload: "  << f.Trigger(i) -> trigger_word << std::dec
          << ", TS: " << f.TimeStamp(i) << std::endl ;
    }
    else if ( f.Word(i)->word_type == 0x3 ) { // Channel Status
      std::cout << "Check Status word " << std::hex
          << " PDS " << f.ChStatus(i) -> pds
          << ", CRT: " << f.ChStatus(i) -> crt
          << ", Beam: " << f.ChStatus(i) -> beam << std::dec
          << ", TS: " << f.TimeStamp(i)
          << std::endl ;

    }
    else if ( f.Word(i)->word_type == 0x0 ) { // Feedback (Error)
      std::cout << "Feedback word  " << std::hex
          << ", Code: " << f.Feedback(i) -> code
          << ", Source: " << f.Feedback(i) -> source
          << ", payload: " << f.Feedback(i) -> payload << std::dec
          << ", TS: " << f.TimeStamp(i) << std::endl ;

    }
    else if ( f.Word(i)->word_type == 0x7 ) { // Timestamp Word
      std::cout << "Timestamp word: " << std::hex << f.Word(i) -> word_type
          << ", payload: " << f.Word(i) -> payload << std::dec
          << ", TS: " << f.TimeStamp(i) << std::endl ;
    }
    else {
      std::cout << "Unknown Word Type!: " << std::hex << f.Word(i) -> word_type
          << std::dec << std::endl;
    }
  }

  
}

void sbndaq::PTBDump::extract_triggers(artdaq::Fragment & frag) {

  // Construct PTB fragment overlay class 
  CTBFragment ptb_fragment(frag);

  // Loop through all the PTB words in the fragment, casting to 
  // one of the 5 word types
  for ( size_t i = 0; i < ptb_fragment.NWords(); i++ ) {
    if ( ptb_fragment.Word(i)->word_type == 0x0 ) {      // Feedback (errors) Word
      // Only get this word if something goes wrong at the firmware level
      // requires expert knowledge to interpret. The appearance of this word
      // should have crashed the run.
      ptb_fragment.Feedback(i)->code;
      ptb_fragment.Feedback(i)->source;
      ptb_fragment.Feedback(i)->payload;
      ptb_fragment.TimeStamp(i);
    } 
    else if ( ptb_fragment.Word(i)->word_type == 0x1 ) { // LL Trigger
      ptb_fragment.Trigger(i)->trigger_word; // bit map of asserted LLTs 
      ptb_fragment.TimeStamp(i); // Timestamp of the word 
    } 
    else if( ptb_fragment.Word(i)->word_type == 0x2 ) {  // HL Trigger
      ptb_fragment.Trigger(i)->trigger_word;
      ptb_fragment.TimeStamp(i);
    } 
    else if ( ptb_fragment.Word(i)->word_type == 0x3 ) { // Channel Status
      // Each PTB input gets a bit map e.g. CRT has 14 inputs and is 14b 
      // (1 is asserted 0 otherwise)
      ptb_fragment.ChStatus(i)->pds;
      ptb_fragment.ChStatus(i)->crt;
      ptb_fragment.ChStatus(i)->beam;
      ptb_fragment.TimeStamp(i);
    } 
    else if ( ptb_fragment.Word(i)->word_type == 0x7 ) { // Timestamp Word
      // We don't care about this word, it only has a TS and is sent periodically.
      ptb_fragment.TimeStamp(i);
    } 
    else { // Unknown, should never happen!
      unknown_word = true;
    }
  }
 

}

void sbndaq::PTBDump::reset() {

  // Initialize to invalid values
  unknown_word = false;
  event_number = -1;        
  run_number = -1;
  wtype = -1;
  tstamp = 0;
  payload = -1;
  crt_status = -1;
  beam_status = -1;
  mtca_status = -1;
  nim_status = -1;
  auxpds_status = -1;

}

void sbndaq::PTBDump::analyze(art::Event const & evt)
{

  // Reset variables
  reset();

  event_number = evt.id().event();
  run_number = evt.run();

  art::EventNumber_t eventNumber = evt.event();
//  TLOG(TLVL_INFO)<<" Processing event "<<eventNumber;

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
          if (print_frags) analyze_fragment(*contf[ii].get());
          if (save_triggers) extract_triggers(*contf[ii].get());
        }
      }
    }
    else {
      //normal fragment
      if (handle->front().type() != sbndaq::detail::FragmentType::PTB) continue;
      for (auto frag : *handle) {
        if(print_frags) analyze_fragment(frag);
        if (save_triggers) extract_triggers(frag);
      }
    }
  }

  // Fill the ttree
  outtree->Fill(); 

} //analyze

DEFINE_ART_MODULE(sbndaq::PTBDump)
//this is where the name is specified
