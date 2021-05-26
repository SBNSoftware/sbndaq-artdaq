//
// sbndaq-artdaq/Generators/SBND/DAPHNEReader.hh
//

#include "artdaq-core/Data/Fragment.hh"
#include "artdaq/Generators/CommandableFragmentGenerator.hh"

namespace sbndaq 
{
  class DAPHNEReader : public artdaq::CommandableFragmentGenerator 
  {
    public:
      DAPHNEReader();
      // "initialize" transition
      explicit DAPHNEReader(fhicl::ParameterSet const& ps);
      // "shutdown" transition
      virtual ~DAPHNEReader();

    protected:

     // "start" transition
     void start() override;
 
     // "stop" transition
     void stop() override;
     bool getNext_(artdaq::FragmentPtrs& output) override;
     void stopNoMutex() override {}

     void setupDAPHNE(fhicl::ParameterSet const& ps);
     uint16_t pedestal;
  };
}
