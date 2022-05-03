#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "fhiclcpp/ParameterSet.h"

#include "sbndaq-artdaq-core/Obj/SBND/testStandTrigger.h"

#include <iostream>
#include <vector>
#include <string>

namespace sbndaq {
  class testStandCoincidenceFilter;
}

class sbndaq::testStandCoincidenceFilter : public art::EDFilter {
public:
  explicit testStandCoincidenceFilter(fhicl::ParameterSet const & p);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  testStandCoincidenceFilter(testStandCoincidenceFilter const &) = delete;
  testStandCoincidenceFilter(testStandCoincidenceFilter &&) = delete;
  testStandCoincidenceFilter & operator = (testStandCoincidenceFilter const &) = delete;
  testStandCoincidenceFilter & operator = (testStandCoincidenceFilter &&) = delete;

  // Required functions.
  bool filter(art::Event & e) override;

  // Selected optional functions.
  void beginJob() override;
  void endJob() override;

private:
 
  // fhicl paramters
  std::string fInputModuleName;
  int fNCoincidence;
  bool fExclusive;
  bool fVerbose;

};

// ------------------------------------------------- 

sbndaq::testStandCoincidenceFilter::testStandCoincidenceFilter(fhicl::ParameterSet const & pset) : art::EDFilter(pset),
fInputModuleName(pset.get<std::string>("InputModule")),
fNCoincidence(pset.get<int>("nCoincidence")),
fExclusive(pset.get<bool>("exclusive")),
fVerbose(pset.get<bool>("verbose"))
// Initialize member data here.
{
  // Call appropriate produces<>() functions here.
}

// ------------------------------------------------- 

bool sbndaq::testStandCoincidenceFilter::filter(art::Event & evt)
{

  int fRun = evt.run();
  art::EventNumber_t fEvent = evt.event();

  std::cout << "Run: " << fRun << ", Event: " << fEvent << std::endl;

  // read trigger information from event
  art::Handle< sbndaq::testStandTrigger > triggerHandle;
  evt.getByLabel(fInputModuleName, triggerHandle);
  if(!triggerHandle.isValid()) {
    throw art::Exception(art::errors::Configuration) << "testStandtestStandCoincidenceFilter: could not find trigger information from producer, check input module name specified is correct." << std::endl;
  }

  int n = triggerHandle->nAboveThreshold;

  // test coincidence condition  
  if (n >= fNCoincidence) {
    if (fExclusive && n != fNCoincidence) return false;
    if (fVerbose) std::cout << "Pass" << std::endl;
    return true;
  }
  else return false;
 
} // filter

// ------------------------------------------------- 

void sbndaq::testStandCoincidenceFilter::beginJob()
{
  // Implementation of optional member function here.
} // beginJob

// ------------------------------------------------- 

void sbndaq::testStandCoincidenceFilter::endJob()
{
  // Implementation of optional member function here.
} // endJob

// -------------------------------------------------

DEFINE_ART_MODULE(sbndaq::testStandCoincidenceFilter)
