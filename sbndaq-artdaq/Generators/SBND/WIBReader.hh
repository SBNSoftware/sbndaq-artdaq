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
     void FEMB_DECTECT(int FEMB_NO, uint32_t FEMB_V);// This function is a modified version of a function availble in shanshan's python script to configure WIB/FEMB
     bool Retry_Read(std::map<std::string,double> &map, std::string var_name, double val, char check_type, int Tries); // check_types allowed are e, g, l
     void FEMB_DECTECT_V2(int FEMB_NO, uint32_t FEMB_V, int Tries); // This function is a modified version of a function availble in shanshan's python script to configure WIB/FEMB 
     void FEMB_DETECT_ALL(std::vector<bool> &FEMB_NOs, uint32_t FEMB_V, int Tries); // This function is a modified version of a function availble in shanshan's python script to configure WIB/FEMB
     void FEMB_SCAN(std::vector<bool> &FEMB_NOs, uint32_t FEMB_V); // This function is a modified version of a function copied from shanshan's python script to configure WIB/FEMB
     void setupFEMB(size_t iFEMB, fhicl::ParameterSet const& FEMB_configure);
     void setupFEMB_to_send_fake_data(int FEMB_NO, int fake_mode); // setup to send fake data as per shanshan's instructions
     void prepFEMB_MBB_Calib(int FEMB_NO);
     void prepFEMB_MBB_Calib(std::vector<bool> enable_FEMBs); 
     void Do_Err_Check(std::vector<bool> enable_FEMBs); // perform error checking for bad channels
     void Change_TST_Pulse_Separation(int FEMB_NO, uint32_t pls_sep); // Change the separation between negative and positvie pulse
     bool use_semaphores;
     uint64_t semaphore_acquire_timeout_ms;
     bool calibration_mode;
     sem_t *sem_wib_yld;
     sem_t *sem_wib_lck;
     bool semaphores_acquired;
     std::unique_ptr<WIB> wib;
  };
}

