#ifndef _NEVISTPC_NEVISCONTROLLERPCIECARD_H
#define _NEVISTPC_NEVISCONTROLLERPCIECARD_H  1

#include "NevisPCIeCard.h"

#define TLVL_SEND TLVL_DEBUG+10
#define TLVL_RECEIVE TLVL_DEBUG+11

namespace nevistpc{
    
  //class NevisControllerPCIeCard : protected NevisPCIeCard {
  class NevisControllerPCIeCard : public NevisPCIeCard { // jcrespo: otherwise NevisPCIeCard public members are not accessible (should not be the case, needs more investigation)

  public:
    
    NevisControllerPCIeCard();
    NevisControllerPCIeCard(DeviceInfo const &deviceInfo);
    ~NevisControllerPCIeCard();
    
    void send(ControlDataPacket const &packet, useconds_t const sleep_time_us=0);
    void receive(StatusPacket &packet);
    void query(ControlDataPacket const& control,
	       StatusPacket & status,
	       useconds_t const sleep_time_us=0);
    
  private:
    int pcie_send(WDC_DEVICE_HANDLE hDev, const int mode, const int nword, const UINT32 *buff_send);
    int pcie_send_debug(WDC_DEVICE_HANDLE hDev, const int mode, const int nword, const UINT32 *buff_send);
    int dumpPacket(const int nword, const UINT32 *buff_send);
    
    int pcie_rec(WDC_DEVICE_HANDLE hDev, int mode, int istart, int nword, int ipr_status, UINT32 *buff_rec);
    int pcie_rec_debug(WDC_DEVICE_HANDLE hDev, int mode, int istart, int nword, int ipr_status, UINT32 *buff_rec);
    
  };
  
  typedef std::unique_ptr<NevisControllerPCIeCard> NevisControllerPCIeCardUPtr;
  typedef std::shared_ptr<NevisControllerPCIeCard> NevisControllerPCIeCardSPtr;  
  
  
} // end of namespace nevistpc

#endif //NEVISPCIECARD_H

// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 



