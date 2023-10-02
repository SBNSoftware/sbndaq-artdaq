//-------------------------------------------------
//---------------------------------------

////////////////////////////////////////////////////////////////////////
// Class:       CRTSinglePEAna
// Module Type: analyzer
// File:        CRTSinglePEAna_module.cc
// Description: Black box for single PE gain measurement
// Author:      Jiaoyang Li/李 娇瑒 (jiaoyang.li@ed.ac.uk)
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art_root_io/TFileService.h"
#include "art_root_io/TFileDirectory.h"

#include "canvas/Utilities/Exception.h"

#include "sbndaq-artdaq-core/Overlays/Common/CAENV1730Fragment.hh"
#include "sbndaq-artdaq-core/Overlays/Common/WhiteRabbitFragment.hh"
#include "sbndaq-artdaq-core/Overlays/Common/BernCRTFragmentV2.hh"
#include "sbndaq-artdaq-core/Overlays/SBND/TDCTimestampFragment.hh"
#include "sbndaq-artdaq-core/Overlays/SBND/PTBFragment.hh"

#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "artdaq-core/Data/Fragment.hh"
#include "artdaq-core/Data/ContainerFragment.hh"

#include "sbndaq-artdaq-core/Obj/SBND/pmtSoftwareTrigger.hh"
#include "sbndaq-artdaq-core/Obj/SBND/CRTmetric.hh"

#include "TH1F.h"
#include "TF1.h"
#include "TNtuple.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TStyle.h"
#include "TImage.h"
#include "TSpectrum.h"
#include "TSystem.h"
#include "TROOT.h"
#include "TPaveText.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <vector>
#include <iostream>
#include <bitset>

namespace sbndaq {
  class CRTSinglePEAna;
}

/**************************************************************************************************/

class sbndaq::CRTSinglePEAna : public art::EDAnalyzer {

public:
  struct Config {
    //--one atom for each parameter
    fhicl::Atom<bool> include_berncrt {
      fhicl::Name("include_berncrt"),
      fhicl::Comment("look for bern CRT V2 fragments true/false"),
      true
    };
    fhicl::Atom<bool> crt_keepall {
      fhicl::Name("crt_keepall"),
      fhicl::Comment("put all crt fluff into tree true/false"),
      false
    };
    fhicl::Atom<bool> verbose {
      fhicl::Name("verbose"),
      fhicl::Comment("lots of text output if set to true"),
      false
    };
    fhicl::Atom<bool> store_basic_info {
      fhicl::Name("store_basic_info"),
      fhicl::Comment("if true make basic CRT hit tree"),
      false
    }; 
    fhicl::Atom<bool> include_timing {
      fhicl::Name("include_timing"),
      fhicl::Comment("if true including the timing info in the tree"),
      false
    }; 
    fhicl::Atom<bool> store_adc_raw_histo {
      fhicl::Name("store_adc_raw_histo"),
      fhicl::Comment("if true plot adc raw histos"),
      false
    }; 
    fhicl::Atom<bool> save_histo_to_pdf {
      fhicl::Name("save_histo_to_pdf"),
      fhicl::Comment("if true save histos with the pdf verison"),
      false
    }; 
    fhicl::Atom<bool> fit_the_adc {
      fhicl::Name("fit_the_adc"),
      fhicl::Comment("if true fit the adc histos"),
      false
    };  
    fhicl::Atom<int> bias_voltage { 
      fhicl::Name("bias_voltage"),
      114
    };
    fhicl::Atom<std::string> photo_dump_dir { 
      fhicl::Name("photo_dump_dir")
    };
    fhicl::Atom<bool> input_FEB_id_manually {
      fhicl::Name("input_FEB_id_manually"),
      fhicl::Comment("if true users needs to manually input the FEB number"),
      false
    };
    fhicl::Sequence<int> FEBModules_vec_manual_input{ 
      fhicl::Name("FEBModules_vec_manual_input"),
      {154}
    };
    fhicl::Atom<bool> input_fit_range_manually {
      fhicl::Name("input_fit_range_manually"),
      fhicl::Comment("if true users needs to manually input the fit range for fingure plot"),
      false
    };
    fhicl::Sequence<int> fit_range_vec{ // (min, max, bins)
      fhicl::Name("fit_range_vec"),
      {200, 800, 150}
    };
  }; //--configuration
  using Parameters = art::EDAnalyzer::Table<Config>;

  explicit CRTSinglePEAna(Parameters const & pset);
  virtual ~CRTSinglePEAna();

  void analyze(const art::Event& evt) override;
  void beginJob() override;
  void endJob() override;

private:
  void analyze_bern_fragment(artdaq::Fragment & frag);
  void MakeSinglePETree(); // single PE search 
  void PeakFindFit(TH1F* h, bool save, art::TFileDirectory tfdir, 
                   float &gain, float &gain_err, float &pedestal, float &pedestal_err, float &chisqr, float &ndf);

  //--default values
  static constexpr int nChannels=32; 
  int fFEB_total_number;
  std::vector<int> fFEBModules_vec;
  std::vector<int> fFEBModules_vec_manual_input;
  std::vector<int> fFitRange_manual_input;
  int fbias_voltage;

  // declare the histogram per FEB per channel. 
  std::vector<std::vector<TH1F *>> adc_1d_ditributions_perChannel_perFEB;
   
  // TTree* events;
  TTree* CRTHitBasicInfoTree;
  TTree* CRTFitInfoTree;
  int fRun;
  art::EventNumber_t fEvent;
  
  //BernCRTV2 Information
  std::vector<int> flags;
  std::vector<int> lostcpu;
  std::vector<int> lostfpga;
  std::vector<uint> ts0;
  std::vector<uint> ts1;
  std::vector<int> adc0; // when hit happens, store the adc value for channel 0. 
  std::vector<int> adc1; // same as adc0...
  std::vector<int> adc2;
  std::vector<int> adc3;
  std::vector<int> adc4;
  std::vector<int> adc5;
  std::vector<int> adc6;
  std::vector<int> adc7;
  std::vector<int> adc8;
  std::vector<int> adc9;
  std::vector<int> adc10;
  std::vector<int> adc11;
  std::vector<int> adc12;
  std::vector<int> adc13;
  std::vector<int> adc14;
  std::vector<int> adc15;
  std::vector<int> adc16;
  std::vector<int> adc17;
  std::vector<int> adc18;
  std::vector<int> adc19;
  std::vector<int> adc20;
  std::vector<int> adc21;
  std::vector<int> adc22;
  std::vector<int> adc23;
  std::vector<int> adc24;
  std::vector<int> adc25;
  std::vector<int> adc26;
  std::vector<int> adc27;
  std::vector<int> adc28;
  std::vector<int> adc29;
  std::vector<int> adc30;
  std::vector<int> adc31;
  std::vector<int> coinc;

  std::vector<int>  feb_hit_number           ; //hit counter for individual FEB, including hits lost in FEB or fragment generator
  std::vector<uint64_t>  timestamp           ; //absolute timestamp
  std::vector<uint>  last_accepted_timestamp ; //timestamp of previous accepted hit
  std::vector<int>  lost_hits                ; //number of lost hits from the previous one

  // CRT metadata
  std::vector<int>   mac5; //last 8 bits of FEB mac5 address
  std::vector<uint>  run_start_time;
  std::vector<uint>  this_poll_start;
  std::vector<uint>  this_poll_end;
  std::vector<uint>  last_poll_start;
  std::vector<uint>  last_poll_end;
  std::vector<int>   system_clock_deviation;
  std::vector<int>   feb_hits_in_poll;
  std::vector<int>   feb_hits_in_fragment;  

  //information from fragment header
  std::vector<int>  sequence_id;
  
  // CRT hit info for single PE peak 
  std::vector<int> febid; 
  std::vector<int> channelA;
  std::vector<int> channelB;
  std::vector<int> adcA;
  std::vector<int> adcB;
  std::vector<uint> ts0_hit;
  std::vector<uint> ts1_hit;

  //sigle PE tree
  bool fStoreBasicInfo;
  bool fverbose;
  bool finclude_berncrt;
  bool fcrt_keepall;
  bool is_firstEvt;
  bool fSaveHistsToPDF;
  bool fHasPedCal = false;
  bool fStoreADCRawHistos;
  bool fFitADCRawHistos;
  bool fIncludeTiming;
  bool isBrokenChannel=false;
  bool isManuallyTypeInFEBID=false;
  bool isManuallyTypeInFitRange=false;

  art::ServiceHandle<art::TFileService> tfs;  

  TString fPhoto_dump_dir;
  int n_count_create_folder;
  float fgain_increment_slope=0.45;
}; //--class CRTSinglePEAna


sbndaq::CRTSinglePEAna::CRTSinglePEAna(CRTSinglePEAna::Parameters const& pset): art::EDAnalyzer(pset){
  
  fverbose = pset().verbose();
  finclude_berncrt = pset().include_berncrt();
  fcrt_keepall = pset().crt_keepall();
  fStoreBasicInfo= pset().store_basic_info(); 
  fbias_voltage = pset().bias_voltage();
  fStoreADCRawHistos = pset().store_adc_raw_histo();
  fIncludeTiming = pset().include_timing();
  fSaveHistsToPDF = pset().save_histo_to_pdf();
  fFitADCRawHistos = pset().fit_the_adc();
  fPhoto_dump_dir = pset().photo_dump_dir();
  isManuallyTypeInFEBID = pset().input_FEB_id_manually();
  fFEBModules_vec_manual_input = pset().FEBModules_vec_manual_input();
  isManuallyTypeInFitRange = pset().input_fit_range_manually();
  fFitRange_manual_input = pset().fit_range_vec();
  
}

void sbndaq::CRTSinglePEAna::beginJob(){
  is_firstEvt=true;
  n_count_create_folder = 0;
  // declare the global plotting style.
  gStyle->SetTitleFont(62, "TXYZ");
  gStyle->SetTitleSize(0.05,"TXYZ");

  // LABELS SIZE AND FONT
  gStyle->SetLabelFont(62, "TXYZ");
  gStyle->SetLabelSize(0.05,"TXYZ");

  // AXIS OFFSETS AND SIZES
  gStyle->SetTitleXOffset(0.80);
  gStyle->SetTitleXSize(0.06);

  gStyle->SetTitleYOffset(0.65);
  gStyle->SetTitleYSize(0.06);

  gStyle->SetMarkerStyle(33);
  gStyle->SetMarkerSize(1.5);
  gStyle->SetLineColor(46);
  gStyle->SetLineWidth(2);
  // Draw horizontal and vertical grids
  gStyle->SetPadGridX(kTRUE);                     
  gStyle->SetPadGridY(kTRUE);

  gROOT->SetBatch(kTRUE);
}

void sbndaq::CRTSinglePEAna::endJob()
{
  // store the histogram
  if (fverbose)  std::cout << "Ending CRTSinglePEAna...\n";


  if (fFitADCRawHistos){

    art::TFileDirectory dir = tfs->mkdir("ADC_histos_fitted");// folder to save all fit plots. 

    // Store the gain info in the TTree. 
    int FEB_id, channel;
    float gain, gain_err, pedestal, pedestal_err, chisqr, ndf;
    CRTFitInfoTree = tfs->make<TTree>("CRTFitInfoTree", "CRT fitted gain and pedestal tree");
    CRTFitInfoTree->Branch("fbias_voltage", &fbias_voltage, "fbias_voltage/I");
    CRTFitInfoTree->Branch("FEB_id", &FEB_id, "FEB_id/I");
    CRTFitInfoTree->Branch("channel", &channel, "channel/I");
    CRTFitInfoTree->Branch("gain", &gain, "gain/F");
    CRTFitInfoTree->Branch("gain_err", &gain_err, "gain_err/F");
    CRTFitInfoTree->Branch("pedestal", &pedestal, "pedestal/F");
    CRTFitInfoTree->Branch("pedestal_err", &pedestal_err, "pedestal_err/F"); 
    CRTFitInfoTree->Branch("chisqr", &chisqr, "chisqr/F"); 
    CRTFitInfoTree->Branch("ndf", &ndf, "ndf/F"); 

    for (auto iFeb=0; iFeb<(int)adc_1d_ditributions_perChannel_perFEB.size(); iFeb++){

      // the array to store the fit info. 
      std::vector<float> gain_perChannel(nChannels);
      std::vector<float> gain_err_perChannel(nChannels);
      std::vector<float> pedestal_perChannel(nChannels); 
      std::vector<float> pedestal_err_perChannel(nChannels); 
      //std::vector<float> fit_chisqr_over_ndf_perChannel(nChannels); 
      std::vector<float> channel_number;

      for (auto iChannel=0; iChannel<(int)adc_1d_ditributions_perChannel_perFEB.at(iFeb).size(); iChannel++){

        //float gain, gain_err, pedestal, pedestal_err, chisqr, ndf; 
        float chisqr, ndf;

        PeakFindFit(adc_1d_ditributions_perChannel_perFEB.at(iFeb).at(iChannel), fSaveHistsToPDF, dir, gain, gain_err, pedestal, pedestal_err, chisqr, ndf);
        
        if(fverbose) std::cout<<"FEB: "<<fFEBModules_vec.at(iFeb)<<", iChannel: "<<iChannel<<", gain "<<gain<<", pedestal "<<pedestal<<std::endl;

        gain_perChannel[iChannel] = gain;   
        gain_err_perChannel[iChannel] = gain_err;    
        pedestal_perChannel[iChannel] = pedestal;   
        pedestal_err_perChannel[iChannel] = pedestal_err;  
        //chisqr_over_ndf = chisqr/ndf; 
        //fit_chisqr_over_ndf_perChannel[iChannel] = chisqr_over_ndf;  
        channel_number.push_back(iChannel);
        channel = iChannel;
        FEB_id = fFEBModules_vec[iFeb];

        if(isBrokenChannel) {
          if(fverbose) std::cout<<"WARNING: check out FEB: "<<FEB_id<<", channel: "<<channel<<std::endl;
          // if broken channel, rewrite everything to be unphysical. 
          gain = -1;     gain_err = -1;
          pedestal = -1; pedestal_err = -1;
          chisqr = -1;   ndf = -1;
        }
        
        CRTFitInfoTree->Fill(); // store the fitting info in the TTree.
      }

      // draw the gain with error for all FEB all channel;
      TString graph_name = "gain_vs_channel"+std::to_string(fFEBModules_vec[iFeb]);
      TCanvas * c = tfs->makeAndRegister<TCanvas>(graph_name,"");

      TGraphErrors* gr_gain_vs_channel = new TGraphErrors(nChannels, &(channel_number[0]), &(gain_perChannel[0]), 0, &(gain_err_perChannel[0]));
      
      TString name = "FEB: "+std::to_string(fFEBModules_vec[iFeb]);
      gr_gain_vs_channel->SetTitle(name);
      gr_gain_vs_channel->GetXaxis()->SetTitle("Channel no.");
      gr_gain_vs_channel->GetYaxis()->SetTitle("Fitted gain value");

      //gr_gain_vs_channel->Draw();
      gr_gain_vs_channel->Draw("a2");
      gr_gain_vs_channel->Draw("p");
      gr_gain_vs_channel->GetYaxis()->SetRangeUser(-10, 150);
      // Highlighted region coordinates
      Double_t xMin = 0;
      Double_t xMax = 34;
      Double_t yMin = fbias_voltage*fgain_increment_slope*0.7;
      Double_t yMax = fbias_voltage*fgain_increment_slope*1.2;
      std::cout<<yMin<<", "<<yMax<<", "<<xMin<<", "<<xMax<<std::endl;
      // Create a TBox to represent the highlighted region
      TBox *highlightBox = new TBox(xMin, yMin, xMax, yMax);
      highlightBox->SetFillColorAlpha(40, 0.5); // Set transparent green color
      highlightBox->SetFillStyle(1001); // Set the fill style
      highlightBox->Draw("same");

      c->Update();

      if(fSaveHistsToPDF){ 
        c->SaveAs(fPhoto_dump_dir+graph_name+".pdf");
        c->SaveAs(fPhoto_dump_dir+graph_name+".png");
      }
    }
  }
  
  std::cout<<"FEB modules are : [ ";
  for (int thisFeb_no=0; thisFeb_no<fFEB_total_number; thisFeb_no++){
    if (thisFeb_no<fFEB_total_number-1) std::cout<<fFEBModules_vec[thisFeb_no]<<",";
    else std::cout<<fFEBModules_vec[thisFeb_no];
  }
  std::cout<<" ]"<<std::endl;
}


sbndaq::CRTSinglePEAna::~CRTSinglePEAna()
{
}

void sbndaq::CRTSinglePEAna::analyze(const art::Event& evt)
{
  fRun = evt.run();
  fEvent = evt.event();
  if (fverbose){
    std::cout << std::endl <<std::endl;
    std::cout << "Run " << fRun << " event " << fEvent << std::endl;
  }

  /************************************************************************************************/
  //BERN CRT 
  mac5.clear();    lostcpu.clear(); lostfpga.clear(); ts0.clear();      ts1.clear();
  adc0.clear();    adc1.clear();    adc2.clear();     adc3.clear();     adc4.clear();    adc5.clear();    adc6.clear();
  adc7.clear();    adc8.clear();    adc9.clear();     adc10.clear();    adc11.clear();   adc12.clear();   adc13.clear();
  adc14.clear();   adc15.clear();   adc16.clear();    adc17.clear();    adc18.clear();   adc19.clear();   adc20.clear();
  adc21.clear();   adc22.clear();   adc23.clear();    adc24.clear();    adc25.clear();   adc26.clear();   adc27.clear();
  adc28.clear();   adc29.clear();   adc30.clear();    adc31.clear();    coinc.clear();

  feb_hit_number.clear();            timestamp.clear();             last_accepted_timestamp.clear() ;
  lost_hits.clear();                 run_start_time.clear();        this_poll_start.clear();   
  this_poll_end.clear();             last_poll_start.clear();       last_poll_end.clear();    
  system_clock_deviation.clear();    feb_hits_in_poll.clear();      feb_hits_in_fragment.clear();
  sequence_id.clear();

  /************************************************************************************************/
  if (finclude_berncrt){

    // get the fragments. 
    std::vector<art::Handle<artdaq::Fragments>> fragmentHandles;
        fragmentHandles = evt.getMany<std::vector<artdaq::Fragment>>();

    for (auto handle : fragmentHandles) {
      if (!handle.isValid() || handle->size() == 0) continue;

      if (handle->front().type() == artdaq::Fragment::ContainerFragmentType) {
        //Container fragment
        for (auto cont : *handle) {
          artdaq::ContainerFragment contf(cont);
          if (contf.fragment_type() != sbndaq::detail::FragmentType::BERNCRTV2) continue;
          for (size_t ii = 0; ii < contf.block_count(); ++ii)
            analyze_bern_fragment(*contf[ii].get());
        }
      }
      else {
        //normal fragment
        if (handle->front().type() != sbndaq::detail::FragmentType::BERNCRTV2) continue;
        for (auto frag : *handle)
          analyze_bern_fragment(frag);
      }
    } // loop over frag handles
  } // if include_berncrt

  if(is_firstEvt){

    // Try to establish what FEBs were running in the datafile. 
    if(isManuallyTypeInFEBID){ // Manually input
      fFEBModules_vec = fFEBModules_vec_manual_input;
      fFEB_total_number = fFEBModules_vec.size(); 
    }else{ //Read from data. 
      fFEBModules_vec = mac5;
      std::sort(fFEBModules_vec.begin(), fFEBModules_vec.end());
      // Remove duplicate values from vector
      fFEBModules_vec.erase(std::unique(fFEBModules_vec.begin(), fFEBModules_vec.end()), fFEBModules_vec.end());
      fFEB_total_number = fFEBModules_vec.size(); 
    }
    
    
    int min(0), max(1000), bins(250); 
    if (isManuallyTypeInFitRange){ 
      min  = fFitRange_manual_input.at(0);
      max  = fFitRange_manual_input.at(1);
      bins = fFitRange_manual_input.at(2);
    }
    
    // Initiate the 2d vector of histograms for adc distributions. 
    for(int thisFeb=0; thisFeb<fFEB_total_number; thisFeb++){
      std::vector<TH1F *> adc_1d_ditributions_perChannel_thisFEB;
      for (int sipm_no = 0; sipm_no < 32; sipm_no++){
        TString histo_name = Form("FEB_%i_channel_%i_ADC_histo", fFEBModules_vec[thisFeb], sipm_no);
        TH1F *h_temp = new TH1F(histo_name,"", bins, min, max); 
        TString histo_title= Form("FEB: %i, channel: %i", fFEBModules_vec[thisFeb], sipm_no);
        h_temp->SetTitle(histo_title);  
        adc_1d_ditributions_perChannel_thisFEB.push_back(h_temp);
      }
      adc_1d_ditributions_perChannel_perFEB.push_back(adc_1d_ditributions_perChannel_thisFEB);
    }
    // end of initiations.

    
    //--make tree to store the analysed single PE info:
    if (fStoreBasicInfo) {
      CRTHitBasicInfoTree = tfs->make<TTree>("CRTHitBasicInfoTree", "cryogenics hit tree");
      CRTHitBasicInfoTree->Branch("Run", &fRun,"fRun/I");
      CRTHitBasicInfoTree->Branch("Event", &fEvent,"fEvent/I");
      CRTHitBasicInfoTree->Branch("febid", &febid);
      CRTHitBasicInfoTree->Branch("channelA", &channelA);
      CRTHitBasicInfoTree->Branch("channelB", &channelB);  
      if (fIncludeTiming){
        CRTHitBasicInfoTree->Branch("ts0_hit", &ts0_hit);   
        CRTHitBasicInfoTree->Branch("ts1_hit", &ts1_hit);  
      } 
    }

    if (fStoreADCRawHistos){
      art::TFileDirectory dir = tfs->mkdir("ADC_histos");
      for (auto iFeb=0; iFeb<(int)adc_1d_ditributions_perChannel_perFEB.size(); iFeb++){
        for (auto iChannel=0; iChannel<(int)adc_1d_ditributions_perChannel_perFEB.at(iFeb).size(); iChannel++){
          TString histo_name = adc_1d_ditributions_perChannel_perFEB.at(iFeb).at(iChannel)->GetName();
          TString histo_title = adc_1d_ditributions_perChannel_perFEB.at(iFeb).at(iChannel)->GetTitle();
          int bins = adc_1d_ditributions_perChannel_perFEB.at(iFeb).at(iChannel)->GetNbinsX(); 
          adc_1d_ditributions_perChannel_perFEB.at(iFeb).at(iChannel) = dir.make<TH1F>(histo_name, histo_title, bins, min, max);
        }
      }
    }
    is_firstEvt=false;
  }

  // Fill the tree
  if (fStoreBasicInfo) MakeSinglePETree();
}


void sbndaq::CRTSinglePEAna::analyze_bern_fragment(artdaq::Fragment & frag)  {

  BernCRTFragmentV2 bern_fragment(frag);
  const BernCRTFragmentMetadataV2* md = bern_fragment.metadata();
  // per hit
  for(unsigned int iHit = 0; iHit < md->hits_in_fragment(); iHit++) {
    
    BernCRTHitV2 const* bevt = bern_fragment.eventdata(iHit);
    
    // Remove the problematic ones. 
    //if (md->MAC5()==154 || md->MAC5()==155 || md->MAC5()==73) continue;
    if (fcrt_keepall) {
      //metadata
      sequence_id.push_back(               frag.sequenceID());
      run_start_time.push_back(            md->run_start_time());
      this_poll_start.push_back(           md->this_poll_start());
      this_poll_end.push_back(             md->this_poll_end());
      last_poll_start.push_back(           md->last_poll_start());
      last_poll_end.push_back(             md->last_poll_end());
      system_clock_deviation.push_back(    md->system_clock_deviation());
      feb_hits_in_poll.push_back(          md->hits_in_poll());
      feb_hits_in_fragment.push_back(      md->hits_in_fragment());
      timestamp.push_back(                 bevt->timestamp);
      //event info
      lostcpu.push_back(                   bevt->lostcpu);
      lostfpga.push_back(                  bevt->lostfpga);
      feb_hit_number.push_back(            bevt->feb_hit_number);
      last_accepted_timestamp.push_back(   bevt->last_accepted_timestamp);
      lost_hits.push_back(                 bevt->lost_hits);
    }

    mac5.push_back(                       md->MAC5());
    flags.push_back(                      bevt->flags);
    ts0.push_back(                        bevt->ts0);
    ts1.push_back(                        bevt->ts1);
    coinc.push_back(                      bevt->coinc);
    adc0.push_back(                       bevt->adc[0]);
    adc1.push_back(                       bevt->adc[1]);
    adc2.push_back(                       bevt->adc[2]);
    adc3.push_back(                       bevt->adc[3]);
    adc4.push_back(                       bevt->adc[4]);
    adc5.push_back(                       bevt->adc[5]);
    adc6.push_back(                       bevt->adc[6]);
    adc7.push_back(                       bevt->adc[7]);
    adc8.push_back(                       bevt->adc[8]);
    adc9.push_back(                       bevt->adc[9]);
    adc10.push_back(                      bevt->adc[10]);
    adc11.push_back(                      bevt->adc[11]);
    adc12.push_back(                      bevt->adc[12]);
    adc13.push_back(                      bevt->adc[13]);
    adc14.push_back(                      bevt->adc[14]);
    adc15.push_back(                      bevt->adc[15]);
    adc16.push_back(                      bevt->adc[16]);
    adc17.push_back(                      bevt->adc[17]);
    adc18.push_back(                      bevt->adc[18]);
    adc19.push_back(                      bevt->adc[19]);
    adc20.push_back(                      bevt->adc[20]);
    adc21.push_back(                      bevt->adc[21]);
    adc22.push_back(                      bevt->adc[22]);
    adc23.push_back(                      bevt->adc[23]);
    adc24.push_back(                      bevt->adc[24]);
    adc25.push_back(                      bevt->adc[25]);
    adc26.push_back(                      bevt->adc[26]);
    adc27.push_back(                      bevt->adc[27]);
    adc28.push_back(                      bevt->adc[28]);
    adc29.push_back(                      bevt->adc[29]);
    adc30.push_back(                      bevt->adc[30]);
    adc31.push_back(                      bevt->adc[31]);


    /*if (fverbose) {
      std::cout << "  mac5                "     <<  (int)(md->MAC5())             << std::endl;
      std::cout << "  run_start_time      "     <<  md->run_start_time()          << std::endl;
      std::cout << "  this_poll_start     "     <<  md->this_poll_start()         << std::endl;
      std::cout << "  this_poll_end        "    <<  md->this_poll_end()           << std::endl;
      std::cout << "  last_poll_start      "    <<  md->last_poll_start()         << std::endl;
      std::cout << "  last_poll_end          "  <<  md->last_poll_end()           << std::endl;
      std::cout << "  system_clock_deviation "  <<  md->system_clock_deviation()  << std::endl;
      std::cout << "  feb_hits_in_poll       "  <<  md->hits_in_poll()            << std::endl;
      std::cout << "  feb_hits_in_fragment  "   <<  md->hits_in_fragment()        << std::endl;
      std::cout << "  flags  "                  <<  (int)(bevt->flags)            << std::endl;
      std::cout << "  lostcpu         "         <<  bevt->lostcpu                 << std::endl;
      std::cout << "  lostfpga    "             <<  bevt->lostfpga                << std::endl;
      std::cout << "  ts0         "             <<  bevt->ts0                     << std::endl;
      std::cout << "  ts1                    "  <<  bevt->ts1                     << std::endl;
      std::cout << "  coinc                  "  <<  bevt->coinc                   << std::endl;
      std::cout << "  feb_hit_number         "  <<  bevt->feb_hit_number          [<< std::endl;
      std::cout << "  timestamp              "  <<  bevt->timestamp               << std::endl;
      std::cout << "  last_accepted_timestamp"  <<  bevt->last_accepted_timestamp << std::endl;
      std::cout << "  lost_hits              "  <<  bevt->lost_hits               << std::endl;

      for(int ch=0; ch<32; ch++) std::cout << "channel " << ch << " has adc value " << bevt->adc[ch] << std::endl;
    }*/ // if verbose
  }// end loop over fragments
}//analyze_bern_fragment


// to-do list: finish this analyzer function. 
void sbndaq::CRTSinglePEAna::MakeSinglePETree()  {

  int hitcount = 0 ;
  int nhits = mac5.size();
  if (nhits<=0) { std::cout << " no crt hits in event " << fEvent << std::endl; return;}
  
  // clean before refill per hit. 
  channelA.clear(); channelB.clear(); febid.clear(); 
  if (fIncludeTiming){
    ts0_hit.clear(); ts1_hit.clear();
  }

  for (int ihit=0; ihit<nhits; ihit++) {

    if (!(flags.at(ihit) == 3 || flags.at(ihit) == 1)) continue;

    // Find the channel.
    std::vector<int> this_adc_array;
    this_adc_array.push_back(adc0[ihit]); // can use std::format, but only c++20. 
    this_adc_array.push_back(adc1[ihit]);
    this_adc_array.push_back(adc2[ihit]);
    this_adc_array.push_back(adc3[ihit]);
    this_adc_array.push_back(adc4[ihit]);
    this_adc_array.push_back(adc5[ihit]);
    this_adc_array.push_back(adc6[ihit]);
    this_adc_array.push_back(adc7[ihit]);
    this_adc_array.push_back(adc8[ihit]);
    this_adc_array.push_back(adc9[ihit]);
    this_adc_array.push_back(adc10[ihit]);
    this_adc_array.push_back(adc11[ihit]);
    this_adc_array.push_back(adc12[ihit]);
    this_adc_array.push_back(adc13[ihit]);
    this_adc_array.push_back(adc14[ihit]);
    this_adc_array.push_back(adc15[ihit]);
    this_adc_array.push_back(adc16[ihit]);
    this_adc_array.push_back(adc17[ihit]);
    this_adc_array.push_back(adc18[ihit]);
    this_adc_array.push_back(adc19[ihit]);
    this_adc_array.push_back(adc20[ihit]);
    this_adc_array.push_back(adc21[ihit]);
    this_adc_array.push_back(adc22[ihit]);
    this_adc_array.push_back(adc23[ihit]);
    this_adc_array.push_back(adc24[ihit]);
    this_adc_array.push_back(adc25[ihit]);
    this_adc_array.push_back(adc26[ihit]);
    this_adc_array.push_back(adc27[ihit]);
    this_adc_array.push_back(adc28[ihit]);
    this_adc_array.push_back(adc29[ihit]);
    this_adc_array.push_back(adc30[ihit]);
    this_adc_array.push_back(adc31[ihit]);

    auto highest_adc_value = max_element(std::begin(this_adc_array), std::end(this_adc_array)); // find the maximum hits. 
    int adc_channel_no = std::distance(std::begin(this_adc_array), highest_adc_value); // find the channel with the maximum hits. 
    if(fIncludeTiming){
      ts0_hit.push_back(ts0[ihit]); ts1_hit.push_back(ts1[ihit]);
    }

    // To find the FEB module.
    int thisFeb=mac5[ihit];
    febid.push_back(thisFeb); 
    std::vector<int>::iterator itr = std::find(fFEBModules_vec.begin(), fFEBModules_vec.end(), thisFeb);
    int index_in_module_number = std::distance(fFEBModules_vec.begin(), itr); // Find the module number. 
    // End of finding the FEB module. 

    if (index_in_module_number >= fFEB_total_number) continue; // if couldn't find the targeted module, cut. 

    // store the info of the adjcent channel ADC value. 
    if (adc_channel_no%2 != 0){ // odd number
      channelA.push_back(adc_channel_no-1); // even
      channelB.push_back(adc_channel_no);   // odd
      adc_1d_ditributions_perChannel_perFEB.at(index_in_module_number).at(adc_channel_no-1)->Fill(this_adc_array.at(adc_channel_no-1));
      adc_1d_ditributions_perChannel_perFEB.at(index_in_module_number).at(adc_channel_no)->Fill(*highest_adc_value);
    }else{ // even number
      channelA.push_back(adc_channel_no);   // even
      channelB.push_back(adc_channel_no+1); // odd
      adc_1d_ditributions_perChannel_perFEB.at(index_in_module_number).at(adc_channel_no)->Fill(*highest_adc_value);
      adc_1d_ditributions_perChannel_perFEB.at(index_in_module_number).at(adc_channel_no+1)->Fill(this_adc_array.at(adc_channel_no+1));
    }
  } //loop over hits

  // Fill the TTree.
  CRTHitBasicInfoTree->Fill();
  if (hitcount>1) std::cout << "event " << fEvent << " has " << hitcount << " crt hits " << std::endl;
}// end of MakeSinglePETree.

void sbndaq::CRTSinglePEAna::PeakFindFit(TH1F* h, bool save, art::TFileDirectory tfdir, float &gain, float &gain_err, float &pedestal, float &pedestal_err, float &chisqr, float &ndf){
  
  float gainSeed = fbias_voltage*fgain_increment_slope;

  Double_t tSpectrumSigma = 1.85;
  Double_t tSpectrumThreshold = 0.18;
  if (fbias_voltage>200.) tSpectrumThreshold = 0.13;
  const size_t nPeakMax = 10;
  
  //h->SetStats(kFALSE);
  TString name = h->GetName();
  TString histo_title = h->GetTitle();

  TCanvas* fit_canvas = tfdir.makeAndRegister<TCanvas>(name, histo_title);

  fit_canvas->Divide(1,2);
  fit_canvas->cd(1);
  h->GetXaxis()->SetTitle("ADC"); h->GetYaxis()->SetTitle("# of hits"); h->SetStats(kFALSE);  
  h->Draw("e0hist");

  TSpectrum *s = new TSpectrum();
  // Find the Peak of the histogram
  int nPeak = s->Search(h, tSpectrumSigma, "", tSpectrumThreshold);

  Double_t *peaks = s->GetPositionX(); //candidate photon-peaks ADC position
  
  // Sort peaks
  double temp = 0.;
  int nchanges = 0;
  do{
    nchanges=0;
    for(int p=0;p<nPeak-1;p++)
      if(peaks[p]>peaks[p+1])
      { 
        temp=peaks[p];
        peaks[p]=peaks[p+1];
        peaks[p+1]=temp;
        nchanges++;
      }
  }while(nchanges != 0);

  //ascending list of peak number(x) vs. ADC value(y) from TSpectrum
  std::vector<float> peak_value;
  std::vector<float> adc_value;

  //same list with "bad" peaks excluded (what eventually goes into the gain fit)
  std::vector<float> peak_value_ammended;
  std::vector<float> adc_value_ammended;
  std::vector<float> adc_value_error;

  //float ped_temp = 260.;
  //int peak_offset = round((peaks[0]-ped_temp)/gainSeed); //estimate peak number

  for (int ipeak=0; ipeak<nPeak; ipeak++){ // store the peak value from TSpectrum in the vector. 
    if(peaks[ipeak]<0) std::cout << "bad peak value: " << peaks[ipeak] << std::endl;
    peak_value.push_back(ipeak+1);
    adc_value.push_back(peaks[ipeak]);
  }

  int ipeak_new = 0;  //index of passing peak array
  int nPeakLow  = 0;  //no. peaks close to low hist edge
  int nPeakHigh = 0;  //no. peaks close to high hist edge
 
  // Fit various gaussians to the peak
  // first and refit chi-squareds
  for (int ipeak=0 ; ipeak<nPeak && ipeak<(int)nPeakMax; ipeak++){
    //initial gaus fit to peak from TSpectrum
    TF1 *gfit = new TF1("gfit", "gaus", adc_value[ipeak]-gainSeed/2, adc_value[ipeak]+gainSeed/2);

    gfit->SetParameter(0, h->GetBinContent(h->FindBin(adc_value[ipeak])));//200);
    gfit->SetParameter(1, adc_value[ipeak]);
    gfit->SetParameter(2, 12);
    gfit->SetParLimits(0, 0, 20000);
    gfit->SetParLimits(1, adc_value[ipeak]-15, adc_value[ipeak]+15);
    gfit->SetParLimits(2, 8, 40);

    h->Fit(gfit,"MQLR"); //fit peak
    gfit->Draw("same");

    //check for peaks near the edges of the histogam
    if(adc_value[ipeak]<h->GetBinLowEdge(1)+15) nPeakLow++;
    if(adc_value[ipeak]>h->GetBinLowEdge(h->GetNbinsX())-15) nPeakHigh++;
    
    if (ipeak==0){ // remove pedestals. 
      if (adc_value[ipeak+1]-adc_value[ipeak]<gainSeed*0.7 || (adc_value[ipeak+1]-adc_value[ipeak])>gainSeed*1.5) continue;
    }
    if (adc_value[ipeak] > 650.) continue; // cut becasue beyond that the adc_value doesn't makes sense (most of times)
    if (adc_value[ipeak]>h->GetBinLowEdge(1)+15 && adc_value[ipeak]<h->GetBinLowEdge(h->GetNbinsX())-15 && abs(adc_value[ipeak]-gfit->GetParameter(1))<15){
      //skip false peaks (may need improvement - currently relies on init values)
      if(ipeak!=0 && ipeak!=nPeak-1 //check it's not first or last peaks
       && (adc_value[ipeak+1]-adc_value[ipeak]<gainSeed*0.7 || adc_value[ipeak]-adc_value[ipeak-1]<gainSeed*0.7)){ //check peak within 30% of expected gain w.r.t adj.
        if (fverbose) std::cout << "determined peak " << ipeak << " (" << adc_value[ipeak] << " ADC) is false peak. removing..." << std::endl;
        for (int j=ipeak; j<nPeak; j++) peak_value[j] = peak_value[j]-1; //overwrite current value, shifting higher values down by 1 index
      }
      else{ //if not missed peak, could there have been a skipped peak?
        if(ipeak!=0 && ipeak!=nPeak-1 && adc_value[ipeak+1]-adc_value[ipeak]<gainSeed*1.2 && adc_value[ipeak]-adc_value[ipeak-1]>gainSeed*1.5){ //missed peak adjust
          if (ipeak==1 && (adc_value[ipeak]-adc_value[ipeak-1])>gainSeed*1.5){  // remove pedestals. 
            for (int j=ipeak; j<nPeak; j++) peak_value[j] = peak_value[j]-1;
            if (fverbose) std::cout << "determined peak " << ipeak-1 << " (" << adc_value[ipeak-1] << " ADC) is pedestal. removing..." << std::endl;
          }else{
            for (int j=ipeak; j<nPeak; j++) peak_value[j] = peak_value[j]+1;
            if (fverbose) std::cout << "missed peak detected. shifting peaks " << ipeak << " and higher up by 1..." << std::endl;
          }
        }
        //if last peak likely occuring after skipped peak
        if(ipeak==nPeak-1 && (adc_value[ipeak]-adc_value[ipeak-1])/gainSeed>1.5) {
          peak_value[ipeak]+=(int)((adc_value[ipeak]-adc_value[ipeak-1])/gainSeed);
          if (fverbose) std::cout << "missed peak(s) detected before last peak...shifting last peak" << std::endl;
        }
        
        peak_value_ammended.push_back(peak_value[ipeak]);
        adc_value_ammended.push_back(gfit->GetParameter(1));
        adc_value_error.push_back(gfit->GetParError(1));
        ipeak_new++;
      }//end else not missed but was there skip
    }// end if not edge peak, not far from TSpectrum value
  }//end for loop of peaks
  if (nPeakLow>0) for (int i=1; i<ipeak_new; i++) peak_value_ammended[i] = peak_value_ammended[i]-(nPeakLow-1);
  
  // To test whether we have enough peak for the fitting. 
  if (ipeak_new < 2) {std::cout<<"Not enough number of peak can be used for fitting..."<<std::endl; isBrokenChannel=true;}
  else{ isBrokenChannel=false; }

  fit_canvas->cd(2);
  if (isBrokenChannel){
    TPaveText* pt = new TPaveText(0.15, 0.4, 0.95, 0.6, "NDC");
    pt->SetFillColor(0);
    pt->SetTextAlign(22);
    pt->SetTextSize(0.06);
    pt->SetTextColor(kRed);
    pt->AddText("Potentially broken Channel: "+name);
    pt->Draw();

    fit_canvas->Update();
  }else{
    //graph of adc(y) vs. photo-peak number &(peak_value_ammended[0]), &(adc_value_ammended[0]), 0, &(adc_value_error[0])
    TGraphErrors* gr_mean = new TGraphErrors(ipeak_new, &(peak_value_ammended[0]), &(adc_value_ammended[0]), 0, &(adc_value_error[0]));
    
    //linear fit function
    TF1 *fit = new TF1("fit","[0] + [1]*x", peak_value_ammended[0]-0.25, peak_value_ammended[ipeak_new-1]+0.25);

    gr_mean->GetXaxis()->SetTitle("# of peaks");
    gr_mean->GetYaxis()->SetTitle("ADC");  

    gr_mean->SetTitle("");

    //name, initialize gain fit parameters
    fit->SetParName(1,"Gain");
    fit->SetParName(0, "Pedestal");
    fit->SetParLimits(1, gainSeed-40, gainSeed+40);
    /*fit->SetParameter(1, gainSeed);
    fit->SetParameter(0, ped_temp);
    fit->SetParLimits(0, ped_temp*0.8, ped_temp*1.2);*/

    if (fverbose) std::cout << "gain fit..." << std::endl;
    //perform gain fit
    gStyle->SetOptStat(0100);
    gStyle->SetOptFit(1111);
    gStyle->SetStatX(0.5);
    gStyle->SetStatY(0.9);
    gStyle->SetStatH(0.15);
    gStyle->SetStatW(0.2);

    gr_mean->Draw("ALP");
    gr_mean->Fit(fit, "QR");  

    //if (fverbose) std::cout << "check chi-square..." << std::endl;
    //check if gain fit is bad according to chi-square
    /*if (fit->GetChisquare()/fit->GetNDF()>5.0 && fit->GetChisquare()/fit->GetNDF()<1000000000000000.0)
    {
      if (fverbose) std::cout << "gain fit X^2 too large...shifting all peaks by 1" << std::endl;
      chisqr=fit->GetChisquare();
      for(int i=1; i<ipeak_new; i++) peak_value_ammended[i]+=1;
      gr_mean = new TGraphErrors(ipeak_new, &(peak_value_ammended[0]), &(adc_value_ammended[0]), 0, &(adc_value_error[0]));
      fit->SetRange(peak_value_ammended[0]-0.25, peak_value_ammended[ipeak_new-1]+0.25);
      gr_mean->Fit(fit, "QR");
      if (fit->GetChisquare()<chisqr) chisqr=fit->GetChisquare();
      else{
        for(int i=1; i<ipeak_new; i++) peak_value_ammended[i]-=2;
        gr_mean = new TGraphErrors(ipeak_new, &(peak_value_ammended[0]), &(adc_value_ammended[0]), 0, &(adc_value_error[0]));
        fit->SetRange(peak_value_ammended[0]-0.25,peak_value_ammended[ipeak_new-1]+0.25);
        gr_mean->Fit(fit, "QR");
        if (fit->GetChisquare()<chisqr) chisqr=fit->GetChisquare();
        else{
          for(int i=1; i<ipeak_new; i++) peak_value_ammended[i]+=1;
          gr_mean = new TGraphErrors(ipeak_new, &(peak_value_ammended[0]), &(adc_value_ammended[0]), 0, &(adc_value_error[0]));
          fit->SetRange(peak_value_ammended[0]-0.25,peak_value_ammended[ipeak_new-1]+0.25);
          gr_mean->Fit(fit, "QR");
        }
      }
    }*/

    // store the fitting results. 
    gain = fit->GetParameter(1); //gain
    gain_err = fit->GetParError(1);  //gain error
    pedestal = fit->GetParameter(0); //pedestal mean
    pedestal_err = fit->GetParError(0);  //pedestal mean error
    chisqr = fit->GetChisquare();  //X^2
    ndf = fit->GetNDF();        //NDF

    //fit_canvas->cd();
    fit_canvas->Update();
  }

  // If save which means we need to save results in pdf/png version.
  if(save){
    if (n_count_create_folder==0){
      gSystem->Exec("mkdir -p " + fPhoto_dump_dir);
      gSystem->Exec("rm "+fPhoto_dump_dir+"*");
      n_count_create_folder++;
    }

    fit_canvas->SaveAs(fPhoto_dump_dir+name+".pdf");
    fit_canvas->SaveAs(fPhoto_dump_dir+name+".png");

  }
}

DEFINE_ART_MODULE(sbndaq::CRTSinglePEAna)
