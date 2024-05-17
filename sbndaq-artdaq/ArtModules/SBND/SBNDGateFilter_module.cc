/////////////////////////////////////////////////////////////////////
// Class: ICARUSGateFilter_module.cc
// Module Type: analyzer
// File: ICARUSGateFilter_module.cc
// Description: Filter if Gate Matches
/////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Utilities/ExceptionMessages.h"

#include "sbndaq-artdaq-core/Overlays/SBND/PTBFragment.hh"
#include "artdaq-core/Data/ContainerFragment.hh"


#include "artdaq/DAQdata/Globals.hh"
#include "artdaq-core/Data/Fragment.hh"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <vector>
#include <iostream>

#ifdef TRACE_NAME
#undef TRACE_NAME
#endif

#define TRACE_NAME "SBNDGateFilter_module"

namespace sbnd {
  class SBNDGateFilter;
}


class sbnd::SBNDGateFilter : public art::EDFilter {
public:
  explicit SBNDGateFilter(fhicl::ParameterSet const & pset);
  virtual ~SBNDGateFilter();

  virtual bool filter(art::Event & evt) override;
  //template <class T>  
  std::vector<uint64_t> GetAllHLTs(artdaq::ContainerFragment *trigfrag);
  std::vector<uint64_t> GetHLT(sbndaq::CTBFragment ptb_fragment);
  bool ApplyGateFilter(std::vector<uint64_t> triggers); //(artdaq::ContainerFragment *trigfrag);

private:
  std::string fInputLabel, fInputContainerInstance, fInputNonContainerInstance;
  int ftrigger_type;
  std::vector<uint64_t> fexcluded_triggers;
};

sbnd::SBNDGateFilter::SBNDGateFilter(fhicl::ParameterSet const & pset)
  : EDFilter(pset),
    fInputLabel(pset.get<std::string>("InputLabel", "daq")),
    fInputContainerInstance(pset.get<std::string>("InputContainerInstance", "ContainerPTB")), 
    fInputNonContainerInstance(pset.get<std::string>("InputNonContainerInstance", "PTB")), 
    ftrigger_type(pset.get<int>("trigger_type",-1)),
    fexcluded_triggers( pset.get< std::vector<uint64_t> >("excluded_triggers", {}))
{
}

sbnd::SBNDGateFilter::~SBNDGateFilter()
{
}

std::vector<uint64_t> sbnd::SBNDGateFilter::GetAllHLTs(artdaq::ContainerFragment*  ptb_container_fragment){
  std::vector<uint64_t> triggers;  
  
  for (size_t f=0; f<ptb_container_fragment->block_count(); ++f){//loop over container of fragments
    artdaq::Fragment frag=*ptb_container_fragment->at(f).get();
    sbndaq::CTBFragment ptb_fragment(frag);
    //==============
    std::vector fragTriggers=GetHLT(ptb_fragment); //Not sure that there could really be multiple HLTs in a single fragment but just in case I'll make it vector
    triggers.insert(triggers.end(), fragTriggers.begin(), fragTriggers.end() );//append list of triggers in this fragment to all of the HLTs in the container
  }//end loop over fragments
  
  return triggers;
}



std::vector<uint64_t> sbnd::SBNDGateFilter::GetHLT(sbndaq::CTBFragment ptb_fragment){
  std::vector<uint64_t> triggers;  

  for ( size_t i = 0; i < ptb_fragment.NWords(); i++ ) {//loop over words in fragment       
    //if  (ptb_fragment.Word(i)->IsHLT()==false) continue;  
    if  (ptb_fragment.Word(i)->word_type !=0x2 ) continue; //0x2 is the type for an HLT (0x1 for LLT) 
    uint64_t hlttrigger=ptb_fragment.Trigger(i)->trigger_word & 0x1FFFFFFFFFFFFFFF;
    hlttrigger= log(1.*hlttrigger)/log(2.) ; 
    if(hlttrigger>=20) continue; //HLT triggers greater then 20 are reserved for non event triggers
    triggers.emplace_back(hlttrigger);
  }
  
  return triggers;
}





//template <class T> 
bool sbnd::SBNDGateFilter::ApplyGateFilter(std::vector<uint64_t> triggers)//artdaq::ContainerFragment*  ptb_container_fragment)//(T trigfrag)
{
  if(triggers.size()==0){
    TLOG(TLVL_WARNING) << "This event has no HLT fragments or none with trigger numbers then 20. It fails filter.";
    return false;
  }
  
  if(triggers.size()>1){
    TLOG(TLVL_INFO) << "This event has "<<triggers.size()<<" HLTs in it. Filter will pass if any are trigger type == "<<ftrigger_type<<".";
  }

  std::string trigstring="";
  bool passesFilter=false;

  for(int hlttrigger: triggers){
    if(fexcluded_triggers.size()>0){//Need to loop through all of the excluded trigger types to check for them for each present HLT 
      for(  int excluded_trigger : fexcluded_triggers){
	if (hlttrigger==excluded_trigger){
	  TLOG(TLVL_DEBUG) << "This Event contains an excluded trigger type " << hlttrigger << "==" << excluded_trigger<< " so fails the filter.";
	  return false;
	}
      }
    }//end loop over excluded triggers
    
    if( hlttrigger==ftrigger_type || ftrigger_type==-1){
      TLOG(TLVL_DEBUG) << "This Event has trigger type " << hlttrigger << "==" << ftrigger_type
		       << "  and passes filter.";
      passesFilter=true;
      if(fexcluded_triggers.size()==0) break;//no need to keep looking through the triggers if none are excluded
    }
    trigstring+= std::to_string(hlttrigger)+", ";
  }//end loop hlts 




  if (passesFilter) return true;
  
  TLOG(TLVL_DEBUG) << "This Event has trigger type { " << trigstring.c_str()  << "} ==" 
		   << ftrigger_type << " and fails filter.";
  return false;

}



bool sbnd::SBNDGateFilter::filter(art::Event & evt)
{
  art::EventNumber_t eventNumber = evt.event();

  // Look for fragments
  // HLT trigger words: 1-BNB Zero bias, 2-BNB+Light, 
  // 3-Offbeam Zero bias, 4-Offbeam+Light, 5-Crossing Muon
 
 
  art::InputTag itag(fInputLabel, fInputContainerInstance);
  auto cont_frags = evt.getHandle<artdaq::Fragments>(itag);


  if(cont_frags) 
  {
    //Are these always container fragments? 
    for(auto const& cont : *cont_frags){
      artdaq::ContainerFragment contf(cont);                                               
      //if (contf.fragment_type() != sbndaq::detail::FragmentType::PTB) continue; 
      bool filter_result = false;
      artdaq::ContainerFragment* contf2=&contf;
      std::vector hlts=GetAllHLTs(&contf);
      filter_result = ApplyGateFilter(hlts);//&contf);//<artdaq::ContainerFragment>(contf);
      return filter_result;
    }
  }
  

  else if(!cont_frags) {
    TLOG(TLVL_WARN) << "Run " << evt.run() << ", subrun " << evt.subRun() << ", event " << eventNumber << " has zero HLT word Fragments in module, not separating by beam type!";
  }

  

  return false;
}

DEFINE_ART_MODULE(sbnd::SBNDGateFilter)
