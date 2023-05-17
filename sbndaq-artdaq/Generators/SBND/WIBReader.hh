//
// sbndaq-artdaq/Generators/SBND/WIBReader.hh
//

#include "artdaq-core/Data/Fragment.hh"
#include "artdaq/Generators/CommandableFragmentGenerator.hh"

#include "WIB.hh"

namespace sbndaq 
{
  class WIBReader : public artdaq::CommandableFragmentGenerator 
  {
    public:
      WIBReader();
      // "initialize" transition
      explicit WIBReader(fhicl::ParameterSet const& ps);
      // "shutdown" transition
      virtual ~WIBReader();

    private:

     // "start" transition
     void start() override;
 
     // "stop" transition
     void stop() override;
     bool getNext_(artdaq::FragmentPtrs& output) override;
     void stopNoMutex() override {}

     void setupWIB(fhicl::ParameterSet const& WIB_config);
     void setupFEMBFakeData(size_t iFEMB, fhicl::ParameterSet const& FEMB_config,bool continueOnFEMBRegReadError);
     void setupWIBFakeData(int datamode); // This function is developed by looking into Modified_WIB_fake_data.py at "/home/nfs/sbnd/BNL_CE/SBND_CE"
     void setupFEMB(size_t iFEMB, fhicl::ParameterSet const& FEMB_configure);
     std::unique_ptr<WIB> wib;
  };
}

