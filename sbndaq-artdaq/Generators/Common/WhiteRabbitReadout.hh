//
// sbndaq-artdaq/Generators/Common/WhiteRabbitReadout.hh
//

#ifndef sbndaq_artdaq_Generators_Common_WhiteRabbitReadout
#define sbndaq_artdaq_Generators_Common_WhiteRabbitReadout

#include <memory>
#include <atomic>
#include <future>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "fhiclcpp/fwd.h"
#include "artdaq-core/Data/Fragment.hh"
#include "artdaq/Generators/CommandableFragmentGenerator.hh"
#include "workerThread.hh"
#include "sbndaq-artdaq-core/Overlays/Common/WhiteRabbitFragment.hh"
#include <unistd.h>
#include <vector>


namespace sbndaq
{
  class WhiteRabbitReadout: public artdaq::CommandableFragmentGenerator
  {
  public:
    explicit WhiteRabbitReadout(fhicl::ParameterSet const & ps);
    virtual ~WhiteRabbitReadout();

  protected:

    bool getNext_(artdaq::FragmentPtrs & output) override;
    void start() override;
    void stop() override;
    void stopNoMutex() override;
    void stopAll();
    void configure();

    // General
    uint32_t runNumber_;
    std::string device;
    uint32_t eventCounter;
    uint32_t messageCounter;
    std::atomic_bool running;
    std::mutex bufferLock;

    fhicl::ParameterSet const ps_;

  private:

    bool getData();
    bool FillFragment(artdaq::FragmentPtrs &, bool clear_buffer=false);

    share::WorkerThreadUPtr GetData_thread_;
  };
}

#endif
