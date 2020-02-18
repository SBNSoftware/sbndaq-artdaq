#ifndef sbndaq_artdaq_Generators_Common_BernCRTZMQData_hh
#define sbndaq_artdaq_Generators_Common_BernCRTZMQData_hh

#include "sbndaq-artdaq/Generators/Common/BernCRTZMQ_GeneratorBase.hh"


#include "zmq.h"

namespace sbndaq {    

  class BernCRTZMQData : public sbndaq::BernCRTZMQ_GeneratorBase {
  public:

    explicit BernCRTZMQData(fhicl::ParameterSet const & ps);
    virtual ~BernCRTZMQData();

  private:

    void ConfigureStart(); //called in start()
    void ConfigureStop();  //called in stop()

    size_t GetZMQData();

    void StartFebdrv();
    uint64_t GetTimeSinceLastRestart();
    std::chrono::time_point<std::chrono::system_clock> last_restart_time;

    std::string zmq_listening_port_;
    std::string zmq_data_pub_port_;
    void*  zmq_context_;
    void*  zmq_subscriber_;
    void*  zmq_requester_;

    enum feb_command { DAQ_BEG, DAQ_END, BIAS_ON, BIAS_OF, GETINFO };
    void febctl(feb_command command, uint8_t mac5 = 255);

    void feb_send_bitstreams(uint8_t mac5);
  };
}

#endif /* artdaq_demo_Generators_BernCRTZMQData_hh */
