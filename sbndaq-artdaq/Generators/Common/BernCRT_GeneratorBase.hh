#ifndef sbndaq_artdaq_Generators_BernCRT_GeneratorBase_hh
#define sbndaq_artdaq_Generators_BernCRT_GeneratorBase_hh

#include "fhiclcpp/fwd.h"
#include "artdaq-core/Data/Fragment.hh" 
#include "artdaq/Generators/CommandableFragmentGenerator.hh"

#include "sbndaq-artdaq-core/Overlays/Common/BernCRTFragmentV2.hh"
#include "sbndaq-artdaq-core/Overlays/Common/BernCRTFragment.hh" //print_timestamp
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

  class BernCRT_GeneratorBase : public artdaq::CommandableFragmentGenerator {
  public:
    explicit BernCRT_GeneratorBase(fhicl::ParameterSet const & ps);
    virtual ~BernCRT_GeneratorBase();
    
  protected:
    
    typedef struct {
      std::vector<BernCRTHitV2> hits;
      BernCRTFragmentMetadataV2 metadata;
      uint64_t fragment_timestamp;
    } CRTBufferObject;

    typedef boost::circular_buffer<CRTBufferObject> FragmentBuffer_t;
    
    typedef struct FEB {
      FragmentBuffer_t                buffer;

      std::unique_ptr<std::mutex>  mutexptr;
      uint16_t                     fragment_id;
      uint64_t                     last_accepted_timestamp = 1; // 1 = a flag in case hits are omitted at the very beginning of the run
      uint64_t                     last_accepted_hit_number = 0;
      uint8_t                      last_lostfpga = 0;
      uint32_t                     last_poll_total_lostfpga = 0;
      uint16_t                     last_poll_total_read_hits = 0;

      uint64_t                     hit_number = 0;
      
      std::vector<BernCRTHitV2> hits;
      BernCRTFragmentMetadataV2 metadata;

      FEB(uint32_t capacity, uint16_t id)
        : buffer(FragmentBuffer_t(capacity)),
          mutexptr(new std::mutex),
          fragment_id(id)
      { Init(id&0xff); }
      FEB() { FEB(0, 0); }
      void Init(uint8_t mac5) {
        buffer.clear();
        metadata.set_mac5(mac5);
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

    uint64_t feb_restart_period_;
    uint32_t feb_poll_period_;
    int32_t fragment_period_;
    int firmware_flag_;       //flag to use older version of firmware
    
  private:

    bool GetData();
    void FillFragment(uint64_t const&, artdaq::FragmentPtrs &);

    size_t EraseFromFEBBuffer(FEB_t &, size_t const&);

    std::string GetFEBIDString(uint64_t const& id) const;
    void SendMetadataMetrics(BernCRTFragmentMetadataV2 const& m);
    void UpdateBufferOccupancyMetrics(uint64_t const& ,size_t const&) const;
    
    share::WorkerThreadUPtr GetData_thread_;

    //sequence id is unique for any fragment coming from this Fragment Generator
    uint32_t sequence_id_;

    uint64_t run_start_time;
    
    //workarounds
    uint64_t initial_delay_ns_;

  };

}

#endif //sbndaq_artdaq_Generators_BernCRT_GeneratorBase_hh
