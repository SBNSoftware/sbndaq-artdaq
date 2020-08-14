#ifndef sbndaq_artdaq_Generators_Common_BernCRTData_hh
#define sbndaq_artdaq_Generators_Common_BernCRTData_hh

#include "sbndaq-artdaq/Generators/Common/BernCRT_GeneratorBase.hh"

#include "febdrv.hh"

namespace sbndaq {

  class BernCRTData : public sbndaq::BernCRT_GeneratorBase {
  public:

    explicit BernCRTData(fhicl::ParameterSet const & ps);
    virtual ~BernCRTData();

  private:

    void ConfigureStart(); //called in start()
    void ConfigureStop();  //called in stop()


    size_t GetFEBData();

    void StartFebdrv();
    uint64_t GetTimeSinceLastRestart();
    std::chrono::time_point<std::chrono::system_clock> last_restart_time;

    FEBDRV febdrv;

    void feb_send_bitstreams(uint8_t mac5);
  };
}

#endif /* artdaq_demo_Generators_BernCRTData_hh */
