#ifndef _NEVISTPC_FPGAFIRMWAREREADER_H
#define _NEVISTPC_FPGAFIRMWAREREADER_H  1

#include "ControlDataPacket.h"

namespace nevistpc
{

  class FPGAFirmwareReader
  {
  public:
    //typedef range_of_for<std::vector, std::uint8_t> range_of_uint8_t;
    typedef std::vector<std::uint8_t> vector_of_uint8_t;
    
    explicit FPGAFirmwareReader(std::string const &fileName);
    std::size_t readAndEncodeFor(ControlDataPacket const &target, ControlDataPackets &control_packets);
    
  private:
    std::string _fileName;
  };
  

} // end of namespace nevistpc

#endif //_NEVISTPC_FPGAFIRMWAREREADER_H
