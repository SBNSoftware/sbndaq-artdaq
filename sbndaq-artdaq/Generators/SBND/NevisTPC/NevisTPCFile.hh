#ifndef _sbnddaq_readout_Generators_NevisTPCFile_generator
#define _sbnddaq_readout_Generators_NevisTPCFile_generator

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
#include "sbnddaq-readout/Generators/NevisTPC/NevisTPC_generatorBase.hh"

namespace sbnddaq {

class NevisTPCFile : public sbnddaq::NevisTPC_generatorBase {
public:
  explicit NevisTPCFile(fhicl::ParameterSet const& _p) : NevisTPC_generatorBase(_p) {}
  virtual ~NevisTPCFile() {}

private:
  void ConfigureStart();
  void ConfigureStop();

  size_t GetFEMCrateData();

  uint32_t fChunkSize;  //!< Number of bytes to read at once
  int fWaitTime;  //!< Time to wait between events
  std::ifstream fInputFile;  //!< Input file
};

}  // namespace sbnddaq

#endif  // _sbnddaq_readout_Generators_NevisTPCFile_generator

