#include "ControlDataPacket.h"

#include <ios>
#include <sstream>

namespace nevistpc
{

  ControlDataPacket::ControlDataPacket()
    :_data {0}, 
    _mode(ControlDataPacket::DMA_MODE) 
    {}
  
  ControlDataPacket::ControlDataPacket(module_number_t module_number,
				       chip_address_t chip_address,
				       operation_id_t operation_id,
				       data_payload_t data_payload)
    : _data { (data_payload_t)(module_number << 11) + (chip_address << 8) + operation_id + (data_payload << 16) },
    _mode(ControlDataPacket::DMA_MODE) {}
  
  ControlDataPacket::ControlDataPacket(module_number_t module_number,
				       chip_address_t chip_address,
				       operation_id_t operation_id)
    : _data { (data_payload_t)(module_number << 11) + (chip_address << 8) + operation_id },
    _mode(ControlDataPacket::DMA_MODE) {}
  
  ControlDataPacket::mode_t ControlDataPacket::mode() const
  {
    return _mode;
  }
  void ControlDataPacket::setMode(ControlDataPacket::mode_t const &mode)
  {
    _mode = mode;
  }
  
  std::size_t  ControlDataPacket::size() const
  {
    return _data.size();
  }
  
  ControlDataPacket::iterator ControlDataPacket::dataBegin()
  {
    return _data.begin();
  }
  
  ControlDataPacket::iterator ControlDataPacket::dataEnd()
  {
    return _data.end();
  }
  
  ControlDataPacket::const_iterator ControlDataPacket::dataBegin() const
  {
    return _data.begin();
  }
  
  ControlDataPacket::const_iterator ControlDataPacket::dataEnd() const
  {
    return _data.end();
  }
  
  void ControlDataPacket::resize(std::size_t sz)
  {
    _data.resize(sz);
  }
  
  void ControlDataPacket::clear()
  {
    _data.clear();
  }
  
  void ControlDataPacket::append(data_payload_t data_payload){
    _data.push_back(data_payload);
  }
  
  std::string ControlDataPacket::toString() const
  {
    std::ostringstream hex;
    hex << " Data: <" << std::hex;
    
    auto begin = _data.begin();
    auto end = _data.end();
    
    while(begin != end)
      {
        hex << static_cast<unsigned>(*begin++) << ", ";
      }
    
    hex << ">";
    
    return hex.str();
  }
  
  
}  // end of namespace nevistpc







