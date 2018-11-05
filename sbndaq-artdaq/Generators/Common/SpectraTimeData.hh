//
// sbnddaq-readout/Generators/SpectraTimeData_generator.hh
//

#ifndef sbndaq_artdaq_Generators_Common_SpectraTimeData_generator
#define sbndaq_artdaq_Generators_Common_SpectraTimeData_generator

#include "sbndaq-artdaq/Generators/Common/SpectraTime_generatorBase.hh"


namespace sbndaq
{    
  class SpectraTimeData: 
    public sbndaq::SpectraTime_generatorBase
  {
  public:
    explicit SpectraTimeData(fhicl::ParameterSet const & ps);
    virtual ~SpectraTimeData();

  protected:

  };
}

#endif
