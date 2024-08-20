//
// sbndaq-artdaq/Generators/SBND/NevisTPC_generatorBase.hh
//

#ifndef _sbndaq_readout_Generators_NevisTPC_generatorBase
#define _sbndaq_readout_Generators_NevisTPC_generatorBase

#include <atomic>
#include <future>
#include <memory>

#include "artdaq-core/Data/Fragment.hh"
#include "artdaq/Generators/CommandableFragmentGenerator.hh"
#include "fhiclcpp/fwd.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "sbndaq-artdaq-core/Overlays/SBND/NevisTPCFragment.hh"
#include "sbndaq-artdaq/Generators/Common/workerThread.hh"
#include <boost/circular_buffer.hpp>
#include <unistd.h>
#include <vector>

namespace sbndaq {
class NevisTPC_generatorBase : public artdaq::CommandableFragmentGenerator {

public:
  explicit NevisTPC_generatorBase(fhicl::ParameterSet const &ps);
  virtual ~NevisTPC_generatorBase();

protected:
  bool getNext_(artdaq::FragmentPtrs &output) override;
  void start() override;
  virtual void runonsyncon() {};
  virtual void startFireCalibTrig(){};
  void stop() override;
  void stopNoMutex() override;
  void stopAll();
  void Initialize();

  // These functions MUST be defined by the derived classes
  virtual void ConfigureStart() = 0; // called in start()
  virtual void ConfigureStop() = 0;  // called in stop()

  // gets the data. Output is size of data filled. Input is FEM ID.
  virtual size_t GetFEMCrateData() = 0;
  virtual int GetDataSetup() { return 1; }
  virtual int GetDataComplete() { return 1; }

  enum {
    TERROR = TLVL_ERROR,
    TWARNING = TLVL_WARNING,
    TINFO = TLVL_INFO,
    TDEBUG = TLVL_DEBUG,
    TCONFIG = 9,
    TSTART = 10,
    TSTOP = 11,
    TSTATUS = 12,
    TGETNEXT = 13,
    TFILLFRAG = 14,
    TGETDATA = 24
  };

  enum { FEM_BASE_SLOT = 3 };

  uint32_t current_subrun_;
  size_t events_seen_;
  bool look_for_xmit_header_;
  bool use_xmit_;

  NevisTPCFragmentMetadata metadata_;
  fhicl::ParameterSet const ps_;

  uint32_t fSamplesPerChannel;
  uint32_t fNChannels;
  bool fUseCompression;
  uint32_t fTimeoutSec;

  std::vector<artdaq::Fragment::fragment_id_t> fragment_ids;
  std::vector<uint64_t> FEMIDs_;
  uint32_t RunNumber_;
  int32_t EventsPerSubrun_;

  int32_t _this_event;
  int32_t _subrun_event_0;

  typedef struct CircularBuffer {
    boost::circular_buffer<uint16_t> buffer;
    std::unique_ptr<std::mutex> mutexptr;

    CircularBuffer(uint32_t capacity)
        : buffer(boost::circular_buffer<uint16_t>(capacity)),
          mutexptr(new std::mutex) {
      Init();
    }
    CircularBuffer() { CircularBuffer(0); }

    void Init() {
      buffer.clear();
      mutexptr->unlock();
    }

    size_t Insert(size_t, std::unique_ptr<uint16_t[]> const &);
    size_t Erase(size_t);
  } CircularBuffer_t;

  uint32_t DMABufferSizeBytes_;
  std::unique_ptr<uint16_t[]> DMABuffer_;

  uint32_t CircularBufferSizeBytes_;
  CircularBuffer_t CircularBuffer_;
  bool GetData();
  share::WorkerThreadUPtr GetData_thread_;

  virtual bool FillFragment(artdaq::FragmentPtrs &, bool clear_buffer = false);

  int current_event,
      current_framenum; // for checking consistency between FEMs within a run
  bool desyncCrash;
};
} // namespace sbndaq

#endif
