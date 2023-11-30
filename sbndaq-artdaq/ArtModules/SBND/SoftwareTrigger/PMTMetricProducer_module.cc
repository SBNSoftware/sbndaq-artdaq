////////////////////////////////////////////////////////////////////////
// Class:       pmtSoftwareTriggerProducer
// Plugin Type: producer (Unknown Unknown)
// File:        pmtSoftwareTriggerProducer_module.cc
//
// Generated at Thu Feb 17 13:22:51 2022 by Patrick Green using cetskelgen
// from  version .

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
//#include "art_root_io/TFileService.h"
#include "artdaq/DAQdata/Globals.hh"

#include "sbndaq-artdaq-core/Overlays/Common/CAENV1730Fragment.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "artdaq-core/Data/Fragment.hh"
#include "artdaq-core/Data/ContainerFragment.hh"

//#include "sbndcode/OpDetSim/sbndPDMapAlg.hh"
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
  double fWindowOffset;    // offset of trigger time in us
  uint32_t fTrigWindowFrag;  //max time difference for frag timestamp to be part of beam window
                         //  in ticks (1 tick=2 ns), default 100 ticks - WAG for now
  double fWindowLength; // window length after trigger time, default 1.6 us
  uint32_t fWvfmLength;  // overwritten in code, fhicl value not used
  uint32_t    fETrigFragid;
  uint32_t    fETrigThresh;
  std::vector<int> fFragidlist;
  std::vector<int> fUncoated;
  std::vector<int> fTPC;

  bool fVerbose;
  bool fSaveHists;

  std::string fBaselineAlgo;
  double fInputBaseline;
  double fInputBaselineSigma;
  int fADCThreshold;
  bool fFindPulses;
  double fPEArea; // conversion factor from ADCxns area to PE count
  // end fhicl parameters

  // histogram info
  std::stringstream histname; //raw waveform hist name
  //art::ServiceHandle<art::TFileService> tfs;

  // event information
  int fRun, fSubrun;
  art::EventNumber_t fEvent;

  // PD information
  //  opdet::sbndPDMapAlg pdMap; // photon detector map
  std::vector<unsigned int> channelList;

  // beam window
  // set in artdaqFragment producer, in reality would be provided by event builder
  uint64_t beamtimestamp;
  bool foundBeamTrigger;
  uint32_t beamWindowStart;
  uint32_t beamWindowEnd;
  double TicksToUs;
  double usToTicks;


  // waveforms
  uint32_t fTriggerTime;
  bool fWvfmsFound;
  std::vector<std::vector<uint16_t>> fWvfmsVec;

  // pmt information
  std::vector<sbnd::trigger::pmtInfo> fpmtInfoVec;
  std::map<int,int> map_fragid_index;

  void checkCAEN1730FragmentTimeStamp(const artdaq::Fragment &frag);
  void analyzeCAEN1730Fragment(const artdaq::Fragment &frag);
  void estimateBaseline(int i_ch);
  void SimpleThreshAlgo(int i_ch);
  void reconfigure(fhicl::ParameterSet const & p);
  void find_beam_spill(const artdaq::Fragment &frag);



};


sbnd::trigger::pmtSoftwareTriggerProducer::pmtSoftwareTriggerProducer(fhicl::ParameterSet const& p)
  : EDProducer{p}
  {
    this->reconfigure(p);
    // Call appropriate produces<>() functions here.
    produces< sbnd::trigger::pmtSoftwareTrigger >("", is_persistable_);


    // build PD map and channel list
    // auto subsetCondition = [](auto const& i)->bool { return i["pd_type"] == "pmt_coated" || i["pd_type"] == "pmt_uncoated"; };
    // auto pmtMap = pdMap.getCollectionFromCondition(subsetCondition);
    // for(auto const& i:pmtMap){
    //   channelList.push_back(i["channel"]);
    // }

    // map from fragID to array index 0-7
    for (size_t i=0;i<fFragidlist.size();++i){
      map_fragid_index.insert(std::make_pair(fFragidlist[i],i));
    }

 TicksToUs = 500.;
 usToTicks = 1.0/500;

}

void sbnd::trigger::pmtSoftwareTriggerProducer::reconfigure(fhicl::ParameterSet const & p)
{
  // Initialize member data here
  is_persistable_ = p.get<bool>("is_persistable", true) ? art::Persistable::Yes : art::Persistable::No;
  fWindowOffset=p.get<double>("WindowOffset",0.1);  //units of us
//max time difference for frag timestamp to be part of beam window
  fTrigWindowFrag=p.get<uint32_t>("TrigWindowFrag",100);  //units of ticks, 1 tick=2ns
  fWindowLength=p.get<double>("WindowLength", 1.8);
  fWvfmLength=p.get<uint32_t>("WvfmLength", 5120);  //not used
  fETrigFragid=p.get<uint32_t>("ETrigFragID",0);
  fETrigThresh=p.get<uint32_t>("ETrigThresh",5000);
  fFragidlist=p.get<std::vector<int> >("FragIDlist");
  fUncoated=p.get<std::vector<int> >("Uncoated");  // =0 for Coated, =1 for Uncoated
  fTPC=p.get<std::vector<int> >("TPC");  // TPC=0(1) is neg(pos) x and beam right(left)
  fVerbose=p.get<bool>("Verbose", false);
  //
  fSaveHists=p.get<bool>("SaveHists",false);
  fBaselineAlgo=p.get<std::string>("BaselineAlgo", "est");
  fInputBaseline=p.get<double>("InputBaseline", 8000);
  fInputBaselineSigma=p.get<double>("InputBaselineSigma", 2);
  fADCThreshold=p.get<double>("ADCThreshold", 7960);
  fFindPulses=p.get<bool>("FindPulses", false);
  fPEArea=p.get<double>("PEArea", 66.33);

}


void sbnd::trigger::pmtSoftwareTriggerProducer::produce(art::Event& e)
{
  // Implementation of required member function here.

  // event information
  fRun = e.run();
  fSubrun = e.subRun();
  fEvent = e.id().event();

  if (fVerbose) std::cout << "Processing Run: " << fRun << ", Subrun: " << fSubrun << ", Event: " << fEvent << std::endl;

  // reset for this event
  foundBeamTrigger = false;
  fTriggerTime =0;
  fWvfmsFound = false;
  fWvfmsVec.clear(); fWvfmsVec.resize(16*8); // 16 1730 channels per fragment, 8 fragments per trigger
  fpmtInfoVec.clear(); fpmtInfoVec.resize(15*8); // 15 PMS per fragment, 8 fragments per trigger


  std::vector<art::Handle<artdaq::Fragments>> fragmentHandles = e.getMany<std::vector<artdaq::Fragment>>();

  // Loop over fragments to find the beam spill time

  beamtimestamp = -1;
  // loop over fragment handles
  for (auto &handle : fragmentHandles) {
    if (!handle.isValid() || handle->size() == 0) continue;
    if (handle->front().type() == artdaq::Fragment::ContainerFragmentType) {
      //Container fragment
      for (auto cont : *handle) {
	artdaq::ContainerFragment contf(cont);
        if (contf.fragment_type()==sbndaq::detail::FragmentType::CAENV1730) {
	  if (fVerbose) std::cout << "    Found " << contf.block_count() << " CAEN Fragments in container " << std::endl;
          for (size_t ii = 0; ii < contf.block_count(); ++ii)
	    find_beam_spill(*contf[ii].get());
        } // if
      } //for loop over fragments in container
    } // if 1730 container
    else if (handle->front().type()==sbndaq::detail::FragmentType::CAENV1730) {
      if (fVerbose) std::cout << "Found " << handle->size() << " CAEN1730 fragments" << std::endl;
      for (auto frag : *handle) find_beam_spill(frag);
    } // if/else container/fragmnet
  } // loop over handles
  if (fVerbose) std::cout << "beamtimestamp is " << beamtimestamp << std::endl;

  // object to store trigger metrics in
  std::unique_ptr<sbnd::trigger::pmtSoftwareTrigger> pmtSoftwareTriggerMetrics = std::make_unique<sbnd::trigger::pmtSoftwareTrigger>();

  if (foundBeamTrigger) {
    // Loop over fragments.  For each beam spill fragment, fill metric info
    for (auto &handle : fragmentHandles) {
      if (!handle.isValid() || handle->size() == 0) continue;
      if (handle->front().type() == artdaq::Fragment::ContainerFragmentType) {
	//Container fragment
	for (auto cont : *handle) {
	  artdaq::ContainerFragment contf(cont);
	  if (contf.fragment_type()==sbndaq::detail::FragmentType::CAENV1730) {
	    for (size_t ii = 0; ii < contf.block_count(); ++ii) {
	      // access fragment timestamp
	      const artdaq::Fragment &frag = *contf[ii].get();
	      analyzeCAEN1730Fragment(frag);
	    } //for loop over
	  } // if
	} //for loop over fragments in container
      } // if container
      else if (handle->front().type()==sbndaq::detail::FragmentType::CAENV1730) {
	for (auto frag : *handle) analyzeCAEN1730Fragment(frag);
      } // if/else container/fragmnet
    } // end loop over handles


    // calculate metrics
    pmtSoftwareTriggerMetrics->foundBeamTrigger = true;
    // store timestamp of trigger, relative to beam window start
    double triggerTimeStamp = (double)fTriggerTime - fWindowOffset*usToTicks;
    pmtSoftwareTriggerMetrics->triggerTimestamp = triggerTimeStamp;
    if (fVerbose) std::cout << "Saving event trigger timestamp: " << triggerTimeStamp << " ns" << std::endl;

    double promptPE = 0;
    double prelimPE = 0;

    int nAboveThreshold = 0;
    // find the waveform bins that correspond to the start and end of the extended spill window (0 -> 1.8 us) within the 10 us waveform
    // if the triggerTimeStamp > 1000, the beginning of the beam spill is *not* contained within the waveform
    int windowStartBin = (triggerTimeStamp >= 1000)? 0 : int(500 - abs((triggerTimeStamp)/2));
    int windowEndBin   = (triggerTimeStamp >= 1000)? int(500 + (fWindowLength*1e3 - (triggerTimeStamp-1000))/2) : (windowStartBin + (fWindowLength*1e3)/2);
    if (fVerbose) std::cout << "windowStartBin " << windowStartBin << " windowEndBin " << windowEndBin  <<
		    " trigger time stamp " << triggerTimeStamp << " window length " << fWindowLength << std::endl;


    // wvfm loop to calculate metrics
    for (int i_ch = 0; i_ch < (int)fpmtInfoVec.size(); ++i_ch){
      auto &pmtInfo = fpmtInfoVec.at(i_ch);
      auto wvfm = fWvfmsVec[i_ch];

      if (wvfm.begin() == wvfm.end()) continue;

      // assign channel
      pmtInfo.channel = i_ch; //channelList.at(i_ch);

      // calculate baseline
      if (fBaselineAlgo == "constant"){ pmtInfo.baseline=fInputBaseline; pmtInfo.baselineSigma = fInputBaselineSigma; }
      else if (fBaselineAlgo == "estimate") estimateBaseline(i_ch);

      // count number of PMTs above threshold
      if (windowEndBin >= (int)wvfm.size()){ windowEndBin = (int)wvfm.size() - 1;}
      for (int bin = windowStartBin; bin < windowEndBin; ++bin){
	       auto adc = wvfm[bin];
	        if (adc!=0 && adc < fADCThreshold){ nAboveThreshold++; break; }
      }

      // quick estimate prompt and preliminary light, assuming sampling rate of 500 MHz (2 ns per bin)
      double baseline = pmtInfo.baseline;
      auto prompt_window = std::vector<uint16_t>(wvfm.begin()+500, wvfm.begin()+1000);
      auto prelim_window = std::vector<uint16_t>(wvfm.begin()+windowStartBin, wvfm.begin()+500);
      if (fFindPulses == false){
	       double ch_promptPE = (baseline-(*std::min_element(prompt_window.begin(), prompt_window.end())))/8;
	        double ch_prelimPE = (baseline-(*std::min_element(prelim_window.begin(), prelim_window.end())))/8;
	         promptPE += ch_promptPE;
	          prelimPE += ch_prelimPE;
      }

      // pulse finder + prompt and prelim calculation with pulses
      if (fFindPulses == true){
	       SimpleThreshAlgo(i_ch);
      	for (auto pulse : pmtInfo.pulseVec){
      	  if (pulse.t_start > 500 && pulse.t_end < 550) promptPE+=pulse.pe;
      	  if ((triggerTimeStamp) >= 1000){ if (pulse.t_end < 500) prelimPE+=pulse.pe; }
      	  else if (triggerTimeStamp < 1000){
      	    if (pulse.t_start > (500 - abs((triggerTimeStamp-1000)/2)) && pulse.t_end < 500) prelimPE+=pulse.pe;
      	  }
      	}
      }

    } // end of wvfm loop

    pmtSoftwareTriggerMetrics->nAboveThreshold = nAboveThreshold;
    pmtSoftwareTriggerMetrics->promptPE = promptPE;
    pmtSoftwareTriggerMetrics->prelimPE = prelimPE;
    if (fVerbose) std::cout << "nPMTs Above Threshold: " << nAboveThreshold << std::endl;
    if (fVerbose) std::cout << "prompt pe: " << promptPE << std::endl;
    if (fVerbose) std::cout << "prelim pe: " << prelimPE << std::endl;

    if(metricMan != nullptr) {
        //send flag metrics
        metricMan->sendMetric(
            "PMT_SWTrig_nAboveThreshold",
            nAboveThreshold,
            "Num PMTs Above Software Trigger Threshold", 5, artdaq::MetricMode::LastPoint);

        if (fVerbose) {
          TLOG(TLVL_INFO) << "nPMTs Above Threshold: " << nAboveThreshold;
        }

    }

    // start histo
    if (fSaveHists == true){
      /*int hist_id = -1;
      for (size_t i_wvfm = 0; i_wvfm < fWvfmsVec.size(); ++i_wvfm){
	       std::vector<uint16_t> wvfm = fWvfmsVec[i_wvfm];
	        hist_id++;
      	if (fEvent<4){
      	  histname.str(std::string());
      	  histname << "event_" << fEvent
      		   << "_channel_" << i_wvfm;
      	  double StartTime = ((fTriggerTime-fWindowOffset) - 500)*1e-3; // us
      	  double EndTime = StartTime + (5210*2)*1e-03; // us
      	  TH1D *wvfmHist = tfs->make< TH1D >(histname.str().c_str(), "Raw Waveform", wvfm.size(), StartTime, EndTime);
      	  wvfmHist->GetXaxis()->SetTitle("t (#mus)");
      	  for(unsigned int i = 0; i < wvfm.size(); i++) {
      	    wvfmHist->SetBinContent(i + 1, (double)wvfm[i]);
      	  }
      	}
      }*/
    } // end histo
  }  // if found beam trigger
  else{

    if (fVerbose) {
      std::cout << "Beam and wvfms not found" << std::endl;
      TLOG(TLVL_INFO) << "PMT Software Trigger Metric: Beam and wvfms not found";

    }
    pmtSoftwareTriggerMetrics->foundBeamTrigger = false;
    pmtSoftwareTriggerMetrics->triggerTimestamp = -9999;
    pmtSoftwareTriggerMetrics->nAboveThreshold = -9999;
    pmtSoftwareTriggerMetrics->promptPE = -9999;
    pmtSoftwareTriggerMetrics->prelimPE = -9999;
  }

  e.put(std::move(pmtSoftwareTriggerMetrics));

}

/*
void sbnd::trigger::pmtSoftwareTriggerProducer::OLDcheckCAEN1730FragmentTimeStamp(const artdaq::Fragment &frag) {

  // get fragment metadata
  sbndaq::CAENV1730Fragment bb(frag);
  auto const* md = bb.Metadata();

  // access timestamp
  uint32_t timestamp = md->timeStampNSec;

  // access beam signal, in ch15 of first PMT of each fragment set
  // check entry 500 (0us), at trigger time
  const uint16_t* data_begin = reinterpret_cast<const uint16_t*>(frag.dataBeginBytes()
								 + sizeof(sbndaq::CAENV1730EventHeader));
  const uint16_t* value_ptr =  data_begin;
  uint16_t value = 0;

  size_t ch_offset = (size_t)(15*fWvfmLength);
  size_t tr_offset = fTriggerTimeOffset*1e3;

  value_ptr = data_begin + ch_offset + tr_offset; // pointer arithmetic
  value = *(value_ptr);

  if (value == 1 && timestamp >= beamWindowStart && timestamp <= beamWindowEnd) {
    foundBeamTrigger = true;
    fTriggerTime = timestamp;
  }
}
*/

void sbnd::trigger::pmtSoftwareTriggerProducer::find_beam_spill(const artdaq::Fragment &frag) {

  // check fragId
  uint fragId = static_cast<int>(frag.fragmentID());
  if (fVerbose) std::cout << "Looking for beam spill, fragID is " << fragId <<
		  " and timestamp is " << frag.timestamp() << std::endl;
  if (fragId!=fETrigFragid)  return;

  //--get number of channels from metadata and waveform length from header
  sbndaq::CAENV1730Fragment bb(frag);
  auto const* md = bb.Metadata();
  sbndaq::CAENV1730Event const* event_ptr = bb.Event();
  sbndaq::CAENV1730EventHeader header = event_ptr->Header;
  // - not used-  seqID = static_cast<int>(frag.sequenceID());
  size_t nChannels = md->nChannels();
  //  if (fVerbose) std::cout << "\tNumber of channels: " << nChannels << "\n";
  //  if (fVerbose) std::cout << "Timestamp in metadata is " << md->timeStampNSec<<std::endl;

  uint32_t ev_size_quad_bytes = header.eventSize;
  if (fVerbose) std::cout << "Event size in quad bytes is: " << ev_size_quad_bytes << "\n";
  uint32_t evt_header_size_quad_bytes = sizeof(sbndaq::CAENV1730EventHeader)/sizeof(uint32_t);
  uint32_t data_size_double_bytes = 2*(ev_size_quad_bytes - evt_header_size_quad_bytes);
  //-  uint32_t wfm_length = data_size_double_bytes/nChannels;
  fWvfmLength = data_size_double_bytes/nChannels;
  if (fVerbose) std::cout << "Channel waveform length = " << fWvfmLength << "\n";
  size_t ch_offset = (size_t)(15*fWvfmLength);

  uint32_t t0 = header.triggerTimeTag;
  int TTT_ns = (int)t0*8;  // 8 ns per tick in trigger time tag
  if (fVerbose)       std::cout << "\n\tTriggerTimeTag in ns is " << TTT_ns << "\n";
  if (fVerbose) std::cout << " frag timesstamp is " << frag.timestamp()  << std::endl;

  // access beam signal, in ch15 of first PMT of each fragment set
  const uint16_t* data_begin = reinterpret_cast<const uint16_t*>(frag.dataBeginBytes()
					   + sizeof(sbndaq::CAENV1730EventHeader));
  const uint16_t* value_ptr =  data_begin;
  uint16_t value = 0;

  // not currently used because there's no guarantee where the trigger is in the waveform
  //  size_t tr_offset = fTriggerTimeOffset*1e3;
  size_t tr_offset = 0;
  value_ptr = data_begin + ch_offset + tr_offset; // pointer arithmetic
  //  value = *(value_ptr);

  //uint64_t timestamp = 0;

  for (uint ind=0;ind<fWvfmLength;++ind){
    value = *(value_ptr+ind);
    if (value>fETrigThresh) {
      foundBeamTrigger = true;
      fTriggerTime = ind;
      beamtimestamp = frag.timestamp();
      std::cout << "Found beam trigger : ind= " << ind << " adc is " << value;
      std::cout << " timestamp is " << beamtimestamp << std::endl;
      break;
    }
  }

  std::cout << "leaving find_beam_spill, timestamp is " << beamtimestamp << std::endl;

}

void sbnd::trigger::pmtSoftwareTriggerProducer::analyzeCAEN1730Fragment(const artdaq::Fragment &frag)
{

  //
  //  DANGER!  This assumes that all beam spill fragments have the same waveform length.
  //     Should be true, however . . .
  ///

  // First check if this is a beam spill fragment
  auto thists = frag.timestamp();
  uint32_t tsdiff = (uint32_t)(fabs((double)thists-(double)beamtimestamp));
  std::cout << "beam time stamp " << beamtimestamp << " this frag ts " << thists <<
    " diff " << tsdiff << std::endl;
  if (tsdiff>fTrigWindowFrag) return;

  //--get number of channels from metadata and waveform length from header
  sbndaq::CAENV1730Fragment bb(frag);
  auto const* md = bb.Metadata();
  sbndaq::CAENV1730Event const* event_ptr = bb.Event();
  sbndaq::CAENV1730EventHeader header = event_ptr->Header;
  // - not used-  seqID = static_cast<int>(frag.sequenceID());
  int fragId = static_cast<int>(frag.fragmentID());
  std::map<int,int>::iterator it = map_fragid_index.find(fragId);
  int findex = it->second;

  size_t nChannels = md->nChannels();
  if (fVerbose) std::cout << "\tNumber of channels: " << nChannels << "\n";

  uint32_t ev_size_quad_bytes = header.eventSize;
  if (fVerbose) std::cout << "Event size in quad bytes is: " << ev_size_quad_bytes << "\n";
  uint32_t evt_header_size_quad_bytes = sizeof(sbndaq::CAENV1730EventHeader)/sizeof(uint32_t);
  uint32_t data_size_double_bytes = 2*(ev_size_quad_bytes - evt_header_size_quad_bytes);
  //-  uint32_t wfm_length = data_size_double_bytes/nChannels;
  //- if (fverbose) std::cout << "Channel waveform length = " << wfm_length << "\n";
  fWvfmLength = data_size_double_bytes/nChannels;
  if (fVerbose) std::cout << "Channel waveform length = " << fWvfmLength << "\n";

  //--access waveforms in fragment and save
  const uint16_t* data_begin = reinterpret_cast<const uint16_t*>(frag.dataBeginBytes()
					+ sizeof(sbndaq::CAENV1730EventHeader));
  const uint16_t* value_ptr =  data_begin;
  uint16_t value = 0;
  size_t ch_offset = 0;

  // loop over channels
  nChannels--; // last waveform isn't a PMT
  for (size_t i_ch = 0; i_ch < nChannels; ++i_ch){
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

/*
void sbnd::trigger::pmtSoftwareTriggerProducer::OLDanalyzeCAEN1730Fragment(const artdaq::Fragment &frag) {

  // access fragment ID; index of fragment out of set of 8 fragments
  int fragId = static_cast<int>(frag.fragmentID());

  // access waveforms in fragment and save
  const uint16_t* data_begin = reinterpret_cast<const uint16_t*>(frag.dataBeginBytes()
								 + sizeof(sbndaq::CAENV1730EventHeader));
  const uint16_t* value_ptr =  data_begin;
  uint16_t value = 0;

  // channel offset
  size_t nChannels = 15; // 15 pmts per fragment
  size_t ch_offset = 0;

  // loop over channels
  for (size_t i_ch = 0; i_ch < nChannels; ++i_ch){
    fWvfmsVec[i_ch + nChannels*fragId].resize(fWvfmLength);
    ch_offset = (size_t)(i_ch * fWvfmLength);
    //--loop over waveform samples
    for(size_t i_t = 0; i_t < fWvfmLength; ++i_t){
      value_ptr = data_begin + ch_offset + i_t; // pointer arithmetic
      value = *(value_ptr);
      fWvfmsVec[i_ch + nChannels*fragId][i_t] = value;
    } //--end loop samples
  } //--end loop channels
}
*/

void sbnd::trigger::pmtSoftwareTriggerProducer::estimateBaseline(int i_ch){
  auto wvfm = fWvfmsVec[i_ch];
  auto &pmtInfo = fpmtInfoVec[i_ch];
  // assuming that the first 500 ns doesn't include peaks, find the mean of the ADC count as the baseline
  std::vector<uint16_t> subset = std::vector<uint16_t>(wvfm.begin(), wvfm.begin()+250);
  double subset_mean = (std::accumulate(subset.begin(), subset.end(), 0))/(subset.size());
  double val = 0;
  for (size_t i=0; i<subset.size();i++){ val += (subset[i] - subset_mean)*(subset[i] - subset_mean);}
  double subset_stddev = sqrt(val/subset.size());

  // if the first 500 ns seem to be messy, use the last 500
  if (subset_stddev > 3){ // make this fcl parameter?
    val = 0; subset.clear(); subset_stddev = 0;
    subset = std::vector<uint16_t>(wvfm.end()-500, wvfm.end());
    subset_mean = (std::accumulate(subset.begin(), subset.end(), 0))/(subset.size());
    for (size_t i=0; i<subset.size();i++){ val += (subset[i] - subset_mean)*(subset[i] - subset_mean);}
    subset_stddev = sqrt(val/subset.size());
  }
  pmtInfo.baseline = subset_mean;
  pmtInfo.baselineSigma = subset_stddev;
}

void sbnd::trigger::pmtSoftwareTriggerProducer::SimpleThreshAlgo(int i_ch){
  auto wvfm = fWvfmsVec[i_ch];
  auto &pmtInfo = fpmtInfoVec[i_ch];
  double baseline = pmtInfo.baseline;
//   double baseline_sigma = pmtInfo.baselineSigma;

  bool fire = false; // bool for if pulse has been detected
  int counter = 0; // counts the bin of the waveform

  // these should be fcl parameters 
  double start_adc_thres = 5, end_adc_thres = 2; 
  // double nsigma_start = 5, nsigma_end = 3; 
  
  // auto start_threshold = ( start_adc_thres > (nsigma_start * baseline_sigma) ? (baseline-start_adc_thres) : (baseline-(nsigma_start * baseline_sigma)));
  // auto end_threshold = ( end_adc_thres > (nsigma_end * baseline_sigma) ? (baseline - end_adc_thres) : (baseline - (nsigma_end * baseline_sigma)));
  auto start_threshold = baseline-start_adc_thres;
  auto end_threshold   = baseline-end_adc_thres; 

  std::vector<sbnd::trigger::pmtPulse> pulse_vec;
  sbnd::trigger::pmtPulse pulse; 
  pulse.area = 0; pulse.peak = 0; pulse.t_start = 0; pulse.t_end = 0; pulse.t_peak = 0;
  for (auto const &adc : wvfm){
    if ( !fire && ((double)adc) <= start_threshold ){ // if its a new pulse 
      fire = true;
      //vic: i move t_start back one, this helps with porch
      pulse.t_start = counter - 1 > 0 ? counter - 1 : counter;    
    }

    else if( fire && ((double)adc) > end_threshold ){ // found end of a pulse
      fire = false;
      //vic: i move t_start forward one, this helps with tail
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

// void sbnd::trigger:pmtSoftwareTriggerProducer::SlidingThreshAlgo(){
// }

DEFINE_ART_MODULE(sbnd::trigger::pmtSoftwareTriggerProducer)
