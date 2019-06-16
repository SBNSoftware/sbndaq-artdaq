#define TRACE_NAME "NevisPCIeCard"
#include "trace.h"

#include "NevisPCIeCard.h"
#include "daqExceptions.h"

//#include <termios.h>
//#include <fstream>
#include <mutex>

nevistpc::NevisPCIeCard::NevisPCIeCard(DeviceInfo const &deviceInfo)
  : _deviceInfo(deviceInfo)
{
  TLOG(TLVL_INFO) <<"NevisPCIeCard initialized: " <<_deviceInfo.info().c_str();
}

nevistpc::NevisPCIeCard::NevisPCIeCard()
{
  TLOG(TLVL_INFO) << "NevisPCIeCard default initialized: " << _deviceInfo.info().c_str();
}

nevistpc::NevisPCIeCard::~NevisPCIeCard()
{
  TLOG(TLVL_INFO)<< "NevisPCIeCard destructor called.";
    if (_deviceInfo.deviceHandle() != NULL)
    {
        deviceClose();
    }
}

    
bool nevistpc::NevisPCIeCard::deviceOpen(){
  
  TLOG(TLVL_DEVICE)<<"Called deviceOpen for "<<_deviceInfo.info().c_str();
  
  std::lock_guard<std::mutex> guard(_deviceInfo_lock);
  TLOG(TLVL_DEVICE)<<"Obtained device lock";
  
  // Initialize the NEVISPCI_ library
  _deviceInfo.lastOpStatus() = ::NEVISPCI_LibInit();
  
  if (WD_STATUS_SUCCESS != _deviceInfo.lastOpStatus()){
    TLOG(TLVL_ERROR)<<"NevisPCIeCard: Failed to initialize the NEVISPCI library: "<< NEVISPCI_GetLastErr();
    return false;
  }
  
  TLOG(TLVL_DEVICE)<<"Initialized NEVISPCI_ library.";
  
  // Find and open a NEVISPCI_ device (by default ID)
  if (_deviceInfo.dwVendorId()){
    _deviceInfo.deviceHandle() = ::deviceFindAndOpen(_deviceInfo.dwVendorId(), _deviceInfo.dwDeviceId(), _deviceInfo.dwBus());
    TLOG(TLVL_DEVICE)<<"Device found and opened?";
  }
  
  if(_deviceInfo.deviceHandle() == NULL){
    TLOG(TLVL_ERROR)<<"NevisPCIeCard: Failed to find/open device handle for "<<_deviceInfo.info().c_str();
    return false;
  }
  
  TLOG(TLVL_DEVICE)<<"Device opened with valid handle.";
  
  return true;
}

bool nevistpc::NevisPCIeCard::deviceClose(){

  TLOG(TLVL_DEVICE)<<"Called deviceClose for " <<_deviceInfo.info().c_str();
  
  std::lock_guard<std::mutex> guard(_deviceInfo_lock);
  TLOG(TLVL_DEVICE)<<"Obtained device lock";

  if (_deviceInfo.deviceHandle() != NULL){
    ::deviceClose(_deviceInfo.deviceHandle());
    TLOG(TLVL_DEVICE)<<"Device closed.";
  }
  
  _deviceInfo.lastOpStatus() = ::NEVISPCI_LibUninit();
  
  if (WD_STATUS_SUCCESS != _deviceInfo.lastOpStatus()){
    TLOG(TLVL_ERROR)<<"NevisPCIeCard: Failed to uninitialize the NEVISPCI library: "<<NEVISPCI_GetLastErr();
    return false;
  }
  
  TLOG(TLVL_DEVICE)<<"Uninitialized NEVISPCI_ library.";
  
  _deviceInfo.deviceHandle() = NULL;
  
  TLOG(TLVL_DEVICE)<<"Device closed and handle reset.";
  
  return true;
}

bool nevistpc::NevisPCIeCard::writeAddr8 (DWORD dwAddrSpace, KPTR dwOffset, BYTE val)
{
  TLOG(TLVL_WRITE)<<"writeAddr8: addr=0x" << std::hex << dwAddrSpace << ", offset=0x" <<std::hex << dwOffset << ", val=0x" << std::hex<< val;

  std::lock_guard<std::mutex> guard(_deviceInfo_lock);
  TLOG(TLVL_WRITE)<<"Obtained device lock.";

  if (!_deviceInfo.deviceHandle()){
    TLOG(TLVL_ERROR)<<"WRITE ERROR: handle not initialized for "<< _deviceInfo.info().c_str();
    throw RuntimeErrorException(std::string("NevisPCIeCard was not initialized; writeAddr8 failed"));
  }

  _deviceInfo.lastOpStatus() =::WDC_WriteAddr8(_deviceInfo.deviceHandle(),  dwAddrSpace,  dwOffset,  val);
  TLOG(TLVL_WRITE)<<"Write finished.";

  return didLastCallSucceed(std::string(__func__),TLVL_WRITE);
}

bool nevistpc::NevisPCIeCard::writeAddr16 (DWORD dwAddrSpace, KPTR dwOffset, WORD val)
{
  TLOG(TLVL_WRITE)<< "writeAddr16: addr=0x" <<std::hex << dwAddrSpace <<", offset=0x"<<std::hex <<dwOffset <<", val=0x" << std::hex << val;

  std::lock_guard<std::mutex> guard(_deviceInfo_lock);
  TLOG(TLVL_WRITE)<<"Obtained device lock.";

  if (!_deviceInfo.deviceHandle()){
    TLOG(TLVL_ERROR)<<"WRITE ERROR: handle not initialized for "<< _deviceInfo.info().c_str();
    throw RuntimeErrorException(std::string("NevisPCIeCard was not initialized; writeAddr16 failed"));
  }

  _deviceInfo.lastOpStatus() =::WDC_WriteAddr16(_deviceInfo.deviceHandle(),  dwAddrSpace,  dwOffset,  val);
  TLOG(TLVL_WRITE)<<"Write finished.";

  return didLastCallSucceed(std::string(__func__),TLVL_WRITE);
}

bool nevistpc::NevisPCIeCard::writeAddr32 (DWORD dwAddrSpace, KPTR dwOffset, UINT32 val)
{
  TLOG(TLVL_WRITE)<< "writeAddr32: addr=0x" <<std::hex << dwAddrSpace <<", offset=0x"<<std::hex <<dwOffset <<", val=0x" << std::hex << val;

  std::lock_guard<std::mutex> guard(_deviceInfo_lock);
  TLOG(TLVL_WRITE)<<"Obtained device lock.";

  if (!_deviceInfo.deviceHandle()){
    TLOG(TLVL_ERROR)<<"WRITE ERROR: handle not initialized for "<<_deviceInfo.info().c_str();
    throw RuntimeErrorException(std::string("NevisPCIeCard was not initialized; writeAddr32 failed"));
  }

  _deviceInfo.lastOpStatus() =::WDC_WriteAddr32(_deviceInfo.deviceHandle(),  dwAddrSpace,  dwOffset,  val);
  TLOG(TLVL_WRITE)<<"Write finished.";

  return didLastCallSucceed(std::string(__func__),TLVL_WRITE);
}

bool nevistpc::NevisPCIeCard::writeAddr64 (DWORD dwAddrSpace, KPTR dwOffset, UINT64 val)
{
  TLOG(TLVL_WRITE)<< "writeAddr64: addr=0x" <<std::hex << dwAddrSpace <<", offset=0x"<<std::hex <<dwOffset <<", val=0x" << std::hex << val;

  std::lock_guard<std::mutex> guard(_deviceInfo_lock);
  TLOG(TLVL_WRITE)<<"Obtained device lock.";

  if (!_deviceInfo.deviceHandle()){
    TLOG(TLVL_ERROR)<<"WRITE ERROR: handle not initialized for "<<_deviceInfo.info().c_str();
    throw RuntimeErrorException(std::string("NevisPCIeCard was not initialized; writeAddr64 failed"));
  }

  _deviceInfo.lastOpStatus() =::WDC_WriteAddr64(_deviceInfo.deviceHandle(),  dwAddrSpace,  dwOffset,  val);
  TLOG(TLVL_WRITE)<<"Write finished.";

  return didLastCallSucceed(std::string(__func__),TLVL_WRITE);
}

bool nevistpc::NevisPCIeCard::readAddr8 (DWORD dwAddrSpace, KPTR dwOffset, BYTE &val){

  TLOG(TLVL_READ)<<"readAddr8: addr=0x" << std::hex << dwAddrSpace << ", offset=0x" << std::hex<< dwOffset;

  std::lock_guard<std::mutex> guard(_deviceInfo_lock);
  TLOG(TLVL_READ)<<"Obtained device lock.";
  
  if (!_deviceInfo.deviceHandle()){
    TLOG(TLVL_ERROR)<<"READ ERROR: handle not initialized for " <<_deviceInfo.info().c_str();
    throw RuntimeErrorException(std::string("NevisPCIeCard was not initialized; readAddr8 failed"));
  }

  _deviceInfo.lastOpStatus() =::WDC_ReadAddr8(_deviceInfo.deviceHandle(),  dwAddrSpace,  dwOffset,  &val);
  TLOG(TLVL_READ)<<"Read finished, val=0x"<< std::hex<<val;

  return didLastCallSucceed(std::string(__func__),TLVL_READ);

}

bool nevistpc::NevisPCIeCard::readAddr16 (DWORD dwAddrSpace, KPTR dwOffset, WORD &val){

  TLOG(TLVL_READ)<<"readAddr16: addr=0x" << std::hex << dwAddrSpace << ", offset=0x" << std::hex<< dwOffset;

  std::lock_guard<std::mutex> guard(_deviceInfo_lock);
  TLOG(TLVL_READ)<<"Obtained device lock.";
  
  if (!_deviceInfo.deviceHandle()){
    TLOG(TLVL_ERROR)<<"READ ERROR: handle not initialized for %s",_deviceInfo.info().c_str();
    throw RuntimeErrorException(std::string("NevisPCIeCard was not initialized; readAddr16 failed"));
  }

  _deviceInfo.lastOpStatus() =::WDC_ReadAddr16(_deviceInfo.deviceHandle(),  dwAddrSpace,  dwOffset,  &val);
  TLOG(TLVL_READ)<<"Read finished, val=0x"<<val;

  return didLastCallSucceed(std::string(__func__),TLVL_READ);

}

bool nevistpc::NevisPCIeCard::readAddr32 (DWORD dwAddrSpace, KPTR dwOffset, UINT32 &val){

  TLOG(TLVL_READ)<<"readAddr32: addr=0x" << std::hex << dwAddrSpace << ", offset=0x" << std::hex<< dwOffset;


  std::lock_guard<std::mutex> guard(_deviceInfo_lock);
  TLOG(TLVL_READ)<<"Obtained device lock.";
  
  if (!_deviceInfo.deviceHandle()){
    TLOG(TLVL_ERROR)<<"READ ERROR: handle not initialized for " << _deviceInfo.info().c_str();
    throw RuntimeErrorException(std::string("NevisPCIeCard was not initialized; readAddr32 failed"));
  }

  _deviceInfo.lastOpStatus() =::WDC_ReadAddr32(_deviceInfo.deviceHandle(),  dwAddrSpace,  dwOffset,  &val);
  TLOG(TLVL_READ)<<"Read finished, val=0x"<<val;

  return didLastCallSucceed(std::string(__func__),TLVL_READ);

}

bool nevistpc::NevisPCIeCard::readAddr64 (DWORD dwAddrSpace, KPTR dwOffset, UINT64 &val){

  TLOG(TLVL_READ)<<"readAddr64: addr=0x" << std::hex << dwAddrSpace << ", offset=0x" << std::hex<< dwOffset;


  std::lock_guard<std::mutex> guard(_deviceInfo_lock);
  TLOG(TLVL_READ)<<"Obtained device lock.";
  
  if (!_deviceInfo.deviceHandle()){
    TLOG(TLVL_ERROR)<<"READ ERROR: handle not initialized for "<<_deviceInfo.info().c_str();
    throw RuntimeErrorException(std::string("NevisPCIeCard was not initialized; readAddr64 failed"));
  }

  _deviceInfo.lastOpStatus() =::WDC_ReadAddr64(_deviceInfo.deviceHandle(),  dwAddrSpace,  dwOffset,  &val);
  TLOG(TLVL_READ)<<"Read finished, val=0x"<<val;

  return didLastCallSucceed(std::string(__func__),TLVL_READ);

}

bool nevistpc::NevisPCIeCard::didLastCallSucceed(std::string const& functName, unsigned int tlvl)
{
  if (WD_STATUS_SUCCESS != _deviceInfo.lastOpStatus()){
    TLOG(TLVL_ERROR)<<"NevisPCIeCard::" <<functName.c_str() << " returned error with 0x" << _deviceInfo.lastOpStatus() <<" " <<Stat2Str(_deviceInfo.lastOpStatus());
    return false;
  }

  TRACE(tlvl,"NevisPCIeCard::%s succeeded.",functName.c_str());
  return true;
}





