//
// sbndaq-artdaq/Generators/SBND/WIBReader.hh
//

#include "artdaq-core/Data/Fragment.hh"
#include "artdaq/Generators/CommandableFragmentGenerator.hh"
#include <semaphore.h>
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
     bool acquireSemaphores();
     bool acquireSemaphores_ThrowOnFailure();
     void releaseSemaphores();
     void disconnectWIB_releaseSemaphores();

     void setupWIB(fhicl::ParameterSet const& WIB_config);
     void setupFEMBFakeData(size_t iFEMB, fhicl::ParameterSet const& FEMB_config,bool continueOnFEMBRegReadError);
     void setupFEMB(size_t iFEMB, fhicl::ParameterSet const& FEMB_configure);
     bool use_semaphores;
     uint64_t semaphore_acquire_timeout_ms;
     bool calibration_mode;
     sem_t *sem_wib_yld;
     sem_t *sem_wib_lck;
     bool semaphores_acquired;
     std::unique_ptr<WIB> wib;
  };
}
