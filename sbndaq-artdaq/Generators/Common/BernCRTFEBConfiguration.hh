#ifndef sbndaq_artdaq_Generators_Common_BernCRTFEBConfiguration_hh
#define sbndaq_artdaq_Generators_Common_BernCRTFEBConfiguration_hh

//#include "fhiclcpp/fwd.h"
#include "fhiclcpp/ParameterSet.h"
#include "BernCRT_TRACE_defines.h"

namespace sbndaq {

  class BernCRTFEBConfiguration  {
    public:

      BernCRTFEBConfiguration(fhicl::ParameterSet const & ps_, int iFEB);

      uint8_t * GetProbeBitStream();
      int       GetProbeBitStreamNBytes();
      uint8_t * GetSlowControlBitStream();
      int       GetSlowControlBitStreamNBytes();

    private:
      static const int PROBE_BITSTREAM_NBITS = 224;
      static const int SLOW_CONTROL_BITSTREAM_NBITS = 1144;
      uint8_t ProbeBitStream[PROBE_BITSTREAM_NBITS/8];
      uint8_t SlowControlBitStream[SLOW_CONTROL_BITSTREAM_NBITS/8];

      int ConvertASCIIToBitstream(std::string ASCIIBitStream, uint8_t *buffer, int nBits);
  };
}

#endif 
