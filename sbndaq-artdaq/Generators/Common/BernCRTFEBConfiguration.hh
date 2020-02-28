#ifndef sbndaq_artdaq_Generators_Common_BernCRTFEBConfiguration_hh
#define sbndaq_artdaq_Generators_Common_BernCRTFEBConfiguration_hh

#include "fhiclcpp/ParameterSet.h"
#include "trace.h"

namespace sbndaq {

  /**
   * Class to hold and manipulate bitstreams sent to BERN/CAEN DT5702.
   * It holds two bitstreams: Probe and SlowControl
   * In fact only SlowControl bitstream contains useful configuration
   * so most methods refer to that bitstream only
   */

  class BernCRTFEBConfiguration  {
    public:
      
      BernCRTFEBConfiguration(fhicl::ParameterSet const & ps_, int iFEB);
      BernCRTFEBConfiguration();
      
      uint8_t * GetProbeBitStream();
      int       GetProbeBitStreamNBytes();
      uint8_t * GetSlowControlBitStream();
      int       GetSlowControlBitStreamNBytes();
      std::string GetString(std::string separator=" # ");
      bool      GetHVOnPermission();
      int32_t   GetPPSOffset();
      
    private:
      static const int PROBE_BITSTREAM_NBITS = 224;
      static const int SLOW_CONTROL_BITSTREAM_NBITS = 1144;
      uint8_t ProbeBitStream[PROBE_BITSTREAM_NBITS/8];
      uint8_t SlowControlBitStream[SLOW_CONTROL_BITSTREAM_NBITS/8];
      bool hv_on_permission;
      int32_t PPS_offset; //cable delay in ns

      //constructors
      bool load_bitstream_format(fhicl::ParameterSet const & ps_, int iFEB);
      bool load_readable_format(fhicl::ParameterSet const & ps_, int MAC5);
      
      static int GetBit(unsigned int bit_number, uint8_t * bitstream, unsigned int nBits);
      int GetBit(unsigned int bit_number);
      static void SetBit(unsigned int bit_number, bool value, uint8_t * bitstream, unsigned int nBits);
      void SetBit(unsigned int bit_number, bool value);
      std::string BitsToASCII(unsigned int firstBit, unsigned int lastBit);
      std::string BitsToASCII(unsigned int bit_number);

      static int ASCIIToBitStream(std::string ASCIIBitStream, uint8_t * bitstream, unsigned int nBits);
      
      //Human readable configuration
      struct boolean_parameter {
        bool        value;
        uint16_t    bit;
        bool        inverted_logic; //if "true" corresponds to bit "0" in bit stream
        std::string comment;
      };
      
      struct numerical_parameter {
        uint16_t    value;
        uint16_t    max_value;
        uint16_t    first_bit;
        uint16_t    last_bit;
        std::string comment;
      };
      
      std::map <std::string, boolean_parameter> boolean_parameters;
      std::map <std::string, numerical_parameter> numerical_parameters;
      std::map <std::string, boolean_parameter> boolean_channel_parameters;
      std::map <std::string, numerical_parameter> numerical_channel_parameters;
      
      static std::vector<std::string> channel_parameter_names;
      
      void InitializeParameters();
      bool parameters_initialized;
  };
}

#endif 
