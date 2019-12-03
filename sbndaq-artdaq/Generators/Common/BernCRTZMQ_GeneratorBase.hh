#include "fhiclcpp/fwd.h"
#include "artdaq-core/Data/Fragment.hh" 
#include "artdaq/Generators/CommandableFragmentGenerator.hh"

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
    uint32_t SequenceTimeWindowSize_; //in nanoseconds

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
	  timebuffer(EventTimeBuffer_t(capacity)), //TODO: note, this buffer, and the following ones are used only in the code that's commented atm
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

    std::unordered_map< uint64_t, FEBBuffer_t  > FEBBuffers_; //TODO possibly we can optimize it by limiting MAC address to uint8_t
    uint32_t FEBBufferCapacity_;

    uint32_t SeqIDMinimumSec_;

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
    size_t GPSCounter_; // it counts how many GPS have occurred
    size_t event_in_clock; // it counts how many events are within a clock of the FEB
    size_t GPS_time; // time past from the very beginning of the DAQ_BEG - with respect to the GPS-PPS

    timeval time_last_poll_start;
    timeval time_last_poll_finished;

    uint32_t feb_event_count; //AA: number of events in a single poll (?)
    uint32_t start_time_metadata_s;
    uint32_t start_time_metadata_ns;
    uint32_t time_poll_start_metadata_s;
    uint32_t time_poll_start_metadata_ns;
    uint32_t time_poll_finish_metadata_s;
    uint32_t time_poll_finish_metadata_ns;
    uint32_t time_last_poll_start_metadata_s;
    uint32_t time_last_poll_start_metadata_ns;
    uint32_t time_last_poll_finish_metadata_s;
    uint32_t time_last_poll_finish_metadata_ns;
    uint32_t fragment_fill_time_metadata_s;
    uint32_t fragment_fill_time_metadata_ns;

    std::vector<uint32_t> time_poll_start_store_sec;
    std::vector<uint32_t> time_poll_start_store_nanosec;
    std::vector<uint32_t> time_poll_finish_store_sec;
    std::vector<uint32_t> time_poll_finish_store_nanosec;
    std::vector<uint32_t> time_last_poll_start_store_sec;
    std::vector<uint32_t> time_last_poll_start_store_nanosec;
    std::vector<uint32_t> time_last_poll_finish_store_sec;
    std::vector<uint32_t> time_last_poll_finish_store_nanosec;
  };
}

