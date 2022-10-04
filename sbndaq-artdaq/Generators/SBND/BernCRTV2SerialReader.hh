#ifndef sbndaq_artdaq_Generators_BernCRTV2SerialReader_hh
#define sbndaq_artdaq_Generators_BernCRTV2SerialReader_hh

#include "artdaq/Generators/CommandableFragmentGenerator.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "sbndaq-artdaq-core/Overlays/Common/BernCRTFragmentV2.hh"
#include "sbndaq-artdaq-core/Overlays/SBND/BernCRTFragmentV2Serial.hh"

#include <fstream>

namespace sbndaq {

class BernCRTV2SerialReader : public artdaq::CommandableFragmentGenerator {
 public:
  explicit BernCRTV2SerialReader(fhicl::ParameterSet const&);
  virtual ~BernCRTV2SerialReader();

  // private:
  BernCRTV2SerialReader(BernCRTV2SerialReader const&) = delete;
  BernCRTV2SerialReader(BernCRTV2SerialReader&&) = delete;
  BernCRTV2SerialReader& operator=(BernCRTV2SerialReader const&) = delete;
  BernCRTV2SerialReader& operator=(BernCRTV2SerialReader&&) = delete;

  bool getNext_(artdaq::FragmentPtrs& frags) override;
  void start() override;
  void stop() override;
  void stopNoMutex() override {}

 private:
  std::string binary_file_path_;
  std::ifstream binary_file_;

  unsigned fragment_counter;
  unsigned event_counter;
};
}  // namespace sbndaq

#endif /* sbndaq_artdaq_Generators_BernCRTV2SerialReader_hh */
