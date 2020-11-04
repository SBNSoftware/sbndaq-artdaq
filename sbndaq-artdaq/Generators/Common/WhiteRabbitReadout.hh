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

// Network includes
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <net/if.h>
#include <netpacket/packet.h>


namespace sbndaq
{
  class WhiteRabbitReadout: public artdaq::CommandableFragmentGenerator
  {
  public:
    explicit WhiteRabbitReadout(fhicl::ParameterSet const & ps);
    virtual ~WhiteRabbitReadout();

    enum
    {
      RULER_PROTO       = 0x5752,
      PRIV_MEZZANINE_ID = SIOCDEVPRIVATE + 14
    };

  protected:

    struct RabbitFrame
    {
      struct ether_header header;
      unsigned char padding[2];
      struct sbndaq::WhiteRabbitEvent rabbitCommand;
    } rabbitFrame;

    std::vector<struct sbndaq::WhiteRabbitEvent> buffer;

    bool getNext_(artdaq::FragmentPtrs & output) override;
    void start() override;
    void stop() override;
    void stopNoMutex() override;
    void stopAll();
    void configure();
    void openWhiteRabbitSocket(const char *deviceName);

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

    struct sockaddr_ll address;
    int	               agentSocket;
    struct ifreq       agentDevice;
  };
}

#endif
