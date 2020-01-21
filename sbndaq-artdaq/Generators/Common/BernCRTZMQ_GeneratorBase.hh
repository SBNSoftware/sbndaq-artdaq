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

    //gets the data. Output is size of data filled. Input is FEM ID.
    virtual size_t GetZMQData() = 0;
    virtual int    GetDataSetup() { return 1; }
    virtual int    GetDataComplete() { return 1; }

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

      FEBBuffer(uint32_t capacity, uint8_t mac5, uint16_t id)
	: buffer(EventBuffer_t(capacity)),
	  mutexptr(new std::mutex),
	  MAC5(mac5),
          fragment_id(id)
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

    size_t InsertIntoFEBBuffer(FEBBuffer_t &,size_t,size_t,size_t);
    size_t EraseFromFEBBuffer(FEBBuffer_t &, size_t const&);

    std::string GetFEBIDString(uint64_t const& id) const;
    void SendMetadataMetrics(BernCRTZMQFragmentMetadata const& m);
    void UpdateBufferOccupancyMetrics(uint64_t const& ,size_t const&) const;
    
    share::WorkerThreadUPtr GetData_thread_;

	//my new variable (AA: Federico?)
    //TODO: there is only a single set of variables for all FEBs read by a single board reader
    //as a result e.g. PPS events in any board will cause the GPS counter to advance. This need
    //to be rewritten
    size_t FragmentCounter_; //it counts the fragments in the buffer

    //AA: values read from the special last zeromq event, containing poll times
    uint64_t this_poll_start;
    uint64_t this_poll_end;
    uint64_t last_poll_start;
    uint64_t last_poll_end;
    int32_t  system_clock_deviation;

    uint64_t run_start_time;

  };
}

