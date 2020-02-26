#include "fhiclcpp/fwd.h"
#include "artdaq-core/Data/Fragment.hh" 
#include "artdaq/Generators/CommandableFragmentGenerator.hh"

#include "sbndaq-artdaq-core/Overlays/Common/BernCRTZMQFragment.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"

#include "BernCRTFEBConfiguration.hh"

#include <unistd.h>
#include <vector>
#include <deque>
#include <unordered_map>
#include <atomic>
#include <mutex>
#include <boost/circular_buffer.hpp>

#include "workerThread.hh"

namespace sbndaq {    

  class BernCRTZMQ_GeneratorBase : public artdaq::CommandableFragmentGenerator {
  public:
    explicit BernCRTZMQ_GeneratorBase(fhicl::ParameterSet const & ps);
    virtual ~BernCRTZMQ_GeneratorBase();

  protected:

    bool getNext_(artdaq::FragmentPtrs & output) override;
    void start() override;
    void stop() override;
    void stopNoMutex() override;

    std::vector<uint8_t> MAC5s_;
    size_t nFEBs() { return MAC5s_.size(); }
    std::unordered_map< uint8_t, BernCRTFEBConfiguration > feb_configuration; //first number is the mac address.

    std::size_t throttle_usecs_;        // Sleep at start of each call to getNext_(), in us
    std::size_t throttle_usecs_check_;  // Period between checks for stop/pause during the sleep (must be less than, and an integer divisor of, throttle_usecs_)

    //These functions MUST be defined by the derived classes
    virtual void ConfigureStart() = 0; //called in start()
    virtual void ConfigureStop() = 0;  //called in stop()

    //gets the data. Output is size of data filled.
    virtual size_t GetZMQData(bool restart) = 0;

    virtual void StartFebdrv() = 0;
    virtual uint64_t GetTimeSinceLastRestart() = 0;

    fhicl::ParameterSet const ps_;

    //These functions could be overwritten by the derived class
    virtual void Initialize();     //called in constructor
    virtual void Cleanup();        //called in destructor

    typedef boost::circular_buffer<BernCRTZMQDataPair> EventBuffer_t;

    std::unique_ptr<BernCRTZMQEvent[]> ZMQBufferUPtr;
    uint32_t ZMQBufferCapacity_;

  private:

    typedef struct FEBBuffer {

      EventBuffer_t               buffer;

      std::unique_ptr<std::mutex>  mutexptr;
      uint8_t                      MAC5;
      uint16_t                     fragment_id;
      uint32_t                     event_number; //for given FEB
      uint64_t                     last_accepted_timestamp;
      uint32_t                     omitted_events;

      FEBBuffer(uint32_t capacity, uint8_t mac5, uint16_t id)
	: buffer(EventBuffer_t(capacity)),
	  mutexptr(new std::mutex),
	  MAC5(mac5),
          fragment_id(id),
          event_number(0),
          last_accepted_timestamp(1), //use 1 as a flag in case events are omitted at the very beginning of the run
          omitted_events(0)
      { Init(); }
      FEBBuffer() { FEBBuffer(0, 0, 0); }
      void Init() {
	buffer.clear();
	mutexptr->unlock();
      }
    } FEBBuffer_t;

    std::unordered_map< uint8_t, FEBBuffer_t  > FEBBuffers_; //first number is the mac address.

    bool GetData();
    bool FillFragment(uint64_t const&, artdaq::FragmentPtrs &);

    size_t InsertIntoFEBBuffer(FEBBuffer_t &,size_t,size_t);
    size_t EraseFromFEBBuffer(FEBBuffer_t &, size_t const&);

    std::string GetFEBIDString(uint64_t const& id) const;
    void SendMetadataMetrics(BernCRTZMQFragmentMetadata const& m);
    void UpdateBufferOccupancyMetrics(uint64_t const& ,size_t const&) const;
    
    share::WorkerThreadUPtr GetData_thread_;

    //sequence id is unique for any fragment coming from this Fragment Generator
    uint32_t sequence_id_;

    //AA: values read from the special last zeromq event, containing poll times
    uint64_t this_poll_start;
    uint64_t this_poll_end;
    uint64_t last_poll_start;
    uint64_t last_poll_end;
    int32_t  system_clock_deviation;

    uint64_t run_start_time;
  };

  //workarounds for issues with FEBs, PPS
  bool omit_out_of_order_events_;
  bool omit_out_of_sync_events_;
  int32_t out_of_sync_tolerance_ns_;

  uint64_t febdrv_restart_period;
}

