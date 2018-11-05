//
// sbnddaq-readout/Generators/SpectraTimeData_generator.cc
//

#include "sbndaq-artdaq/Generators/Common/SpectraTimeData.hh"
#include "artdaq/Application/GeneratorMacros.hh"

sbndaq::SpectraTimeData::SpectraTimeData(fhicl::ParameterSet const & ps):
  SpectraTime_generatorBase(ps)
{
}

sbndaq::SpectraTimeData::~SpectraTimeData()
{
}

DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(sbndaq::SpectraTimeData)
