////////////////////////////////////////////////////////////////////////
// Class:       testStandTriggerProducer
// Plugin Type: producer (Unknown Unknown)
// File:        testStandTriggerProducer_module.cc
//
// Generated at Fri Jan 28 12:03:29 2022 by Patrick Green using cetskelgen
// from  version .
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "sbndaq-artdaq-core/Overlays/Common/CAENV1730Fragment.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "artdaq-core/Data/Fragment.hh"
#include "artdaq-core/Data/ContainerFragment.hh"

#include "sbndaq-artdaq-core/Obj/SBND/testStandTrigger.h"

#include <memory>
#include <iostream>

namespace sbndaq {
  class testStandTriggerProducer;
}

class sbndaq::testStandTriggerProducer : public art::EDProducer {
public:
  explicit testStandTriggerProducer(fhicl::ParameterSet const& p);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  testStandTriggerProducer(testStandTriggerProducer const&) = delete;
  testStandTriggerProducer(testStandTriggerProducer&&) = delete;
  testStandTriggerProducer& operator=(testStandTriggerProducer const&) = delete;
  testStandTriggerProducer& operator=(testStandTriggerProducer&&) = delete;

  // Required functions.
  void produce(art::Event& evt) override;

private:

  // fhicl parameters
  art::Persistable is_persistable_;
  int fThreshold;
  int fShift;
  bool fVerbose;

  // event information
  int fRun;
  art::EventNumber_t fEvent;

  // paddles above threshold
  int fNAboveThreshold;

  //--default values
  uint32_t nChannels    = 16;
  uint32_t Ttt_DownSamp =  4; 
  /* the waveforms are sampled at 500MHz sampling. The trigger timestamp is sampled 4 times slower than input channels */

  // paddle data
  std::vector<uint64_t>  fTicksVec;
  std::vector< std::vector<uint16_t> >  fWvfmsVec;
  std::vector<int> fPMT_chA;
  std::vector<int> fPMT_chB;
  std::vector<int> fPMT_chC;
  std::vector<int> fPMT_chD;
  std::vector<int> fPMT_chE;
  std::vector<int> fRWM;

  int TTT;  // will be set to value in CAEN fragement header
  int TTT_ns; 

  void analyze_caen_fragment(artdaq::Fragment & frag);

  void findLeadingEdges(int i_ch, int fragId, uint32_t wfm_length, std::vector<int> &fPMT);

  int checkCoincidence();

};


sbndaq::testStandTriggerProducer::testStandTriggerProducer(fhicl::ParameterSet const& p)
  : EDProducer{p},
  is_persistable_(p.get<bool>("is_persistable") ? art::Persistable::Yes : art::Persistable::No),
  fThreshold(p.get<int>("threshold")),
  fShift(p.get<int>("shift_fragment_id")),
  fVerbose(p.get<bool>("verbose"))
  {
  // Call appropriate produces<>() functions here.
  produces< sbndaq::testStandTrigger >("", is_persistable_);
  
  // Call appropriate consumes<>() for any products to be retrieved by this module.
}

void sbndaq::testStandTriggerProducer::produce(art::Event& evt)
{
  
  // load event information
  int fRun = evt.run();
  art::EventNumber_t fEvent = evt.event();

  if (fVerbose) std::cout << "Processing: Run " << fRun << ", Event " << fEvent << std::endl;

  // object to store required trigger information in
  std::unique_ptr<sbndaq::testStandTrigger> testStandTriggerInformation = std::make_unique<sbndaq::testStandTrigger>();

  // clear variables at the beginning of the event
  fPMT_chA.clear();   fPMT_chB.clear();   fPMT_chC.clear();
  fPMT_chD.clear();   fPMT_chE.clear();   fRWM.clear();
  fWvfmsVec.clear();  fTicksVec.clear();

  //  note that this code expects exactly 1 CAEN fragment per event
  TTT = 0;
  TTT_ns = 0;  // will be set to value in CAEN fragement header

  // get fragment handles
  std::vector<art::Handle<artdaq::Fragments>> fragmentHandles = evt.getMany<std::vector<artdaq::Fragment>>();

  // loop over fragment handles (caen)
  for (auto handle : fragmentHandles) {
    if (!handle.isValid() || handle->size() == 0) continue;
        
    if (handle->front().type() == artdaq::Fragment::ContainerFragmentType) {
      // container fragment                                                                                               
      for (auto cont : *handle) {
        artdaq::ContainerFragment contf(cont);
        if (contf.fragment_type()==sbndaq::detail::FragmentType::CAENV1730) {
          if (fVerbose)     std::cout << "    Found " << contf.block_count() << " CAEN Fragments in container " << std::endl;
          fWvfmsVec.resize(16*contf.block_count());
          for (size_t ii = 0; ii < contf.block_count(); ++ii) {            
            analyze_caen_fragment(*contf[ii].get());
          }
        }    
      } 
    }   
    else {      
      // normal fragment
      if (handle->front().type()==sbndaq::detail::FragmentType::CAENV1730) {
        if (fVerbose)   std::cout << "   found normal caen fragments " << handle->size() << std::endl;
        fWvfmsVec.resize(16*handle->size());
        for (auto frag : *handle) {
          analyze_caen_fragment(frag);
        }
      }  
    }    
  } // loop over frag handles

  // determine relevant triggering information
  fNAboveThreshold = checkCoincidence();
  
  // add to trigger object
  testStandTriggerInformation->nAboveThreshold = fNAboveThreshold;
  testStandTriggerInformation->PMT_chA = fPMT_chA;
  testStandTriggerInformation->PMT_chB = fPMT_chB;
  testStandTriggerInformation->PMT_chC = fPMT_chC;
  testStandTriggerInformation->PMT_chD = fPMT_chD;
  testStandTriggerInformation->PMT_chE = fPMT_chE;

  if (fVerbose) std::cout << "Number saved: " << testStandTriggerInformation->nAboveThreshold << std::endl;

  // add to event
  evt.put(std::move(testStandTriggerInformation));

}


// ------------------------------------------------- 

int sbndaq::testStandTriggerProducer::checkCoincidence() {

  // counter number above threshold  
  int counter = 0;
  if (fPMT_chA.size() > 0) counter++;
  if (fPMT_chB.size() > 0) counter++;
  if (fPMT_chC.size() > 0) counter++;
  if (fPMT_chD.size() > 0) counter++;
  if (fPMT_chE.size() > 0) counter++;

  if (fVerbose) std::cout << "Number coincidence: " << counter << std::endl;

  return counter;
}

// -------------------------------------------------

void sbndaq::testStandTriggerProducer::analyze_caen_fragment(artdaq::Fragment & frag)  {
  

  if (fVerbose) std::cout <<  "     timestamp is  " << frag.timestamp() << std::endl;
  if (fVerbose) std::cout <<  "     seq ID is " << frag.sequenceID() << std::endl;  
  
  sbndaq::CAENV1730Fragment bb(frag);
  auto const md = bb.Metadata();
  sbndaq::CAENV1730Event const* event_ptr = bb.Event();
  sbndaq::CAENV1730EventHeader header = event_ptr->Header;
  
  int fragId = static_cast<int>(frag.fragmentID()); 
  fragId-=fShift; 
  //
  if (fVerbose)      std::cout << "\tFrom CAEN header, event counter is "  << header.eventCounter   << "\n"; 
  if (fVerbose)      std::cout << "\tFrom CAEN header, triggerTimeTag is " << header.triggerTimeTag << "\n";
  if (fVerbose)      std::cout << "\tFrom CAEN header, board id is "       << header.boardID       << "\n";
  if (fVerbose)      std::cout << "\tFrom CAEN fragment, fragment id is "  << fragId << "\n";
  if (fVerbose)      std::cout << "\tShift back, fragment id of "  << fShift << "\n";
  
  uint32_t t0 = header.triggerTimeTag;
  TTT = (int)t0;
  TTT_ns = t0*8;
  if (fVerbose)       std::cout << "\n\tTriggerTimeTag in ns is " << TTT_ns << "\n";  // 500 MHz is 2 ns per tick

  nChannels = md.nChannels;
  if (fVerbose)       std::cout << "\tNumber of channels: " << nChannels << "\n";
  
  //--get the number of 32-bit words (quad_bytes) from the header
  uint32_t ev_size_quad_bytes = header.eventSize;
  if (fVerbose)       std::cout << "Event size in quad bytes is: " << ev_size_quad_bytes << "\n";
  uint32_t evt_header_size_quad_bytes = sizeof(sbndaq::CAENV1730EventHeader)/sizeof(uint32_t);
  uint32_t data_size_double_bytes = 2*(ev_size_quad_bytes - evt_header_size_quad_bytes);
  uint32_t wfm_length = data_size_double_bytes/nChannels;
  //--note, needs to take into account channel mask
  if (fVerbose) std::cout << "Channel waveform length = " << wfm_length << "\n";
  
  //--store the tick value for each acquisition 
  fTicksVec.resize(wfm_length);
  
  const uint16_t* data_begin = reinterpret_cast<const uint16_t*>(frag.dataBeginBytes() 
                 + sizeof(sbndaq::CAENV1730EventHeader));
  
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
      fWvfmsVec[i_ch+nChannels*fragId][i_t] = value;
    } //--end loop samples
  } //--end loop channels
  
  // find leading edges in waveforms for each channel
  findLeadingEdges(2, fragId, wfm_length, fPMT_chA);
  findLeadingEdges(4, fragId, wfm_length, fPMT_chB);
  findLeadingEdges(8, fragId, wfm_length, fPMT_chC);
  findLeadingEdges(10, fragId, wfm_length, fPMT_chD);
  findLeadingEdges(12, fragId, wfm_length, fPMT_chE);
  findLeadingEdges(15, fragId, wfm_length, fRWM);

} // analyze_caen_fragment

// -------------------------------------------------
void sbndaq::testStandTriggerProducer::findLeadingEdges(int i_ch, int fragId, uint32_t wfm_length, std::vector<int> &fPMT) {
  int toggle=0;
  auto this_value = fWvfmsVec[i_ch+nChannels*fragId][0];
  if (this_value<fThreshold) toggle=1;
  for(size_t i_t=1; i_t<wfm_length; ++i_t){     
    this_value = fWvfmsVec[i_ch+nChannels*fragId][i_t];
    if (toggle==0 && this_value<fThreshold) {
      toggle=1;
      fPMT.emplace_back(i_t);
    }
    if (toggle==1 && this_value>fThreshold) toggle=0;
  }
} // findLeadingEdges

// -------------------------------------------------

DEFINE_ART_MODULE(sbndaq::testStandTriggerProducer)
