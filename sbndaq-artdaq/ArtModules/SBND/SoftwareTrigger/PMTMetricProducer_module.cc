////////////////////////////////////////////////////////////////////////
// Class:       pmtSoftwareTriggerProducer
// Plugin Type: producer (Unknown Unknown)
// File:        pmtSoftwareTriggerProducer_module.cc
//
// Generated at Thu Feb 17 13:22:51 2022 by Patrick Green using cetskelgen
// from  version .

// Authors: Erin Yandel, Patrick Green, Lynn Tung
// Contact: Lynn Tung, lynnt@uchicago.edu 

// Module to produce PMT Flash Metrics
// Input: CAEN 1730 fragments 

// Output: sbnd::trigger::pmtSoftwareTrigger data product

// More information can be found at:
// https://sbnsoftware.github.io/sbndcode_wiki/SBND_Trigger
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
#include "artdaq/DAQdata/Globals.hh"

#include "sbndaq-artdaq-core/Overlays/Common/CAENV1730Fragment.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "artdaq-core/Data/Fragment.hh"
#include "artdaq-core/Data/ContainerFragment.hh"
#include "artdaq-core/Data/RawEvent.hh"

#include "sbndaq-artdaq-core/Obj/SBND/pmtSoftwareTrigger.hh"

// ROOT includes
#include "TFile.h"

#include <memory>
#include <algorithm>
#include <valarray>
#include <numeric>


using artdaq::MetricMode;

namespace sbnd {
  namespace trigger {
    class pmtSoftwareTriggerProducer;
  }
}

class sbnd::trigger::pmtSoftwareTriggerProducer : public art::EDProducer {
public:
  explicit pmtSoftwareTriggerProducer(fhicl::ParameterSet const& p);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  pmtSoftwareTriggerProducer(pmtSoftwareTriggerProducer const&) = delete;
  pmtSoftwareTriggerProducer(pmtSoftwareTriggerProducer&&) = delete;
  pmtSoftwareTriggerProducer& operator=(pmtSoftwareTriggerProducer const&) = delete;
  pmtSoftwareTriggerProducer& operator=(pmtSoftwareTriggerProducer&&) = delete;

  // Required functions.
  void produce(art::Event& e) override;

private:

  // Declare member data here.

  // fhicl parameters
  art::Persistable is_persistable_;
  // bool     fUseBeamTrigger;  // whether to use flash trigger or beam trigger 
  // float   fWindowOffset;    // offset of trigger time in us
  // uint32_t fTrigWindowFrag;  //max time difference for frag timestamp to be part of beam window
                             //  in ticks (1 tick=2 ns), default 100 ticks - WAG for now
  float    fWindowLength;    // in us, window length after trigger time, default 1.6 us
  float    fWvfmPostPercent; // post percent of wvfm, 9 us after wvfm = 0.9
  uint16_t fWvfmNominalLength;      // in ticks, length of wvfm, default 5000 ticks

  // uint32_t    fETrigFragid;
  // uint32_t    fETrigThresh;
  
  uint8_t     fTimingType;  // 0 for rawheader, 1 for SPEC TDC, 2 for PTB, and 3 for the **TIMING** CAEN board
  uint32_t    fNTBDelay;    // in ns, delay between TDC and PPS
  
  std::vector<uint16_t> fFragIDs;
  // std::vector<uint8_t> fUncoated;
  // std::vector<uint8_t> fTPC;

  uint16_t fVerbose;

  bool fCalculateBaseline;  
  bool fCountPMTs;          
  bool fCalculatePEMetrics; 
  bool fFindFlashInfo;

  std::vector<float> fInputBaseline; 
  // float fBaselineWindow;
  float fPromptWindow;
  float fPrelimWindow; 

  float fADCThreshold;  
  float fADCtoPE;       
  // float fPEArea;        
  // end fhicl parameters

  // event-by-event global variables
  // bool foundBeamTrigger;

  // other global variables
  float ticks_to_us;
  float us_to_ticks;

  // uint32_t fTriggerBin; 

  // pmt information
  std::map<int,int> map_fragid_index;

  void     getWaveforms(const artdaq::Fragment &frag, std::vector<std::vector<uint16_t>> &wvfm_v);
  uint32_t getStartTime(const artdaq::Fragment &frag);
  uint32_t getLength   (const artdaq::Fragment &frag);
  // void   find_beam_spill(const artdaq::Fragment &frag);
  float    estimateBaseline(std::vector<uint32_t> &wvfm);
  float    estimateBaseline(std::vector<uint16_t> &wvfm);
  void     reconfigure(fhicl::ParameterSet const & p);
  std::vector<uint32_t> sumWvfms(const std::vector<uint32_t>& v1, const std::vector<uint16_t>& v2);
};


sbnd::trigger::pmtSoftwareTriggerProducer::pmtSoftwareTriggerProducer(fhicl::ParameterSet const& p)
  : EDProducer{p}
  {
    this->reconfigure(p);
    // Call appropriate produces<>() functions here.
    produces< sbnd::trigger::pmtSoftwareTrigger >("", is_persistable_);
    // map from fragID to array index 0-7
    for (size_t i=0;i<fFragIDs.size();++i){
      map_fragid_index.insert(std::make_pair(fFragIDs[i],i));
    }

    us_to_ticks = 500.; // ticks per us 
    ticks_to_us = 1/us_to_ticks; // us per ticks
 
  }

void sbnd::trigger::pmtSoftwareTriggerProducer::reconfigure(fhicl::ParameterSet const & p)
{
  // Initialize member data here
  is_persistable_     = p.get<bool>("is_persistable", true) ? art::Persistable::Yes : art::Persistable::No;
  // fUseBeamTrigger     = p.get<bool>("UseBeamTrigger",false);

  // fWindowOffset       = p.get<float>("WindowOffset",0.1);  //units of us

  // fTrigWindowFrag     = p.get<uint32_t>("TrigWindowFrag",100);  //units of ticks, 1 tick=2ns
  fWindowLength       = p.get<float>("WindowLength", 1.8);
  fWvfmPostPercent    = p.get<float>("WvfmPostPercent", 0.9);
  fWvfmNominalLength         = p.get<uint16_t>("WvfmNominalLength", 5000); // units of ticks

  fTimingType         = p.get<uint8_t>("TimingType", 0);
  fNTBDelay           = p.get<uint32_t>("NTBDelay", 367392); // units of ns

  // fETrigFragid        = p.get<uint32_t>("ETrigFragID",0);
  // fETrigThresh        = p.get<uint32_t>("ETrigThresh",5000);

  fFragIDs            = p.get<std::vector<uint16_t>>("FragIDs");

  // fUncoated           = p.get<std::vector<uint8_t>>("Uncoated");  // =0 for Coated, =1 for Uncoated
  // fTPC                = p.get<std::vector<uint8_t>>("TPC");  // TPC=0(1) is neg(pos) x and beam right(left)

  fVerbose            = p.get<uint8_t>("Verbose", 0);

  fCalculateBaseline  = p.get<bool>("CalculateBaseline",true);
  fCountPMTs          = p.get<bool>("CountPMTs",true);
  fCalculatePEMetrics = p.get<bool>("CalculatePEMetrics",false);
  fFindFlashInfo      = p.get<bool>("FindFlashInfo",false);

  fInputBaseline      = p.get<std::vector<float>>("InputBaseline");
  // fBaselineWindow     = p.get<float>("BaselineWindow", 0.5);
  fPromptWindow       = p.get<float>("PromptWindow", 0.1);
  fPrelimWindow       = p.get<float>("PrelimWindow", 0.5);
  fADCThreshold       = p.get<float>("ADCThreshold", 7960);
  fADCtoPE            = p.get<float>("ADCtoPE", 8.0);
}


void sbnd::trigger::pmtSoftwareTriggerProducer::produce(art::Event& e)
{
  if (fVerbose==1) std::cout << "Processing Run: " << e.run() << ", Subrun: " <<  e.subRun() << ", Event: " << e.id().event() << std::endl;

  // the reference time stamp, usually the event trigger time
  uint32_t refTimestamp=0; 

  art::Handle<artdaq::detail::RawEventHeader> header_handle;
  e.getByLabel("daq", "RawEventHeader", header_handle);
  if (fTimingType==0){
    if ((header_handle.isValid())){
      auto rawheader = artdaq::RawEvent(*header_handle);
      refTimestamp = rawheader.timestamp()%int(1e9) - fNTBDelay;
      std::cout << "timestamp from rawheader is: " << refTimestamp << std::endl;
    }
    else{
      std::cout << "RawEventHeader not valid. Using SPEC TDC..." << std::endl;
      fTimingType++;
    }
  }

  // reset for this event
  // foundBeamTrigger = false;
  // fTriggerBin = 0; 


  std::vector<std::vector<uint16_t>> wvfms_v;
  std::vector<float>                 baselines_v;
  // std::vector<sbnd::trigger::pmtInfo> fpmtInfoVec;

  auto nboards = fFragIDs.size();
  wvfms_v.resize(15*nboards); // 15 PMT channels per 1730 fragment, 8 fragments per trigger
  baselines_v.resize(15*nboards); 
  // fpmtInfoVec.resize(15*8); // 15 PMTs per fragment, 8 fragments per trigger

  std::vector<art::Handle<artdaq::Fragments>> fragmentHandles = e.getMany<std::vector<artdaq::Fragment>>();

  // variables to find the beam fragment index 
  // the index inside a container to find the "beam" fragment (closest to the event trigger time)
  size_t beam_frag_idx = 0;
  // to store the time difference between the beam fragment start time and event trigger time 
  uint32_t beam_frag_dt = 1e9; 

  // loop over fragment handles
  for (auto &handle : fragmentHandles) {
    if (!handle.isValid() || handle->size() == 0) continue;
    //Container fragment
    if (handle->front().type() == artdaq::Fragment::ContainerFragmentType) {
      for (auto cont : *handle) {
	      artdaq::ContainerFragment contf(cont);
        if (contf.fragment_type()==sbndaq::detail::FragmentType::CAENV1730) {
          // check the fragment ID for the first fragment in the container
          if (std::find(fFragIDs.begin(), fFragIDs.end(), contf[0].get()->fragmentID()) == fFragIDs.end()) continue;

          // if the time difference is equal to its initial value (first valid container)
          if (beam_frag_dt==1e9){
            for (size_t ii = 0; ii < contf.block_count(); ++ii){
              // find the absolute time difference between the fragment start time and the reference time stamp 
              int dt = int(refTimestamp) - int(getStartTime(*contf[ii].get()));
              if (abs(dt) < beam_frag_dt){
                  beam_frag_dt = dt;
                  beam_frag_idx = ii;
              }
            }
          }
          // get waveforms for the beam fragment
          getWaveforms(*contf[beam_frag_idx].get(), wvfms_v);
        }
      } // loop over containers 
    } // if 1730 container
    //Regular CAEN fragment
    // else if (handle->front().type()==sbndaq::detail::FragmentType::CAENV1730) {
    //   if (fVerbose==2) std::cout << "Found " << handle->size() << " CAEN1730 fragments" << std::endl;
    //   for (auto frag : *handle){
    //     getWaveforms(frag);
    //     // if (fUseBeamTrigger) find_beam_spill(frag);
    //   }
    // } // if/else container/fragmnet
    ///
  } // loop over handles
  
  if (fVerbose==1) std::cout << "reference time stamp is " << refTimestamp << std::endl;
  // object to store trigger metrics in
  std::unique_ptr<sbnd::trigger::pmtSoftwareTrigger> pmtSoftwareTriggerMetrics = std::make_unique<sbnd::trigger::pmtSoftwareTrigger>();

  // if we're looking for the beam, make sure we found the beam trigger 
  // if (fUseBeamTrigger && !foundBeamTrigger) refTimestamp=0;

    // calculate metrics
  if (refTimestamp!=0) {    
    
    // store whether (1) we're using the beam trigger and (2) if we found it.
    // if both conditions (1) and (2) are fulfilled, then it will be true. Otherwise, false.
    // if (fUseBeamTrigger && foundBeamTrigger) pmtSoftwareTriggerMetrics->foundBeamTrigger = true;
    pmtSoftwareTriggerMetrics->foundBeamTrigger = false;

    // store timestamp of trigger (where the 0 is the reference time stamp, in ns)
    pmtSoftwareTriggerMetrics->trig_ts = (float)beam_frag_dt;
    if (fVerbose==1) std::cout << "Saving flash timestamp: " << beam_frag_dt << " ns" << std::endl;

    float promptPE = 0;
    float prelimPE = 0;

    // to store flash metrics 
    float flash_promptPE = 0;
    float flash_prelimPE = 0;
    float flash_peakPE = 0;
    float flash_peaktime = 0;

    int nAboveThreshold = 0;

    // create a vector to contain the sum of all the wvfms
    std::vector<uint32_t> wvfm_sum(fWvfmNominalLength, 0);
    // wvfm loop to calculate metrics
    for (size_t i_ch = 0; i_ch < wvfms_v.size(); ++i_ch){
      // auto &pmtInfo = fpmtInfoVec.at(i_ch);
      auto wvfm = wvfms_v[i_ch];

      if (wvfm.begin() == wvfm.end()) continue;

      int windowStartBin = (1.0 - fWvfmPostPercent)*wvfm.size(); 
      int windowEndBin = windowStartBin + int(fWindowLength*us_to_ticks);

      // assign channel
      // pmtInfo.channel = i_ch;

      // calculate baseline
      if (fCalculateBaseline) baselines_v.at(i_ch) = estimateBaseline(wvfm);
      else baselines_v.at(i_ch) = fInputBaseline.at(0);
      // else { pmtInfo.baseline=fInputBaseline.at(0); pmtInfo.baselineSigma = fInputBaseline.at(1); }
      
      // count number of PMTs above threshold
      if (fCountPMTs){
        if (windowEndBin >= (int)wvfm.size()){ windowEndBin = (int)wvfm.size() - 1;}
        for (int bin = windowStartBin; bin < windowEndBin; ++bin){
	         auto adc = wvfm[bin];
	          if (adc!=0 && adc < fADCThreshold){ nAboveThreshold++; break; }
        }
      }
      else { nAboveThreshold = -9999; }

      // quick estimate prompt and preliminary light, assuming sampling rate of 500 MHz (2 ns per bin)
      if (fCalculatePEMetrics){
        float baseline = baselines_v.at(i_ch);
        
        auto prelimStart = (windowStartBin - fPrelimWindow*us_to_ticks) < 0 ? 0 : windowStartBin - fPrelimWindow*us_to_ticks;
        auto promptEnd   = windowStartBin + fPromptWindow*us_to_ticks;

        // this sums the peaks, not the integral!
        float ch_prelimPE = (baseline-(*std::min_element(wvfm.begin()+prelimStart, wvfm.begin()+windowStartBin)))/fADCtoPE;
        float ch_promptPE = (baseline-(*std::min_element(wvfm.begin()+windowStartBin, wvfm.begin()+promptEnd)))/fADCtoPE;

        promptPE += ch_promptPE;
        prelimPE += ch_prelimPE;
      }
      else  { promptPE = -9999; prelimPE = -9999;}
      if (fFindFlashInfo) wvfm_sum = sumWvfms(wvfm_sum, wvfm);
    }
    if (fFindFlashInfo){
      // find and remove the baseline
      int windowStartBin = (1.0 - fWvfmPostPercent)*wvfm_sum.size(); 
      int windowEndBin = windowStartBin + int(fWindowLength*us_to_ticks);
      auto prelimStart = (windowStartBin - fPrelimWindow*us_to_ticks) < 0 ? 0 : windowStartBin - fPrelimWindow*us_to_ticks;
      auto promptEnd   = windowStartBin + fPromptWindow*us_to_ticks;

      auto flash_baseline = estimateBaseline(wvfm_sum);

      flash_prelimPE = (flash_baseline-(*std::min_element(wvfm_sum.begin()+prelimStart,wvfm_sum.begin()+windowStartBin)))/fADCtoPE;
      flash_promptPE = (flash_baseline-(*std::min_element(wvfm_sum.begin()+windowStartBin,wvfm_sum.begin()+promptEnd)))/fADCtoPE;

      flash_peakPE   = (flash_baseline-(*std::min_element(wvfm_sum.begin()+windowStartBin,wvfm_sum.begin()+windowEndBin)))/fADCtoPE;

      auto flash_peak_it = std::min_element(wvfm_sum.begin()+windowStartBin,wvfm_sum.begin()+windowEndBin);
      // get the peak time in reference to the reference time!!!  
      flash_peaktime = ((std::distance(wvfm_sum.begin(), flash_peak_it))*ticks_to_us*1e3 + beam_frag_dt)*1e-3; // us
    }
    pmtSoftwareTriggerMetrics->nAboveThreshold = nAboveThreshold;
    pmtSoftwareTriggerMetrics->promptPE = promptPE;
    pmtSoftwareTriggerMetrics->prelimPE = prelimPE;
    pmtSoftwareTriggerMetrics->peakPE = flash_peakPE;
    pmtSoftwareTriggerMetrics->peaktime = flash_peaktime;

    if (fVerbose==1){
      if (fCountPMTs)
        std::cout << "nPMTs Above Threshold: " << nAboveThreshold << std::endl;
      if (fCalculatePEMetrics){
        std::cout << "prelim pe: " << prelimPE << std::endl;
        std::cout << "prompt pe: " << promptPE << std::endl;
      }
      if (fFindFlashInfo){
        std::cout << "flash prelim pe: " << flash_prelimPE << std::endl;
        std::cout << "flash prompt pe: " << flash_promptPE << std::endl;
        std::cout << "flash peak pe: "   << flash_peakPE << std::endl;
        std::cout << "flash peak time: " << flash_peaktime << " us" << std::endl;
      }
    }
    // clear variables to free memory
    wvfms_v.clear(); 
    // fpmtInfoVec.clear();

    if(metricMan != nullptr) {
        //send flag metrics
        metricMan->sendMetric(
            "PMT_SWTrig_nAboveThreshold",
            nAboveThreshold,
            "Num PMTs Above Software Trigger Threshold", 5, artdaq::MetricMode::LastPoint);

        if (fVerbose==1) {
          TLOG(TLVL_INFO) << "nPMTs Above Threshold: " << nAboveThreshold;
        }

    }
  }  // if found beam trigger
  else{
    if (fVerbose==1) {
      std::cout << "Beam and wvfms not found" << std::endl;
      TLOG(TLVL_INFO) << "PMT Software Trigger Metric: Beam and wvfms not found";

    }
    pmtSoftwareTriggerMetrics->foundBeamTrigger = false;
    pmtSoftwareTriggerMetrics->trig_ts = -9999;
    pmtSoftwareTriggerMetrics->nAboveThreshold = -9999;
    pmtSoftwareTriggerMetrics->promptPE = -9999;
    pmtSoftwareTriggerMetrics->prelimPE = -9999;
    pmtSoftwareTriggerMetrics->peakPE   = -9999;
    pmtSoftwareTriggerMetrics->peaktime = -9999;
  }
  e.put(std::move(pmtSoftwareTriggerMetrics));
}

std::vector<uint32_t> sbnd::trigger::pmtSoftwareTriggerProducer::sumWvfms(const std::vector<uint32_t>& v1, const std::vector<uint16_t>& v2) {

  size_t result_len = (v1.size() > v2.size()) ? v1.size() : v2.size();
  std::vector<uint32_t>  result(result_len,0);
  for (size_t i = 0; i < result_len; i++){
    auto value1 = (i < v1.size()) ? v1[i] : 0;
    auto value2 = (i < v2.size()) ? v2[i] : 0;
    result.at(i) = value1 + value2;
  }
  return result;
}

// void sbnd::trigger::pmtSoftwareTriggerProducer::find_beam_spill(const artdaq::Fragment &frag) {

//   // check fragId
//   uint fragId = static_cast<int>(frag.fragmentID());
//   if (fVerbose==2) std::cout << "Looking for beam spill, fragID is " << fragId <<
// 		  " and timestamp is " << frag.timestamp() << std::endl;
//   if (fragId!=fETrigFragid)  return;

//   //--get number of channels from metadata and waveform length from header
//   sbndaq::CAENV1730Fragment bb(frag);
//   auto const* md = bb.Metadata();
//   sbndaq::CAENV1730Event const* event_ptr = bb.Event();
//   sbndaq::CAENV1730EventHeader header = event_ptr->Header;
//   size_t nChannels = md->nChannels;

//   uint32_t ev_size_quad_bytes = header.eventSize;
//   if (fVerbose==2) std::cout << "Event size in quad bytes is: " << ev_size_quad_bytes << "\n";
//   uint32_t evt_header_size_quad_bytes = sizeof(sbndaq::CAENV1730EventHeader)/sizeof(uint32_t);
//   uint32_t data_size_double_bytes = 2*(ev_size_quad_bytes - evt_header_size_quad_bytes);
//   //-  uint32_t wfm_length = data_size_double_bytes/nChannels;
//   fWvfmLength = data_size_double_bytes/nChannels;
//   if (fVerbose==2) std::cout << "Channel waveform length = " << fWvfmLength << " [ticks], or " << fWvfmLength*ticks_to_us << "us\n";
//   size_t ch_offset = (size_t)(15*fWvfmLength);
//   if (fVerbose==2) std::cout << " frag timestamp is " << frag.timestamp()  << std::endl;

//   // access beam signal, in ch15 of first PMT of each fragment set
//   const uint16_t* data_begin = reinterpret_cast<const uint16_t*>(frag.dataBeginBytes()
// 					   + sizeof(sbndaq::CAENV1730EventHeader));
//   const uint16_t* value_ptr =  data_begin;
//   uint16_t value = 0;

//   // not currently used because there's no guarantee where the trigger is in the waveform
//   //  size_t tr_offset = refTimestampOffset*1e3;
//   size_t tr_offset = 0;
//   value_ptr = data_begin + ch_offset + tr_offset; // pointer arithmetic

//   for (uint ind=0;ind<fWvfmLength;++ind){
//     value = *(value_ptr+ind);
//     if (value>fETrigThresh) {
//       foundBeamTrigger = true;
//       fTriggerBin = ind;
//       refTimestamp = frag.timestamp();
//       std::cout << "Found beam trigger : ind= " << ind << " adc is " << value;
//       std::cout << "timestamp is " << refTimestamp << std::endl;
//       break;
//     }
//   }

//   std::cout << "leaving find_beam_spill, timestamp is " << refTimestamp << std::endl;
// }

void sbnd::trigger::pmtSoftwareTriggerProducer::getWaveforms(const artdaq::Fragment &frag, std::vector<std::vector<uint16_t>> & wvfm_v)
{
  //--get number of channels from metadata and waveform length from header
  sbndaq::CAENV1730Fragment bb(frag);
  auto const* md = bb.Metadata();
  sbndaq::CAENV1730Event const* event_ptr = bb.Event();
  sbndaq::CAENV1730EventHeader header = event_ptr->Header;
  
  uint32_t ttt = header.triggerTimeTag*8; // downsampled trigger time tag; TTT points to end of wvfm w.r.t. pps

  int fragId = static_cast<int>(frag.fragmentID());
  int findex = map_fragid_index.find(fragId)->second;

  size_t nChannels = md->nChannels;
  uint32_t ev_size_quad_bytes = header.eventSize;
  uint32_t evt_header_size_quad_bytes = sizeof(sbndaq::CAENV1730EventHeader)/sizeof(uint32_t);
  uint32_t data_size_double_bytes = 2*(ev_size_quad_bytes - evt_header_size_quad_bytes);
  uint32_t wvfm_length = data_size_double_bytes/nChannels;

  if (fVerbose==2) std::cout << "From header, downsampled TTT is " << ttt << "\n";
  if (fVerbose==2) std::cout << "\tNumber of channels: " << nChannels << "\n";
  if (fVerbose==2) std::cout << "\tChannel waveform length = " << wvfm_length << "\n";

  auto wvfm_start = ttt - wvfm_length*ticks_to_us*1e3; // ns, start of waveform w.r.t. pps
  // auto trigger_position = ttt - fWvfmPostPercent*wvfm_length; // ticks, position of trigger in waveform (bin)
  // auto trigger_time = trigger_position*ticks_to_us;
  if (fVerbose==2) std::cout << "\tFlash start time is " << int(wvfm_start) << " us" << std::endl;

  // refTimestamp = wvfm_start;

  //--access waveforms in fragment and save
  const uint16_t* data_begin = reinterpret_cast<const uint16_t*>(frag.dataBeginBytes()
					+ sizeof(sbndaq::CAENV1730EventHeader));
  const uint16_t* value_ptr =  data_begin;
  uint16_t value = 0;
  size_t ch_offset = 0;

  // loop over channels
  if (nChannels==16) nChannels--; // last waveform isn't a PMT
  for (size_t i_ch = 0; i_ch < nChannels; ++i_ch){
    wvfm_v[i_ch + nChannels*findex].resize(wvfm_length);
    ch_offset = (size_t)(i_ch * wvfm_length);
    //--loop over waveform samples
    for(size_t i_t = 0; i_t < wvfm_length; ++i_t){
      value_ptr = data_begin + ch_offset + i_t; // pointer arithmetic
      value = *(value_ptr);
      wvfm_v[i_ch + nChannels*findex][i_t] = value;
    } //--end loop samples
  } //--end loop channels
}

uint32_t sbnd::trigger::pmtSoftwareTriggerProducer::getStartTime(const artdaq::Fragment &frag){
  //--get number of channels from metadata and waveform length from header
  sbndaq::CAENV1730Fragment bb(frag);
  auto const* md = bb.Metadata();
  sbndaq::CAENV1730Event const* event_ptr = bb.Event();
  sbndaq::CAENV1730EventHeader header = event_ptr->Header;
  
  uint32_t ttt = header.triggerTimeTag*8; // downsampled trigger time tag; TTT points to end of wvfm w.r.t. pps

  size_t nChannels = md->nChannels;
  uint32_t ev_size_quad_bytes = header.eventSize;
  uint32_t evt_header_size_quad_bytes = sizeof(sbndaq::CAENV1730EventHeader)/sizeof(uint32_t);
  uint32_t data_size_double_bytes = 2*(ev_size_quad_bytes - evt_header_size_quad_bytes);
  auto wvfm_length = data_size_double_bytes/nChannels;

  uint32_t wvfm_start = ttt - wvfm_length*ticks_to_us*1e3; // ns, start of waveform w.r.t. pps
  return wvfm_start;
}

uint32_t sbnd::trigger::pmtSoftwareTriggerProducer::getLength(const artdaq::Fragment &frag){
  sbndaq::CAENV1730Fragment bb(frag);
  auto const* md = bb.Metadata();
  size_t nChannels = md->nChannels;

  sbndaq::CAENV1730Event const* event_ptr = bb.Event();
  sbndaq::CAENV1730EventHeader header = event_ptr->Header;
  uint32_t ev_size_quad_bytes = header.eventSize;
  uint32_t evt_header_size_quad_bytes = sizeof(sbndaq::CAENV1730EventHeader)/sizeof(uint32_t);
  uint32_t data_size_double_bytes = 2*(ev_size_quad_bytes - evt_header_size_quad_bytes);
  uint32_t wvfm_length = data_size_double_bytes/nChannels;

  return wvfm_length;
}


float sbnd::trigger::pmtSoftwareTriggerProducer::estimateBaseline(std::vector<uint32_t>& wvfm){
    const auto median_it = wvfm.begin() + wvfm.size() / 2;
    std::nth_element(wvfm.begin(), median_it , wvfm.end());
    auto median = *median_it;
    return median;
}

float sbnd::trigger::pmtSoftwareTriggerProducer::estimateBaseline(std::vector<uint16_t>& wvfm){
    const auto median_it = wvfm.begin() + wvfm.size() / 2;
    std::nth_element(wvfm.begin(), median_it , wvfm.end());
    auto median = *median_it;
    return median;
}

DEFINE_ART_MODULE(sbnd::trigger::pmtSoftwareTriggerProducer)
