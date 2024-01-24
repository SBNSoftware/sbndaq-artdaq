//
// sbndaq-artdaq/Generators/SBND/MBBReader.hh
//

#include "artdaq-core/Data/Fragment.hh"
#include "artdaq/Generators/CommandableFragmentGenerator.hh"

#include "MBB.hh"

namespace sbndaq 
{
  class MBBReader : public artdaq::CommandableFragmentGenerator 
  {
    public:
      MBBReader();
      // "initialize" transition
      explicit MBBReader(fhicl::ParameterSet const& ps);
      // "shutdown" transition
      virtual ~MBBReader();

    private:

     // "start" transition
    void start() override;
 
     // "stop" transition
    void stop() override;
     bool getNext_(artdaq::FragmentPtrs& output) override;
     void stopNoMutex() override {}

     void setupMBB(fhicl::ParameterSet const& MBB_config);

    uint32_t stop_femb_daq;
    uint32_t start_femb_daq;
    uint32_t sleep_time;     
std::unique_ptr<MBB> mbb;
  };
}
