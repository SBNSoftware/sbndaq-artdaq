#include "fhiclcpp/fwd.h"
#include "artdaq-core/Data/Fragment.hh" 
#include "artdaq/Application/CommandableFragmentGenerator.hh"

#include "sbndaq-artdaq-core/Overlays/Common/BernCRTZMQFragment.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"

#include <unistd.h>
#include <vector>
#include <deque>
#include <unordered_map>
#include <atomic>
#include <mutex>
#include <boost/circular_buffer.hpp>
#include <sys/time.h>

#include "workerThread.hh"

#define FEB_OVRFLW_TIME 73741824 //2^30 - 1e9

namespace sbndaq {    

  class BernCRTZMQ_GeneratorBase : public artdaq::CommandableFragmentGenerator{
  public:
    explicit BernCRTZMQ_GeneratorBase(fhicl::ParameterSet const & ps);
    virtual ~BernCRTZMQ_GeneratorBase();

    //private:
  protected:

    bool getNext_(artdaq::FragmentPtrs & output) override;
    void start() override;
    void stop() override;
    void stopNoMutex() override;

    uint32_t RunNumber_;
    uint64_t SubrunTimeWindowSize_;
    uint32_t SequenceTimeWindowSize_; //in nanoseconds

    uint32_t ReaderID_;
    uint32_t nChannels_;
    uint32_t nADCBits_;

    std::vector<uint64_t> FEBIDs_;
    size_t nFEBs() { return FEBIDs_.size(); }

    std::vector<uint32_t> MaxTimeDiffs_;

    std::size_t throttle_usecs_;        // Sleep at start of each call to getNext_(), in us
    std::size_t throttle_usecs_check_;  // Period between checks for stop/pause during the sleep (must be less than, and an integer divisor of, throttle_usecs_)

    uint32_t current_subrun_;
    size_t event_number_;


    //These functions MUST be defined by the derived classes
    virtual void ConfigureStart() = 0; //called in start()
    virtual void ConfigureStop() = 0;  //called in stop()

    //gets the data. Output is size of data filled. Input is FEM ID.
    virtual size_t GetZMQData() = 0;
    virtual int    GetDataSetup() { return 1; }
    virtual int    GetDataComplete() { return 1; }

    size_t last_read_data_size_;
    int    last_status_;

  protected:

    BernCRTZMQFragmentMetadata metadata_;
    fhicl::ParameterSet const ps_;

    //These functions could be overwritten by the derived class
    virtual void Initialize();     //called in constructor
    virtual void Cleanup();        //called in destructor

    typedef boost::circular_buffer<BernCRTZMQEvent> EventBuffer_t;
    typedef boost::circular_buffer< std::pair<timeval,timeval> > EventTimeBuffer_t;
    typedef boost::circular_buffer<unsigned int> EventsDroppedBuffer_t;
    typedef boost::circular_buffer<uint64_t>     EventsCorrectedTimeBuffer_t;
    //typedef std::deque<BernCRTZMQEvent> ZMQEventBuffer_t;

    typedef std::chrono::high_resolution_clock hires_clock;

    std::unique_ptr<BernCRTZMQEvent[]> ZMQBufferUPtr;
    uint32_t ZMQBufferCapacity_;
    uint32_t ZMQBufferSizeBytes_;

  private:

    typedef struct FEBBuffer{

      EventBuffer_t               buffer;
      EventTimeBuffer_t           timebuffer;
      EventsDroppedBuffer_t       droppedbuffer;
      EventsCorrectedTimeBuffer_t correctedtimebuffer;

      std::unique_ptr<std::mutex>  mutexptr;
      uint32_t                     overwritten_counter;
      uint32_t                     max_time_diff;
      uint64_t                     id;
      timeval                      last_timenow;

      FEBBuffer(uint32_t capacity, uint32_t td, uint64_t i)
	: buffer(EventBuffer_t(capacity)),
	  timebuffer(EventTimeBuffer_t(capacity)),
	  droppedbuffer(EventsDroppedBuffer_t(capacity)),
	  correctedtimebuffer(EventsCorrectedTimeBuffer_t(capacity)),
	  mutexptr(new std::mutex),
	  overwritten_counter(0),
	  max_time_diff(td),
	  id(i)
      { Init(); }
      FEBBuffer() { FEBBuffer(0,10000000,0); }
      void Init() {
	buffer.clear();
	timebuffer.clear();
	correctedtimebuffer.clear();
	mutexptr->unlock();
	overwritten_counter = 0;
	last_timenow.tv_sec = 0;
	last_timenow.tv_usec = 0;
      }
    } FEBBuffer_t;

    std::chrono::system_clock insertTimer_;

    std::unordered_map< uint64_t, FEBBuffer_t  > FEBBuffers_;
    uint32_t FEBBufferCapacity_;
    uint32_t FEBBufferSizeBytes_;

    uint32_t SeqIDMinimumSec_;

    bool GetData();
    bool FillFragment(uint64_t const&, artdaq::FragmentPtrs &,bool clear_buffer=false);

    size_t InsertIntoFEBBuffer(FEBBuffer_t &,size_t,size_t,size_t);
    size_t EraseFromFEBBuffer(FEBBuffer_t &, size_t const&);

    std::string GetFEBIDString(uint64_t const& id) const;
    void SendMetadataMetrics(BernCRTZMQFragmentMetadata const& m);
    void UpdateBufferOccupancyMetrics(uint64_t const& ,size_t const&) const;
    
    share::WorkerThreadUPtr GetData_thread_;

  };
}
