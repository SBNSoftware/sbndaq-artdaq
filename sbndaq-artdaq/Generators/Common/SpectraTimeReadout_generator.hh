//
// sbndaq-artdaq/Generators/Common/SpectraTimeReadout_generator.hh
//

#ifndef sbndaq_artdaq_Generators_Common_SpectraTimeReadout_generator
#define sbndaq_artdaq_Generators_Common_SpectraTimeReadout_generator

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
#include "sbndaq-artdaq-core/Overlays/Common/SpectratimeEventFragment.hh"
#include "sbndaq-artdaq/Generators/Common/GPS/GPSInfo.hh"
#include <unistd.h>
#include <vector>


namespace sbndaq
{
  class SpectraTimeReadout: public artdaq::CommandableFragmentGenerator
  {
  public:
    explicit SpectraTimeReadout(fhicl::ParameterSet const & ps);
    virtual ~SpectraTimeReadout();
    enum
    {
      GPS_MQ         = 0xCDF,
      BUFFER_SIZE    = 120,
      READOUT_PUSH   = 1,
      READOUT_PULL   = 2,
      STATUS_FULL    = 1,
      STATUS_MINIMAL = 2
    };

  protected:

    bool getNext_(artdaq::FragmentPtrs & output) override;
    void start() override;
    void stop() override;
    void stopNoMutex() override;
    void stopAll();
    void init();

    int readoutMode;
    int statusMode;
    int nextSleep;
    int dataSleep;

    // Message queue
    key_t daqMQ;
    int daqID;
    
    // General
    uint32_t runNumber_;
    std::string device;
    uint32_t eventCounter;
    uint32_t messageCounter;
    std::atomic_bool running;
    std::mutex bufferLock;

    // SpectraTimeConfiguration config_;

    fhicl::ParameterSet const ps_;

    struct SpectratimeMessage
    {
      long mtype;
      bool unsent;
      SpectratimeEvent data;
    };

    struct SpectratimeMessage buffer[BUFFER_SIZE];
    uint32_t currentMessage;

  private:

    bool getData();
    bool FillFragment(artdaq::FragmentPtrs &, bool clear_buffer=false);

    share::WorkerThreadUPtr GetData_thread_;
  };
}

#endif
