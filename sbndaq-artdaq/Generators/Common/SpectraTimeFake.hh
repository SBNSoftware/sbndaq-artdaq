//
// sbnddaq-readout/Generators/SpectraTimeFake_generator.hh
//

#ifndef sbndaq_artdaq_Generators_Common_SpectraTimeFake_generator
#define sbndaq_artdaq_Generators_Common_SpectraTimeFake_generator

#include "sbndaq-artdaq/Generators/Common/SpectraTime_generatorBase.hh"


namespace sbndaq
{    
  class SpectraTimeFake: 
    public sbndaq::SpectraTime_generatorBase
  {
  public:
    explicit SpectraTimeFake(fhicl::ParameterSet const & ps);
    virtual ~SpectraTimeFake();

  protected:

  };
}

#endif
