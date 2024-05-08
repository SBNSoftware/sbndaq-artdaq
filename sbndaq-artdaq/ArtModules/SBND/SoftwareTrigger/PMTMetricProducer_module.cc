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


#include "artdaq-core/Data/Fragment.hh"
#include "artdaq-core/Data/ContainerFragment.hh"
#include "artdaq-core/Data/RawEvent.hh"

#include "sbndaq-artdaq-core/Overlays/Common/CAENV1730Fragment.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "sbndaq-artdaq-core/Overlays/SBND/TDCTimestampFragment.hh"
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
  std::vector<std::string> fCAENInstanceLabels; // instance labels for the CAEN V1730 modules
  float    fWindowLength;    // in us, window length after trigger time, default 1.6 us
  float    fWvfmPostPercent; // post percent of wvfm, 9 us after wvfm = 0.9
  uint16_t fWvfmNominalLength;      // in ticks, length of wvfm, default 5000 ticks
  
  uint8_t     fTimingType;  // 0 for rawheader, 1 for SPEC TDC, 2 for PTB, and 3 for the **TIMING** CAEN board
  uint32_t    fNTBDelay;    // in ns, delay between TDC and PPS
  
  std::string              fSPECTDCModuleLabel;
  std::vector<std::string> fSPECTDCInstanceLabels;
  uint8_t    fSPECTDCTimingChannel; // 1 is bes, 2 is rwm, and 4 is ett 
  int32_t   fSPECTDCDelay; // in ns, time difference between tdc ftrig and caen ftrig 

  std::vector<uint16_t> fFragIDs;

  uint16_t fVerbose;

  bool fCalculateBaseline;  
  bool fCountPMTs;          
  bool fCalculatePEMetrics; 
  bool fFindFlashInfo;

  std::vector<float> fInputBaseline; 
  float fPromptWindow;
  float fPrelimWindow; 

  float fADCThreshold;  
  float fADCtoPE;       
  // end fhicl parameters

  // event-by-event global variables
  // bool foundBeamTrigger;

  // other global variables
  float ticks_to_us;
  float us_to_ticks;

  // pmt information
  std::map<int,int> map_fragid_index;

  bool     getTDCTime(artdaq::Fragment & frag, uint64_t & tdcTime);
  void     getWaveforms(const artdaq::Fragment &frag, std::vector<std::vector<uint16_t>> &wvfm_v);
  uint32_t getStartTime(const artdaq::Fragment &frag);
  uint32_t getTriggerTime(const artdaq::Fragment &frag);
  uint32_t getLength   (const artdaq::Fragment &frag);
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

  fCAENInstanceLabels = p.get<std::vector<std::string>>("CAENInstanceLabels");
  fWindowLength       = p.get<float>("WindowLength", 1.8);
  fWvfmPostPercent    = p.get<float>("WvfmPostPercent", 0.9);
  fWvfmNominalLength         = p.get<uint16_t>("WvfmNominalLength", 5000); // units of ticks

  fTimingType         = p.get<uint8_t>("TimingType", 0);
  
  fNTBDelay           = p.get<uint32_t>("NTBDelay", 367392); // units of ns

  fSPECTDCModuleLabel    = p.get<std::string>("SPECTDCModuleLabel", "daq");
  fSPECTDCInstanceLabels = p.get<std::vector<std::string>>("SPECTDCInstanceLabels"); 
  fSPECTDCTimingChannel  = p.get<uint8_t>("SPECTDCTimingChannel", 4);
  fSPECTDCDelay          = p.get<int32_t>("SPECTDCDelay", 2133);

  fFragIDs            = p.get<std::vector<uint16_t>>("FragIDs");

  fVerbose            = p.get<uint8_t>("Verbose", 0);

  fCalculateBaseline  = p.get<bool>("CalculateBaseline",true);
  fCountPMTs          = p.get<bool>("CountPMTs",true);
  fCalculatePEMetrics = p.get<bool>("CalculatePEMetrics",false);
  fFindFlashInfo      = p.get<bool>("FindFlashInfo",false);

  fInputBaseline      = p.get<std::vector<float>>("InputBaseline");
  fPromptWindow       = p.get<float>("PromptWindow", 0.1);
  fPrelimWindow       = p.get<float>("PrelimWindow", 0.5);
  fADCThreshold       = p.get<float>("ADCThreshold", 7960);
  fADCtoPE            = p.get<float>("ADCtoPE", 8.0);
}


void sbnd::trigger::pmtSoftwareTriggerProducer::produce(art::Event& e)
{
  if (fVerbose==1) std::cout << "Processing Run: " << e.run() << ", Subrun: " <<  e.subRun() << ", Event: " << e.id().event() << std::endl;
  // object to store trigger metrics in
  std::unique_ptr<sbnd::trigger::pmtSoftwareTrigger> pmtSoftwareTriggerMetrics = std::make_unique<sbnd::trigger::pmtSoftwareTrigger>();

  // the reference time stamp, usually the event trigger time
  int32_t refTimestamp=0; 
  auto timing_type = fTimingType;

  // section to obtain global timing information 
  art::Handle<artdaq::detail::RawEventHeader> header_handle;
  e.getByLabel("daq", "RawEventHeader", header_handle);
  if (timing_type==0){
    if ((header_handle.isValid())){
      auto rawheader = artdaq::RawEvent(*header_handle);
      refTimestamp = rawheader.timestamp()%int(1e9) - fNTBDelay;
      std::cout << "Using rawheader timestamp..." << std::endl;
    }
    else{
      std::cout << "RawEventHeader not valid. Using SPEC TDC..." << std::endl;
      timing_type++;
    }
  }
  if (timing_type==1){
    bool found_tdc_timing_ch = false;
    uint64_t tdcTS = 0;
    for(const std::string &SPECTDCInstanceLabel : fSPECTDCInstanceLabels){
      art::Handle<std::vector<artdaq::Fragment>> tdcHandle;
      e.getByLabel(fSPECTDCModuleLabel, SPECTDCInstanceLabel, tdcHandle);

      if(!tdcHandle.isValid() || tdcHandle->size() == 0)
        continue;

      if(tdcHandle->front().type() == artdaq::Fragment::ContainerFragmentType){
        for(auto cont : *tdcHandle){
          artdaq::ContainerFragment contf(cont);
          if(contf.fragment_type() == sbndaq::detail::FragmentType::TDCTIMESTAMP){
            for(unsigned i = 0; i < contf.block_count(); ++i)
              found_tdc_timing_ch = getTDCTime(*contf[i].get(),tdcTS);
          }
        }
      }
      else if((tdcHandle->front().type() == sbndaq::detail::FragmentType::TDCTIMESTAMP) && (found_tdc_timing_ch==false)){
        for(auto frag : *tdcHandle)       
          getTDCTime(frag,tdcTS);     
      }
    }
    refTimestamp = int32_t(tdcTS) - int32_t(fSPECTDCDelay);

    if (refTimestamp==0){
      std::cout << "No valid TDC timestamp found. Using PTB..." << std::endl;
      timing_type++;
    }
  }
  if (timing_type>=2){
    std::cout << "PTB Timing Reference not implemented...No usable timing reference.\nProducing empty metrics." << std::endl;
    e.put(std::move(pmtSoftwareTriggerMetrics));
    return;
  }


  std::vector<std::vector<uint16_t>> wvfms_v;
  std::vector<float>                 baselines_v;

  auto nboards = fFragIDs.size();
  wvfms_v.resize(15*nboards); // 15 PMT channels per 1730 fragment, 8 fragments per trigger
  baselines_v.resize(15*nboards); 

  // variables to find the beam fragment index 
  // storage for the beam fragments 
  // the index inside a container to find the "beam" fragment (closest to the event trigger time)
  size_t beam_frag_idx = 0;
  // to store the time difference between the beam fragment start time and event trigger time 
  int32_t beam_frag_dt = 1e9; 

  // loop over fragment handles
  for (const std::string &caen_name : fCAENInstanceLabels){
    art::Handle<std::vector<artdaq::Fragment>> fragmentHandle;
    e.getByLabel("daq", caen_name, fragmentHandle);

    if (!fragmentHandle.isValid() || fragmentHandle->size() == 0){
        std::cout << "No CAEN V1730 fragments with label " << caen_name << " found." << std::endl;
        continue;
    }
    // Container fragment
    // # of containers = # of boards
    // # of entries inside the container = # of triggers 
    if (fragmentHandle->front().type() == artdaq::Fragment::ContainerFragmentType) {
      for (auto cont : *fragmentHandle) {
	      artdaq::ContainerFragment contf(cont);
        if (contf.fragment_type()==sbndaq::detail::FragmentType::CAENV1730) {
          if (std::find(fFragIDs.begin(), fFragIDs.end(), contf[0].get()->fragmentID()) == fFragIDs.end()) continue;

          if (beam_frag_dt==1e9){
            for (size_t ii = 0; ii < contf.block_count(); ++ii){
              // find the absolute time difference between the fragment start time and the reference time stamp 
              int dt = int(refTimestamp) - int(getTriggerTime(*contf[ii].get()));
              if (abs(dt) < beam_frag_dt){
                  beam_frag_dt = dt;
                  beam_frag_idx = ii;
              }
            }
          }
          getWaveforms(*contf[beam_frag_idx].get(), wvfms_v);
        }
      } // loop over containers 
    } // if 1730 container
    //Regular CAEN fragment
    else if (fragmentHandle->front().type()==sbndaq::detail::FragmentType::CAENV1730) {
      if (fVerbose==2) std::cout << "Found " << fragmentHandle->size() << " CAEN1730 fragments" << std::endl;
        for (size_t ii = 0; ii < fragmentHandle->size(); ++ii){
          auto frag = fragmentHandle->at(ii);        
          if (std::find(fFragIDs.begin(), fFragIDs.end(), frag.fragmentID()) == fFragIDs.end()) continue;
          if (ii==beam_frag_idx)
            getWaveforms(frag, wvfms_v);
      }
    }
  } // loop over handles
  
  if (fVerbose==1) std::cout << "reference time stamp is " << refTimestamp << " ns" << std::endl;
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

    for (size_t i_ch = 0; i_ch < wvfms_v.size(); ++i_ch){
      auto wvfm = wvfms_v[i_ch];

      if (wvfm.begin() == wvfm.end()) continue;

      int windowStartBin = (1.0 - fWvfmPostPercent)*wvfm.size(); 
      int windowEndBin = windowStartBin + int(fWindowLength*us_to_ticks);

      // calculate baseline
      if (fCalculateBaseline) baselines_v.at(i_ch) = estimateBaseline(wvfm);
      else baselines_v.at(i_ch) = fInputBaseline.at(0);
      
      // count number of PMTs above threshold
      if (fCountPMTs){
        if (windowEndBin >= (int)wvfm.size()){ windowEndBin = (int)wvfm.size() - 1;}
        for (int bin = windowStartBin; bin < windowEndBin; ++bin){
	         auto adc = wvfm[bin];
	          if (adc!=0 && adc < fADCThreshold){ nAboveThreshold++; break; }
        }
      }
      else { nAboveThreshold = -9999; }

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

    if(metricMan != nullptr) {
        //send flag metrics
        metricMan->sendMetric(
            "PMT_SWTrig_nAboveThreshold",
            nAboveThreshold,
            "Num PMTs Above Software Trigger Threshold", 5, artdaq::MetricMode::LastPoint);

        // if (fVerbose==1) {
        //   TLOG(TLVL_INFO) << "nPMTs Above Threshold: " << nAboveThreshold;
        // }

    }
  }  // if found beam trigger
  else{
    if (fVerbose==1) {
      // std::cout << "Beam and wvfms not found" << std::endl;
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

bool sbnd::trigger::pmtSoftwareTriggerProducer::getTDCTime(artdaq::Fragment & frag, uint64_t & tdcTime) {

  bool found_timing_ch = false;
  const auto tsfrag = sbndaq::TDCTimestampFragment(frag);
  const auto ts = tsfrag.getTDCTimestamp();
  if (ts->vals.channel == fSPECTDCTimingChannel){
    found_timing_ch = true;
    tdcTime = ts->timestamp_ns()%(uint64_t(1e9));
    std::cout << "TDC CH "<< ts->vals.channel
                          << " -> timestamp: " << tdcTime << " ns" 
                          << ", name: "
                          << ts->vals.name[0]
                          << ts->vals.name[1]
			                    << ts->vals.name[2]
			                    << ts->vals.name[3]
                          << ts->vals.name[4]
                          << std::endl;
  }
  return found_timing_ch;
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
  if (fVerbose==2) std::cout << "\tFlash start time is " << int(wvfm_start) << " us" << std::endl;

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

uint32_t sbnd::trigger::pmtSoftwareTriggerProducer::getTriggerTime(const artdaq::Fragment &frag){
  //--get number of channels from metadata and waveform length from header
  uint32_t timestamp = frag.timestamp()%uint(1e9);
  sbndaq::CAENV1730Fragment bb(frag);
  auto const* md = bb.Metadata();
  sbndaq::CAENV1730Event const* event_ptr = bb.Event();
  sbndaq::CAENV1730EventHeader header = event_ptr->Header;
  
  uint32_t ttt = header.triggerTimeTag*8;

  uint32_t trigger_ts; 
  // this assumes that if timestamp != ttt... then the time tag shift is enabled in the caen configuration
  if (timestamp!=ttt){
    std::cout << "timestamp: " << timestamp << ", ttt: " << ttt << std::endl;
    trigger_ts = timestamp;
  }
  // if the time tag shift is not enabled in the caen configuration
  // use the postpercent fcl parameter
  else if (timestamp==ttt){
    size_t nChannels = md->nChannels;
    uint32_t ev_size_quad_bytes = header.eventSize;
    uint32_t evt_header_size_quad_bytes = sizeof(sbndaq::CAENV1730EventHeader)/sizeof(uint32_t);
    uint32_t data_size_double_bytes = 2*(ev_size_quad_bytes - evt_header_size_quad_bytes);
    uint32_t wvfm_length = data_size_double_bytes/nChannels;
    
    trigger_ts = ttt - 2*wvfm_length*fWvfmPostPercent;
  }

  return trigger_ts;
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
