////////////////////////////////////////////////////////////////////////
// Class:       pmtSoftwareTriggerProducer
// Plugin Type: producer (Unknown Unknown)
// File:        pmtSoftwareTriggerProducer_module.cc
//
// Generated at Thu Feb 17 13:22:51 2022 by Patrick Green using cetskelgen
// from  version .

// Authors: Erin Yandel, Patrick Green, Lynn Tung
// Module to implement software trigger metrics to the PMT Trigger simulation
// Input: artdaq fragment output from the pmtArtdaqFragmentProducer.cc module
// Calculates various PMT metrics for every event (that passes the hardware trigger)
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

#include "sbndaq-artdaq-core/Obj/SBND/pmtSoftwareTrigger.hh"

// ROOT includes
#include "TH1D.h"
#include "TFile.h"
#include "TTree.h"

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
  bool     fUseBeamTrigger;  // whether to use flash trigger or beam trigger 
  double   fWindowOffset;    // offset of trigger time in us
  uint32_t fTrigWindowFrag;  //max time difference for frag timestamp to be part of beam window
                             //  in ticks (1 tick=2 ns), default 100 ticks - WAG for now
  double   fWindowLength;    // in us, window length after trigger time, default 1.6 us
  double   fWvfmPostPercent; // post percent of wvfm, 9 us after wvfm = 0.9
  
  uint32_t    fETrigFragid;
  uint32_t    fETrigThresh;
  
  std::vector<int> fFragidlist;
  std::vector<int> fUncoated;
  std::vector<int> fTPC;

  int fVerbose;

  bool fCalculateBaseline;  
  bool fCountPMTs;          
  bool fCalculatePEMetrics; 
  bool fFindFlashInfo;
  bool fFindPulses;         

  std::vector<double> fInputBaseline; 
  double fBaselineWindow;
  double fPromptWindow;
  double fPrelimWindow; 

  double fADCThreshold;  
  double fADCtoPE;       
  double fPEArea;        
  // end fhicl parameters

  // event information
  int fRun, fSubrun;
  art::EventNumber_t fEvent;

  // event-by-event global variables
  uint64_t refTimestamp; // the reference time stamp, either beam trigger or trigger time stamp (flash) depending on configuration
  bool foundBeamTrigger;

  // other global variables
  double ticks_to_us;
  double us_to_ticks;

  // waveforms
  uint32_t fTriggerBin; 
  uint32_t fWvfmLength;
  std::vector<std::vector<uint16_t>> fWvfmsVec;

  // pmt information
  std::vector<sbnd::trigger::pmtInfo> fpmtInfoVec;
  std::map<int,int> map_fragid_index;

  void   analyzeCAEN1730Fragment(const artdaq::Fragment &frag);
  void   estimateBaseline(int i_ch);
  double estimateBaseline(std::vector<uint32_t> wvfm);
  void   SimpleThreshAlgo(int i_ch);
  void   reconfigure(fhicl::ParameterSet const & p);
  void   find_beam_spill(const artdaq::Fragment &frag);
  std::vector<uint32_t> sumWvfms(const std::vector<uint32_t>& v1, const std::vector<uint16_t>& v2);
};


sbnd::trigger::pmtSoftwareTriggerProducer::pmtSoftwareTriggerProducer(fhicl::ParameterSet const& p)
  : EDProducer{p}
  {
    this->reconfigure(p);
    // Call appropriate produces<>() functions here.
    produces< sbnd::trigger::pmtSoftwareTrigger >("", is_persistable_);
    // map from fragID to array index 0-7
    for (size_t i=0;i<fFragidlist.size();++i){
      map_fragid_index.insert(std::make_pair(fFragidlist[i],i));
    }

    us_to_ticks = 500.; // ticks per us 
    ticks_to_us = 1/us_to_ticks; // us per ticks
 
  }

void sbnd::trigger::pmtSoftwareTriggerProducer::reconfigure(fhicl::ParameterSet const & p)
{
  // Initialize member data here
  is_persistable_     = p.get<bool>("is_persistable", true) ? art::Persistable::Yes : art::Persistable::No;
  fUseBeamTrigger     = p.get<bool>("UseBeamTrigger",false);
  fWindowOffset       = p.get<double>("WindowOffset",0.1);  //units of us

  fTrigWindowFrag     = p.get<uint32_t>("TrigWindowFrag",100);  //units of ticks, 1 tick=2ns
  fWindowLength       = p.get<double>("WindowLength", 1.8);
  fWvfmPostPercent    = p.get<double>("WvfmPostPercent", 0.9);

  fETrigFragid        = p.get<uint32_t>("ETrigFragID",0);
  fETrigThresh        = p.get<uint32_t>("ETrigThresh",5000);

  fFragidlist         = p.get<std::vector<int>>("FragIDlist");

  fUncoated           = p.get<std::vector<int>>("Uncoated");  // =0 for Coated, =1 for Uncoated
  fTPC                = p.get<std::vector<int>>("TPC");  // TPC=0(1) is neg(pos) x and beam right(left)

  fVerbose            = p.get<int>("Verbose", 0);

  fCalculateBaseline  = p.get<bool>("CalculateBaseline",true);
  fCountPMTs          = p.get<bool>("CountPMTs",true);
  fCalculatePEMetrics = p.get<bool>("CalculatePEMetrics",false);
  fFindFlashInfo      = p.get<bool>("FindFlashInfo",false);
  fFindPulses         = p.get<bool>("FindPulses", false);

  fInputBaseline      = p.get<std::vector<double>>("InputBaseline");
  fBaselineWindow     = p.get<double>("BaselineWindow", 0.5);
  fPromptWindow       = p.get<double>("PromptWindow", 0.1);
  fPrelimWindow       = p.get<double>("PrelimWindow", 0.5);
  fADCThreshold       = p.get<double>("ADCThreshold", 7960);
  fADCtoPE            = p.get<double>("ADCtoPE", 8.0);
  fPEArea             = p.get<double>("PEArea", 66.3);
}


void sbnd::trigger::pmtSoftwareTriggerProducer::produce(art::Event& e)
{
  fRun = e.run();
  fSubrun = e.subRun();
  fEvent = e.id().event();

  if (fVerbose==1) std::cout << "Processing Run: " << fRun << ", Subrun: " << fSubrun << ", Event: " << fEvent << std::endl;

  // reset for this event
  foundBeamTrigger = false;
  fTriggerBin = 0; 

  fWvfmsVec.clear(); fWvfmsVec.resize(16*8); // 16 1730 channels per fragment, 8 fragments per trigger
  fpmtInfoVec.clear(); fpmtInfoVec.resize(15*8); // 15 PMTs per fragment, 8 fragments per trigger

  std::vector<art::Handle<artdaq::Fragments>> fragmentHandles = e.getMany<std::vector<artdaq::Fragment>>();

  // Loop over fragments to find the beam spill time
  refTimestamp = 0;
  // loop over fragment handles
  for (auto &handle : fragmentHandles) {
    if (!handle.isValid() || handle->size() == 0) continue;
    //Container fragment
    if (handle->front().type() == artdaq::Fragment::ContainerFragmentType) {
      for (auto cont : *handle) {
	      artdaq::ContainerFragment contf(cont);
        if (contf.fragment_type()==sbndaq::detail::FragmentType::CAENV1730) {
	        if (fVerbose==2) std::cout << "Found " << contf.block_count() << " CAEN Fragments in container " << std::endl;
          for (size_t ii = 0; ii < contf.block_count(); ++ii){
            analyzeCAEN1730Fragment(*contf[ii].get());
	          if (fUseBeamTrigger) find_beam_spill(*contf[ii].get());
          }
        } // if
      } //for loop over fragments in container
    } // if 1730 container
    //Regular CAEN fragment
    else if (handle->front().type()==sbndaq::detail::FragmentType::CAENV1730) {
      if (fVerbose==2) std::cout << "Found " << handle->size() << " CAEN1730 fragments" << std::endl;
      for (auto frag : *handle){
        analyzeCAEN1730Fragment(frag);
        if (fUseBeamTrigger) find_beam_spill(frag);
      }
    } // if/else container/fragmnet
  } // loop over handles
  
  if (fVerbose==1) std::cout << "reference time stamp is " << refTimestamp << std::endl;
  // object to store trigger metrics in
  std::unique_ptr<sbnd::trigger::pmtSoftwareTrigger> pmtSoftwareTriggerMetrics = std::make_unique<sbnd::trigger::pmtSoftwareTrigger>();

  // if we're looking for the beam, make sure we found the beam trigger 
  if (fUseBeamTrigger && !foundBeamTrigger) refTimestamp=0;

    // calculate metrics
  if (refTimestamp!=0) {
    
    // store whether (1) we're using the beam trigger and (2) if we found it.
    // if both conditions (1) and (2) are fulfilled, then it will be true. Otherwise, false.
    if (fUseBeamTrigger && foundBeamTrigger) pmtSoftwareTriggerMetrics->foundBeamTrigger = true;
    else pmtSoftwareTriggerMetrics->foundBeamTrigger = false;

    // store timestamp of trigger 
    double triggerTimeStamp = (double)refTimestamp; // - fWindowOffset*ticks_to_us;
    pmtSoftwareTriggerMetrics->triggerTimestamp = triggerTimeStamp;
    if (fVerbose==1) std::cout << "Saving event trigger timestamp: " << triggerTimeStamp << " ns" << std::endl;

    double promptPE = 0;
    double prelimPE = 0;
    double flash_peakPE = 0;
    double flash_peaktime = 0;

    int nAboveThreshold = 0;

    int windowStartBin=0; 
    int windowEndBin=0;
    if (!fUseBeamTrigger){
      // if using the flash trigger...
      windowStartBin = (1.0 - fWvfmPostPercent)*fWvfmLength;
      windowEndBin = windowStartBin + fWindowLength*us_to_ticks;
    } 
    else{
      // ! this probably doesn't work fyi :) 
      windowStartBin = (triggerTimeStamp >= 1000)? 0 : int(500 - abs((triggerTimeStamp)/2));
      windowEndBin = (triggerTimeStamp >= 1000)? int(500 + (fWindowLength*1e3 - (triggerTimeStamp-1000))/2) : (windowStartBin + (fWindowLength*1e3)/2);
    }

    if (fVerbose==1) std::cout << "windowStartBin " << windowStartBin << " windowEndBin " << windowEndBin  <<
		    " trigger time stamp " << triggerTimeStamp << " window length " << fWindowLength << std::endl;

    // create a vector to contain the sum of all the wvfms
    std::vector<uint32_t> wvfm_sum(fWvfmLength, 0);
    // wvfm loop to calculate metrics
    for (int i_ch = 0; i_ch < (int)fpmtInfoVec.size(); ++i_ch){
      auto &pmtInfo = fpmtInfoVec.at(i_ch);
      auto wvfm = fWvfmsVec[i_ch];

      if (wvfm.begin() == wvfm.end()) continue;

      // assign channel
      pmtInfo.channel = i_ch;

      // calculate baseline
      if (fCalculateBaseline) estimateBaseline(i_ch);
      else { pmtInfo.baseline=fInputBaseline.at(0); pmtInfo.baselineSigma = fInputBaseline.at(1); }
      
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
        double baseline = pmtInfo.baseline;
        
        auto prelim_offset = (windowStartBin - fPrelimWindow*us_to_ticks) < 0 ? 0 : windowStartBin - fPrelimWindow*us_to_ticks;
        auto prompt_window = std::vector<uint16_t>(wvfm.begin()+windowStartBin, wvfm.begin()+windowStartBin+fPromptWindow*us_to_ticks);
        auto prelim_window = std::vector<uint16_t>(wvfm.begin()+prelim_offset, wvfm.begin()+windowStartBin);

        if (fFindPulses == false){
          // this sums the peaks, not the integral!
          double ch_promptPE = (baseline-(*std::min_element(prompt_window.begin(), prompt_window.end())))/fADCtoPE;
          double ch_prelimPE = (baseline-(*std::min_element(prelim_window.begin(), prelim_window.end())))/fADCtoPE;
          promptPE += ch_promptPE;
          prelimPE += ch_prelimPE;
        }
        // pulse finder + prompt and prelim calculation with pulses
        if (fFindPulses == true){
          SimpleThreshAlgo(i_ch);
          for (auto pulse : pmtInfo.pulseVec){
            if (pulse.t_start > windowStartBin && pulse.t_end < (windowStartBin+fPromptWindow*us_to_ticks)) promptPE+=pulse.pe;
            if (pulse.t_start > prelim_offset && pulse.t_end < windowStartBin) prelimPE+=pulse.pe;
            }
          }
        }
      else { promptPE = -9999; prelimPE = -9999;}

      if (fFindFlashInfo) wvfm_sum = sumWvfms(wvfm_sum, wvfm);
    } // end of wvfm loop
    if (fFindFlashInfo){
      // find and remove the baseline
      auto flash_baseline = estimateBaseline(wvfm_sum);
      flash_peakPE   = (flash_baseline-(*std::min_element(wvfm_sum.begin()+windowStartBin, wvfm_sum.end())))/fADCtoPE;
      flash_peaktime = (std::min_element(wvfm_sum.begin()+windowStartBin, wvfm_sum.end()) - wvfm_sum.begin()-windowStartBin)*ticks_to_us;
    }
    pmtSoftwareTriggerMetrics->nAboveThreshold = nAboveThreshold;
    pmtSoftwareTriggerMetrics->promptPE = promptPE;
    pmtSoftwareTriggerMetrics->prelimPE = prelimPE;
    pmtSoftwareTriggerMetrics->peakPE = flash_peakPE;
    pmtSoftwareTriggerMetrics->peaktime = flash_peaktime;

    if (fVerbose==1) std::cout << "nPMTs Above Threshold: " << nAboveThreshold << std::endl;
    if (fVerbose==1) std::cout << "prompt pe: " << promptPE << std::endl;
    if (fVerbose==1) std::cout << "prelim pe: " << prelimPE << std::endl;
    if (fVerbose==1) std::cout << "peak pe: "   << flash_peakPE << std::endl;
    if (fVerbose==1) std::cout << "peak time: " << flash_peaktime << " us" << std::endl;

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
    pmtSoftwareTriggerMetrics->triggerTimestamp = -9999;
    pmtSoftwareTriggerMetrics->nAboveThreshold = -9999;
    pmtSoftwareTriggerMetrics->promptPE = -9999;
    pmtSoftwareTriggerMetrics->prelimPE = -9999;
    pmtSoftwareTriggerMetrics->peakPE   = -9999;
    pmtSoftwareTriggerMetrics->peaktime = -9999;
  }
  e.put(std::move(pmtSoftwareTriggerMetrics));
}

std::vector<uint32_t> sbnd::trigger::pmtSoftwareTriggerProducer::sumWvfms(const std::vector<uint32_t>& v1, const std::vector<uint16_t>& v2) {
    std::vector<uint32_t>  result(v1.size(),0);
    for (size_t i = 0; i < v1.size(); i++)
        result.at(i) = (v1[i] + v2[i]);
    return result;
}

void sbnd::trigger::pmtSoftwareTriggerProducer::find_beam_spill(const artdaq::Fragment &frag) {

  // check fragId
  uint fragId = static_cast<int>(frag.fragmentID());
  if (fVerbose==2) std::cout << "Looking for beam spill, fragID is " << fragId <<
		  " and timestamp is " << frag.timestamp() << std::endl;
  if (fragId!=fETrigFragid)  return;

  //--get number of channels from metadata and waveform length from header
  sbndaq::CAENV1730Fragment bb(frag);
  auto const* md = bb.Metadata();
  sbndaq::CAENV1730Event const* event_ptr = bb.Event();
  sbndaq::CAENV1730EventHeader header = event_ptr->Header;
  size_t nChannels = md->nChannels;

  uint32_t ev_size_quad_bytes = header.eventSize;
  if (fVerbose==2) std::cout << "Event size in quad bytes is: " << ev_size_quad_bytes << "\n";
  uint32_t evt_header_size_quad_bytes = sizeof(sbndaq::CAENV1730EventHeader)/sizeof(uint32_t);
  uint32_t data_size_double_bytes = 2*(ev_size_quad_bytes - evt_header_size_quad_bytes);
  //-  uint32_t wfm_length = data_size_double_bytes/nChannels;
  fWvfmLength = data_size_double_bytes/nChannels;
  if (fVerbose==2) std::cout << "Channel waveform length = " << fWvfmLength << " [ticks], or " << fWvfmLength*ticks_to_us << "us\n";
  size_t ch_offset = (size_t)(15*fWvfmLength);
  if (fVerbose==2) std::cout << " frag timestamp is " << frag.timestamp()  << std::endl;

  // access beam signal, in ch15 of first PMT of each fragment set
  const uint16_t* data_begin = reinterpret_cast<const uint16_t*>(frag.dataBeginBytes()
					   + sizeof(sbndaq::CAENV1730EventHeader));
  const uint16_t* value_ptr =  data_begin;
  uint16_t value = 0;

  // not currently used because there's no guarantee where the trigger is in the waveform
  //  size_t tr_offset = refTimestampOffset*1e3;
  size_t tr_offset = 0;
  value_ptr = data_begin + ch_offset + tr_offset; // pointer arithmetic

  for (uint ind=0;ind<fWvfmLength;++ind){
    value = *(value_ptr+ind);
    if (value>fETrigThresh) {
      foundBeamTrigger = true;
      fTriggerBin = ind;
      refTimestamp = frag.timestamp();
      std::cout << "Found beam trigger : ind= " << ind << " adc is " << value;
      std::cout << "timestamp is " << refTimestamp << std::endl;
      break;
    }
  }

  std::cout << "leaving find_beam_spill, timestamp is " << refTimestamp << std::endl;
}

void sbnd::trigger::pmtSoftwareTriggerProducer::analyzeCAEN1730Fragment(const artdaq::Fragment &frag)
{
  //--get number of channels from metadata and waveform length from header
  sbndaq::CAENV1730Fragment bb(frag);
  auto const* md = bb.Metadata();
  sbndaq::CAENV1730Event const* event_ptr = bb.Event();
  sbndaq::CAENV1730EventHeader header = event_ptr->Header;
  
  uint32_t ttt = header.triggerTimeTag*4; // downsampled trigger time tag; TTT points to end of wvfm w.r.t. pps

  int fragId = static_cast<int>(frag.fragmentID());
  std::map<int,int>::iterator it = map_fragid_index.find(fragId);
  int findex = it->second;

  size_t nChannels = md->nChannels;
  uint32_t ev_size_quad_bytes = header.eventSize;
  uint32_t evt_header_size_quad_bytes = sizeof(sbndaq::CAENV1730EventHeader)/sizeof(uint32_t);
  uint32_t data_size_double_bytes = 2*(ev_size_quad_bytes - evt_header_size_quad_bytes);
  fWvfmLength = data_size_double_bytes/nChannels;

  if (fVerbose==2) std::cout << "From header, downsampled TTT is " << ttt << "\n";
  if (fVerbose==2) std::cout << "\tNumber of channels: " << nChannels << "\n";
  if (fVerbose==2) std::cout << "Event size in quad bytes is: " << ev_size_quad_bytes << "\n";
  if (fVerbose==2) std::cout << "Channel waveform length = " << fWvfmLength << "\n";

  auto trigger_position = ttt - fWvfmPostPercent*fWvfmLength; // ticks, position of trigger in waveform (bin)
  auto trigger_time = trigger_position*ticks_to_us;
  if (fVerbose==2) std::cout << "\tFlash trigger time is " << trigger_time << " us" << std::endl;

  refTimestamp = trigger_time;
  fTriggerBin = trigger_position;

  //--access waveforms in fragment and save
  const uint16_t* data_begin = reinterpret_cast<const uint16_t*>(frag.dataBeginBytes()
					+ sizeof(sbndaq::CAENV1730EventHeader));
  const uint16_t* value_ptr =  data_begin;
  uint16_t value = 0;
  size_t ch_offset = 0;

  // loop over channels
  // nChannels--; // last waveform isn't a PMT
  for (size_t i_ch = 0; i_ch < (nChannels-1); ++i_ch){
    fWvfmsVec[i_ch + nChannels*findex].resize(fWvfmLength);
    ch_offset = (size_t)(i_ch * fWvfmLength);
    //--loop over waveform samples
    for(size_t i_t = 0; i_t < fWvfmLength; ++i_t){
      value_ptr = data_begin + ch_offset + i_t; // pointer arithmetic
      value = *(value_ptr);
      fWvfmsVec[i_ch + nChannels*findex][i_t] = value;
    } //--end loop samples
  } //--end loop channels
}

void sbnd::trigger::pmtSoftwareTriggerProducer::estimateBaseline(int i_ch){
  auto wvfm = fWvfmsVec[i_ch];
  auto &pmtInfo = fpmtInfoVec[i_ch];
  // assuming that the first 500 ns doesn't include peaks, find the mean of the ADC count as the baseline
  std::vector<uint16_t> subset = std::vector<uint16_t>(wvfm.begin(),  wvfm.begin()+int(fBaselineWindow*us_to_ticks));
  double subset_mean = (std::accumulate(subset.begin(), subset.end(), 0))/(subset.size());
  double val = 0;
  for (size_t i=0; i<subset.size();i++){ val += (subset[i] - subset_mean)*(subset[i] - subset_mean);}
  double subset_stddev = sqrt(val/subset.size());

  if (subset_stddev > 3){ // make this fcl parameter?
    val = 0; subset.clear(); subset_stddev = 0;
    subset = std::vector<uint16_t>(wvfm.end()-int(fBaselineWindow*us_to_ticks), wvfm.end());
    subset_mean = (std::accumulate(subset.begin(), subset.end(), 0))/(subset.size());
    for (size_t i=0; i<subset.size();i++){ val += (subset[i] - subset_mean)*(subset[i] - subset_mean);}
    subset_stddev = sqrt(val/subset.size());
  }
  if (fVerbose==3) std::cout << "Baseline mean: " << subset_mean << " ADC, with " << subset_stddev << " sigma" << std::endl;
  pmtInfo.baseline = subset_mean;
  pmtInfo.baselineSigma = subset_stddev;
}

double sbnd::trigger::pmtSoftwareTriggerProducer::estimateBaseline(std::vector<uint32_t> wvfm){
  std::vector<uint32_t> subset = std::vector<uint32_t>(wvfm.begin(), wvfm.begin()+int(fBaselineWindow*us_to_ticks));
  double subset_mean = (std::accumulate(subset.begin(), subset.end(), 0))/(subset.size());
  double val = 0;
  for (size_t i=0; i<subset.size();i++){ val += (subset[i] - subset_mean)*(subset[i] - subset_mean);}
  double subset_stddev = sqrt(val/subset.size());

  if (subset_stddev > 3){ // make this fcl parameter?
    val = 0; subset.clear(); subset_stddev = 0;
    subset = std::vector<uint32_t>(wvfm.end()-int(fBaselineWindow*us_to_ticks), wvfm.end());
    subset_mean = (std::accumulate(subset.begin(), subset.end(), 0))/(subset.size());
    for (size_t i=0; i<subset.size();i++){ val += (subset[i] - subset_mean)*(subset[i] - subset_mean);}
    subset_stddev = sqrt(val/subset.size());
  }
  return subset_mean;
}

void sbnd::trigger::pmtSoftwareTriggerProducer::SimpleThreshAlgo(int i_ch){
  auto wvfm = fWvfmsVec[i_ch];
  auto &pmtInfo = fpmtInfoVec[i_ch];
  double baseline = pmtInfo.baseline;

  bool fire = false; // bool for if pulse has been detected
  int counter = 0; // counts the bin of the waveform

  // these should be fcl parameters 
  double start_adc_thres = 5, end_adc_thres = 2; 
  auto start_threshold = baseline-start_adc_thres;
  auto end_threshold   = baseline-end_adc_thres; 

  std::vector<sbnd::trigger::pmtPulse> pulse_vec;
  sbnd::trigger::pmtPulse pulse; 
  pulse.area = 0; pulse.peak = 0; pulse.t_start = 0; pulse.t_end = 0; pulse.t_peak = 0;
  for (auto const &adc : wvfm){
    if ( !fire && ((double)adc) <= start_threshold ){ // if its a new pulse 
      fire = true;
      pulse.t_start = counter - 1 > 0 ? counter - 1 : counter;    
    }

    else if( fire && ((double)adc) > end_threshold ){ // found end of a pulse
      fire = false;
      pulse.t_end = counter < ((int)wvfm.size())  ? counter : counter - 1;
      pulse_vec.push_back(pulse);
      pulse.area = 0; pulse.peak = 0; pulse.t_start = 0; pulse.t_end = 0; pulse.t_peak = 0;
    }   

    else if(fire){ // if we're in a pulse 
      pulse.area += (baseline-(double)adc);
      if ((baseline-(double)adc) > pulse.peak) { // Found a new maximum
        pulse.peak = (baseline-(double)adc);
        pulse.t_peak = counter;
      }
    }
    counter++;
  }

  if(fire){ // Take care of a pulse that did not finish within the readout window.
    fire = false;
    pulse.t_end = counter - 1;
    pulse_vec.push_back(pulse);
    pulse.area = 0; pulse.peak = 0; pulse.t_start = 0; pulse.t_end = 0; pulse.t_peak = 0;
  }

  pmtInfo.pulseVec = pulse_vec;
  // calculate PE from area 
  for (auto &pulse : pmtInfo.pulseVec){pulse.pe = pulse.area/fPEArea;}
}

DEFINE_ART_MODULE(sbnd::trigger::pmtSoftwareTriggerProducer)
