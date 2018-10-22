#ifndef icarus_artdaq_Generators_PhysCrateFakeData_hh
#define icarus_artdaq_Generators_PhysCrateFakeData_hh

#include "sbndaq-artdaq/Generators/ICARUS/PhysCrate_GeneratorBase.hh"
#include <random>

namespace icarus {    

  class PhysCrateFakeData : public icarus::PhysCrate_GeneratorBase {
    
  public:
    
    explicit PhysCrateFakeData(fhicl::ParameterSet const & ps);
    
  private:
    
    void ConfigureStart(); //called in start()
    void ConfigureStop();  //called in stop()

    int  GetData(size_t&,uint32_t*);       //called in getNext_()
    void FillStatPack(statpack&);
    bool Monitor();

    std::mt19937 engine_;
    std::unique_ptr<std::uniform_int_distribution<int>> uniform_distn_;
    uint32_t event_number_;

  };
}


#endif /* icarus_artdaq_Generators_PhysCrateFakeData_hh */
