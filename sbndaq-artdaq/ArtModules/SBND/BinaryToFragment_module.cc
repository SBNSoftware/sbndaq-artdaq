//-------------------------------------------------
//---------------------------------------

////////////////////////////////////////////////////////////////////////
// Class:       BinaryToFragment
// Module Type: analyzer
// File:        BinaryToFragment_module.cc
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
#include "sbndaq-artdaq-core/Overlays/Common/BernCRTFragmentSerial.hh"
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

#include <boost/archive/binary_iarchive.hpp>

namespace sbndaq {
  class BinaryToFragment;
}

/**************************************************************************************************/

class sbndaq::BinaryToFragment : public art::EDAnalyzer {

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

  explicit BinaryToFragment(Parameters const & pset);
  virtual ~BinaryToFragment();

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

  std::ifstream fFile;

}; //--class BinaryToFragment


sbndaq::BinaryToFragment::BinaryToFragment(BinaryToFragment::Parameters const& pset): art::EDAnalyzer(pset)
{
  finclude_caen = pset().include_caen();
  finclude_wr = pset().include_wr();
  finclude_berncrt = pset().include_berncrt();
  fverbose = pset().verbose();
}

void sbndaq::BinaryToFragment::beginJob()
{
  if (fverbose)  std::cout << "Starting BinaryToFragment...\n";
  //  fFile = std::ifstream("binary_test.dat");
}

void sbndaq::BinaryToFragment::endJob()
{
  if (fverbose)  std::cout << "Ending BinaryToFragment...\n";
}


sbndaq::BinaryToFragment::~BinaryToFragment()
{
}


void sbndaq::BinaryToFragment::analyze(const art::Event& /*evt*/)
{
  std::ifstream file("binary_test.dat");
  for(int i = 0; i < 400; ++i)
    {
      try
	{
	  sbndaq::BernCRTFragmentSerial serial;
	  boost::archive::binary_iarchive ia(file);
	  ia >> serial;
	  std::cout << serial.sequence_id << " " << serial.fragment_id << " " << serial.timestamp << std::endl;
	}
      catch (const std::exception& ex) {
	std::cout << ex.what() << std::endl;
      }
    }
}

DEFINE_ART_MODULE(sbndaq::BinaryToFragment)
