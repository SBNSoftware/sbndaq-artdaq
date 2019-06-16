#ifndef _NEVISTPC_NEVISREADOUTPCIECARD_H
#define _NEVISTPC_NEVISREADOUTPCIECARD_H  1

#include "NevisPCIeCard.h"

#define  TLVL_DMA TLVL_DEBUG+10

namespace nevistpc{
    
  //class NevisReadoutPCIeCard : protected NevisPCIeCard {
  class NevisReadoutPCIeCard : public NevisPCIeCard { // jcrespo: otherwise NevisPCIeCard public members are not accessible (should not be the case, needs more investigation)

  public:
    
    NevisReadoutPCIeCard();
    NevisReadoutPCIeCard(DeviceInfo const &deviceInfo);
    ~NevisReadoutPCIeCard();
    
    bool dmaContigBufLock(PVOID *ppBuf, DWORD dwOptions, DWORD dwDMABufSize, WD_DMA **ppDma);    
    
  private:
    
  };
  
  typedef std::unique_ptr<NevisReadoutPCIeCard> NevisReadoutPCIeCardUPtr;
  typedef std::shared_ptr<NevisReadoutPCIeCard> NevisReadoutPCIeCardSPtr;
  
  
} // end of namespace nevistpc

#endif //NEVISPCIECARD_H

// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 



