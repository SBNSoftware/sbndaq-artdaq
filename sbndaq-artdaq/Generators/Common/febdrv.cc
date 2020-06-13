#include "febdrv.hh"

#include <unistd.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>

#include <functional>

#include "trace.h"
#define TRACE_NAME "febdrv"

sbndaq::FEBDRV::FEBDRV() :
  sockfd_w(-1),
  sockfd_r(-1),
  driver_state(DRV_OK),
  dstmac{0x00,0x60,0x37,0x12,0x34,0x00}, //base mac for FEBs, last byte 0->255
  overwritten(0),
  lostinfpga(0),
  total_lost(0),
  total_acquired(0)
  { }


void sbndaq::FEBDRV::Init(std::string iface) {
  //NOTE OK
  TLOG(TLVL_DEBUG) << __func__ << "("<<iface<<") called";
  
  sprintf(ifName,"%s",iface.c_str());
  
  // network interface to febs
  if(!initif()) {
    TLOG(TLVL_ERROR)<<__func__<<"() Can't initialize network interface "<<ifName<<"! Exiting.";
    driver_state=DRV_INITERR;
    return;
    //TODO crash
  }
  
  pingclients();
  
  driver_state=DRV_OK;
  TLOG(TLVL_DEBUG) << __func__ << "("<<iface<<") completed";
}

bool sbndaq::FEBDRV::initif() {
  //NOTE checked
  struct timeval tv;
  tv.tv_sec = 0;  /* 0 Secs Timeout */
  tv.tv_usec = 500000;  // 500ms
  
  /* Open RAW socket to send on */
  if ((sockfd_w = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1) {
    TLOG(TLVL_ERROR)<<__func__<<"() sender socket error";
    return false;
  }
  /* Bind RAW to device */
  if (setsockopt(sockfd_w, SOL_SOCKET, SO_BINDTODEVICE, ifName, IF_NAMESIZE-1) == -1){
    TLOG(TLVL_ERROR)<<__func__<<"() SO_BINDTODEVICE RAW";
    return false;
  }
  struct ifreq ifreq_s;
  strncpy( ifreq_s.ifr_name, ifName, IF_NAMESIZE );
  ifreq_s.ifr_name[IF_NAMESIZE-1] = '\0';
  ioctl(sockfd_w, SIOCGIFINDEX, &ifreq_s );
  struct sockaddr_ll sockaddr_ll_s;
  memset(&sockaddr_ll_s, 0, sizeof(struct sockaddr_ll) );
  sockaddr_ll_s.sll_family = AF_PACKET;
  sockaddr_ll_s.sll_ifindex = ifreq_s.ifr_ifindex;
  if (bind(sockfd_w, (struct sockaddr *)&sockaddr_ll_s, sizeof(struct sockaddr_ll)) == -1){
    TLOG(TLVL_ERROR)<<__func__<<"() bind sockfd_w";
    return 0;
  }
  
  /* Open PF_PACKET socket, listening for EtherType ETHER_TYPE */
  FEBDTP_PKT_t spkt;
  spkt.iptype=0x0108; // IP type 0x0801
  if ((sockfd_r = socket(PF_PACKET, SOCK_RAW, spkt.iptype)) == -1) {
    TLOG(TLVL_ERROR)<<__func__<<"() listener socket error";
    return false;
  }
  
  if (setsockopt(sockfd_r, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval))== -1){
    TLOG(TLVL_ERROR)<<__func__<<"() SO_RCVTIMEO error";
    return false;
  }
  /* Bind listener to device */
  if (setsockopt(sockfd_r, SOL_SOCKET, SO_BINDTODEVICE, ifName, IFNAMSIZ-1) == -1){
    TLOG(TLVL_ERROR)<<__func__<<"() SO_BINDTODEVICE error";
    return false;
  }
  if (bind(sockfd_r, (struct sockaddr *)&sockaddr_ll_s, sizeof(struct sockaddr_ll)) == -1){
    TLOG(TLVL_ERROR)<<__func__<<"() bind sockfd_r";
    return false;
  }

  /* Get the index of the interface to send on */
  memset(&if_idx, 0, sizeof(struct ifreq));
  strncpy(if_idx.ifr_name, ifName, IFNAMSIZ-1);
  if_idx.ifr_name[IFNAMSIZ-1] = '\0';
  if (ioctl(sockfd_w, SIOCGIFINDEX, &if_idx) < 0){
    TLOG(TLVL_ERROR)<<__func__<<"() SIOCGIFINDEX error";
    return false;
  }
  /* Get the MAC address of the interface to send on */
  memset(&if_mac, 0, sizeof(struct ifreq));
  strncpy(if_mac.ifr_name, ifName, IFNAMSIZ);
  if_idx.ifr_name[IFNAMSIZ-1] = '\0';
  if (ioctl(sockfd_w, SIOCGIFHWADDR, &if_mac) < 0) {
    TLOG(TLVL_ERROR)<<__func__<<"() SIOCGIFHWADDR error";
    return false;
  }

  memcpy(&hostmac,((uint8_t *)&if_mac.ifr_hwaddr.sa_data),6);
  TLOG(TLVL_INFO)<<__func__<<"() Initialized "<<ifName<<" with host MAC  "<<std::hex<< hostmac[0]<<":"<<hostmac[1]<<":"<<hostmac[2]<<":"<<hostmac[3]<<":"<<hostmac[4]<<":"<<hostmac[5]<<std::dec;
  return true;
} //initif

void sbndaq::FEBDRV::pingclients() {
  /**
   * Check the list of FEBs connected to the ethernet port and save in macs
   */
  
  //NOTE OK
  dstmac[5]=0xff;
  sendcommand(dstmac, FEB_SET_RECV, 0, hostmac);
  
  while(recvfromfeb(10000,rpkt)){
    if(rpkt.CMD==FEB_OK) {
      std::array<uint8_t, 6> tmp_mac;
      std::copy(std::begin(rpkt.src_mac), std::end(rpkt.src_mac), tmp_mac.begin());
      macs.push_back(tmp_mac);
      
      TLOG(TLVL_INFO)<<__func__<<"() Found FEB: "
            <<std::hex<<tmp_mac[0]<<":"<<tmp_mac[1]<<":"<<tmp_mac[2]<<":"<<tmp_mac[3]<<":"<<tmp_mac[4]<<":"<<tmp_mac[5]<<" (decimal: "<<std::dec<<tmp_mac[5]<<") firmware: "
            <<rpkt.Data;
      
    }
  }

  TLOG(TLVL_INFO)<<__func__<<"() In total "<<macs.size()<<" FEBs connected.";
}

std::vector<uint8_t> sbndaq::FEBDRV:: GetMACs() {
  /**
   * Returns sorted vector with last 8 bits of the FEB MAC addresses
   */
  std::vector<uint8_t> out;
  for(auto mac : macs) out.push_back(mac[5]);
  std::sort(out.begin(), out.end());
  return out;
}

uint32_t sbndaq::FEBDRV::GrayToBin(uint32_t n) {
  //NOTE OK
  uint32_t res=0;
  bool a[32],b[32];
  for(int i=0; i<32; i++) { //read bits of n into a[]
    if(n & 0x80000000) a[i]=1; else a[i]=0;
    n = n<<1;
  }
  b[0]=a[0];
  for(int i=1; i<32; i++) {
    if(a[i]) if(!b[i-1]) b[i]=1; else b[i]=0;
    else b[i]=b[i-1];
  }
  for(int i=0; i<32; i++) {
    res=(res<<1);
    res=(res | b[i]);
  }
  return res;
}

void sbndaq::FEBDRV::ConfigSetBit(uint8_t *buffer, uint16_t bitlen, uint16_t bit_index, bool value) {
  //NOTE function never used
  uint8_t byte = buffer[(bitlen-1-bit_index)/8];
  uint8_t mask = 1 << (7-bit_index%8);
  byte &= ~mask;
  if(value) byte=byte | mask;
  buffer[(bitlen-1-bit_index)/8]=byte;
}

bool ConfigGetBit(uint8_t *buffer, uint16_t bitlen, uint16_t bit_index) {
  //NOTE function never used
  uint8_t byte = buffer[(bitlen-1-bit_index)/8];
  uint8_t mask = 1 << (7-bit_index%8);
  byte &= mask;
  return byte;
}


bool sbndaq::FEBDRV::sendtofeb(int len, FEBDTP_PKT_t const& spkt) { //sending spkt
  //NOTE OK
  struct ifreq if_idx;
  struct ifreq if_mac;
  uint8_t thisdstmac[6];
  memcpy(thisdstmac,spkt.dst_mac,6);
  struct sockaddr_ll socket_address;

  /* Get the index of the interface to send on */
  memset(&if_idx, 0, sizeof(struct ifreq));
  strncpy(if_idx.ifr_name, ifName, IFNAMSIZ);
  if (ioctl(sockfd_w, SIOCGIFINDEX, &if_idx) < 0) {
    TLOG(TLVL_ERROR)<<__func__<<"(len="<<len<<",cmd="<<spkt.CMD<<",mac5="<<thisdstmac[5]<<") : SIOCGIFINDEX error";
    return 0;
  }
  /* Get the MAC address of the interface to send on */
  memset(&if_mac, 0, sizeof(struct ifreq));
  strncpy(if_mac.ifr_name, ifName, IFNAMSIZ);
  if (ioctl(sockfd_w, SIOCGIFHWADDR, &if_mac) < 0) {
    TLOG(TLVL_ERROR)<<__func__<<"(len="<<len<<",cmd="<<spkt.CMD<<",mac5="<<thisdstmac[5]<<") : SIOCGIFHWADDR error";
    return 0;
  }
  
  /* Index of the network device */
  socket_address.sll_ifindex = if_idx.ifr_ifindex;
  /* Address length*/
  socket_address.sll_halen = ETH_ALEN;
  /* Destination MAC */
  memcpy(socket_address.sll_addr,spkt.dst_mac,6);
  /* Send packet */
  if (sendto(sockfd_w, (char*)&spkt, len, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0) {
    driver_state=DRV_SENDERROR;
    TLOG(TLVL_ERROR)<<__func__<<"len="<<len<<",cmd="<<spkt.CMD<<",mac5="<<thisdstmac[5]<<") : sendto error";
    return 0;
  }

  return 1;
}

int sbndaq::FEBDRV::recvfromfeb(int timeout_us, FEBDTP_PKT_t & rcvrpkt) {//result is in rpkt
  struct timeval tv;
  tv.tv_sec = 0;  /* 0 Secs Timeout */
  tv.tv_usec=timeout_us;
  
  if (setsockopt(sockfd_r, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval))== -1){
    TLOG(TLVL_ERROR)<<__func__<<"SO_SETTIMEOUT";
    TLOG(TLVL_ERROR)<<__func__<<"(cmd="<<rcvrpkt.CMD<<",mac5="<<rcvrpkt.src_mac[5]<<") : socket timeout error";
    return 0;
  }
  int numbytes = recvfrom(sockfd_r, &rcvrpkt, 1500, 0, NULL, NULL); //TODO 1500 = MAXPACKLEN
  
  if (numbytes<=0) {
    driver_state=DRV_RECVERROR;
    //TLOG(TLVL_ERROR)<<__func__<<"FEBDRV::recvfeb(cmd=%u,mac5=%2X) : nbytes=%d error",rcvrpkt.CMD,rcvrpkt.src_mac[5],numbytes);
    return 0;
  } //timeout
  return numbytes;
}

int sbndaq::FEBDRV::flushlink() {
  //NOTE OK
  // set short timeout and Flush input buffer 
  while(recvfromfeb(1000,rpkt)>0) {}
  driver_state=DRV_OK;
  return 1;
}

int sbndaq::FEBDRV::sendcommand(const uint8_t *mac, uint16_t cmd, uint16_t reg, uint8_t * buf) {
  //NOTE OK
  FEBDTP_PKT_t spkt;
  spkt.iptype = 0x0108;
  int packlen=64;
  memcpy(&spkt.dst_mac,mac,6);
  memcpy(&spkt.src_mac,hostmac,6);
  memcpy(&spkt.CMD,&cmd,2);
  memcpy(&spkt.REG,&reg,2);
  switch (cmd) {
  case FEB_GET_RATE :
  case FEB_GEN_INIT :
  case FEB_GEN_HVON :
  case FEB_GEN_HVOF :
    break;
  case FEB_SET_RECV :
    memcpy(&spkt.Data,buf,6); // Copy source mac to the data buffer
    break;
  case FEB_WR_SR :
    memcpy(&spkt.Data,buf,1); // Copy register value to the data buffer 
    break;
  case FEB_WR_SRFF :
  case FEB_WR_SCR  :
  case FEB_WR_PMR  :
  case FEB_WR_CDR :
    memcpy(&spkt.Data,buf,256); // Copy 256 register values to the data buffer
    packlen=256+18;
    break;
  case FEB_RD_CDR :
    break;
  case FEB_RD_FW :
  case FEB_WR_FW :
    memcpy(&spkt.Data,buf,5); // Copy address and numblocks
    break;
  case FEB_DATA_FW :
    memcpy(&spkt.Data,buf,1024); // Copy 1kB  data buffer
    packlen=1024+18;
    break;
  }
  flushlink();
  return sendtofeb(packlen,spkt);
}

bool sbndaq::FEBDRV::startDAQ(uint8_t mac5) {
  //NOTE OK
  //TODO process errors

  TLOG(TLVL_INFO)<<__func__<<"("<<mac5<<") called";

  unsigned int nreplies=0;
  dstmac[5]=mac5;
  
  sendcommand(dstmac,FEB_GEN_INIT,0,buf); //stop DAQ on the FEB
  
  //calibrate offset between system and steady clock
  steady_clock_offset = std::chrono::system_clock::now().time_since_epoch().count() - std::chrono::steady_clock::now().time_since_epoch().count();
  
  while(recvfromfeb(10000,rpkt)) {
    if(rpkt.CMD!=FEB_OK) {
      TLOG(TLVL_ERROR)<<__func__<<"() RESPONSE COMMAND (after stop DAQ) NOT OK!"<<rpkt.CMD;
      return 0;
    }
    nreplies++;
  }

  if(nreplies!=macs.size() && mac5==255) {
    TLOG(TLVL_ERROR)<<__func__<<
          " Received different number of replies than clients (after stop DAQ) "<<nreplies<<" ≠ "<<macs.size();
    return 0;
  }
  
  nreplies=0;
  sendcommand(dstmac,FEB_GEN_INIT,1,buf); //reset buffer
  
  while(recvfromfeb(10000,rpkt)) {
    if(rpkt.CMD!=FEB_OK){
      TLOG(TLVL_ERROR)<<__func__<<"() RESPONSE COMMAND (after reset buffer) NOT OK! "<<rpkt.CMD;
      return 0;
    }
    nreplies++;
  }
  if(nreplies==0) return 0;
  if(nreplies!=macs.size() && mac5==255){
    TLOG(TLVL_INFO)<<__func__<<
          " Received different number of replies than clients (after reset buffer) "<<nreplies<<" ≠ "<<macs.size();
    return 0;
  }

  nreplies=0;
  sendcommand(dstmac,FEB_GEN_INIT,2,buf); //set DAQ_Enable flag on FEB

  while(recvfromfeb(10000,rpkt)) {
    if(rpkt.CMD!=FEB_OK){
      TLOG(TLVL_ERROR)<<__func__<<"() RESPONSE COMMAND (after DAQ enable) NOT OK! "<<rpkt.CMD;
      return 0;
    }
    nreplies++;
  }
  if(nreplies==0) return 0;
  if(nreplies!=macs.size() && mac5==255){
    TLOG(TLVL_ERROR)<<__func__<<
          " Received different number of replies than clients (after DAQ enable) "<<nreplies<<" ≠ "<<macs.size();
    return 0;
  }
  return 1;
}


void sbndaq::FEBDRV::biasON(uint8_t mac5) {
  //NOTE: OK
  TLOG(TLVL_INFO)<<__func__<<"("<<mac5<<") called.";

  unsigned int nreplies=0;
  dstmac[5]=mac5;
  sendcommand(dstmac,FEB_GEN_HVON,0,buf); //reset buffer
  
  bool status = true;
  
  while(recvfromfeb(10000,rpkt)) {
    if(rpkt.CMD!=FEB_OK){
      TLOG(TLVL_ERROR)<<__func__<<"("<<mac5<<")  RESPONSE COMMAND NOT OK! "<<rpkt.CMD;
      status = false;
    }
    nreplies++;
  }
  if(nreplies==0) {
    TLOG(TLVL_ERROR)<<__func__<<"("<<mac5<<")  nreplies = "<<nreplies<<" ≠ "<<macs.size();
    status = false;
  }
  if(nreplies!=macs.size() && mac5==255) {
    TLOG(TLVL_ERROR)<<__func__<<"("<<mac5<<")  nreplies ≠ nclients ("<<nreplies<<" ≠ "<<macs.size()<<")";
    status = false;
  }
  
  if(!status){
    throw cet::exception( std::string(TRACE_NAME) +  __func__ + "("+std::to_string(mac5)+") Failed to turn on bias voltage");
  }
}

void sbndaq::FEBDRV::biasOFF(uint8_t mac5) {
  /**
   * Turn off bias voltage in FEB with given MAC
   * If mac5 equals 255 voltage is turned off on all FEBs
   * NOTE: OK
   */
  TLOG(TLVL_INFO)<<__func__<<"("<<mac5<<") called.";

  unsigned int nreplies=0;
  dstmac[5]=mac5;
  sendcommand(dstmac,FEB_GEN_HVOF,0,buf); //reset buffer
  
  bool status = true;
  
  while(recvfromfeb(10000,rpkt)) {
    if(rpkt.CMD!=FEB_OK){
      TLOG(TLVL_ERROR)<<__func__<<"("<<mac5<<")  RESPONSE COMMAND NOT OK! "<<rpkt.CMD;
      status = false;
    }
    nreplies++;
  }
  if(nreplies==0) {
    TLOG(TLVL_ERROR)<<__func__<<"("<<mac5<<")  nreplies = "<<nreplies;
    status = false;
  }
  if(nreplies!=macs.size() && mac5==255) {
    TLOG(TLVL_ERROR)<<__func__<<"("<<mac5<<")  nreplies ("<<nreplies<<") ≠ nclients ("<<macs.size()<<")";
    status = false;
  }
  
  if(!status){
    throw cet::exception( std::string(TRACE_NAME) +  __func__ + "("+std::to_string(mac5)+") Failed to turn off bias voltage");
  }
}

bool sbndaq::FEBDRV::sendconfig(uint8_t mac5, uint8_t * bufSCR, uint16_t lenSCR, uint8_t * bufPMR, uint16_t lenPMR) {
    /**
   * Send PROBE and SLOW CONTROL configuration to FEB
   * //NOTE: OK
   * TODO add crash?
   */
  TLOG(TLVL_DEBUG)<<__func__<<"("<<mac5<<", bufSCR, "<<lenSCR<<", bufPMR, "<< lenPMR<<") called";
  
  if(lenSCR != 1144/8){
    TLOG(TLVL_ERROR)<<__func__<<"()  Bad slow control config length: "<<lenSCR<<" is not the expected "<<(1144/8);
    return 0;
  }
  if(lenPMR != 224/8){
    TLOG(TLVL_ERROR)<<__func__<<"()  Bad PROBE config length: "<<lenPMR<<" is not the expected "<<(224/8);
    return 0;
  }
  
  unsigned int nreplies=0;
  dstmac[5]=mac5;
  sendcommand(dstmac,FEB_WR_SCR,0x0000,bufSCR);
  while(recvfromfeb(50000,rpkt)) {
    if(rpkt.CMD!=FEB_OK_SCR) {
      TLOG(TLVL_ERROR)<<__func__<<"("<<mac5<<")  SCR bad config! cmd "<<rpkt.CMD<<" ≠ "<<FEB_OK_SCR;
      return 0;
    }
    nreplies++;
  }
  if(nreplies==0){
    TLOG(TLVL_ERROR)<<__func__<<"("<<mac5<<")  SCR bad config! nreplies="<<nreplies;
    return 0;
  }
  if(nreplies!=macs.size() && mac5==255){
    TLOG(TLVL_ERROR)<<__func__<<"("<<mac5<<")  SCR bad config! nreplies("<<nreplies<<") ≠ nclients("<<macs.size()<<")";
    return 0;
  }

  sendcommand(dstmac,FEB_WR_PMR,0x0000,bufPMR);
  if(!recvfromfeb(50000,rpkt)){
    TLOG(TLVL_ERROR)<<__func__<<"("<<mac5<<")  PMR bad config! No cmd received";
    return 0;
  }
  while(recvfromfeb(50000,rpkt)) {
    if(rpkt.CMD!=FEB_OK_PMR) {
      TLOG(TLVL_ERROR)<<__func__<<"("<<mac5<<")  PMR bad config! cmd "<<rpkt.CMD<<" ≠ "<<FEB_OK_PMR;
      return 0;
    }
    nreplies++;
  }
  if(nreplies==0){
    TLOG(TLVL_ERROR)<<__func__<<"("<<mac5<<")  PMR bad config! nreplies="<<nreplies;
    return 0;
  }
  if(nreplies!=macs.size() && mac5==255){
    TLOG(TLVL_ERROR)<<__func__<<"("<<mac5<<")  PMR bad config! nreplies("<<nreplies<<")≠ nclients("<<macs.size()<<")";
    return 0;
  }
  
  TLOG(TLVL_DEBUG)<<__func__<<"("<<mac5<<", bufSCR, "<<lenSCR<<", bufPMR, "<< lenPMR<<") completed, configuration loaded successfully";
  return 1;
}



int sbndaq::FEBDRV::recvL2pack() {
  return recvfromfeb(5000,rpkt);
}

void sbndaq::FEBDRV::processL2pack(int datalen,const uint8_t mac){
  //reset_buffer
  feb_buffer_counter[mac] = 0;

  sbndaq::BernCRTZMQEvent *evt=0; //TODO update, something wrong here
  
  int jj=0;
  while(jj<datalen) {
    auto ovrwr_ptr = reinterpret_cast<uint16_t*>(&(rpkt).Data[jj]);
    overwritten=*ovrwr_ptr;
    jj=jj+2;
    
    auto lif_ptr = reinterpret_cast<uint16_t*>(&(rpkt).Data[jj]);
    lostinfpga=*lif_ptr;
    jj=jj+2;
    
    total_lost+=lostinfpga;
    lostperpoll_fpga[rpkt.src_mac[5]] += lostinfpga;
    
    auto ts0_ptr = reinterpret_cast<uint32_t*>(&(rpkt).Data[jj]);
    uint32_t ts0=*ts0_ptr; jj=jj+4;
    auto ts1_ptr = reinterpret_cast<uint32_t*>(&(rpkt).Data[jj]);
    uint32_t ts1=*ts1_ptr; jj=jj+4;
    
    uint8_t ls2b0=ts0 & 0x00000003;
    uint8_t ls2b1=ts1 & 0x00000003;
    uint32_t tt0=(ts0 & 0x3fffffff) >>2;
    uint32_t tt1=(ts1 & 0x3fffffff) >>2;
    tt0=(GrayToBin(tt0) << 2) | ls2b0;
    tt1=(GrayToBin(tt1) << 2) | ls2b1;
    tt0=tt0+5;//IK: correction based on phase drift w.r.t GPS
    tt1=tt1+5; //IK: correction based on phase drift w.r.t GPS
    bool NOts0=ts0 & 0x40000000; // check overflow bit
    bool NOts1=ts1 & 0x40000000;
    
    bool REFEVTts0,REFEVTts1;
    if((ts0 & 0x80000000)>0) {
      REFEVTts0=1;
      ts0=tt0;
      ts0_ref_MEM[rpkt.src_mac[5]]=tt0;
    }
    else {
      REFEVTts0=0;
      ts0=tt0;
    }
    if((ts1 & 0x80000000)>0) {
      REFEVTts1=1;
      ts1=tt1;
      ts1_ref_MEM[rpkt.src_mac[5]]=tt1;
    }
    else {
      REFEVTts1=0;
      ts1=tt1;
    }

    evt = &feb_buffer[mac][feb_buffer_counter[mac]++];
    if(evt==0) {
      driver_state=DRV_BUFOVERRUN; 
      TLOG(TLVL_ERROR)<<__func__<<"Buffer overrun for FEB "<<mac<<" !! Aborting."; 
      continue;
    }
    evt->ts0=ts0;
    evt->ts1=ts1;
    evt->flags=0;
    evt->mac5=rpkt.src_mac[5];
    if(!NOts0) evt->flags|=0x0001;    //opposite logic! 1 if TS is present, 0 if not!
    if(!NOts1) evt->flags|=0x0002;
    if(REFEVTts0) evt->flags|=0x0004; //bit indicating TS0 reference event
    if(REFEVTts1) evt->flags|=0x0008; //bit indicating TS1 reference event
    for(int kk=0; kk<32; kk++) { 
      auto adc_ptr = reinterpret_cast<uint16_t*>(&(rpkt).Data[jj]);
      evt->adc[kk]=*adc_ptr; 
      jj++; 
      jj++;
    }
    total_acquired++;
    evtsperpoll[rpkt.src_mac[5]]++;
  }
  
}

int sbndaq::FEBDRV::recvandprocessL2pack(const uint8_t mac){
  
  int numbytes = recvL2pack();
  
  if(numbytes<=0) return numbytes;
  if(rpkt.CMD!=FEB_DATA_CDR) {
    TLOG(TLVL_ERROR)<<__func__<<"("<<mac<<")  rpkt.CMD("<<rpkt.CMD<<") is not "<<FEB_DATA_CDR<<"!";
    return -1; //should not happen, but just in case..
  }
  processL2pack(numbytes-18,mac);
  
  return numbytes;
}

int sbndaq::FEBDRV::polldata_setup() {
  
  msperpoll=0;
  overwritten=0;
  lostinfpga=0;

//   memset(lostperpoll_cpu,0,sizeof(lostperpoll_cpu));
//   memset(lostperpoll_fpga,0,sizeof(lostperpoll_fpga));
//   memset(evtsperpoll,0,sizeof(evtsperpoll));
  
  return 1;
}

void sbndaq::FEBDRV::pollfeb(const uint8_t* mac) {
  TLOG(TLVL_DEBUG)<<__func__<<"("<<mac[5]<<") called";
  sendcommand(mac,FEB_RD_CDR,0,buf);
  rpkt.CMD=0; //clear these out
}

void sbndaq::FEBDRV::updateoverwritten() {
  total_lost+=overwritten;
  lostperpoll_cpu[rpkt.src_mac[5]]+=overwritten;
}

void sbndaq::FEBDRV::polldata_complete() {
//   ftime(&mstime1);
//   msperpoll=(mstime1.time-mstime0.time)*1000+(mstime1.millitm-mstime0.millitm);
}


bool sbndaq::FEBDRV::polldata() {// poll data from daisy-chain

  if(!polldata_setup()) return false;
  
  for(auto mac : macs) { //loop over all connected febs
    uint8_t tmp_mac[6];
    for(int i = 0; i < 6; i++) tmp_mac[i] = mac[i];
    pollfeb(tmp_mac);
    
    while(rpkt.CMD!=FEB_EOF_CDR) { //loop on messages from one FEB
      int numbytes = recvandprocessL2pack(mac[5]);
      if(numbytes <= 0) break;
    }
    updateoverwritten();
    
  } //loop over FEBS
  
  polldata_complete();
  return false;
  
}
