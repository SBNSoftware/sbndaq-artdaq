#ifndef sbndaq_artdaq_Generators_BernCRTSerialReader_hh
#define sbndaq_artdaq_Generators_BernCRTSerialReader_hh

#include "artdaq/Generators/CommandableFragmentGenerator.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "sbndaq-artdaq-core/Overlays/Common/BernCRTFragmentV2.hh"
#include "sbndaq-artdaq-core/Overlays/SBND/BernCRTFragmentSerial.hh"

#include <fstream>

namespace sbndaq {

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
  void start() override;
  void stop() override;
  void stopNoMutex() override {}

 private:
  std::string binary_file_path_;
  std::ifstream binary_file_;
};
}  // namespace sbndaq

#endif /* sbndaq_artdaq_Generators_BernCRTSerialReader_hh */
