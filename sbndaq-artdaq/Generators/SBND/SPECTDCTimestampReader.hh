#ifndef sbndaq_artdaq_Generators_SPECTDCTimestampReader_hh
#define sbndaq_artdaq_Generators_SPECTDCTimestampReader_hh

// Some C++ conventions used:

// -Append a "_" to every private member function and variable

#include "artdaq-core/Data/Fragment.hh"
#include "artdaq/Generators/CommandableFragmentGenerator.hh"
#include "fhiclcpp/fwd.h"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "sbndaq-artdaq-core/Overlays/SBND/TDCTimestampFragment.hh"

#include "SPECTDC/SPECTDC_Interface.hh"

#include <atomic>
#include <random>
#include <vector>

namespace share {
class WorkerThread;
}  // namespace share

namespace sbndaq {
class PoolBuffer;

class SPECTDCTimestampReader : public artdaq::CommandableFragmentGenerator {
 public:
  explicit SPECTDCTimestampReader(fhicl::ParameterSet const&);
  virtual ~SPECTDCTimestampReader();

 private:
  SPECTDCTimestampReader(SPECTDCTimestampReader const&) = delete;
  SPECTDCTimestampReader(SPECTDCTimestampReader&&) = delete;
  SPECTDCTimestampReader& operator=(SPECTDCTimestampReader const&) = delete;
  SPECTDCTimestampReader& operator=(SPECTDCTimestampReader&&) = delete;

  bool getNext_(artdaq::FragmentPtrs& frags) override;
  bool checkHWStatus_() override;
  bool getData_();
  void start() override;
  void stop() override;
  void requestStop();
  void stopNoMutex() override {}
  bool configure(fhicl::ParameterSet const&);

 private:
  uint64_t fragment_id_;
  std::unique_ptr<PoolBuffer> buffer_;
  std::unique_ptr<SPECTDCInterface::SPECCard> hardware_;
  std::unique_ptr<share::WorkerThread> data_thread_;
  TDCTimestampFragment::Metadata metadata_;
  bool configured_;
  uint64_t sleep_on_no_data_us_;
  uint64_t events_to_generate_;
  bool separate_data_thread_;
  bool stop_requested_;
};
}  // namespace sbndaq

#endif /* sbndaq_artdaq_Generators_SPECTDCTimestampReader_hh */
