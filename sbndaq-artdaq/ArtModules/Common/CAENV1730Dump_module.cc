//-------------------------------------------------
//---------------------------------------

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

//#include "canvas/Persistency/Provenance/FileMetaData.h"
#include "art/Framework/Services/System/FileCatalogMetadata.h"
//#include "art/Framework/Services/Optional/FileCatalogMetadataPlugin.h"

#include "canvas/Utilities/Exception.h"

#include "sbndaq-artdaq-core/Overlays/Common/CAENV1730Fragment.hh"
#include "artdaq-core/Data/Fragment.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "artdaq-core/Data/ContainerFragment.hh"

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
#include <bitset>
#include <numeric>

namespace sbndaq {
  class CAENV1730Dump;
}

/**************************************************************************************************/

class sbndaq::CAENV1730Dump : public art::EDAnalyzer {

public:
  struct Config {
    //--one atom for each parameter
    fhicl::Atom<art::InputTag> DataLabel {
      fhicl::Name("data_label"),
      fhicl::Comment("Tag for the input data product")
    };
    fhicl::Atom<bool> Verbose {
      fhicl::Name("verbose"), 
      fhicl::Comment("toggle for additional text output")
    };
    fhicl::Atom<bool> SaveWaveforms {
      fhicl::Name("SaveWaveforms"),
      fhicl::Comment("save aveforms in TTree")
    };
  }; //--configuration
  using Parameters = art::EDAnalyzer::Table<Config>;

  explicit CAENV1730Dump(Parameters const & pset);
  virtual ~CAENV1730Dump();

  void analyze(const art::Event& evt) override;
  void beginJob() override;
  void endJob() override;
  void analyze_caen_fragment(artdaq::Fragment & frag, art::EventNumber_t eventNumber);


private:

  //--default values
  uint32_t nChannels;//    = 16;
  uint32_t Ttt_DownSamp;// =  4; 
 /* the trigger time resolution is 16ns when waveforms are sampled at
                               * 500MHz sampling. The trigger timestamp is
                               * sampled 4 times slower than input channels*/

  TNtuple* nt_header;
  
  TH1F*    hEventCounter;
  TH1F*    hTriggerTimeTag;
  TH1F*    h_wvfm_ev0_ch0;

  TTree* fEventTree;
  int fRun;
  art::EventNumber_t fEvent;
  int fragID;
  int seqID;
  std::vector<uint64_t>  fTicksVec;
  std::vector< std::vector<uint16_t> >  fWvfmsVec;

  TTree* nt_wvfm;
  int fArt_ev, fCaen_ev, fBoardId, fFragmentId, fCh;
  double fCaen_ev_tts, fPed, fRMS, fTemp, fStampTime;
  
  bool firstEvt = true;

  // fcl params
  art::InputTag fDataLabel;
  bool fverbose;
  bool fSaveWaveforms;

}; //--class CAENV1730Dump


sbndaq::CAENV1730Dump::CAENV1730Dump(CAENV1730Dump::Parameters const& pset): art::EDAnalyzer(pset)
{
  fDataLabel = pset().DataLabel();
  fverbose = pset().Verbose();
  fSaveWaveforms = pset().SaveWaveforms();
}


void sbndaq::CAENV1730Dump::beginJob()
{
  art::ServiceHandle<art::TFileService> tfs; 
  nt_header       = tfs->make<TNtuple>("nt_header","CAENV1730 Header Ntuple","art_ev:caen_ev:caenv_ev_tts"); 
  /************************************************************************************************/
  hEventCounter   = tfs->make<TH1F>("hEventCounter","Event Counter Histogram",10000,0,10000);
  hTriggerTimeTag = tfs->make<TH1F>("hTriggerTimeTag","Trigger Time Tag Histogram",10,2000000000,4500000000);
  h_wvfm_ev0_ch0  = tfs->make<TH1F>("h_wvfm_ev0_ch0","Waveform",2000,0,2000);  
  /************************************************************************************************/
  if(fSaveWaveforms){
    //--make tree to store the channel waveform info:
    fEventTree = tfs->make<TTree>("events","waveform tree");
    fEventTree->Branch("fRun",&fRun,"fRun/I");
    fEventTree->Branch("fEvent",&fEvent,"fEvent/I");
    fEventTree->Branch("fragID",&fragID,"fragID/I");
    fEventTree->Branch("seqID",&seqID,"seqID/I");
    fEventTree->Branch("fTicksVec",&fTicksVec);
    fEventTree->Branch("fWvfmsVec",&fWvfmsVec);
  }

  //--ana tree
  nt_wvfm = tfs->make<TTree>("nt_wvfm","Waveform information TTree");
  nt_wvfm->Branch("art_ev",&fArt_ev,"art_ev/I");
  nt_wvfm->Branch("caen_ev",&fCaen_ev,"caen_ev/I");
  nt_wvfm->Branch("caen_ev_tts",&fCaen_ev_tts,"caen_ev_tts/D");
  nt_wvfm->Branch("boardId",&fBoardId,"boardId/I");
  nt_wvfm->Branch("fragmentId",&fFragmentId,"fragmentId/I");
  nt_wvfm->Branch("ch",&fCh,"ch/I");
  nt_wvfm->Branch("ped",&fPed,"ped/D");
  nt_wvfm->Branch("rms",&fRMS,"rms/D");
  nt_wvfm->Branch("temp",&fTemp,"temp/D");
  nt_wvfm->Branch("stamp_time",&fStampTime,"stamp_time/D");

}

void sbndaq::CAENV1730Dump::endJob()
{
  std::cout << "Ending CAENV1730Dump...\n";
}


sbndaq::CAENV1730Dump::~CAENV1730Dump()
{
}


void sbndaq::CAENV1730Dump::analyze(const art::Event& evt)
{


  fRun = evt.run();
  fEvent = evt.event();

  // Get file metadata
  /************************************************************************************************/
  //  art::ServiceHandle<art::FileCatalogMetadata> metadata;
  //std::string inputFile = metadata->FileCatalogMetadata().get<std::string>("input_file_name");
  //std::cout << "Input file name: " << inputFile << std::endl;
  std::cout<<" READING METADATA\n";
  art::FileCatalogMetadata::collection_type artmd;
  art::ServiceHandle<art::FileCatalogMetadata> metadata;
  metadata->getMetadata(artmd);
  //const auto& fileMeta = metadata->getMetadata(artmd);
  //auto runIter = fileMeta.find("run_number");
  //std::cout<<runIter<<std::endl;
  for(const auto& d: artmd)
    std::cout<<d.first<<" "<<d.second<<std::endl;
  /************************************************************************************************/
    
  /************************************************************************************************/


  std::vector<art::Handle<artdaq::Fragments>> fragmentHandles;

  #if ART_HEX_VERSION < 0x30900
          evt.getManyByType(fragmentHandles);
  #else
          fragmentHandles = evt.getMany<std::vector<artdaq::Fragment>>();
  #endif

  /************************************************************************************************/
  for (auto handle : fragmentHandles) {
    if (!handle.isValid() || handle->size() == 0) continue;

    if (handle->front().type() == artdaq::Fragment::ContainerFragmentType) {
      //Container fragment
      for (auto cont : *handle) {
	artdaq::ContainerFragment contf(cont);
	if (contf.fragment_type()==sbndaq::detail::FragmentType::CAENV1730) {
	  if (fverbose) 	  std::cout << "    Found " << contf.block_count() << " CAEN Fragments in container " << std::endl;
	  fWvfmsVec.resize(16*contf.block_count());
	  for (size_t ii = 0; ii < contf.block_count(); ++ii)
	    analyze_caen_fragment(*contf[ii].get(), evt.event());
	}
      }
    }
    else {
      //normal fragment
      if (handle->front().type()==sbndaq::detail::FragmentType::CAENV1730) {
	if (fverbose) std::cout << "   found normal caen fragments " << handle->size() << std::endl;
	fWvfmsVec.resize(16*handle->size());
	for (auto frag : *handle)
	  analyze_caen_fragment(frag, evt.event());
      }
    }
  } // loop over frag handles



}

void sbndaq::CAENV1730Dump::analyze_caen_fragment(artdaq::Fragment & frag, art::EventNumber_t eventNumber)  {


    CAENV1730Fragment bb(frag);
    auto const* md = bb.Metadata();
    CAENV1730Event const* event_ptr = bb.Event();
    
    CAENV1730EventHeader header = event_ptr->Header;
    
    fragID = static_cast<int>(frag.fragmentID()); 

    size_t const fragment_id = frag.fragmentID();
    size_t const eff_fragment_id = fragment_id & 0x0fff;

    seqID = static_cast<int>(frag.sequenceID()); 
    
    if (fverbose) std::cout << "\tFrom header, event counter is "  << header.eventCounter   << "\n";
    if (fverbose) std::cout << "\tFrom header, triggerTimeTag is " << header.triggerTimeTag << "\n";
    if (fverbose) std::cout << "\tFrom header, board id is "       << header.boardID       << "\n";
    if (fverbose) std::cout << "\tFrom fragment, fragment id is "  << fragID << "\n";
    if (fverbose) std::cout << "\tFrom fragment, sequence id is "  << seqID << "\n";
    if (fverbose) std::cout <<  "\tFrom fragment, timestamp is  " << frag.timestamp() << std::endl;
      
    uint32_t t0 = header.triggerTimeTag;
    hEventCounter->Fill(header.eventCounter);
    hTriggerTimeTag->Fill(t0);
    nt_header->Fill(fEvent,header.eventCounter,t0);
    nChannels = md->nChannels;
    std::cout << "\tNumber of channels: " << nChannels << "\n";
    
    //--get the number of 32-bit words (quad_bytes) from the header
    uint32_t ev_size_quad_bytes = header.eventSize;
    if (fverbose) std::cout << "Event size in quad bytes is: " << ev_size_quad_bytes << "\n";
    uint32_t evt_header_size_quad_bytes = sizeof(CAENV1730EventHeader)/sizeof(uint32_t);
    uint32_t data_size_double_bytes = 2*(ev_size_quad_bytes - evt_header_size_quad_bytes);
    uint32_t wfm_length = data_size_double_bytes/nChannels;
    if (fverbose) std::cout << "Channel waveform length = " << wfm_length << "\n";
    
    //--store the tick value for each acquisition 
    fTicksVec.resize(wfm_length);
    const uint16_t* data_begin = reinterpret_cast<const uint16_t*>(frag.dataBeginBytes() 
								   + sizeof(CAENV1730EventHeader));
    const uint16_t* value_ptr =  data_begin;
    uint16_t value = 0;
    size_t ch_offset = 0;
    std::cout<<"DataBegin "<<data_begin<<std::endl;
    //--loop over channels
    for (size_t i_ch=0; i_ch<nChannels; ++i_ch){
      fWvfmsVec[i_ch].resize(wfm_length);
      ch_offset = (size_t)(i_ch * wfm_length);
      //      std::cout << "ch" << i_ch << " offset =" << ch_offset << std::endl;
      std::cout<<"CAENDUMP: "<<i_ch<<" off="<<ch_offset<<std::endl;
      //--loop over waveform samples
      for(size_t i_t=0; i_t<wfm_length; ++i_t){ 
	fTicksVec[i_t] = t0*Ttt_DownSamp + i_t;   /*timestamps, event level*/
	value_ptr = data_begin + ch_offset + i_t; /*pointer arithmetic*/
	value = *(value_ptr);
	
	if (i_ch == 0 && firstEvt) { 
	  h_wvfm_ev0_ch0->SetBinContent(i_t,value);
	}
	
	fWvfmsVec[i_ch][i_t] = value;
	
      } //--end loop samples
      firstEvt = false;

      fArt_ev = eventNumber;
      fCaen_ev = header.eventCounter;
      fCaen_ev_tts = header.triggerTimeTag;
      fBoardId = header.boardID;
      fFragmentId = fragID;
      //fCh = i_ch;
      fCh = i_ch + nChannels*eff_fragment_id;
      //get mean
      fPed = std::accumulate(fWvfmsVec[i_ch].begin(),fWvfmsVec[i_ch].end(),0.0) / fWvfmsVec[i_ch].size();
      
      //get rms
      fRMS = 0.0;
      for(auto const& val : fWvfmsVec[i_ch])
	fRMS += (val-fPed)*(val-fPed);
      fRMS = std::sqrt(fRMS/fWvfmsVec[i_ch].size());
      
      fTemp = md->chTemps[i_ch];
      fStampTime = md->timeStampSec;
      
      std::cout<<"Temp Monitor..ch "<<i_ch<<":"<<fTemp<<" TS:"<<fStampTime<<std::endl;
      
      nt_wvfm->Fill();
    } //--end loop channels
    
    if(fSaveWaveforms) fEventTree->Fill();
}

DEFINE_ART_MODULE(sbndaq::CAENV1730Dump)

