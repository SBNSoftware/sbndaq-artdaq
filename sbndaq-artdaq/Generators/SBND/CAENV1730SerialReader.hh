#ifndef sbndaq_artdaq_Generators_CAENV1730SerialReader_hh
#define sbndaq_artdaq_Generators_CAENV1730SerialReader_hh

#include "artdaq/Generators/CommandableFragmentGenerator.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "sbndaq-artdaq-core/Overlays/Common/CAENV1730Fragment.hh"
#include "sbndaq-artdaq-core/Overlays/SBND/CAENV1730FragmentSerial.hh"

#include <fstream>

namespace sbndaq {

class CAENV1730SerialReader : public artdaq::CommandableFragmentGenerator {
 public:
  explicit CAENV1730SerialReader(fhicl::ParameterSet const&);
  virtual ~CAENV1730SerialReader();

  // private:
  CAENV1730SerialReader(CAENV1730SerialReader const&) = delete;
  CAENV1730SerialReader(CAENV1730SerialReader&&) = delete;
  CAENV1730SerialReader& operator=(CAENV1730SerialReader const&) = delete;
  CAENV1730SerialReader& operator=(CAENV1730SerialReader&&) = delete;

  bool getNext_(artdaq::FragmentPtrs& frags) override;
  void start() override;
  void stop() override;
  void stopNoMutex() override {}

 private:
  std::string binary_file_path_;
  std::ifstream binary_file_;
};
}  // namespace sbndaq

#endif /* sbndaq_artdaq_Generators_CAENV1730SerialReader_hh */
