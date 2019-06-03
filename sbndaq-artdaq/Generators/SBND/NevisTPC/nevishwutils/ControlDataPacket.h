#ifndef _NEVISTPC_CONTROLLDATAPACKET_H
#define _NEVISTPC_CONTROLLDATAPACKET_H  1

#include <vector>
#include <cstdint>
#include <string>

namespace nevistpc
{
  
  
  typedef std::uint8_t   operation_id_t;
  typedef std::uint8_t   chip_address_t;
  typedef std::uint8_t   module_number_t;
  typedef std::uint32_t  data_payload_t;
  typedef std::uint16_t  data_payload_bool_t;
  

  class ControlDataPacket
  {
  public:
    typedef std::vector<data_payload_t>::reference       reference;
    typedef std::vector<data_payload_t>::iterator        iterator;
    typedef std::vector<data_payload_t>::const_iterator  const_iterator;
    typedef std::vector<data_payload_t>::value_type      value_type;
    typedef std::vector<data_payload_t>::difference_type difference_type;
    typedef std::vector<data_payload_t>::size_type       size_type;
    
    enum mode:
      
      /// What are these mode? Can anyone comment? --kazu 09/08/14
      std::uint8_t
    {
      SINGLEWORD_MODE =0,
	DMA_MODE ,
        UNKNOWN_MODE 
	};
    
    typedef mode mode_t;
    
    ControlDataPacket();
    
    explicit ControlDataPacket(module_number_t module_number_,
                            chip_address_t chip_address_,
                            operation_id_t operation_id_,
                            data_payload_t data_payload_);
    
    explicit ControlDataPacket(module_number_t module_number_,
                            chip_address_t chip_address_,
                            operation_id_t operation_id_);
    
    std::size_t   size() const;
    void resize(std::size_t sz);
    
    void clear();
    void append(data_payload_t data_payload);
    
    iterator dataBegin();
    iterator dataEnd();
    const_iterator dataBegin() const;
    const_iterator dataEnd() const;
    
    mode_t mode() const;
    void setMode(mode_t const &mode);
    
    std::string toString() const;
    
  private:
    std::vector<data_payload_t> _data;
    mode_t _mode;
  };
  
  typedef std::vector<ControlDataPacket> ControlDataPackets;
  
  
} // end of namespace nevistpc

#endif //_H

// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 


