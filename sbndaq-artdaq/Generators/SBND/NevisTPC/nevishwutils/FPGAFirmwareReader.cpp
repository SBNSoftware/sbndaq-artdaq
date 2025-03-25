#include "daqExceptions.h"
#include "FPGAFirmwareReader.h"

#include <fstream>

namespace nevistpc
{

  FPGAFirmwareReader::FPGAFirmwareReader(std::string const &fileName)
    : _fileName(fileName)
  {
  }

  std::size_t FPGAFirmwareReader::readAndEncodeFor( ControlDataPacket const &target, ControlDataPackets &control_packets )
  {
    const std::size_t chunkSize = 1000; 
    
    std::ifstream inputFile(_fileName, std::ios::binary);
    if (!inputFile.good())
      throw InvalidConfigurationException(std::string("Unable to open file:").append(_fileName));
    
    std::vector<char> blob((std::istreambuf_iterator <char> (inputFile)), std::istreambuf_iterator<char>());
    
    control_packets.clear();
    
    control_packets.resize(div(blob.size(), chunkSize).quot + 1);
    
    std::vector<char>::const_iterator blobPos = blob.begin();
    std::vector<char>::const_iterator blobEnd = blob.end();
    
    for(auto & control_packet: control_packets)
      {
	control_packet.resize(2 + (chunkSize >> 1));
	
	ControlDataPacket::iterator currentPos = control_packet.dataBegin();
	
	uint32_t firstByte = (((uint8_t)(*blobPos++)) << 16);
	
	//fist byte of a chunk
	*currentPos++ = firstByte | *target.dataBegin();
	
	std::size_t cpybytes = (chunkSize >> 1) - 1;
	
	std::size_t i = 0;
	
	for(; i < cpybytes && blobPos != blobEnd && blobPos + 1 != blobEnd; ++i)
	  {
	    *currentPos++ = (uint8_t)(*blobPos++) | (((uint8_t)(*blobPos++)) << 16);
	  }
	
	//last byte of a chunk
	if(i++ == cpybytes || blobPos + 1 == blobEnd)
	  {
	    *currentPos++ = (uint8_t)(*blobPos++);
	  }
	
	//handle the trailing bytes
	if(blobPos == blobEnd)
	  {
	    //making data paylod to have even number of words;
	    if(std::distance(control_packet.dataBegin(), currentPos) % 2)
	      {
		*currentPos++ = 0;
	      }
	  }
	
	//trim leftovers
	control_packet.resize(std::distance(control_packet.dataBegin(), currentPos));
      }
    
    return control_packets.size();
  }
  
} // end of namespace nevistpc





