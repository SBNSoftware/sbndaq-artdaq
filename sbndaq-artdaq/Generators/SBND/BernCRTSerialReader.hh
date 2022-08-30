#ifndef sbndaq_artdaq_Generators_BernCRTSerialReader_hh
#define sbndaq_artdaq_Generators_BernCRTSerialReader_hh

// Some C++ conventions used:

// -Append a "_" to every private member function and variable

#include "artdaq-core/Data/Fragment.hh"
#include "artdaq/Generators/CommandableFragmentGenerator.hh"
#include "fhiclcpp/fwd.h"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "sbndaq-artdaq-core/Overlays/Common/BernCRTFragmentV2.hh"
#include "sbndaq-artdaq-core/Overlays/Common/BernCRTFragmentSerial.hh"

#include <atomic>
#include <random>
#include <vector>

#include<fstream>

namespace share {
class WorkerThread;
}  // namespace share

namespace sbndaq {
class PoolBuffer;

class BernCRTSerialReader : public artdaq::CommandableFragmentGenerator {
 public:
  explicit BernCRTSerialReader(fhicl::ParameterSet const&);
  virtual ~BernCRTSerialReader();

  // private:
  BernCRTSerialReader(BernCRTSerialReader const&) = delete;
  BernCRTSerialReader(BernCRTSerialReader&&) = delete;
  BernCRTSerialReader& operator=(BernCRTSerialReader const&) = delete;
  BernCRTSerialReader& operator=(BernCRTSerialReader&&) = delete;

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
  std::unique_ptr<share::WorkerThread> data_thread_;
  bool configured_;
  uint64_t sleep_on_no_data_us_;
  uint64_t hardware_poll_interval_us_;
  uint64_t events_to_generate_;
  bool separate_data_thread_;
  bool separate_monitoring_thread_;
  uint64_t next_hardware_poll_time_us_;
  uint64_t next_status_report_time_us_;
  bool stop_requested_;
  std::string binary_file_path_;
  std::ifstream binary_file_;
};
}  // namespace sbndaq

#endif /* sbndaq_artdaq_Generators_BernCRTSerialReader_hh */
