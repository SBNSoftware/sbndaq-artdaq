//-------------------------------------------------
//---------------------------------------

////////////////////////////////////////////////////////////////////////
// Class:       FragmentToBinary
// Module Type: analyzer
// File:        FragmentToBinary_module.cc
// Description: Takes fragments and unpacks them into a binary file
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"

#include "canvas/Utilities/Exception.h"

#include "sbndaq-artdaq-core/Overlays/Common/CAENV1730Fragment.hh"
#include "sbndaq-artdaq-core/Overlays/Common/WhiteRabbitFragment.hh"
#include "sbndaq-artdaq-core/Overlays/Common/BernCRTFragmentV2.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "artdaq-core/Data/Fragment.hh"
#include "artdaq-core/Data/ContainerFragment.hh"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <vector>
#include <iostream>
#include <bitset>
#include <random>

#include <boost/archive/binary_oarchive.hpp>

namespace sbndaq {
  class FragmentToBinary;
}

/**************************************************************************************************/

class sbndaq::FragmentToBinary : public art::EDAnalyzer {

public:
  struct Config {
    //--one atom for each parameter
    fhicl::Atom<bool> include_caen {
      fhicl::Name("include_caen"),
      fhicl::Comment("look for caen 1730 fragments true/false")
    };
    fhicl::Atom<bool> include_wr {
      fhicl::Name("include_wr"),
      fhicl::Comment("look for wr dio fragments true/false")
    };
    fhicl::Atom<bool> include_berncrt {
      fhicl::Name("include_berncrt"),
      fhicl::Comment("look for bern CRT V2 fragments true/false")
    };
    fhicl::Atom<bool> verbose {
      fhicl::Name("verbose"),
      fhicl::Comment("lots of text output if set to true")
    };
  }; //--configuration
  using Parameters = art::EDAnalyzer::Table<Config>;

  explicit FragmentToBinary(Parameters const & pset);
  virtual ~FragmentToBinary();

  void analyze(const art::Event& evt) override;
  void beginJob() override;
  void endJob() override;

private:

  int fRun;
  art::EventNumber_t fEvent;

  bool finclude_caen;
  bool finclude_wr;
  bool finclude_berncrt;
  bool fverbose;

  std::ofstream fFile;
}; //--class FragmentToBinary


sbndaq::FragmentToBinary::FragmentToBinary(FragmentToBinary::Parameters const& pset): art::EDAnalyzer(pset)
{
  finclude_caen = pset().include_caen();
  finclude_wr = pset().include_wr();
  finclude_berncrt = pset().include_berncrt();
  fverbose = pset().verbose();
}

void sbndaq::FragmentToBinary::beginJob()
{
  if (fverbose)  std::cout << "Starting FragmentToBinary...\n";
  fFile = std::ofstream("binary_test.dat");
}

void sbndaq::FragmentToBinary::endJob()
{
  if (fverbose)  std::cout << "Ending FragmentToBinary...\n";
}


sbndaq::FragmentToBinary::~FragmentToBinary()
{
}


void sbndaq::FragmentToBinary::analyze(const art::Event& evt)
{
  fRun = evt.run();
  fEvent = evt.event();
  if (fverbose)   std::cout << "Run: " << fRun << " event: " << fEvent << std::endl;
  
  std::vector<art::Handle<artdaq::Fragments>> fragmentHandles;
  
#if ART_HEX_VERSION < 0x30900
  evt.getManyByType(fragmentHandles);
#else
  fragmentHandles = evt.getMany<std::vector<artdaq::Fragment>>();
#endif



  /************************************************************************************************/

  // Loop fragment handles
  for (auto handle : fragmentHandles) {
    if (!handle.isValid() || handle->size() == 0) continue;

    // Container or standard?
    
    if (handle->front().type() == artdaq::Fragment::ContainerFragmentType) {
      for (auto cont : *handle) {
	artdaq::ContainerFragment contf(cont);

	// Type of fragment?

	if (contf.fragment_type() == sbndaq::detail::FragmentType::CAENV1730 && finclude_caen) {

	  if (fverbose) 
	    std::cout << "\tFound " << contf.block_count() << " CAEN Fragments in container " << std::endl;

	  for (size_t ii = 0; ii < contf.block_count(); ++ii) {
	    // turn caen fragment to binary
	    const CAENV1730Fragment caen_frag(*contf[ii].get());
	    //	    oa << caen_frag;
	    //	    oa << caen_frag.Event();
	    //	    oa << *contf[ii].get();
	    std::cout << *(*contf[ii].get()).dataBegin() << std::endl;
	    
	  }
	} 
	else if (contf.fragment_type() == sbndaq::detail::FragmentType::WhiteRabbit && finclude_wr) {
	  
	  if (fverbose) 
	    std::cout << "\tFound " << contf.block_count() << " WR Fragments in container " << std::endl;
	  
	  for (size_t ii = 0; ii < contf.block_count(); ++ii) {
	    // turn wr fragment to binary
	  }
	}
	else if (contf.fragment_type() == sbndaq::detail::FragmentType::BERNCRTV2 && finclude_berncrt) {

	  if (fverbose) 
	    std::cout << "\tFound " << contf.block_count() << " BERNCRT Fragments in container " << std::endl;

	  for (size_t ii = 0; ii < contf.block_count(); ++ii) {
	    // turn bern fragment to binary
	  }
	}
      }
    }
    else {
      
      // Type of fragment?

      if (handle->front().type() == sbndaq::detail::FragmentType::CAENV1730 && finclude_caen) {
	
	if (fverbose) 
	  std::cout << "\tFound " << handle->size() << " normal CAEN fragments" << std::endl;

	for (auto frag : *handle) {
	  boost::archive::binary_oarchive oa(fFile);
	  // turn caen fragment to binary
	  CAENV1730Fragment caen_frag(frag);
	  //	  oa << caen_frag;
	  //	  oa << caen_frag.Event();
	  int a = std::rand();
	  std::random_device rd{};
	  std::mt19937 gen{rd()};
	  std::normal_distribution<> d{5,2};
	  double b = d(gen);
	  sbndaq::SerializableTestStruct st;
	  st.a = a;
	  st.b = b;
	  oa << st;
	  std::cout << st.a << " & " << st.b << std::endl;
	}
      }

      else if (handle->front().type()==sbndaq::detail::FragmentType::WhiteRabbit && finclude_wr) {

	if (fverbose) 
	  std::cout << "\tFound " << handle->size() << " normal WR fragments" << std::endl;

	for (auto frag : *handle) {
	  // turn wr fragment to binary
	}
      }
      else if (handle->front().type() == sbndaq::detail::FragmentType::BERNCRTV2 && finclude_berncrt) {

	if (fverbose) 
	  std::cout << "\tFound " << handle->size() << " normal BERNCRT fragments" << std::endl;

        for (auto frag : *handle) {
	  // turn bern fragment to binary
	}
      }
    }
  }
}

DEFINE_ART_MODULE(sbndaq::FragmentToBinary)