#ifndef _NEVISTPC_STATUSPACKET_H
#define _NEVISTPC_STATUSPACKET_H  1

#include "ControlDataPacket.h"

namespace nevistpc
{

class StatusPacket: public ControlDataPacket
{
public:
 StatusPacket(int start):_start(start){setMode(ControlDataPacket::SINGLEWORD_MODE);}
 StatusPacket(int start,std::size_t size ):_start(start){setMode(ControlDataPacket::SINGLEWORD_MODE); resize(size);}
  int start(){return _start;}
  void setStart(int start){_start=start;}
  void setStatusMode(mode_t const &mode){setMode(mode);}
  
 private:
  int _start;
  
};
  
} // end of namespace nevistpc

#endif //_UBOONE_HWUTILLS_STATUSPACKET_H

// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 





