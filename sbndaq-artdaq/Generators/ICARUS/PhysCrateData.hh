#ifndef icarus_artdaq_Generators_PhysCrateData_hh
#define icarus_artdaq_Generators_PhysCrateData_hh

#include "sbndaq-artdaq/Generators/ICARUS/PhysCrate_GeneratorBase.hh"
#include <chrono>
#include "icarus-artdaq-base/VetoUDP.h"

class PhysCrate;

namespace icarus {    

  class PhysCrateData : public icarus::PhysCrate_GeneratorBase {
    
  public:
    
    explicit PhysCrateData(fhicl::ParameterSet const & ps);
    
  private:
    
    void ConfigureStart(); //called in start()
    void ConfigureStop();  //called in stop()

    int  GetData(size_t&,uint32_t*);       //called in getNext_()
    void FillStatPack(statpack&);
    bool Monitor();

    void InitializeHardware();
    BoardConf GetBoardConf();
    TrigConf GetTrigConf();

    std::unique_ptr<PhysCrate> physCr;
    
    std::chrono::high_resolution_clock::time_point _tloop_start;
    std::chrono::high_resolution_clock::time_point _tloop_end;
    std::chrono::duration<double> _tloop_duration;

    void UpdateDuration()
    { 
      _tloop_duration =
	std::chrono::duration_cast< std::chrono::duration<double> >(_tloop_end-_tloop_start);
    }

    std::string veto_host;
    int         veto_host_port;
    VetoUDP     veto_udp;
    bool        veto_state;
    void VetoOn();
    void VetoOff();
    void InitializeVeto();

    bool         VetoTest();
    bool         _doVetoTest;
    unsigned int _vetoTestPeriod;
    share::WorkerThreadUPtr _vetoTestThread;
    
 };
}


#endif /* icarus_artdaq_Generators_PhysCrateData_hh */
