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
    int    GetDataSetup();
    int    GetDataComplete();

    int    zmq_data_receive_timeout_ms_;
    std::string zmq_data_pub_port_;
    void*  zmq_context_;
    void*  zmq_subscriber_;

    enum feb_command { DAQ_BEG, DAQ_END, BIAS_ON, BIAS_OF, GETINFO };
    
    void febctl(feb_command command, unsigned char mac5);

    const int PROBE_BITSTREAM_LENGTH = 224;
    const int SLOW_CONTROL_BITSTREAM_LENGTH = 1144;
    void feb_send_bitstreams(char * Probe_bitStream, char * SlowControl_bitStream, unsigned char mac5);

  };
}

#endif /* artdaq_demo_Generators_BernCRTZMQData_hh */
