#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "fhiclcpp/ParameterSet.h"

#include "sbndaq-artdaq-core/Overlays/Common/CAENV1730Fragment.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "artdaq-core/Data/Fragment.hh"
#include "artdaq-core/Data/ContainerFragment.hh"

#include <iostream>
#include <vector>

namespace sbndaq {
  class CoincidenceFilter;
}

class sbndaq::CoincidenceFilter : public art::EDFilter {
public:
  explicit CoincidenceFilter(fhicl::ParameterSet const & p);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  CoincidenceFilter(CoincidenceFilter const &) = delete;
  CoincidenceFilter(CoincidenceFilter &&) = delete;
  CoincidenceFilter & operator = (CoincidenceFilter const &) = delete;
  CoincidenceFilter & operator = (CoincidenceFilter &&) = delete;

  // Required functions.
  bool filter(art::Event & e) override;

  // Selected optional functions.
  void beginJob() override;
  void endJob() override;

private:
 
  // fhicl paramters
  int fNCoincidence;
  int fCoincidenceWindow;
  int fThreshold;
  bool fExclusive;
  int fShift;
  bool fVerbose;


  // event information
  int fRun;
  art::EventNumber_t fEvent;

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

  bool checkCoincidence();
		    
};

// ------------------------------------------------- 

sbndaq::CoincidenceFilter::CoincidenceFilter(fhicl::ParameterSet const & pset) : art::EDFilter(pset),
fNCoincidence(pset.get<int>("nCoincidence")),
fCoincidenceWindow(pset.get<int>("coincidenceWindow")),
fThreshold(pset.get<int>("threshold")),
fExclusive(pset.get<bool>("exclusive")),
fShift(pset.get<int>("shift_fragment_id")),
fVerbose(pset.get<bool>("verbose"))
// Initialize member data here.
{
  // Call appropriate produces<>() functions here.
}

// ------------------------------------------------- 

bool sbndaq::CoincidenceFilter::filter(art::Event & evt)
{

  int fRun = evt.run();
  art::EventNumber_t fEvent = evt.event();

  std::cout << "Run: " << fRun << ", Event: " << fEvent << std::endl;

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

  // test coincidence condition  
  bool filter = checkCoincidence();

  return filter;

} // filter

// ------------------------------------------------- 

void sbndaq::CoincidenceFilter::beginJob()
{
  // Implementation of optional member function here.
} // beginJob

// ------------------------------------------------- 

void sbndaq::CoincidenceFilter::endJob()
{
  // Implementation of optional member function here.
} // endJob

// ------------------------------------------------- 

bool sbndaq::CoincidenceFilter::checkCoincidence() {

  // counter number above threshold  
  int counter = 0;
  if (fPMT_chA.size() > 0) counter++;
  if (fPMT_chB.size() > 0) counter++;
  if (fPMT_chC.size() > 0) counter++;
  if (fPMT_chD.size() > 0) counter++;
  if (fPMT_chE.size() > 0) counter++;

  if (fVerbose) std::cout << "Number coincidence: " << counter << std::endl;

  // make vector containing initial times above threshold
  std::vector<int> times; times.reserve(5);
  if (fVerbose) std::cout << "chA:" << std::endl;
  for (size_t i = 0; i < fPMT_chA.size(); i++) {
    if (fVerbose) std::cout << fPMT_chA[i] << std::endl;
    times.push_back(fPMT_chA[i]);
  }
  if (fVerbose) std::cout << "chB:" << std::endl;
  for (size_t i = 0; i < fPMT_chB.size(); i++) {
    if (fVerbose) std::cout << fPMT_chB[i] << std::endl;
    times.push_back(fPMT_chB[i]);
  }
  if (fVerbose) std::cout << "chC:" << std::endl;
  for (size_t i = 0; i < fPMT_chC.size(); i++) {
    if (fVerbose) std::cout << fPMT_chC[i] << std::endl;
    times.push_back(fPMT_chC[i]);
  }
  if (fVerbose) std::cout << "chD:" << std::endl;
  for (size_t i = 0; i < fPMT_chD.size(); i++) {
    if (fVerbose) std::cout << fPMT_chD[i] << std::endl;
    times.push_back(fPMT_chD[i]);
  }
  if (fVerbose) std::cout << "chE:" << std::endl;
  for (size_t i = 0; i < fPMT_chE.size(); i++) {
    if (fVerbose) std::cout << fPMT_chE[i] << std::endl;
    times.push_back(fPMT_chE[i]);
  }

  if (counter >= fNCoincidence) {
    if (fExclusive && counter != fNCoincidence) return false;
    if (fVerbose) std::cout << "Pass" << std::endl;
    return true;
  }
  else return false;

  // time coincidence
  /*
  // reject events with too few
  if (fVerbose) std::cout << counter << " paddles above threshold" << std::endl;
  if (counter < fNCoincidence) {
    if (fVerbose) std::cout << "Fail" << std::endl;
    return false;
  }
  
  // special case, 1 required
  if (fNCoincidence == 1 && counter >= 1) {
    if (fVerbose) std::cout << "Pass" << std::endl;
    return true;
  }

  

  // special case, 2 required
  if (fNCoincidence == 2 && counter == 2 && times.size() == 2) {
    // check in time coincidence
    if (std::abs(times[0] - times[1]) < fCoincidenceWindow) {
      if (fVerbose) std::cout << "Pass" << std::endl;  
      return true;
    }     
  }

  // general case (not yet implemented...)
  
  */

} // checkCoincidence

// -------------------------------------------------

void sbndaq::CoincidenceFilter::analyze_caen_fragment(artdaq::Fragment & frag)  {
  

  std::cout <<  "     timestamp is  " << frag.timestamp() << std::endl;
  std::cout <<  "     seq ID is " << frag.sequenceID() << std::endl;
  
  
  sbndaq::CAENV1730Fragment bb(frag);
  auto const* md = bb.Metadata();
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

  nChannels = md->nChannels;
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
void sbndaq::CoincidenceFilter::findLeadingEdges(int i_ch, int fragId, uint32_t wfm_length, std::vector<int> &fPMT) {
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

DEFINE_ART_MODULE(sbndaq::CoincidenceFilter)