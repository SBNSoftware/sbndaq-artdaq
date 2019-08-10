#include "BernCRTFEBConfiguration.hh"

sbndaq::BernCRTFEBConfiguration::BernCRTFEBConfiguration(fhicl::ParameterSet const & ps_, int iFEB) {
  if(!ConvertASCIIToBitstream(
        ps_.get<std::string>("CITIROC_Probe_bitStream"),
        ProbeBitStream,
        PROBE_BITSTREAM_LENGTH)) {   
    TRACE(TR_ERROR, std::string("BernCRTFEBConfiguration::") + __func__ + " Failed to load PROBE bit stream");
  }
  if(!ConvertASCIIToBitstream(
        ps_.get<std::string>("CITIROC_SlowControl_bitStream"+std::to_string(iFEB)),
        SlowControlBitStream,
        SLOW_CONTROL_BITSTREAM_LENGTH)) {   
    TRACE(TR_ERROR, std::string("BernCRTFEBConfiguration::") + __func__ + " Failed to load Slow Control bit stream");
  }
}


int sbndaq::BernCRTFEBConfiguration::ConvertASCIIToBitstream(std::string ASCIIBitStream, uint8_t *buffer, int length) {
  /**
   * Converts bitstream saved in ASCII format to an actual bitstream
   * Bitstream is returned via argument buffer
   * Returns false if the number of read bits differs from the parameter length
   * The format of the ASCII stream is the following:
   * Read '0' and '1' characters, ignoring spaces until you encounter character different than '0', '1' or ' ', then skip to the next line.
   */

  memset(buffer,0,length); //reset buffer
  int read_bits = 0;
  std::istringstream iASCIIBitStream(ASCIIBitStream);
  std::string line;
  while (std::getline(iASCIIBitStream, line)) { //loop over lines
    for(char& c : line) { //loop over characters
      if(c == ' ') continue; //ignore blank characters
      if(c == '0' || c == '1') { //encode the bit into the bitstream
        const int byte = read_bits / 8;
        const int bit = read_bits % 8;

        ++read_bits;
        if(read_bits > length) {
          TRACE(TR_WARNING, std::string("BernCRTFEBConfiguration::") + __func__ + " too long bitstream!!!");
          memset(buffer,0,length); //reset buffer
          return false;
        }

        if(c == '1')  buffer[byte] |= 1 << bit;
      }
      else break; //if the character is not a space, '0' or '1', go to next line
    }
  }

  if(read_bits < length) {
    TRACE(TR_WARNING, std::string("BernCRTFEBConfiguration::") + __func__ + " too short bitstream!!!");
    memset(buffer,0,length); //reset buffer
    return false;
  }

  return true;
}


uint8_t * sbndaq::BernCRTFEBConfiguration::GetProbeBitStream() { return ProbeBitStream; }
int       sbndaq::BernCRTFEBConfiguration::GetProbeBitStreamLength() { return PROBE_BITSTREAM_LENGTH/8; }
uint8_t * sbndaq::BernCRTFEBConfiguration::GetSlowControlBitStream() { return SlowControlBitStream; }
int       sbndaq::BernCRTFEBConfiguration::GetSlowControlBitStreamLength() { return SLOW_CONTROL_BITSTREAM_LENGTH/8; }
