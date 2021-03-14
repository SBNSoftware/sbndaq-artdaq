//-------------------------------------------------
//---------------------------------------

////////////////////////////////////////////////////////////////////////
// Class:       FragmentDump
// Module Type: analyzer
// File:        FragmentDump_module.cc
// Description: Prints out information about each event.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"

#include "canvas/Utilities/Exception.h"

#include "sbndaq-artdaq-core/Overlays/Common/CAENV1730Fragment.hh"
#include "sbndaq-artdaq-core/Overlays/Common/WhiteRabbitFragment.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "artdaq-core/Data/Fragment.hh"
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

namespace sbndaq {
  class FragmentDump;
}

/**************************************************************************************************/

class sbndaq::FragmentDump : public art::EDAnalyzer {

public:
  struct Config {
    //--one atom for each parameter
    fhicl::Atom<int> Shift {
      fhicl::Name("shift_fragment_id"), 
      fhicl::Comment("Number to subtract to the fragment_id")
    };
  }; //--configuration
  using Parameters = art::EDAnalyzer::Table<Config>;

  explicit FragmentDump(Parameters const & pset);
  virtual ~FragmentDump();

  void analyze(const art::Event& evt) override;
  void beginJob() override;
  void endJob() override;
  void analyze_wr_fragment(artdaq::Fragment & frag);
  void analyze_caen_fragment(artdaq::Fragment & frag);
  void get_frag_desc(artdaq::Fragment & frag);

private:

  
  int fRun;
  art::EventNumber_t fEvent;
  
  int fShift; 
	
}; //--class FragmentDump


sbndaq::FragmentDump::FragmentDump(FragmentDump::Parameters const& pset): art::EDAnalyzer(pset)
{
  fShift = pset().Shift();

}

void sbndaq::FragmentDump::beginJob()
{

}

void sbndaq::FragmentDump::endJob()
{
  std::cout << "Ending FragmentDump...\n";
}


sbndaq::FragmentDump::~FragmentDump()
{
}


void sbndaq::FragmentDump::analyze(const art::Event& evt)
{

  fRun = evt.run();
  fEvent = evt.event();
  std::cout << "Run " << fRun << " event " << fEvent << std::endl;


  std::vector<art::Handle<artdaq::Fragments>> fragmentHandles;
  evt.getManyByType(fragmentHandles);

  /************************************************************************************************/
  for (auto handle : fragmentHandles) {
    if (!handle.isValid() || handle->size() == 0) continue;
    
    if (handle->front().type() == artdaq::Fragment::ContainerFragmentType) {
      //Container fragment                                                                                               
      for (auto cont : *handle) {
	artdaq::ContainerFragment contf(cont);
	std::cout << "    Found " << contf.block_count() << " Fragments in container of type " ;
	artdaq::Fragment::timestamp_t start,end;
	for (size_t ii = 0; ii < contf.block_count(); ++ii) {
	  auto thisfrag = *contf[ii].get();
	  if (ii==0) { get_frag_desc(thisfrag); start=thisfrag.timestamp(); end=thisfrag.timestamp(); 
	  }
	  else {
	    if (thisfrag.timestamp()<start) start=thisfrag.timestamp();
	    else if (thisfrag.timestamp()>end) end=thisfrag.timestamp();
	    
	  }
	}
	std::cout << "timestamp range from " << start << " to " << end << std::endl;
      }
    }
    else {      
      //normal fragment                
      std::cout << " MAIN " ;
      std::cout << " type is " ;
      for (auto frag : *handle) get_frag_desc(frag); 
    }
    std::cout << " -------------------------------------------"   << std::endl;
  } // loop over frag handles
}

void sbndaq::FragmentDump::get_frag_desc(artdaq::Fragment & frag)
{

  //  std::cout << "type is " << frag.typeString() << std::endl;  // gives integer value for type
  // sbndaq::detail::FragmentType thistype = frag.type();
  //   std::cout << "type is " << fragmentTypeToString(thistype) << std::endl;
    //std::cout << "type is " << frag.typeString() << std::endl;
  //  fragmentTypeToString(frag.fragment_id_t());
  if (frag.type()==sbndaq::detail::FragmentType::CAENV1730) std::cout << " CAENV1730 ";
  else if (frag.type()==sbndaq::detail::FragmentType::WhiteRabbit) std::cout << " WhiteRabbit ";
  else if (frag.type()==sbndaq::detail::FragmentType::BERNCRT) std::cout << " BERNCRT ";
  else if (frag.type()==sbndaq::detail::FragmentType::BERNCRTV2) std::cout << " BERNCRTV2 ";
  else if (frag.type()==sbndaq::detail::FragmentType::BERNCRTZMQ) std::cout << " BERNCRTZMQ " ;
  else if (frag.type()==sbndaq::detail::FragmentType::NevisTPC) std::cout << " NevisTPC " ;
  else if (frag.type()==sbndaq::detail::FragmentType::PTB) std::cout << " PTB " ;
  else if (frag.type()==sbndaq::detail::FragmentType::INVALID) std::cout << " INVALID " ;
  else if (frag.type()==sbndaq::detail::FragmentType::DummyGenerator) std::cout << " DummyGenerator " ;
  else if (frag.type()==sbndaq::detail::FragmentType::SpectratimeEvent) std::cout << " SpectratimeEvent " ;
  else std::cout << " not in list of fragment types, add to code in Fragment_dump.cc  " ;
	//ICARUS
        // PHYSCRATEDATA = artdaq::Fragment::FirstUserFragmentType + 4,
	//   PHYSCRATESTAT = artdaq::Fragment::FirstUserFragmentType + 5,
	//   ICARUSTriggerUDP = artdaq::Fragment::FirstUserFragmentType + 10,
	//   ICARUSPMTGate = artdaq::Fragment::FirstUserFragmentType + 11,


  std::cout <<   std::endl<< "     timestamp " << frag.timestamp()  <<  "     seq ID is " << frag.sequenceID() <<  std::endl;;


}


void sbndaq::FragmentDump::analyze_wr_fragment(artdaq::Fragment & frag)  {

      const WhiteRabbitEvent *event_ptr = reinterpret_cast<WhiteRabbitEvent const*>(frag.dataBeginBytes()); 
      timespec sysTime=event_ptr->systemTime;
      std::cout << "systime  " << sysTime.tv_sec << " " << sysTime.tv_nsec << std::endl;
      WhiteRabbitData fragdata=event_ptr->data;

      // each WR fragment has data from only one channel. The fragments are not always in time order
      std::cout << "WR: command " << fragdata.command << std::endl;
      std::cout << "WR: channel " << fragdata.channel << std::endl;
      std::cout << "WR: value " << fragdata.value << std::endl;
      std::cout << "WR: flags " << fragdata.flags << std::endl;
      std::cout << "WR: number of time stamps in this fragment " << fragdata.nstamp << std::endl;
      std::cout << " ----------------------- " << std::endl;

}

void sbndaq::FragmentDump::analyze_caen_fragment(artdaq::Fragment & frag)  {
  

      CAENV1730Fragment bb(frag);
      auto const* md = bb.Metadata();
      CAENV1730Event const* event_ptr = bb.Event();
      CAENV1730EventHeader header = event_ptr->Header;
      
      int fragId = static_cast<int>(frag.fragmentID()); 
      fragId-=fShift; 
      //
      std::cout << "\tFrom header, event counter is "  << header.eventCounter   << "\n";
      std::cout << "\tFrom header, triggerTimeTag in ns is " << 8*header.triggerTimeTag << "\n";
      std::cout << "\tFrom header, board id is "       << header.boardID       << "\n";
      std::cout << "\tFrom fragment, fragment id is "  << fragId << "\n";
      std::cout << "\tShift back, fragment id of "  << fShift << "\n";
      

}
  


DEFINE_ART_MODULE(sbndaq::FragmentDump)

