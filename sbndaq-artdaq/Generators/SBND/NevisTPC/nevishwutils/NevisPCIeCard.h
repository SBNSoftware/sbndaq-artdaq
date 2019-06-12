#ifndef _NEVISTPC_NEVISPCIECARD_H
#define _NEVISTPC_NEVISPCIECARD_H  1

#include   <mutex>
#include   <unordered_map> 
#include   <cstdint>

#include "NevisPCIeCard-generated.h"
#include "ControlDataPacket.h"
#include "StatusPacket.h"

#include "fhiclcpp/ParameterSet.h"

#include "trace.h"

#define  TLVL_DEVICE TLVL_DEBUG+10 
#define  TLVL_WRITE TLVL_DEBUG+11
#define  TLVL_READ TLVL_DEBUG+12

namespace nevistpc{
  
  class DeviceInfo{
    
  public:
    std::string toString() const { return _name; }
    
    std::string info() const {
      std::stringstream ss;
      ss << "name,vendor,device,bus=["<< _name <<","<<_dwVendorId<<","<<_dwDeviceId<<","<<_dwBus << "]";  
      return ss.str();       
    }
    
    DWORD &dwVendorId() { return _dwVendorId; }
    DWORD &dwDeviceId() { return _dwDeviceId; }
    DWORD &dwBus()      { return _dwBus;      }
    
    WDC_DEVICE_HANDLE &deviceHandle() { return _deviceHandle; }
    WD_PCI_SLOT       &deviceSlot()   { return _deviceSlot;   }
    
    DWORD &lastOpStatus()             { return _lastOpStatus; }
    const DWORD &lastOpStatus() const { return _lastOpStatus; }
    
  DeviceInfo(DWORD dwVendorId, DWORD dwDeviceId, DWORD dwBus, std::string name="none") 
    :_dwVendorId(dwVendorId), 
      _dwDeviceId(dwDeviceId),
      _dwBus(dwBus),
      _name(name),
      _deviceHandle(0), 
      _deviceSlot(), 
      _lastOpStatus(0) 
	{}
    
  DeviceInfo() 
    :_dwVendorId(0x1172), 
      _dwDeviceId(0x4), 
      _dwBus(0), 
      _deviceHandle(0),
      _deviceSlot(), 
      _lastOpStatus(0) {}
    
  DeviceInfo(fhicl::ParameterSet const& params,  std::string name="none") 
    :_dwVendorId(params.get<uint32_t>("vendor_id",0x1172)), 
      _dwDeviceId(params.get<uint32_t>("device_id",0x4)), 
      _dwBus(params.get<uint32_t>("bus",0)), 
      _name(params.get<std::string>("device_name",name)),
      _deviceHandle(0), 
      _deviceSlot(), 
      _lastOpStatus(0){}
    
  private:
    DWORD _dwVendorId;
    DWORD _dwDeviceId;
    DWORD _dwBus;
    std::string _name;
    WDC_DEVICE_HANDLE _deviceHandle;
    WD_PCI_SLOT _deviceSlot;
    DWORD _lastOpStatus;
  };
  
  struct dma_detail
  {
    static constexpr DWORD t1_tr_bar{0};
    static constexpr DWORD t2_tr_bar{4};
    static constexpr DWORD cs_bar{2};
    
    /**  command register location **/
    static constexpr DWORD tx_mode_reg {0x28};
    static constexpr DWORD t1_cs_reg {0x18};
    static constexpr DWORD r1_cs_reg {0x1c};
    static constexpr DWORD t2_cs_reg {0x20};
    static constexpr DWORD r2_cs_reg {0x24};
    
    static constexpr DWORD tx_md_reg {0x28};
    static constexpr DWORD cs_dma_add_low_reg {0x0};
    static constexpr DWORD cs_dma_add_high_reg {0x4};
    
    static constexpr DWORD cs_dma_by_cnt {0x8};
    static constexpr DWORD cs_dma_cntrl {0xc};
    static constexpr DWORD cs_dma_msi_abort {0x10};
    
    /** define status bits **/
    
    static constexpr DWORD cs_init  {0x20000000};
    static constexpr DWORD cs_mode_p {0x8000000};
    static constexpr DWORD cs_mode_n {0x0};
    static constexpr DWORD cs_start {0x40000000};
    static constexpr DWORD cs_done  {0x80000000};
    static constexpr DWORD dma_tr1  {0x100000};
    
    static constexpr DWORD dma_tr2  {0x200000};
    static constexpr DWORD dma_tr12 {0x300000};
    static constexpr DWORD dma_3dw_trans {0x0};
    static constexpr DWORD dma_3dw_rec  {0x40};
    static constexpr DWORD dma_4dw_trans {0x0};
    static constexpr DWORD dma_4dw_rec   {0x60};
    static constexpr DWORD dma_in_progress {0x80000000};
    
    static constexpr DWORD dma_abort {0x2};
    //    DWORD dwAddrSpace,
    //    KPTR dwOffset,
    //    UINT32 val
  };
  
  struct dma_flags
  {
    static constexpr DWORD send{DMA_TO_DEVICE | DMA_ALLOW_CACHE};
    static constexpr DWORD receive{DMA_FROM_DEVICE | DMA_ALLOW_CACHE};// | DMA_ALLOW_64BIT_ADDRESS;
  };
  
  class NevisPCIeCard {


  public:
    
    NevisPCIeCard();
    NevisPCIeCard(DeviceInfo const &deviceInfo);
    ~NevisPCIeCard();
    
    bool deviceOpen();
    bool deviceClose();

    bool setupTXModeRegister();
    
    DeviceInfo const& getDeviceInfo(){
      return _deviceInfo;
    };

    // forward calls to windriver
    bool writeAddr8 (DWORD dwAddrSpace, KPTR dwOffset, BYTE val);
    bool writeAddr16(DWORD dwAddrSpace, KPTR dwOffset, WORD val);
    bool writeAddr32(DWORD dwAddrSpace, KPTR dwOffset, UINT32 val);
    bool writeAddr64(DWORD dwAddrSpace, KPTR dwOffset, UINT64 val);
    
    bool readAddr8 (DWORD dwAddrSpace, KPTR dwOffset, BYTE &val);
    bool readAddr16(DWORD dwAddrSpace, KPTR dwOffset, WORD &val);
    bool readAddr32(DWORD dwAddrSpace, KPTR dwOffset, UINT32 &val);
    bool readAddr64(DWORD dwAddrSpace, KPTR dwOffset, UINT64 &val);    
    
    bool didLastCallSucceed(std::string const& functName, unsigned int tlvl=TLVL_DEBUG);
    
  protected:
    
    DeviceInfo _deviceInfo;
    std::mutex _deviceInfo_lock;
    
    
    static constexpr std::uint32_t  dma_tr1 = 0x100000;
    static constexpr std::uint32_t  dma_tr2 = 0x200000;
    static constexpr std::uint32_t  dma_tr12 = 0x300000;
    static constexpr std::uint32_t  dma_3dw_trans = 0x0;
    static constexpr std::uint32_t  dma_3dw_rec = 0x40;
    static constexpr std::uint32_t  dma_4dw_trans = 0x0;
    static constexpr std::uint32_t  dma_4dw_rec = 0x60;
    static constexpr std::uint32_t  dma_in_progress = 0x80000000;
    static constexpr std::uint32_t  dma_abort = 0x2;
    static constexpr std::uint32_t  cs_dma_add_low_reg = 0x0;
    static constexpr std::uint32_t  cs_dma_add_high_reg = 0x4;
    static constexpr std::uint32_t  cs_dma_by_cnt = 0x8;
    static constexpr std::uint32_t  cs_dma_cntrl = 0xc;
    static constexpr std::uint32_t  cs_dma_msi_abort = 0x10;
    
    
  };
  
  typedef std::unique_ptr<NevisPCIeCard> NevisPCIeCardUPtr;
  typedef std::shared_ptr<NevisPCIeCard> NevisPCIeCardSPtr;
} // end of namespace nevistpc

#endif //NEVISPCIECARD_H

// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 



