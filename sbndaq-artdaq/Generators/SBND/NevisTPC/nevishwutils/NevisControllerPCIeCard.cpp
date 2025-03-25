#define TRACE_NAME "NevisControllerPCIeCard"

#include "trace.h"
#include "daqExceptions.h"
#include "NevisControllerPCIeCard.h"

#include <fstream>
#include <sstream>
#include <ios>
#include <iomanip>

nevistpc::NevisControllerPCIeCard::NevisControllerPCIeCard(DeviceInfo const &deviceInfo)
{
  _deviceInfo = deviceInfo;
  TLOG(TLVL_INFO)<< "NevisControllerPCIeCard initialized: " <<_deviceInfo.info().c_str();
}

nevistpc::NevisControllerPCIeCard::NevisControllerPCIeCard()
{
  TLOG(TLVL_INFO)<< "NevisControllerPCIeCard default initialized:" << _deviceInfo.info().c_str();
}

nevistpc::NevisControllerPCIeCard::~NevisControllerPCIeCard()
{
  TLOG(TLVL_INFO)<<  "NevisControllerPCIeCard destructor called.";
    if (_deviceInfo.deviceHandle() != NULL)
    {
        deviceClose();
    }
}

int nevistpc::NevisControllerPCIeCard::pcie_send_debug(WDC_DEVICE_HANDLE hDev, const int mode, const int nword, const UINT32 *buff_send){
   dumpPacket(nword,buff_send);
   return pcie_send(hDev, mode, nword,buff_send);
}

int nevistpc::NevisControllerPCIeCard::pcie_rec_debug(WDC_DEVICE_HANDLE hDev, int mode, int istart, int nword, int ipr_status, UINT32 *buff_rec){
  int retCode=pcie_rec(hDev,mode, istart, nword, ipr_status, buff_rec);
  dumpPacket(nword,buff_rec);
  return retCode;
}
/*    
int nevistpc::NevisControllerPCIeCard::pcie_rec_largebuf_debug(WDC_DEVICE_HANDLE hDev, int mode, int istart, int nword, int ipr_status, UINT32 *buff_rec){
   int retCode= pcie_rec_largebuf(hDev, mode, istart, nword,ipr_status,buff_rec);
   dumpPacket(nword,buff_rec);
   return retCode;
}
*/
int nevistpc::NevisControllerPCIeCard::dumpPacket(const int nword, const UINT32 *buff_send)
{
  TLOG(TLVL_INFO)<< "Calling dumpPacket to file /tmp/daq-pcie-dump.txt";
  
  static unsigned int cmd = 0;
  std::ofstream os;
  
  os.open("/tmp/daq-pcie-dump.txt", std::ios::out | std::ios::app);
  
  if (nword < 3)
    {
      os <<  std::setw(4) << std::setfill('0') << cmd++ << std::hex
	 << " M:" << ((((*buff_send) & (~0xFFFF07FF)) >> 11))
	 << " C:" << ((((*buff_send) & (~0xFFFFF8FF)) >> 8))
	 << " O:" << (((*buff_send) & (~0xFFFFFF00)))
	 << " D:" << ((int)((*buff_send) >> 16))
	 << " HEX:" << ((*buff_send))
	 << std::dec;
    }
  
  if (nword == 2)
    {
      os << std::setw(8) << " ," << (* (buff_send + 1));
    }
  
  if (nword > 2)
    {
      os <<  std::setw(4) << std::setfill('0') << cmd++;
      
      os << " RAW:";
      
      for (int i = 0; i < nword; i++)
        {
	  os << std::setw(8) << std::setfill('0') << std::hex << (* (buff_send + i)) << " ";
        }
      
      //  os << "- ";
      
      // for(int i=nword-10;i < nword;++i)
      //    os << std::setw(8)<<std::setfill('0') << std::hex << (*(buff_send+i)) << " ";
      
      os << " S: " << std::setw(8) << std::setfill('0') << std::dec << nword;
    }
  
  os << std::endl;
  
  os.close();
  
  return 0;
}


void nevistpc::NevisControllerPCIeCard::send(ControlDataPacket const &packet,
					     useconds_t const sleep_time_us)
{
  TLOG(TLVL_SEND)<< "Calling send packet << " << packet.toString().c_str();

  std::lock_guard<std::mutex> guard(_deviceInfo_lock);
  TLOG(TLVL_SEND)<< "Obtained device lock.";

  if (!_deviceInfo.deviceHandle()){
    TLOG(TLVL_ERROR)<<"SEND ERROR: handle not initialized for %s",_deviceInfo.info().c_str();
    throw RuntimeErrorException(std::string("NevisControllerPCIeCard was not initialized, unable to send a packet ").append(packet.toString()));
  }

  pcie_send(_deviceInfo.deviceHandle(), (int) packet.mode(), (int) packet.size(), &*packet.dataBegin());
  TLOG(TLVL_SEND)<< "Sent packet.";
  
  if(sleep_time_us)
    usleep(sleep_time_us);
  else
    usleep(128);

  TLOG(TLVL_SEND)<< "Exiting send after sleep.";
}

void nevistpc::NevisControllerPCIeCard::receive(StatusPacket &packet)
{
  TLOG(TLVL_RECEIVE) <<"Calling receive packet...";

  std::lock_guard<std::mutex> guard(_deviceInfo_lock);
  TLOG(TLVL_RECEIVE) << "Obtained device lock.";

  if (!_deviceInfo.deviceHandle()){
    TLOG(TLVL_ERROR) << "RECEIVE ERROR: handle not initialized for " << _deviceInfo.info().c_str();
    throw RuntimeErrorException(std::string("NevisControllerPCIeCard was not initialized, unable to receive a packet."));
  }

  pcie_rec(_deviceInfo.deviceHandle(), (int) packet.mode(), (int) packet.start(), (int) packet.size(),0, &*packet.dataBegin());
  TLOG(TLVL_RECEIVE) << "Received packet " << packet.toString().c_str();

}

void nevistpc::NevisControllerPCIeCard::query(ControlDataPacket const &control,
					      StatusPacket &status,
					      useconds_t const sleep_time_us)
{
  send(control,sleep_time_us);
  receive(status);
}

int nevistpc::NevisControllerPCIeCard::pcie_send(WDC_DEVICE_HANDLE hDev, const int mode, const int nword, const UINT32 *buff_send)
{
  TLOG(TLVL_SEND)<< "Calling pcie_send in mode " << mode << ", nwords=" << nword;

  /* imode =0 single word transfer, imode =1 DMA */
  static DWORD dwAddrSpace;
  static DWORD dwDMABufSize;
  
  static UINT32 *buf_send;
  static WD_DMA *pDma_send;
  static DWORD dwStatus;
  static DWORD dwOptions_send = DMA_TO_DEVICE;
  static DWORD dwOffset;
  static UINT32 u32Data;
  static PVOID pbuf_send;
  
  int nwrite, i, j;
  
  i=0;
  
  static int ifr = 0;
  
  if(ifr == 0)
    {
      TLOG(TLVL_SEND)<< "First pcie_send, ifr=" << ifr << ". Setup DMAbuffer of size 140000";
      ifr = 1;
      dwDMABufSize = 140000;
      dwStatus = ::WDC_DMAContigBufLock(hDev, &pbuf_send, dwOptions_send, dwDMABufSize, &pDma_send);
      
      if(WD_STATUS_SUCCESS != dwStatus)
	TLOG(TLVL_ERROR) << "pcie_send: Failed locking a send Contiguous DMA buffer. Error 0x" <<std::hex <<dwStatus  << "- " << Stat2Str(dwStatus);
      else
	TLOG(TLVL_SEND)<< "pcie_send DMAContigBufLock successful.";
      
      buf_send = (UINT32 *) pbuf_send;
    }
  
  if(mode == 1){
    
    TLOG(TLVL_SEND)<< "Filling send buffer in DMA mode.";
    for(i = 0; i < nword; i++)
      {
	* (buf_send + i) = *buff_send++;
      }
  }
  
  if(mode == 0){
    TLOG(TLVL_SEND)<< "Starting send in single word mode. Total of " << nword << "words";

    nwrite = nword * 4;
    
    /*setup transmiter */
    dwAddrSpace = 2;
    u32Data = 0x20000000;
    dwOffset = 0x18;
    ::WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);
    dwAddrSpace = 2;
    u32Data = 0x40000000 + nwrite;
    dwOffset = 0x18;
    ::WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);

    TLOG(TLVL_SEND)<< "Transmitter setup. Preparing to perform data write.";
    
    for(j = 0; j < nword; j++){
      dwAddrSpace = 0;
      dwOffset = 0x0;
      u32Data = *buff_send++;
      ::WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);
    }
    TLOG(TLVL_SEND)<< "Write call completed. Preparing to verify transfer.";
    
    for(i = 0; i < 2000; i++){
      dwAddrSpace = 2;
      dwOffset = 0xC;
      ::WDC_ReadAddr32(hDev, dwAddrSpace, dwOffset, &u32Data);
      TLOG(TLVL_SEND)<< "Status read "<< i << "0x" <<std::hex << u32Data;
      
      if((u32Data & 0x80000000) == 0)
	{
	  TLOG(TLVL_SEND)<< "Data Transfer complete (after " << i << "checks)";
	  break;
	}
    }
  }
  
  if(mode == 1){
    TLOG(TLVL_SEND)<< "Starting send in dma mode. Total of " << nword << "words";

    nwrite = nword * 4;
    ::WDC_DMASyncCpu(pDma_send);
    TLOG(TLVL_SEND)<< "DMA Sync complete.";
    
    /*setup transmiter */
    dwAddrSpace = 2;
    u32Data = 0x20000000;
    dwOffset = 0x18;
    ::WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);
    dwAddrSpace = 2;
    u32Data = 0x40000000 + nwrite;
    dwOffset = 0x18;
    ::WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);

    TLOG(TLVL_SEND)<< "Transmitter setup.";
    
    /* set up sending DMA starting address */
    
    dwAddrSpace = 2;
    u32Data = 0x20000000;
    dwOffset = 0x0;
    u32Data = pDma_send->Page->pPhysicalAddr & 0xffffffff;
    ::WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);
    
    dwAddrSpace = 2;
    u32Data = 0x20000000;
    dwOffset = 0x4;
    u32Data = (pDma_send->Page->pPhysicalAddr >> 32) & 0xffffffff;
    ::WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);
    
    TLOG(TLVL_SEND)<< "DMA sending address setup.";
    
    /* byte count */
    dwAddrSpace = 2;
    dwOffset = 0x8;
    u32Data = nwrite;
    ::WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);
    
    TLOG(TLVL_SEND)<< "Sent byte count of "<<nwrite;
    
    /* write this will start DMA */
    dwAddrSpace = 2;
    dwOffset = 0xc;
    u32Data = 0x00100000;
    ::WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);
    
    TLOG(TLVL_SEND)<< "DMA started.";

    for(i = 0; i < 20000; i++){

      dwAddrSpace = 2;
      dwOffset = 0xC;
      ::WDC_ReadAddr32(hDev, dwAddrSpace, dwOffset, &u32Data);
      TLOG(TLVL_SEND)<< "DMA status read << " << i <<  " 0x" << std::hex << u32Data;
      
      if((u32Data & 0x80000000) == 0){
	TLOG(TLVL_SEND)<< "DMA complete.";
	break;
      }
    }
    
    if((u32Data & 0x80000000) != 0)
      TLOG(TLVL_ERROR) << "DMA INCOMPLETE!";
    
    ::WDC_DMASyncIo(pDma_send);
    TLOG(TLVL_SEND)<< "DMASyncIo Complete";
  }
  
  TLOG(TLVL_SEND)<< "pcie_send complete (after " << i << " status checks)";
  return i;
}

//******************************************************
int nevistpc::NevisControllerPCIeCard::pcie_rec(WDC_DEVICE_HANDLE hDev, int mode, int istart, int nword, int ipr_status, UINT32 *buff_rec)
{
  TLOG(TLVL_RECEIVE) <<"Calling pcie_rec in mode=" << mode << ", istart=" << istart << ", nword=" <<nword << ", ipr_status=" << ipr_status;

  
  /* imode =0 single word transfer, imode =1 DMA */
  static DWORD dwAddrSpace;
  static DWORD dwDMABufSize;
  
  static UINT32 *buf_rec;
  static WD_DMA *pDma_rec;
  static DWORD dwStatus;
  static DWORD dwOptions_rec = DMA_FROM_DEVICE;
  static DWORD dwOffset;
  static UINT32 u32Data;
  static UINT64 u64Data;
  static PVOID pbuf_rec;
  
  int nread, i, j, icomp;
  
  static int ifr = 0;
  
  if(ifr == 0)
    {
      TLOG(TLVL_RECEIVE) << "First pcie_rec, ifr=" <<  ifr <<". Setup DMAbuffer of size 140000";
      ifr = 1;
      dwDMABufSize = 140000;
      dwStatus = ::WDC_DMAContigBufLock(hDev, &pbuf_rec, dwOptions_rec, dwDMABufSize, &pDma_rec);
      
      if(WD_STATUS_SUCCESS != dwStatus)
	TLOG(TLVL_ERROR) << "pcie_send: Failed locking a receive Contiguous DMA buffer. Error 0x" <<std::hex <<dwStatus  << "- " << Stat2Str(dwStatus);
      else
	TLOG(TLVL_RECEIVE) <<"pcie_rec DMAContigBufLock successful.";
      
      buf_rec = (UINT32 *) pbuf_rec;
    }
  
  if((istart == 1) | (istart == 3)){

    TLOG(TLVL_RECEIVE) << "Performing receiver setup sequence.";
    
    /** set up the receiver **/
    dwAddrSpace = 2;
    u32Data = 0xf0000008;
    dwOffset = 0x28;
    ::WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);
    
    TLOG(TLVL_RECEIVE) << "Receiver setup.";

    /*initialize the receiver */
    dwAddrSpace = 2;
    u32Data = 0x20000000;
    dwOffset = 0x1c;
    ::WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);
    
    TLOG(TLVL_RECEIVE) <<"Receiver initialized.";

    /* write byte count **/
    dwAddrSpace = 2;
    u32Data = 0x40000000 + nword * 4;
    dwOffset = 0x1c;
    ::WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);

    TLOG(TLVL_RECEIVE) <<"Receiver ready for "<< nword*4 <<" bytes";
    
    if(ipr_status == 1)
    {
      TLOG(TLVL_RECEIVE) <<"Doing ipr status";
      dwAddrSpace = 2;
      u64Data = 0;
      dwOffset = 0x18;
      ::WDC_ReadAddr64(hDev, dwAddrSpace, dwOffset, &u64Data);
      
      TLOG(TLVL_RECEIVE) << "pcie_rec: status word before read =0x" << (u64Data >> 32) << "0x"<<(u64Data & 0xffff);
    }
    
    return 0;
  }
  
  if((istart == 2) | (istart == 3))
  {
    TLOG(TLVL_RECEIVE) <<"Performing read with mode " << mode;
    
    if(mode == 0){

      nread = nword / 2 + 1;
      
      if(nword % 2 == 0)
        nread = nword / 2;

      TLOG(TLVL_RECEIVE) <<"nword=" << nword <<", nread=" << nread;
      
      for(j = 0; j < nread; j++)
      {
        dwAddrSpace = 0;
        dwOffset = 0x0;
        u64Data = 0xbad;
        ::WDC_ReadAddr64(hDev, dwAddrSpace, dwOffset, &u64Data);
        *buff_rec++ = (u64Data & 0xffffffff);
        *buff_rec++ = u64Data >> 32;
      }
      TLOG(TLVL_RECEIVE) <<"Read loop complete.";
      
      if(ipr_status == 1)
      {
	TLOG(TLVL_RECEIVE) <<"Doing ipr status";
        dwAddrSpace = 2;
        u64Data = 0;
        dwOffset = 0x18;
        ::WDC_ReadAddr64(hDev, dwAddrSpace, dwOffset, &u64Data);
        
        TLOG(TLVL_RECEIVE) << "pcie_rec: status word after read =0x" << (u64Data >> 32) << "0x"<<(u64Data & 0xffff);
      }
      
      return 0;
    }
    
    if(mode == 1){

      nread = nword * 4;
      ::WDC_DMASyncCpu(pDma_rec);
      TLOG(TLVL_RECEIVE) <<"DMASyncCpu done. nword=" << nword << ", nread=" << nread;
      
      /* set up sending DMA starting address */
      
      dwAddrSpace = 2;
      u32Data = 0x20000000;
      dwOffset = 0x0;
      u32Data = pDma_rec->Page->pPhysicalAddr & 0xffffffff;
      ::WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);
      
      dwAddrSpace = 2;
      u32Data = 0x20000000;
      dwOffset = 0x4;
      u32Data = (pDma_rec->Page->pPhysicalAddr >> 32) & 0xffffffff;
      ::WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);

      TLOG(TLVL_RECEIVE) <<"DMA starting address sent.";
      
      /* byte count */
      dwAddrSpace = 2;
      dwOffset = 0x8;
      u32Data = nread;
      ::WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);

      TLOG(TLVL_RECEIVE) <<"Read "<< nread << " bytes";
      
      /* write this will start DMA */
      dwAddrSpace = 2;
      dwOffset = cs_dma_cntrl;
      u32Data = dma_tr1 + dma_3dw_rec;
      ::WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);

      TLOG(TLVL_RECEIVE) <<"Start DMA.";

      icomp = 0;      
      for(i = 0; i < 20000; i++){

        dwAddrSpace = 2;
        dwOffset = 0xC;
        ::WDC_ReadAddr32(hDev, dwAddrSpace, dwOffset, &u32Data);
        
	TLOG(TLVL_RECEIVE) <<"pcie_rec: DMA status read "<< i <<" 0x"<< std::hex << u32Data;
        
        if((u32Data & 0x80000000) == 0){
	  icomp=1;
	  TLOG(TLVL_RECEIVE) <<"pcie_rec: DMA complete after " << i << " checks";
          break;
        }
      }
      
      if(icomp == 0){
	TLOG(TLVL_RECEIVE) <<"pcie_rec: DMA timeout";
        return 1;
      }
      
      WDC_DMASyncIo(pDma_rec);
      TLOG(TLVL_RECEIVE) <<"DMASyncIo complete.";
      
      for(i = 0; i < nword; i++)
        *buff_rec++ = * (buf_rec + i);
      
      TLOG(TLVL_RECEIVE) <<"Receive buffer updated.";
      
    }
  }

  TLOG(TLVL_RECEIVE) <<"pcie_rec hit end of function.";
  return 0;
}
