#define TRACE_NAME "NevisReadoutPCIeCard"
#include "trace.h"
#include "daqExceptions.h"
#include "NevisReadoutPCIeCard.h"

nevistpc::NevisReadoutPCIeCard::NevisReadoutPCIeCard(DeviceInfo const &deviceInfo)
{
  _deviceInfo = deviceInfo;
  TLOG(TLVL_INFO)<< "NevisReadoutPCIeCard initialized: ",_deviceInfo.info().c_str();
}

nevistpc::NevisReadoutPCIeCard::NevisReadoutPCIeCard()
{
  TLOG(TLVL_INFO)<< "NevisReadoutPCIeCard default initialized: ",_deviceInfo.info().c_str();
}

nevistpc::NevisReadoutPCIeCard::~NevisReadoutPCIeCard()
{
  TLOG(TLVL_INFO)<< "NevisReadoutPCIeCard destructor called.";
    if (_deviceInfo.deviceHandle() != NULL)
    {
        deviceClose();
    }
}


bool nevistpc::NevisReadoutPCIeCard::dmaContigBufLock(PVOID *ppBuf, DWORD dwOptions, DWORD dwDMABufSize, WD_DMA **ppDma)
{
  TLOG(TLVL_DMA)<<"dmaContigBufLock called: options=0x"<<std::hex<< dwOptions <<", bufsize="<<std::dec<<dwDMABufSize;

  std::lock_guard<std::mutex> guard(_deviceInfo_lock);
  TLOG(TLVL_DMA)<<"Obtained device lock.";
  
  if (!_deviceInfo.deviceHandle()){
    TLOG(TLVL_ERROR)<<"DMA ERROR: handle not initialized for "<<_deviceInfo.info().c_str();
    throw RuntimeErrorException(std::string("NevisPCIeCard was not initialized; dmaContigBufLock failed"));
  }
  
  _deviceInfo.lastOpStatus() =::WDC_DMAContigBufLock(_deviceInfo.deviceHandle(), ppBuf, dwOptions, dwDMABufSize, ppDma);
  TLOG(TLVL_DMA)<<"DMA buffer lock finished.";
  
  return didLastCallSucceed(std::string(__func__),TLVL_DMA);
}
