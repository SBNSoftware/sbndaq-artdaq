//
// sbnddaq-readout/Generators/SpectraTime_generatorBase.hh
//

#ifndef sbndaq_artdaq_Generators_Common_SpectraTime_generatorBase
#define sbndaq_artdaq_Generators_Common_SpectraTime_generatorBase

#include <memory>
#include <atomic>
#include <future>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <boost/circular_buffer.hpp>
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "fhiclcpp/fwd.h"
#include "artdaq-core/Data/Fragment.hh"
#include "artdaq/Application/CommandableFragmentGenerator.hh"
#include "workerThread.hh"
#include "sbndaq-artdaq-core/Overlays/Common/SpectratimeEventFragment.hh"
#include "sbndaq-artdaq/Generators/Common/GPS/GPSInfo.hh"
#include <unistd.h>
#include <vector>


namespace sbndaq
{
  class SpectraTime_generatorBase: public artdaq::CommandableFragmentGenerator
  {
  public:
    explicit SpectraTime_generatorBase(fhicl::ParameterSet const & ps);
    virtual ~SpectraTime_generatorBase();
    enum
    {
      GPS_MQ = 0xCDF
    };

  protected:

    bool getNext_(artdaq::FragmentPtrs & output) override;
    void start() override;
    void stop() override;
    void stopNoMutex() override;
    void stopAll();
    void init();

    // Message queue
    key_t daqMQ;
    int daqID;
    GPSInfo gpsInfo;

    // General
    uint32_t runNumber_;
    std::string device;
    uint32_t SequenceTimeWindowSize_;
    uint32_t eventCounter;
    std::atomic_bool running;

    // SpectraTimeConfiguration config_;

    fhicl::ParameterSet const ps_;

    SpectratimeEvent SpectraTimeTemp;

  private:
    typedef struct SpectraTimeBuffer
    {
      // fill this up once i understand it...
      //	std::deque<SpectratimeEvent> buffer;
      boost::circular_buffer<SpectratimeEvent> buffer;
      size_t   time_resets;
      int64_t  next_time_start;
      uint32_t overwritten_counter;
      int32_t  last_time_counter;
      SpectraTimeBuffer():buffer(boost::circular_buffer<SpectratimeEvent>(60)),
			  time_resets(0),
			  next_time_start(0),
			  overwritten_counter(0),
			  last_time_counter(-1){};
    } SpectraTimeBuffer_t;
    
    SpectraTimeBuffer_t SpectraTimeCircularBuffer_;

    bool getData();
    bool FillFragment(artdaq::FragmentPtrs &, bool clear_buffer=false);

    share::WorkerThreadUPtr GetData_thread_;
  };
}

#endif
