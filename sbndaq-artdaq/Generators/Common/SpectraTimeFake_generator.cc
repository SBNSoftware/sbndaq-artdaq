//
// sbnddaq-readout/Generators/SpectraTimeFake_generator.cc
//

#include "sbndaq-artdaq/Generators/Common/SpectraTimeFake.hh"
#include "artdaq/Application/GeneratorMacros.hh"

sbndaq::SpectraTimeFake::SpectraTimeFake(fhicl::ParameterSet const & ps):
  SpectraTime_generatorBase(ps)
{
}

sbndaq::SpectraTimeFake::~SpectraTimeFake()
{
}

DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(sbndaq::SpectraTimeFake)
