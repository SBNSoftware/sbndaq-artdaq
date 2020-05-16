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
  GLOB_daqon(0),
  nclients(0),
  sockfd_w(-1), 
  sockfd_r(-1),
  driver_state(DRV_OK),
  dstmac{0x00,0x60,0x37,0x12,0x34,0x00}, //base mac for FEBs, last byte 0->255
  brcmac{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},
  overwritten(0),
  lostinfpga(0),
  total_lost(0),
  total_acquired(0)
  { TLOG(TLVL_INFO)<<__func__<<"Constructed FEBDRV\n"; }


void sbndaq::FEBDRV::Init(const char* iface){
  
  sprintf(ifName,"%s",iface);

  TLOG(TLVL_INFO) << __func__ << " Calling driver Init with iface = " << iface;
  
  // network interface to febs
  if(!initif()) {
    TLOG(TLVL_ERROR)<<__func__<<"FEBDRV::Init  Can't initialize network interface "<<iface<<"! Exiting.";
    driver_state=DRV_INITERR;
    return;
  }
  TLOG(TLVL_INFO)<<__func__<<"FEBDRV::Init  FEB network interface initialized.";
  
  pingclients();
  
  for(int i = 0; i < nclients; i++) {
    std::vector<sbndaq::BernCRTZMQEvent> single_feb_buffer(1024); //TODO remove this buffer entirely
    feb_buffer[macs[i][5]] = single_feb_buffer;
  }
  driver_state=DRV_OK;
}

uint32_t sbndaq::FEBDRV::GrayToBin(uint32_t n) {
  uint32_t res=0;
  int a[32],b[32],i=0;//,c=0;

  for(i=0; i<32; i++){ if((n & 0x80000000)>0) a[i]=1; else a[i]=0; 
    n=n<<1; 
  }
  b[0]=a[0];

  for(i=1; i<32; i++){ 
    if(a[i]>0){
      if(b[i-1]==0) b[i]=1; 
      else b[i]=0;
    }
    else 
      b[i]=b[i-1]; 
  }

  for(i=0; i<32; i++){
    res=(res<<1);
    res=(res | b[i]); 
  }

  return res;
}


void sbndaq::FEBDRV::ConfigSetBit(uint8_t *buffer, uint16_t bitlen, uint16_t bit_index, bool value)
{
  uint8_t byte;
  uint8_t mask;
  byte=buffer[(bitlen-1-bit_index)/8];
  mask= 1 << (7-bit_index%8);
  byte=byte & (~mask);
  if(value) byte=byte | mask;
  buffer[(bitlen-1-bit_index)/8]=byte;
}

bool sbndaq::FEBDRV::ConfigGetBit(uint8_t *buffer, uint16_t bitlen, uint16_t bit_index)
{
  uint8_t byte;
  uint8_t mask;
  byte=buffer[(bitlen-1-bit_index)/8];
  mask= 1 << (7-bit_index%8);
  byte=byte & mask;
  if(byte!=0) return true; else return false; 
}


bool sbndaq::FEBDRV::initif() {
  tv.tv_sec = 0;  /* 0 Secs Timeout */
  tv.tv_usec = 500000;  // 500ms
  
  /* Open RAW socket to send on */
  if ((sockfd_w = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1) {
    TLOG(TLVL_ERROR)<<__func__<<" sender socket error";
    return false;
  }
  /* Bind RAW to device */
  if (setsockopt(sockfd_w, SOL_SOCKET, SO_BINDTODEVICE, ifName, IF_NAMESIZE-1) == -1){
    TLOG(TLVL_ERROR)<<__func__<<"SO_BINDTODEVICE RAW";
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
    TLOG(TLVL_ERROR)<<__func__<<"bind sockfd_w";
    return 0;
  }
  
  /* Open PF_PACKET socket, listening for EtherType ETHER_TYPE */
  spkt.iptype=0x0108; // IP type 0x0801
  if ((sockfd_r = socket(PF_PACKET, SOCK_RAW, spkt.iptype)) == -1) {
    TLOG(TLVL_ERROR)<<__func__<<" listener socket error";
    return false;
  }
  
  if (setsockopt(sockfd_r, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval))== -1){
    TLOG(TLVL_ERROR)<<__func__<<" SO_RCVTIMEO error";
    return false;
  }
  /* Bind listener to device */
  if (setsockopt(sockfd_r, SOL_SOCKET, SO_BINDTODEVICE, ifName, IFNAMSIZ-1) == -1){
    TLOG(TLVL_ERROR)<<__func__<<" SO_BINDTODEVICE error";
    return false;
  }
  if (bind(sockfd_r, (struct sockaddr *)&sockaddr_ll_s, sizeof(struct sockaddr_ll)) == -1){
    TLOG(TLVL_ERROR)<<__func__<<"bind sockfd_r";
    return false;
  }

  /* Get the index of the interface to send on */
  memset(&if_idx, 0, sizeof(struct ifreq));
  strncpy(if_idx.ifr_name, ifName, IFNAMSIZ-1);
  if_idx.ifr_name[IFNAMSIZ-1] = '\0';
  if (ioctl(sockfd_w, SIOCGIFINDEX, &if_idx) < 0){
    TLOG(TLVL_ERROR)<<__func__<<" SIOCGIFINDEX error";
    return false;
  }
  /* Get the MAC address of the interface to send on */
  memset(&if_mac, 0, sizeof(struct ifreq));
  strncpy(if_mac.ifr_name, ifName, IFNAMSIZ-1);
  if (ioctl(sockfd_w, SIOCGIFHWADDR, &if_mac) < 0) {
    TLOG(TLVL_ERROR)<<__func__<<" SIOCGIFHWADDR error";
    return false;
  }

  memcpy(&hostmac,((uint8_t *)&if_mac.ifr_hwaddr.sa_data),6);
  TLOG(TLVL_INFO)<<__func__<<"initialized "<<ifName<<" with host MAC  "<<std::hex<< hostmac[0]<<":"<<hostmac[1]<<":"<<hostmac[2]<<":"<<hostmac[3]<<":"<<hostmac[4]<<":"<<hostmac[5]<<std::dec;
  return true;

}


int sbndaq::FEBDRV::sendtofeb(int len, FEBDTP_PKT_t const& spkt) { //sending spkt
  struct ifreq if_idx;
  struct ifreq if_mac;
  uint8_t   thisdstmac[6];
  memcpy(thisdstmac,spkt.dst_mac,6);
  struct sockaddr_ll socket_address;

  /* Get the index of the interface to send on */
  memset(&if_idx, 0, sizeof(struct ifreq));
  strncpy(if_idx.ifr_name, ifName, IFNAMSIZ-1);
  if (ioctl(sockfd_w, SIOCGIFINDEX, &if_idx) < 0) {
    TLOG(TLVL_ERROR)<<__func__<<"SIOCGIFINDEX"; 
    TLOG(TLVL_ERROR)<<__func__<<"FEBDRV::sendtofeb(len="<<len<<",cmd="<<spkt.CMD<<",mac5="<<std::hex<<thisdstmac[5]<<") : SIOCGIFINDEX error";
    return 0;
  }
  /* Get the MAC address of the interface to send on */
  memset(&if_mac, 0, sizeof(struct ifreq));
  strncpy(if_mac.ifr_name, ifName, IFNAMSIZ-1);
  if (ioctl(sockfd_w, SIOCGIFHWADDR, &if_mac) < 0) {
    TLOG(TLVL_ERROR)<<__func__<<"SIOCGIFHWADDR";
    TLOG(TLVL_ERROR)<<__func__<<"FEBDRV::sendtofeb(len="<<len<<",cmd="<<spkt.CMD<<",mac5="<<std::hex<<thisdstmac[5]<<") : SIOCGIFHWADDR error";
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
    TLOG(TLVL_ERROR)<<__func__<<"FEBDRV::sendtofeb(len="<<len<<",cmd="<<spkt.CMD<<",mac5="<<std::hex<<thisdstmac[5]<<") : sendto error";
    return 0;
  }

  return 1; 
}

int sbndaq::FEBDRV::recvfromfeb(int timeout_us, FEBDTP_PKT_t & rcvrpkt) {//result is in rpkt

  tv.tv_usec=timeout_us;
  if (setsockopt(sockfd_r, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval))== -1){
    TLOG(TLVL_ERROR)<<__func__<<"SO_SETTIMEOUT";
    TLOG(TLVL_ERROR)<<__func__<<"(cmd="<<rcvrpkt.CMD<<",mac5="<<std::hex<<rcvrpkt.src_mac[5]<<") : socket timeout error";
    return 0;
  }
  numbytes = recvfrom(sockfd_r, &rcvrpkt, MAXPACKLEN, 0, NULL, NULL);
  
  if (numbytes<=0) {
    driver_state=DRV_RECVERROR;
    //TLOG(TLVL_ERROR)<<__func__<<"FEBDRV::recvfeb(cmd=%u,mac5=%2X) : nbytes=%d error",rcvrpkt.CMD,rcvrpkt.src_mac[5],numbytes);
    return 0;
  } //timeout

  FEB_lastheard[rcvrpkt.src_mac[5]]=time(NULL);

  return numbytes;
}

int sbndaq::FEBDRV::flushlink()
{
  // set short timeout and Flush input buffer 
  while(recvfromfeb(1000,rpkt)>0) {}
  driver_state=DRV_OK; 
  return 1;
}

int sbndaq::FEBDRV::sendcommand(const uint8_t *mac, uint16_t cmd, uint16_t reg, uint8_t * buf) {

  FEBDTP_PKT_t spkt;
  //int numbytes=1;
  //int retval=0;
  int packlen=64;
  //int tout=50000;
  memcpy(&spkt.dst_mac,mac,6);
  memcpy(&spkt.src_mac,hostmac,6);
  spkt.iptype = 0x0108;
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
  packlen=sendtofeb(packlen,spkt);
  if(packlen<=0) return 0; 
  return packlen;
}

uint64_t sbndaq::FEBDRV::MACAddress(int client) const {
  if(client >= nclients)
    return 0;

  uint64_t thismac=0,tmp;
  for(int i=5; i>=0; --i){
    tmp = macs[client][i];
    thismac = thismac + ( tmp << (8*(5-i)) );
  }
  return thismac;
}

int sbndaq::FEBDRV::pingclients() {
  if(GLOB_daqon!=0) {
    TLOG(TLVL_ERROR)<<__func__<<" called while DAQ ON is not allowed!";
    return 0; //ping only allowed when DAQ is off
  }
  nclients=0;
  int changed=0;
  uint8_t febs[256];
  memset(febs,0,256);

  dstmac[5]=0xff; 
  sendcommand(dstmac,FEB_SET_RECV,FEB_VCXO[nclients],hostmac);
  
  while(recvfromfeb(10000,rpkt)) if(rpkt.CMD==FEB_OK) {
      memcpy(macs[nclients],rpkt.src_mac,6);
      febs[macs[nclients][5]]=1;
      if(FEB_present[macs[nclients][5]]==0) {
        TLOG(TLVL_INFO)<<__func__<<"FEBDRV::pingclients()  Newly connected FEB: ",
              std::hex<<macs[nclients][0]<<":"<<macs[nclients][1]<<":"<<macs[nclients][2]<<":"<<macs[nclients][3]<<":"<<macs[nclients][4]<<":"<<macs[nclients][5]<<" "
              std::dec<<rpkt.Data;
        sprintf(verstr[nclients],"%s",rpkt.Data); 
        changed=1;
      }
      nclients++;
  }
  for(int f=0;f<256;f++) {
      if(FEB_present[f]==1 && febs[f]==0) {
        TLOG(TLVL_INFO)<<__func__<<"FEBDRV::pingclients()  Disconnected FEB: "<<
          std::hex<<macs[nclients][0]<<":"<<macs[nclients][1]<<":"<<macs[nclients][2]<<":"<<macs[nclients][3]<<":"<<macs[nclients][4]<<":"<<f<<std::dec;
        changed=1;
      }
      FEB_present[f]=febs[f];
    }
  if(changed) {
    TLOG(TLVL_INFO)<<__func__<<"FEBDRV::pingclients()  In total "<<nclients<<" FEBs connected.";
  }
  return nclients;
}

int sbndaq::FEBDRV::sendconfig(uint8_t mac5)
{
  int nreplies=0;
  int PrevConfigured=FEB_configured[mac5];
  dstmac[5]=mac5;
  FEB_configured[mac5]=0;
  sendcommand(dstmac,FEB_WR_SCR,0x0000,bufSCR[mac5]);
  while(recvfromfeb(50000,rpkt)) { 
    if(rpkt.CMD!=FEB_OK_SCR) {
      TLOG(TLVL_ERROR)<<__func__<<"FEBDRV::sendconfig("<<std::hex<<mac5<<")  SCR bad config! cmd "<<rpkt.CMD<<"!="<<FEB_OK_SCR;
      return 0;
    }
    nreplies++;
  }
  if(nreplies==0){
    TLOG(TLVL_ERROR)<<__func__<<"FEBDRV::sendconfig("<<std::hex<<mac5<<")  SCR bad config! nreplies="<<nreplies;
    return 0;
  }
  if(nreplies!=nclients && mac5==255){
    TLOG(TLVL_ERROR)<<__func__<<"FEBDRV::sendconfig("<<std::hex<<mac5<<")  SCR bad config! nreplies("<<nreplies<<")!=nclients("<<nclients<<")";
    return 0;
  }


  sendcommand(dstmac,FEB_WR_PMR,0x0000,bufPMR[mac5]);
  if(!recvfromfeb(50000,rpkt)){
    TLOG(TLVL_ERROR)<<__func__<<"FEBDRV::sendconfig("<<std::hex<<mac5<<")  PMR bad config! No cmd received";
    return 0;
  }
  while(recvfromfeb(50000,rpkt)) { 
    if(rpkt.CMD!=FEB_OK_PMR) {
      TLOG(TLVL_ERROR)<<__func__<<"FEBDRV::sendconfig("<<std::hex<<mac5<<")  PMR bad config! cmd "<<rpkt.CMD<<"!="<<FEB_OK_PMR;
      return 0;
    }
    nreplies++;
  }
  if(nreplies==0){
    TLOG(TLVL_ERROR)<<__func__<<"FEBDRV::sendconfig("<<std::hex<<mac5<<")  PMR bad config! nreplies="<<nreplies;
    return 0;
  }
  if(nreplies!=nclients && mac5==255){
    TLOG(TLVL_ERROR)<<__func__<<"FEBDRV::sendconfig("<<std::hex<<mac5<<")  PMR bad config! nreplies("<<nreplies<<")!=nclients("<<nclients<<")";
    return 0;
  }
  FEB_configured[mac5]=PrevConfigured+1;
  
  return 1;
}


int sbndaq::FEBDRV::stopDAQ(uint8_t)// mac5)
{
  TLOG(TLVL_INFO)<<__func__<<"FEBDRV::stopDAQ called.";
  GLOB_daqon=0;
  return 1;
}

int sbndaq::FEBDRV::startDAQ(uint8_t mac5)
{

  TLOG(TLVL_INFO)<<__func__<<"FEBDRV::startDAQ(%02X) called.",mac5);

  int nreplies=0;
  dstmac[5]=mac5; 
  
  memset(evbuf,0, sizeof(evbuf));
  memset(evnum,0, sizeof(evnum));
  memset(evbufstat,0, sizeof(evbufstat));
  
  nreplies=0;
  sendcommand(dstmac,FEB_GEN_INIT,0,buf); //stop DAQ on the FEB
  
  while(recvfromfeb(10000,rpkt)) { 
    if(rpkt.CMD!=FEB_OK) {
      TLOG(TLVL_ERROR)<<__func__<<"FEBDRV::startDAQ  RESPONSE COMMAND (after stop DAQ) NOT OK! %u",rpkt.CMD);
      return 0;
    }
    nreplies++;
  }

  if(nreplies!=nclients && mac5==255) {
    TLOG(TLVL_ERROR)<<__func__<<
          "FEBDRV::startDAQ  Received different number of replies than clients (after stop DAQ), %d!=%d",
          nreplies,nclients);
    return 0;
  }
  nreplies=0;
  sendcommand(dstmac,FEB_GEN_INIT,1,buf); //reset buffer
  
  while(recvfromfeb(10000,rpkt)) { 
    if(rpkt.CMD!=FEB_OK){
      TLOG(TLVL_ERROR)<<__func__<<"FEBDRV::startDAQ  RESPONSE COMMAND (after reset buffer) NOT OK! %u",rpkt.CMD);
      return 0;
    }
    nreplies++;
  }
  if(nreplies==0) return 0;
  if(nreplies!=nclients && mac5==255){
    TLOG(TLVL_INFO)<<__func__<<
          "FEBDRV::startDAQ  Received different number of replies than clients (after reset buffer), %d!=%d",
          nreplies,nclients);
    return 0;
  }

  nreplies=0;
  sendcommand(dstmac,FEB_GEN_INIT,2,buf); //set DAQ_Enable flag on FEB

  while(recvfromfeb(10000,rpkt)) { 
    if(rpkt.CMD!=FEB_OK){
      TLOG(TLVL_ERROR)<<__func__<<"FEBDRV::startDAQ  RESPONSE COMMAND (after DAQ enable) NOT OK! %u",rpkt.CMD);
      return 0;
    }
    FEB_daqon[rpkt.src_mac[5]]=1;
    nreplies++;
  }
  if(nreplies==0) return 0;
  if(nreplies!=nclients && mac5==255){
    TLOG(TLVL_INFO)<<__func__<<
          "FEBDRV::startDAQ  Received different number of replies than clients (after DAQ enable), %d!=%d",
          nreplies,nclients);
    return 0;
  }
  GLOB_daqon=1;
  return 1;
}


int sbndaq::FEBDRV::biasON(uint8_t mac5)
{
  TLOG(TLVL_INFO)<<__func__<<"FEBDRV::biasON(%02X) called.",mac5);

  int nreplies=0;
  dstmac[5]=mac5; 
  sendcommand(dstmac,FEB_GEN_HVON,0,buf); //reset buffer
  
  while(recvfromfeb(10000,rpkt)) { 
    if(rpkt.CMD!=FEB_OK){
      TLOG(TLVL_ERROR)<<__func__<<"FEBDRV::biasOn(%02X)  RESPONSE COMMAND NOT OK! %u",mac5,rpkt.CMD);
      return 0;
    }
  nreplies++;
  FEB_biason[rpkt.src_mac[5]]=1;
  }
  if(nreplies==0) {
    TLOG(TLVL_ERROR)<<__func__<<"FEBDRV::biasOn(%02X)  nreplies= %d",mac5,nreplies);
    return 0;
  }
  if(nreplies!=nclients && mac5==255) {
    TLOG(TLVL_ERROR)<<__func__<<"FEBDRV::biasOn(%02X)  nreplies!=nclients (%d!=%d)",mac5,nreplies,nclients);
    return 0;
  }
  return 1;

}
int sbndaq::FEBDRV::biasOFF(uint8_t mac5)
{
  TLOG(TLVL_INFO)<<__func__<<"FEBDRV::biasOFF(%02X) called.",mac5);

  int nreplies=0;
  dstmac[5]=mac5; 
  sendcommand(dstmac,FEB_GEN_HVOF,0,buf); //reset buffer
  
  while(recvfromfeb(10000,rpkt)) { 
    if(rpkt.CMD!=FEB_OK){
      TLOG(TLVL_ERROR)<<__func__<<"FEBDRV::biasOFF(%02X)  RESPONSE COMMAND NOT OK! %u",mac5,rpkt.CMD);
      return 0;
    }
  nreplies++;
  FEB_biason[rpkt.src_mac[5]]=0;
  }
  if(nreplies==0) {
    TLOG(TLVL_ERROR)<<__func__<<"FEBDRV::biasOFF(%02X)  nreplies= %d",mac5,nreplies);
    return 0;
  }
  if(nreplies!=nclients && mac5==255) {
    TLOG(TLVL_ERROR)<<__func__<<"FEBDRV::biasOFF(%02X)  nreplies!=nclients (%d!=%d)",mac5,nreplies,nclients);
    return 0;
  }

  return 1;
}

int sbndaq::FEBDRV::configu(uint8_t mac5, const uint8_t *buf1, int len)
{
  int rv=1;
  TLOG(TLVL_INFO)<<__func__<<"FEBDRV::configu(%02x,buf,%d) called",mac5,len);

  if(mac5==0xFF){
    TLOG(TLVL_ERROR)<<__func__<<"FEBDRV::configu  Cannot call configuration in broadcast mode!";
    return 0;    
  }

  if(len != (1144+224)/8){ 
    rv=0; 
    TLOG(TLVL_ERROR)<<__func__<<"FEBDRV::configu  Bad config length: %d is not the expected %d",len,(1144+224)/8);
    return 0;
  }
  memcpy(bufSCR[mac5],buf1,1144/8);
  memcpy(bufPMR[mac5],buf1+1144/8,224/8);
  
  rv=sendconfig(mac5);
  return rv;
}

int sbndaq::FEBDRV::getSCR(uint8_t mac5, uint8_t *buf1)
{
  int rv=1;
  memcpy(buf1,bufSCR[mac5],1144/8);
  return rv;
}


void sbndaq::FEBDRV::free_subbufer (void*, void *hint) //call back from ZMQ sent function, hint points to subbufer index
{
  int sbi;
  sbi =  (intptr_t)hint; //reset buffer status to empty
  evbufstat[sbi]=0;
  evnum[sbi]=0; 
}


int sbndaq::FEBDRV::sendstats2() //send statistics in binary packet format
{
  void* ptr;
  DRIVER_STATUS_t ds;
  FEB_STATUS_t fs;
  
 ds.datime=time(NULL);
 ds.daqon=GLOB_daqon;  
 ds.status=driver_state;
 ds.nfebs=nclients;  
 ds.msperpoll=msperpoll;
 zmq_msg_t msg;
 zmq_msg_init_size (&msg, sizeof(ds)+nclients*sizeof(fs));
 ptr=zmq_msg_data (&msg);
 memcpy(ptr,&ds,sizeof(ds)); ptr=(char*)ptr+sizeof(ds);

 // Get event rates
 for(int f=0; f<nclients;f++) //loop on all connected febs : macs[f][6]
   {
     sendcommand(macs[f],FEB_GET_RATE,0,buf);
     memset(&fs,0,sizeof(fs));
     fs.connected=1;
     if(recvfromfeb(10000,rpkt)==0) fs.connected=0;  
     if(rpkt.CMD!=FEB_OK)   fs.error=1;  
     
     auto evtrate_ptr = reinterpret_cast<float*>(rpkt.Data);
     fs.evtrate = *evtrate_ptr;
     
     fs.configured=FEB_configured[macs[f][5]];
     fs.biason=FEB_biason[macs[f][5]];
     sprintf(fs.fwcpu,"%s",verstr[f]);
     sprintf(fs.fwfpga,"rev3.1";
     memcpy(fs.mac,macs[f],6);
     
     if(GLOB_daqon){
       fs.evtperpoll=evtsperpoll[macs[f][5]];
       fs.lostcpu=lostperpoll_cpu[macs[f][5]];
       fs.lostfpga=lostperpoll_fpga[macs[f][5]];
     }
     memcpy(ptr,&fs,sizeof(fs)); ptr=(char*)ptr+sizeof(fs);
     
   }
 
 zmq_msg_send (&msg, pubstats2, ZMQ_DONTWAIT);
 zmq_msg_close (&msg);
 return 1; 
}

int sbndaq::FEBDRV::sendstats()
{
  char str[2048]; //stats string
  float Rate;
  memset(str,0,sizeof(str));
  
  time_t result=time(NULL);
  if(GLOB_daqon) sprintf(str+strlen(str), "\nDAQ ON; System time: %s", asctime(gmtime(&result))); 
  else sprintf(str+strlen(str), "\nDAQ OFF; System time: %s", asctime(gmtime(&result))); 
  
  // Get event rates
  for(int f=0; f<nclients;f++) //loop on all connected febs : macs[f][6]
    {
      sendcommand(macs[f],FEB_GET_RATE,0,buf); 
      if(recvfromfeb(10000,rpkt)==0) 
        sprintf(str+strlen(str), "Timeout for FEB %02x:%02x:%02x:%02x:%02x:%02x !\n",macs[f][0],macs[f][1],macs[f][2],macs[f][3],macs[f][4],macs[f][5]);
      if(rpkt.CMD!=FEB_OK)    
        sprintf(str+strlen(str), "no FEB_OK for FEB %02x:%02x:%02x:%02x:%02x:%02x !\n",macs[f][0],macs[f][1],macs[f][2],macs[f][3],macs[f][4],macs[f][5]);
      
      auto evtrate_ptr = reinterpret_cast<float*>(rpkt.Data);
      Rate = *evtrate_ptr;
      
      //Rate=*(reinterpret_cast<float*>(rpkt.Data));
      sprintf(str+strlen(str), "FEB %02x:%02x:%02x:%02x:%02x:%02x ",macs[f][0],macs[f][1],macs[f][2],macs[f][3],macs[f][4],macs[f][5]);
      sprintf(str+strlen(str), "%s Conf: %d Bias: %d ",verstr[f],FEB_configured[macs[f][5]],FEB_biason[macs[f][5]]);
      if(GLOB_daqon)
        {
          sprintf(str+strlen(str), "Per poll: %d ",evtsperpoll[macs[f][5]]);
          sprintf(str+strlen(str), "Lost CPU: %d ",lostperpoll_cpu[macs[f][5]]);
          sprintf(str+strlen(str), "FPGA: %d ",lostperpoll_fpga[macs[f][5]]);
        }
      sprintf(str+strlen(str), "rate %5.1f Hz\n",Rate);
    }
  if(GLOB_daqon) sprintf(str+strlen(str), "Poll %d ms.\n",msperpoll);
  
  zmq_msg_t msg;
  zmq_msg_init_size (&msg, strlen(str)+1);
  memcpy(zmq_msg_data (&msg), str, strlen(str)+1);
  zmq_msg_send (&msg, pubstats, ZMQ_DONTWAIT);
  zmq_msg_close (&msg);
  return 1; 
  
}

int sbndaq::FEBDRV::recvL2pack(){
  return recvfromfeb(5000,rpkt);
}

void sbndaq::FEBDRV::processL2pack(int datalen,const uint8_t* mac){
  //reset_buffer
  feb_buffer_counter[mac] = 0;

  sbndaq::BernCRTZMQEvent *evt=0;
  
  int jj=0;
  while(jj<datalen) {
    auto ovrwr_ptr = reinterpret_cast<uint16_t*>(&(rpkt).Data[jj]);
    overwritten=*ovrwr_ptr;
    jj=jj+2;
    
    auto lif_ptr = reinterpret_cast<uint16_t*>(&(rpkt).Data[jj]);
    lostinfpga=*lif_ptr;
    jj=jj+2;
    
    total_lost+=lostinfpga;
    lostperpoll_fpga[rpkt.src_mac[5]]+=lostinfpga;
    
    auto ts0_ptr = reinterpret_cast<uint32_t*>(&(rpkt).Data[jj]);
    ts0=*ts0_ptr; jj=jj+4;
    auto ts1_ptr = reinterpret_cast<uint32_t*>(&(rpkt).Data[jj]);
    ts1=*ts1_ptr; jj=jj+4;
    
    ls2b0=ts0 & 0x00000003;
    ls2b1=ts1 & 0x00000003;
    tt0=(ts0 & 0x3fffffff) >>2;
    tt1=(ts1 & 0x3fffffff) >>2;
    tt0=(GrayToBin(tt0) << 2) | ls2b0;
    tt1=(GrayToBin(tt1) << 2) | ls2b1;
    tt0=tt0+5;//IK: correction based on phase drift w.r.t GPS
    tt1=tt1+5; //IK: correction based on phase drift w.r.t GPS
    NOts0=((ts0 & 0x40000000)>0); // check overflow bit
    NOts1=((ts1 & 0x40000000)>0);
    
    if((ts0 & 0x80000000)>0)
      {
        REFEVTts0=1; 
        ts0=tt0; 
        ts0_ref_MEM[rpkt.src_mac[5]]=tt0;
      } 
    else {
      REFEVTts0=0; 
      ts0=tt0; 
    }
    if((ts1 & 0x80000000)>0) 
      {
        REFEVTts1=1; 
        ts1=tt1; 
        ts1_ref_MEM[rpkt.src_mac[5]]=tt1;
      } 
    else {
      REFEVTts1=0; 
      ts1=tt1; 
    }

    evt = &feb_buffer[mac][febbuffer_counter[mac]++];
    if(evt==0) {
      driver_state=DRV_BUFOVERRUN; 
      TLOG(TLVL_ERROR)<<__func__<<"Buffer overrun for FEB S/N %02X !! Aborting.",mac[5]); 
      continue;} 
    evt->ts0=ts0;
    evt->ts1=ts1;
    evt->flags=0;
    evt->mac5=rpkt.src_mac[5];
    if(NOts0==0) evt->flags|=0x0001;    //opposite logic! 1 if TS is present, 0 if not!    
    if(NOts1==0) evt->flags|=0x0002;        
    if(REFEVTts0==1) evt->flags|=0x0004; //bit indicating TS0 reference event        
    if(REFEVTts1==1) evt->flags|=0x0008; //bit indicating TS1 reference event        
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

int sbndaq::FEBDRV::recvandprocessL2pack(const uint8_t* mac){
  
  numbytes = recvL2pack();
  
  if(numbytes<=0) return numbytes;
  if(rpkt.CMD!=FEB_DATA_CDR) {
    TLOG(TLVL_ERROR)<<__func__<<"FEBDRV::recvandprocessL2pack(%02x)  rpkt.CMD(%u) is not %u!",
          mac[5],rpkt.CMD,FEB_DATA_CDR);
    return -1; //should not happen, but just in case..
  }
  processL2pack(numbytes-18,mac);
  
  return numbytes;
}

int sbndaq::FEBDRV::polldata_setup(){

  ftime(&mstime0);
  
  msperpoll=0;
  overwritten=0;
  lostinfpga=0;

  memset(lostperpoll_cpu,0,sizeof(lostperpoll_cpu));
  memset(lostperpoll_fpga,0,sizeof(lostperpoll_fpga));
  memset(evtsperpoll,0,sizeof(evtsperpoll));
  
  if(GLOB_daqon==0) {sleep(1); return 0;} //if no DAQ running - just delay for not too fast ping
  
  return 1;
}

void sbndaq::FEBDRV::pollfeb(const uint8_t* mac) { 
  TLOG(TLVL_INFO)<<__func__<<"FEBDRV::pollfeb("<<std::hex<<mac[5]<<") called.";
  sendcommand(mac,FEB_RD_CDR,0,buf); 
  numbytes=1; rpkt.CMD=0; //clear these out
}

void sbndaq::FEBDRV::updateoverwritten() {
  total_lost+=overwritten;
  lostperpoll_cpu[rpkt.src_mac[5]]+=overwritten;
}

void sbndaq::FEBDRV::polldata_complete() {
  ftime(&mstime1);
  msperpoll=(mstime1.time-mstime0.time)*1000+(mstime1.millitm-mstime0.millitm);
}


int sbndaq::FEBDRV::polldata() // poll data from daysy-chain and send it to the publisher socket
{  

  int rv=0;
  if(polldata_setup()==0) return 0;
  
  for(int f=0; f<nclients;f++) //loop on all connected febs : macs[f][6]
    {
      pollfeb(macs[f]);
      
      while(numbytes>0 && rpkt.CMD!=FEB_EOF_CDR) //loop on messages from one FEB
        if( recvandprocessL2pack(macs[f])<=0 ) continue;
      
      updateoverwritten();
      
    } //loop on FEBS
  
  polldata_complete();
  return rv;
  
}
