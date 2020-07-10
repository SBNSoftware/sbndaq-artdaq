#ifndef sbndaq_artdaq_Generators_BernCRTZMQ_GeneratorBase_hh
#define sbndaq_artdaq_Generators_BernCRTZMQ_GeneratorBase_hh

#include "fhiclcpp/fwd.h"
#include "artdaq-core/Data/Fragment.hh" 
#include "artdaq/Generators/CommandableFragmentGenerator.hh"

#include "sbndaq-artdaq-core/Overlays/Common/BernCRTZMQFragment.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"

#include "BernCRTFEBConfiguration.hh"
// #include "febdrv.hh"

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
    
    typedef boost::circular_buffer<BernCRTZMQDataPair> EventBuffer_t;
    
    typedef struct FEB {
      EventBuffer_t                buffer;

      std::unique_ptr<std::mutex>  mutexptr;
      uint8_t                      MAC5;
      uint16_t                     fragment_id;
      uint32_t                     event_number; //for given FEB
      uint64_t                     last_accepted_timestamp;
      uint32_t                     omitted_events;
      
      
      sbndaq::BernCRTZMQEvent event;
      BernCRTZMQFragmentMetadata metadata;

      FEB(uint32_t capacity, uint8_t mac5, uint16_t id)
        : buffer(EventBuffer_t(capacity)),
          mutexptr(new std::mutex),
          MAC5(mac5),
          fragment_id(id),
          event_number(0),
          last_accepted_timestamp(1), //use 1 as a flag in case events are omitted at the very beginning of the run
          omitted_events(0)
      { Init(); }
      FEB() { FEB(0, 0, 0); }
      void Init() {
        buffer.clear();
        mutexptr->unlock();
      }
    } FEB_t;
  
    std::unordered_map< uint8_t, FEB_t > FEBs_; //first number is the mac address.
    

    bool getNext_(artdaq::FragmentPtrs & output) override;
    void start() override;
    void stop() override;
    void stopNoMutex() override;

    std::vector<uint8_t> MAC5s_; //list of FEB MAC addresses defined in the FHiCL file
    size_t nFEBs() { return MAC5s_.size(); }
    std::unordered_map< uint8_t, BernCRTFEBConfiguration > feb_configuration; //first number is the mac address.

    std::size_t throttle_usecs_;        // Sleep at start of each call to getNext_(), in us
    std::size_t throttle_usecs_check_;  // Period between checks for stop/pause during the sleep (must be less than, and an integer divisor of, throttle_usecs_)

    //These functions MUST be defined by the derived classes
    virtual void ConfigureStart() = 0; //called in start()
    virtual void ConfigureStop() = 0;  //called in stop()


    //gets the data.
    virtual size_t GetFEBData() = 0;

    virtual void StartFebdrv() = 0;
    virtual uint64_t GetTimeSinceLastRestart() = 0;

    fhicl::ParameterSet const ps_;

    //These functions could be overwritten by the derived class
    virtual void Initialize();     //called in constructor
    virtual void Cleanup();        //called in destructor
    
    int64_t steady_clock_offset; //difference between system and steady clock
    
  private:


    bool GetData();
    bool FillFragment(uint64_t const&, artdaq::FragmentPtrs &);

    size_t EraseFromFEBBuffer(FEB_t &, size_t const&);

    std::string GetFEBIDString(uint64_t const& id) const;
    void SendMetadataMetrics(BernCRTZMQFragmentMetadata const& m);
    void UpdateBufferOccupancyMetrics(uint64_t const& ,size_t const&) const;
    
    share::WorkerThreadUPtr GetData_thread_;

    //sequence id is unique for any fragment coming from this Fragment Generator
    uint32_t sequence_id_;

    uint64_t run_start_time;
  };

  //workarounds for issues with FEBs, PPS
  bool omit_out_of_order_events_;
  bool omit_out_of_sync_events_;
  int32_t out_of_sync_tolerance_ns_;

  uint64_t feb_restart_period_;
  
  uint32_t feb_poll_period_;
}

#endif //sbndaq_artdaq_Generators_BernCRTZMQ_GeneratorBase_hh
