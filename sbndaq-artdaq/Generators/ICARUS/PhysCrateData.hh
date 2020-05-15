#ifndef icarus_artdaq_Generators_PhysCrateData_hh
#define icarus_artdaq_Generators_PhysCrateData_hh

#include "sbndaq-artdaq/Generators/ICARUS/PhysCrate_GeneratorBase.hh"
#include <chrono>

#include "icarus-base/VetoUDP.h"
#include "hiredis/hiredis.h"

class PhysCrate;

namespace icarus {    

  class PhysCrateData : public icarus::PhysCrate_GeneratorBase {
    
  public:
    
    explicit PhysCrateData(fhicl::ParameterSet const & ps);
    ~PhysCrateData();

    enum TestPulseType{
      kDisable = 0,
      kExternal = 1,
      kInternal_Even = 2,
      kInternal_Odd = 3
    };
    
  private:
    
    void ConfigureStart(); //called in start()
    void ConfigureStop();  //called in stop()

    int  GetData();
    void FillStatPack(statpack&);
    bool Monitor();

    void InitializeHardware();
    BoardConf GetBoardConf();
    TrigConf GetTrigConf();

    void SetTestPulse();
    void SetDCOffset();
    void ForceClear();
    void ForceReset();

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
    share::WorkerThreadUPtr GetData_thread_;

    //for redis connection...
    redisContext *_redisCtxt;
    bool        _doRedis;
    std::string _redisHost;
    int _redisPort;

    //if not using trigger board, issue our own start
    bool _issueStart;

    //if we want to read board temps
    bool _readTemps;

 };
}


#endif /* icarus_artdaq_Generators_PhysCrateData_hh */
