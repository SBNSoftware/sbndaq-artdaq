#ifndef _sbndaq_readout_Generators_NevisTPCFile_generator
#define _sbndaq_readout_Generators_NevisTPCFile_generator

/**
 * Nevis TPC File data generator
 *
 * Reads in a binary file from the Nevis test stand.
 *
 * Based on fake data NevisTPCData generator.
 *
 * Author: A. Mastbaum <mastbaum@uchicago.edu>, 2018/01/23
 */

#include <fstream>
#include "sbndaq-artdaq/Generators/SBND/NevisTPC/NevisTPC_generatorBase.hh"

namespace sbndaq {

class NevisTPCFile : public sbndaq::NevisTPC_generatorBase {
public:
  explicit NevisTPCFile(fhicl::ParameterSet const& _p) : NevisTPC_generatorBase(_p) {}
  virtual ~NevisTPCFile() {}

private:
  void ConfigureStart() override;
  void ConfigureStop() override;

  size_t GetFEMCrateData() override;

  uint32_t fChunkSize;  //!< Number of bytes to read at once
  int fWaitTime;  //!< Time to wait between events
  std::ifstream fInputFile;  //!< Input file
};

}  // namespace sbndaq

#endif  // _sbndaq_readout_Generators_NevisTPCFile_generator

