#include "fhiclcpp/fwd.h"
#include "artdaq-core/Data/Fragment.hh" 
#include "artdaq/Generators/CommandableFragmentGenerator.hh"

#include "sbndaq-artdaq-core/Overlays/ICARUS/PhysCrateFragment.hh"
#include "sbndaq-artdaq-core/Overlays/ICARUS/PhysCrateStatFragment.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
//#include "icarus-base/packs.h"
#include <unistd.h>
#include <vector>
#include <atomic>

#include "sbndaq-artdaq/Generators/Common/CircularBuffer.hh"
#include "sbndaq-artdaq/Generators/Common/workerThread.hh"

namespace icarus {    

  class PhysCrate_GeneratorBase : public artdaq::CommandableFragmentGenerator{
  public:
    explicit PhysCrate_GeneratorBase(fhicl::ParameterSet const & ps);
    virtual ~PhysCrate_GeneratorBase();

    //private:
  protected:

    bool getNext_(artdaq::FragmentPtrs & output) override;
    void start() override;
    void stop() override;

    void stopNoMutex() override { stop(); }
    
    uint32_t RunNumber_;
    
    uint32_t SamplesPerChannel_;
    uint8_t  nADCBits_;
    uint16_t ChannelsPerBoard_;
    uint16_t nBoards_;
    uint32_t CompressionScheme_;

    uint8_t  CrateID_;
    std::vector<PhysCrateFragmentMetadata::id_t> BoardIDs_;
    bool assignBoardID_;

    std::vector<uint8_t> BoardTemps1_; //value of temp sensor 1 on each board
    std::vector<uint8_t> BoardTemps2_; //value of temp sensor 2 on each board

    std::size_t throttle_usecs_;        // Sleep at start of each call to getNext_(), in us
    std::size_t throttle_usecs_check_;  // Period between checks for stop/pause during the sleep (must be less than, and an integer divisor of, throttle_usecs_)

    uint32_t current_subrun_;
    size_t event_number_;
    size_t least_data_block_bytes_;


    //These functions MUST be defined by the derived classes
    virtual void ConfigureStart() = 0; //called in start()
    virtual void ConfigureStop() = 0;  //called in stop()
    virtual int  GetData() = 0;
    virtual void FillStatPack(statpack&) = 0; //called in getNext_()
    virtual bool Monitor() = 0; //called as separate thread

    size_t   last_read_data_size_;
    int      last_status_;
    statpack last_stat_pack_;

    std::vector<int> pcieLinks_;

    size_t event_offset_;
    size_t packSize_zero_counter_;
    
  protected:

    PhysCrateFragmentMetadata metadata_;
    fhicl::ParameterSet const ps_;

    //These functions could be overwritten by the derived class
    virtual void Initialize();     //called in constructor
    virtual void Cleanup();        //called in destructor

    sbndaq::CircularBuffer<uint16_t> fCircularBuffer;
    size_t fCircularBufferSize;

    uint32_t fTimeOffsetNanoSec; //offset to be used for fragment timestamp    

  private:
    
    share::WorkerThreadUPtr Monitor_thread_;

    std::chrono::high_resolution_clock::time_point _tloop_getnext_start;
    std::chrono::high_resolution_clock::time_point _tloop_getnext_end;
    
  };
}

//#endif /* icarus_artdaq_Generators_PhysCrate_hh */
