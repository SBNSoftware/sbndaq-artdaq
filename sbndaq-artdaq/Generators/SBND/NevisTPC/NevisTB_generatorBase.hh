//
// sbndaq-artdaq/Generators/SBND/NevisTB_generatorBase.hh
//

#ifndef _sbndaq_readout_Generators_NevisTB_generatorBase
#define _sbndaq_readout_Generators_NevisTB_generatorBase

#include <memory>
#include <atomic>
#include <future>

#include <boost/circular_buffer.hpp>
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "fhiclcpp/fwd.h"
#include "artdaq-core/Data/Fragment.hh"
#include "artdaq/Generators/CommandableFragmentGenerator.hh"
#include "sbndaq-artdaq/Generators/Common/workerThread.hh"
#include "sbndaq-artdaq-core/Overlays/SBND/NevisTBFragment.hh"
#include <unistd.h>
#include <vector>
#include <zmq.hpp>

namespace sbndaq
{
  class NevisTB_generatorBase: public artdaq::CommandableFragmentGenerator
  {
  public:
    
    explicit NevisTB_generatorBase(fhicl::ParameterSet const & ps);
    virtual ~NevisTB_generatorBase();
    
    zmq::context_t rec_context;
    zmq::socket_t _zmqGPSSubscriber;
    
    class GPSstamp {
    public:
    GPSstamp( uint32_t new_frame, uint16_t new_sample, uint16_t new_div ) 
      : gps_frame(new_frame),
        gps_sample(new_sample),
        gps_sample_div(new_div)
        {}
    
    ~GPSstamp() {}
    uint32_t gps_frame; // Frame number when GPS pulse was received
    uint16_t gps_sample; // 2 MHz sample when GPS pulse was received
    uint16_t gps_sample_div; // 16 MHz sample when GPS pulse was received
    };  

  protected:
    
    bool getNext_(artdaq::FragmentPtrs & output) override;
    void start() override;
    void stop() override;
    void stopNoMutex() override;
    void stopAll();
    void Initialize();
    bool GPSinitialized;
    //These functions MUST be defined by the derived classes
    virtual void ConfigureNTBStart() = 0; //called in start()
    virtual void ConfigureNTBStop() = 0;  //called in stop()

    //gets the data. Output is size of data filled. Input is FEM ID. 
    virtual size_t GetNevisTBData() = 0; 
    virtual int    GetDataSetup() { return 1; }
    virtual int    GetDataComplete() { return 1; }

    enum {
      TERROR    = TLVL_ERROR,
      TWARNING  = TLVL_WARNING,
      TINFO     = TLVL_INFO,
      TDEBUG    = TLVL_DEBUG,
      TCONFIG=9,
      TSTART=10,
      TSTOP=11,
      TSTATUS=12,
      TGETNEXT=13,
      TFILLFRAG=14,
      TGETDATA=24
    };

    uint32_t current_subrun_;
    size_t events_seen_;

    NevisTBFragmentMetadata ntbmetadata_;
    fhicl::ParameterSet const ps_;

    uint32_t RunNumber_;
    int32_t EventsPerSubrun_;

    int32_t _this_event;
    int32_t _subrun_event_0;
    typedef struct CircularBuffer{
      boost::circular_buffer<uint16_t> buffer;
      std::unique_ptr<std::mutex> mutexptr;
      
      CircularBuffer(uint32_t capacity): buffer( boost::circular_buffer<uint16_t>(capacity) ),mutexptr(new std::mutex){  Init();}
      CircularBuffer(){ CircularBuffer(0); }
      
      void Init(){
	buffer.clear();
	mutexptr->unlock();
      }


      size_t Insert(size_t,std::unique_ptr<uint16_t[]>  const& );
      size_t Erase(size_t);
    } CircularBuffer_t;
    
    uint32_t DMABufferSizeBytesNTB_;
    std::unique_ptr<uint16_t[]> DMABufferNTB_;
    
    uint32_t CircularBufferSizeBytesNTB_;
    CircularBuffer_t CircularBufferNTB_;

    bool GetNTBData();
    share::WorkerThreadUPtr GetNTBData_thread_;

    bool GPStime();
    share::WorkerThreadUPtr GPStime_thread_;
    void setGPSstamp(GPSstamp);

    virtual bool FillNTBFragment(artdaq::FragmentPtrs &,bool clear_buffer=false);
    
    int current_ntbevent, current_ntbframenum; // for checking consistency between FEMs within a run
    bool desyncCrash;
    uint64_t pseudo_ntbfragment; 

    uint32_t GPSframe;
    uint16_t GPSsample;
    uint16_t GPSdiv;

      };
}

#endif
