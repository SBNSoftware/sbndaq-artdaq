//
// sbndaq-artdaq/Generators/SBND/NevisTPC_generatorBase.hh
//

#ifndef _sbnddaq_readout_Generators_NevisTPC_generatorBase
#define _sbnddaq_readout_Generators_NevisTPC_generatorBase

#include <memory>
#include <atomic>
#include <future>

#include <boost/circular_buffer.hpp>
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "fhiclcpp/fwd.h"
#include "artdaq-core/Data/Fragment.hh"
#include "artdaq/Application/CommandableFragmentGenerator.hh"
#include "sbndaq-artdaq/Generators/Common/workerThread.hh"
#include "sbndaq-artdaq-core/Overlays/SBND/NevisTPCFragment.hh"
#include <unistd.h>
#include <vector>


namespace sbndaq
{
  class NevisTPC_generatorBase: public artdaq::CommandableFragmentGenerator
  {
    
  public:
    
    explicit NevisTPC_generatorBase(fhicl::ParameterSet const & ps);
    virtual ~NevisTPC_generatorBase();
    
  protected:
    
    bool getNext_(artdaq::FragmentPtrs & output) override;
    void start() override;
    void stop() override;
    void stopNoMutex() override;
    void stopAll();
    void Initialize();
    
    //These functions MUST be defined by the derived classes
    virtual void ConfigureStart() = 0; //called in start()
    virtual void ConfigureStop() = 0;  //called in stop()
    
    //gets the data. Output is size of data filled. Input is FEM ID.	 	
    virtual size_t GetFEMCrateData() = 0;
    virtual int    GetDataSetup() { return 1; }
    virtual int    GetDataComplete() { return 1; }
    
    enum {
      TERROR=0,
      TWARNING=1,
      TINFO=2,
      TDEBUG=3,
      TCONFIG=4,
      TSTART=5,
      TSTOP=6,
      TSTATUS=7,
      TGETNEXT=16,
      TFILLFRAG=17,
      TGETDATA=24
    };
    
    uint32_t current_subrun_;
    size_t events_seen_;
    bool look_for_xmit_header_;
    
    NevisTPCFragmentMetadata metadata_;
    fhicl::ParameterSet const ps_;
    
    std::vector<uint64_t> FEMIDs_;
    uint32_t RunNumber_;
    uint32_t EventsPerSubrun_;
    
    typedef struct CircularBuffer{
      boost::circular_buffer<uint16_t> buffer;
      std::unique_ptr<std::mutex> mutexptr;
      
      CircularBuffer(uint32_t capacity): buffer( boost::circular_buffer<uint16_t>(capacity) ),mutexptr(new std::mutex){  Init();	}
      CircularBuffer(){ CircularBuffer(0); }
      
      void Init(){
	buffer.clear();
	mutexptr->unlock();
      }
      
      size_t Insert(size_t,std::unique_ptr<uint16_t[]>  const& );
      size_t Erase(size_t);
    } CircularBuffer_t;
    
    uint32_t DMABufferSizeBytes_;
    std::unique_ptr<uint16_t[]> DMABuffer_;
    
    uint32_t CircularBufferSizeBytes_;
    CircularBuffer_t CircularBuffer_;
    bool GetData();
    share::WorkerThreadUPtr GetData_thread_;
    
    //    bool FillFragment(artdaq::FragmentPtrs &,bool clear_buffer=false);
    bool FillFragment(artdaq::FragmentPtrs &);
    
  };
}

#endif
