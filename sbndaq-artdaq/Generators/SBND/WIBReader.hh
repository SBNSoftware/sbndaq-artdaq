//
// sbndaq-artdaq/Generators/SBND/WIBReader.hh
//

#include "artdaq-core/Data/Fragment.hh"
#include "artdaq/Generators/CommandableFragmentGenerator.hh"
#include <semaphore.h>
#include "WIB.hh"
//#include "WIBBase.hh"
//#include "WIBStatus.hh"

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
     bool acquireSemaphores();
     bool acquireSemaphores_ThrowOnFailure();
     void releaseSemaphores();
     void disconnectWIB_releaseSemaphores();
     void setupWIB(fhicl::ParameterSet const& WIB_config);
     void setupFEMBFakeData(size_t iFEMB, fhicl::ParameterSet const& FEMB_config,bool continueOnFEMBRegReadError);
     void setupWIBFakeData(int datamode); // This function is developed by looking into Modified_WIB_fake_data.py at "/home/nfs/sbnd/BNL_CE/SBND_CE"
     void setupNoiseMinConfig(int FEMB_NO, int tries); // This is the Minimum configuration suggested by Shanshan to see FEMB noise after WIB config
     void IssueWIBSYNC();
     void FEMBHsLinkCheck(int FEMB_NO, int tries);
     void InitFEMBRegCheck(uint32_t expected_val, std::string reg_addrs, int FEMB_NO, int tries);
     void disable_dat_stream_and_sync_to_NEVIS();
     void setupFEMB(size_t iFEMB, fhicl::ParameterSet const& FEMB_configure);
     uint64_t semaphore_acquire_timeout_ms;
     bool calibration_mode;
     sem_t *sem_wib_yld;
     sem_t *sem_wib_lck;
     bool semaphores_acquired;
     std::unique_ptr<WIB> wib;
  };
}

