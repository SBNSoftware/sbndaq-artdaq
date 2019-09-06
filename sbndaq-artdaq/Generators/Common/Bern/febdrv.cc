#include <zmq.h>
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
#include <sys/timeb.h>
#include <string>
#include "febdrv.h"
#include "febevt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sbndaq-artdaq-core/Overlays/Common/BernCRTZMQFragment.hh"

#define DEBUG_umut 0
#define EVSPERFEB 1024   // max events per feb per poll to buffer
#define NBUFS 2   // number of buffers for double-buffering

void *context = NULL;
int infoLength = 0;

//  Socket to respond to clients
void *responder = NULL;
//  Socket to send data to clients
void *publisher = NULL;
//  Socket to send statistics to clients
void *pubstats = NULL;
void *pubstats2 = NULL;

FEBDTP_PKT_t spkt,rpkt; //send and receive packets
FEBDTP_PKT_t ipkt[FEB_MAX_CHAIN]; // store info packets
sbndaq::BernCRTZMQEvent evbuf[NBUFS][256*EVSPERFEB+1]; //0MQ backend event buffer, first index-triple-buffering, second - feb, third-event
int evnum[NBUFS]; //number of good events in the buffer fields
int evbufstat[NBUFS]; //flag, showing current status of sub-buffer: 0= empty, 1= being filled, 2= full, 3=being sent out
int evtsperpoll[256];
int msperpoll=0;
int lostperpoll_cpu[256];
int lostperpoll_fpga[256];
uint8_t ts0ok[256],ts1ok[256];
struct timeb mstime0, mstime1;


int nclients=0;
uint8_t macs[256][6]; //list of detected clients
char verstr[256][32]; //list of version strings of clients
uint8_t hostmac[6];
char ifName[IFNAMSIZ];
//int sockfd;
int sockfd_w=-1; 
int sockfd_r=-1;
struct timeval tv;
struct ifreq if_idx;
struct ifreq if_mac;
int driver_state=DRV_OK;
uint8_t dstmac[6]={0x00,0x60,0x37,0x12,0x34,0x00}; //base mac for FEBs, last byte 0->255
uint8_t brcmac[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

///////////////Forward declarations
int startDAQ(uint8_t mac5);
int getInnfo();

///////////////End Forward declarations


void printdate()
{
  char str[64];
  time_t result=time(NULL);
  sprintf(str,"%s", asctime(gmtime(&result))); 
  str[strlen(str)-1]=0; //remove CR simbol
  printf("%s ", str); 
}

void printmac( uint8_t *mac)
{
  printf("%02x:%02x:%02x:%02x:%02x:%02x",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
}

uint32_t GrayToBin(uint32_t n)
{
  uint32_t res=0;
  int a[32],b[32],i=0,c=0;
  for(i=0; i<32; i++)
    { 
      if((n & 0x80000000)>0) a[i]=1; else a[i]=0; 
      n=n<<1; 
    }
  b[0]=a[0];
  for(i=1; i<32; i++)
    { 
      if(a[i]>0) if(b[i-1]==0) b[i]=1; else b[i]=0;
      else b[i]=b[i-1]; 
    }
  for(i=0; i<32; i++)
    {
      res=(res<<1);
      res=(res | b[i]); 
    }
  return res;
}

void ConfigSetBit(uint8_t *buffer, uint16_t bitlen, uint16_t bit_index, bool value)
{
  uint8_t byte;
  uint8_t mask;
  byte=buffer[(bitlen-1-bit_index)/8];
  mask= 1 << (7-bit_index%8);
  byte=byte & (~mask);
  if(value) byte=byte | mask;
  buffer[(bitlen-1-bit_index)/8]=byte;
}

bool ConfigGetBit(uint8_t *buffer, uint16_t bitlen, uint16_t bit_index)
{
  uint8_t byte;
  uint8_t mask;
  byte=buffer[(bitlen-1-bit_index)/8];
  mask= 1 << (7-bit_index%8);
  byte=byte & mask;
  if(byte!=0) return true; else return false; 
}

int initif(char *iface)
{
  spkt.iptype=0x0108; // IP type 0x0801
  tv.tv_sec = 0;  /* 0 Secs Timeout */
  tv.tv_usec = 500000;  // 500ms
  sprintf(ifName,"%s",iface); 
  
  /* Open RAW socket to send on */
  if ((sockfd_w = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1) 
    {
      perror("sender: socket");
      return 0;
    }
  /* Open PF_PACKET socket, listening for EtherType ETHER_TYPE */
  if ((sockfd_r = socket(PF_PACKET, SOCK_RAW, spkt.iptype)) == -1) 
    {
      perror("listener: socket");	
      return 0;
    }
  
  if (setsockopt(sockfd_r, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval))== -1)
    {
      perror("SO_SETTIMEOUT");
      return 0;
    }
  /* Bind listener to device */
  if (setsockopt(sockfd_r, SOL_SOCKET, SO_BINDTODEVICE, ifName, IFNAMSIZ-1) == -1)
    {
      perror("SO_BINDTODEVICE");
      return 0;
    }
  
  /* Get the index of the interface to send on */
  memset(&if_idx, 0, sizeof(struct ifreq));
  strncpy(if_idx.ifr_name, ifName, IFNAMSIZ-1);
  if (ioctl(sockfd_w, SIOCGIFINDEX, &if_idx) < 0)
    {
      perror("SIOCGIFINDEX"); 
      return 0;
    }
  /* Get the MAC address of the interface to send on */
  memset(&if_mac, 0, sizeof(struct ifreq));
  strncpy(if_mac.ifr_name, ifName, IFNAMSIZ-1);
  if (ioctl(sockfd_w, SIOCGIFHWADDR, &if_mac) < 0)
    {
      perror("SIOCGIFHWADDR"); 
      return 0;
    }
  memcpy(&hostmac,((uint8_t *)&if_mac.ifr_hwaddr.sa_data),6);
  memcpy(&(spkt.src_mac),hostmac,6);
  printdate(); printf("febdrv: initialized %s with MAC  ", ifName);
  printmac(hostmac);
  printf("\n");
  return 1;
}

int sendtofeb(int len)  //sending spkt
{
  struct ifreq if_idx;
  struct ifreq if_mac;
  uint8_t   thisdstmac[6];
  memcpy(thisdstmac,spkt.dst_mac,6);
  struct sockaddr_ll socket_address;
  /* Get the index of the interface to send on */
  memset(&if_idx, 0, sizeof(struct ifreq));
  strncpy(if_idx.ifr_name, ifName, IFNAMSIZ-1);
  if (ioctl(sockfd_w, SIOCGIFINDEX, &if_idx) < 0)
    {
      perror("SIOCGIFINDEX"); 
      return 0;
    }
  /* Get the MAC address of the interface to send on */
  memset(&if_mac, 0, sizeof(struct ifreq));
  strncpy(if_mac.ifr_name, ifName, IFNAMSIZ-1);
  if (ioctl(sockfd_w, SIOCGIFHWADDR, &if_mac) < 0)
    {
      perror("SIOCGIFHWADDR");
      return 0;
    }
  /* Index of the network device */
  socket_address.sll_ifindex = if_idx.ifr_ifindex;
  /* Address length*/
  socket_address.sll_halen = ETH_ALEN;
  /* Destination MAC */
  memcpy(socket_address.sll_addr,spkt.dst_mac,6);
  /* Send packet */
  if (sendto(sockfd_w, (char*)&spkt, len, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0)
    {
      driver_state=DRV_SENDERROR; 
      return 0;
    }
  //	printf("host->feb: sent packet %d bytes from  %02x:%02x:%02x:%02x:%02x:%02x ", len,spkt.src_mac[0],spkt.src_mac[1],spkt.src_mac[2],spkt.src_mac[3],spkt.src_mac[4],spkt.src_mac[5]);
  //	printf("to %02x:%02x:%02x:%02x:%02x:%02x.\n", spkt.dst_mac[0],spkt.dst_mac[1],spkt.dst_mac[2],spkt.dst_mac[3],spkt.dst_mac[4],spkt.dst_mac[5]);
  return 1; 
}

int recvfromfeb(int timeout_us) //result is in rpkt
{
  int numbytes;
  // set receive timeout
  tv.tv_usec=timeout_us;
  if (setsockopt(sockfd_r, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval))== -1)
    {
      perror("SO_SETTIMEOUT");
      return 0;
    }
  numbytes = recvfrom(sockfd_r, &rpkt, MAXPACKLEN, 0, NULL, NULL);
  if(numbytes<=0) 
    {
      driver_state=DRV_RECVERROR; 
      return 0;
    } //timeout
  //printf("feb->host: received packet %d bytes from  %02x:%02x:%02x:%02x:%02x:%02x.\n", numbytes,rpkt.src_mac[0],rpkt.src_mac[1],rpkt.src_mac[2],rpkt.src_mac[3],rpkt.src_mac[4],rpkt.src_mac[5]);
  FEB_lastheard[rpkt.src_mac[5]]=time(NULL);
  return numbytes;
}

int flushlink()
{
  // set short timeout and Flush input buffer 
  while(recvfromfeb(1000)>0) {}
  driver_state=DRV_OK; 
  return 1;
}

int sendcommand(uint8_t *mac, uint16_t cmd, uint16_t reg, uint8_t * buf)
{
  int numbytes=1;
  int retval=0;
  int packlen=64;
  int tout=50000;
  memcpy(&spkt.dst_mac,mac,6);        
  memcpy(&spkt.CMD,&cmd,2);
  memcpy(&spkt.REG,&reg,2);
  switch (cmd) 
    {
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
  packlen=sendtofeb(packlen);
  if(packlen<=0) return 0; 
  return packlen;
}

void usage()
{
  printf("Usage: to init febdrv on eth0 interface with poll duration extended by 300 ms, type \n");
  printf("febdrv eth0 300 [listening_port]\n");
  printf("if listening_port is not specified, default is 5555\n");
  printf("The data publisher, stats and stats2 port numberss are larger by 1, 2 and 3, respectively\n");
}

int pingclients()
{
  if(GLOB_daqon!=0) return 0; //ping only allowed when DAQ is off
  nclients=0;
  int changed=0;
  uint8_t febs[256];
  memset(febs,0,256);
  //printf("Pinging connected FEBs with FEB_SET_RECV..\n");
  dstmac[5]=0xff; 
  sendcommand(dstmac,FEB_SET_RECV,FEB_VCXO[nclients],hostmac);
  
  while(recvfromfeb(10000)) 
    if(rpkt.CMD==FEB_OK) 
      {
	memcpy(&ipkt[nclients],&rpkt,sizeof(rpkt));
	memcpy(macs[nclients],rpkt.src_mac,6);
	febs[macs[nclients][5]]=1;
	if(FEB_present[macs[nclients][5]]==0)
	  {
	    printdate(); printf("Newly connected FEB: ");
	    for(int i=0;i<5;i++) printf("%02x:",macs[nclients][i]);
	    printf("%02x ",macs[nclients][5]);
	    printf("%s\n",rpkt.Data); // new client reply received
	    sprintf(verstr[nclients],"%s",rpkt.Data); 
	    changed=1;
	    // startDAQ(macs[nclients][5]); GLOB_daqon=0;
	  }
	nclients++;
      }
  for(int f=0;f<256;f++)
    {
      if(FEB_present[f]==1 && febs[f]==0) 
	{
	  printdate(); printf("Disconnected FEB: ");
	  for(int i=0;i<5;i++) printf("%02x:",macs[0][i]);
	  printf("%02x \n",f);
	  changed=1;
	}
      FEB_present[f]=febs[f];
    }
  if(changed) 
    {
      printdate();  
      printf("In total %d FEBs connected.\n",nclients);
    }
  return nclients;
}

int getInfo()
{
  pingclients();
  char ethernet[80];
  std::string text = std::to_string(nclients) + "\n";
  for ( int i=0; i<nclients; i++)
  {
    sprintf(ethernet,
        "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X ",
        ipkt[i].src_mac[0], ipkt[i].src_mac[1],
        ipkt[i].src_mac[2], ipkt[i].src_mac[3],
        ipkt[i].src_mac[4], ipkt[i].src_mac[5]);
    text += ethernet;
    text += (char *)&ipkt[i].Data[0];
    text += '\n';
  }
  infoLength = text.size();
  memcpy(&buf[0],text.c_str(),infoLength + 1);
  return(0);
}

int sendconfig(uint8_t mac5)
{
  //t->SendCMD(t->dstmac,FEB_SET_RECV,fNumberEntry75->GetNumber(),t->srcmac);
  int nreplies=0;
  int PrevConfigured=FEB_configured[mac5];
  dstmac[5]=mac5;
  FEB_configured[mac5]=0;
  sendcommand(dstmac,FEB_WR_SCR,0x0000,bufSCR[mac5]);
  while(recvfromfeb(50000)) 
    { 
      if(rpkt.CMD!=FEB_OK_SCR) return 0;
      nreplies++;
    }
  if(nreplies==0) return 0;
  if(nreplies!=nclients && mac5==255) return 0;
  
  sendcommand(dstmac,FEB_WR_PMR,0x0000,bufPMR[mac5]);
  if(!recvfromfeb(50000))  return 0;
  while(recvfromfeb(50000)) 
    { 
      if(rpkt.CMD!=FEB_OK_PMR) return 0;
      nreplies++;
    }
  if(nreplies==0) return 0;
  if(nreplies!=nclients && mac5==255) return 0;
  
  FEB_configured[mac5]=PrevConfigured+1;
  
  return 1;
}

int stopDAQ()
{
  GLOB_daqon=0;
  return 1;
}

int startDAQ(uint8_t mac5)
{
  int nreplies=0;
  dstmac[5]=mac5; 
  
  memset(evbuf,0, sizeof(evbuf));
  memset(evnum,0, sizeof(evnum));
  memset(evbufstat,0, sizeof(evbufstat));
  
  nreplies=0;
  sendcommand(dstmac,FEB_GEN_INIT,0,buf); //stop DAQ on the FEB
  
  while(recvfromfeb(10000)) 
    { 
      if(rpkt.CMD!=FEB_OK) return 0;
      nreplies++;
    }
  if(nreplies==0) return 0;
  if(nreplies!=nclients && mac5==255) return 0;
  
  nreplies=0;
  sendcommand(dstmac,FEB_GEN_INIT,1,buf); //reset buffer
  
  while(recvfromfeb(10000)) 
    { 
      if(rpkt.CMD!=FEB_OK) return 0;
      nreplies++;
    }
  if(nreplies==0) return 0;
  if(nreplies!=nclients && mac5==255) return 0;
  
  nreplies=0;
  sendcommand(dstmac,FEB_GEN_INIT,2,buf); //set DAQ_Enable flag on FEB
  while(recvfromfeb(10000)) 
    { 
      if(rpkt.CMD!=FEB_OK) return 0;
      FEB_daqon[rpkt.src_mac[5]]=1;
      nreplies++;
    }
  if(nreplies==0) return 0;
  if(nreplies!=nclients && mac5==255) return 0;
  GLOB_daqon=1;
  return 1;
}

int biasON(uint8_t mac5)
{
  int nreplies=0;
  dstmac[5]=mac5; 
  sendcommand(dstmac,FEB_GEN_HVON,0,buf); //reset buffer
  
  while(recvfromfeb(10000)) 
    { 
      if(rpkt.CMD!=FEB_OK) return 0;
      nreplies++;
      FEB_biason[rpkt.src_mac[5]]=1;
    }
  if(nreplies==0) return 0;
  if(nreplies!=nclients && mac5==255) return 0;
  
  return 1;
}
int biasOFF(uint8_t mac5)
{
  int nreplies=0;
  dstmac[5]=mac5; 
  sendcommand(dstmac,FEB_GEN_HVOF,0,buf); //reset buffer
  
  while(recvfromfeb(10000)) 
    { 
      if(rpkt.CMD!=FEB_OK) return 0;
      nreplies++;
      FEB_biason[rpkt.src_mac[5]]=0;
    }
  if(nreplies==0) return 0;
  if(nreplies!=nclients && mac5==255) return 0;
  return 1;
}

int configu(uint8_t mac5, uint8_t *buf1, int len)
{
  int rv=1;
  //printf("called configu(%02x,buf,%d)\n",mac5,len);
  if(len != (1144+224)/8) 
    { 
      rv=0; 
      return 0;
    }
  memcpy(bufSCR[mac5],buf1,1144/8);
  memcpy(bufPMR[mac5],buf1+1144/8,224/8);
  
  rv=sendconfig(mac5);
  return rv;
}

int getSCR(uint8_t mac5, uint8_t *buf1)
{
  int rv=1;
  memcpy(buf1,bufSCR[mac5],1144/8);
  return rv;
}


sbndaq::BernCRTZMQEvent * getnextevent() //flag, showing current status of sub-buffer: 0= empty, 1= being filled, 2= full, 3=being sent out
{
  sbndaq::BernCRTZMQEvent * retval=0;
  //sbndaq::BernCRTZMQEvent evbuf[2][256*EVSPERFEB]; //0MQ backend event buffer, first index-triple-buffering, second - feb, third-event
  //int evnum[2]; //number of good events in the buffer fields
  //int evbufstat[2]; //flag, showing current status of sub-buffer: 0= empty, 1= being filled, 2= ready, 3=being sent out   
  int sbi=0;
  // check for available buffers
  for(sbi=0;sbi<NBUFS;sbi++) 
    if(evbufstat[sbi]==1)  ///check for buffer being filled
      {
	retval=&(evbuf[sbi][evnum[sbi]]); 
	evnum[sbi]++; 
	if(evnum[sbi]==EVSPERFEB*256)  evbufstat[sbi]=2; //buffer full, set to ready
	//      printf("%d",sbi);
	return retval;
      } //found buffer being filled, return pointer
  for(sbi=0;sbi<NBUFS;sbi++) 
    if(evbufstat[sbi]==0) ///check for empty buffer
      {
	retval=&(evbuf[sbi][0]); 
	evnum[sbi]=1; 
	evbufstat[sbi]=1; //buffer being filled
	//        printf("%d",sbi);
	return retval;
      } //started new buffer, return pointer
  //if we get here, than no buffers are available!
  return 0;
}


uint32_t ts0_ref_MEM[256];
uint32_t ts1_ref_MEM[256];

uint32_t overwritten=0;
uint32_t lostinfpga=0;
uint32_t total_lost=0;
uint32_t total_acquired=0;

void free_subbufer (void *data, void *hint) //call back from ZMQ sent function, hint points to subbufer index
{
  //return;
  uint64_t sbi;
  sbi =  (uint64_t)hint; //reset buffer status to empty
  evbufstat[sbi]=0;
  evnum[sbi]=0; 
  // printf("Subbuf %d transmission complete.\n",sbi); 
}

int senddata()
{
  //printf("enter sendata()\n");
  int sbi=0;
  void* bptr=0;
  int64_t sbitosend=-1; // check for filled buffers
  //printf("bufstat "); for(sbi=0;sbi<NBUFS;sbi++) { printf("%d ",evbufstat[sbi]); if(evbufstat[sbi]==1) printf("found 1 ");}  printf("\n");
  for(sbi=0;sbi<NBUFS;sbi++) 
    {
      if(evbufstat[sbi]==1) sbitosend=sbi;  //the if there is being filled buffer
      if(evbufstat[sbi]==2) sbitosend=sbi;  //first check for buffer that is full
    }
  //printf("sbitosend %d\n",sbitosend);
  if(sbitosend==-1) return 0; //no buffers to send out
  //
  evbufstat[sbitosend]=3; //  reset to 0 by the callback only when transmission is done!
  //fill buffer trailer in the last event
  //printf("p1\n");
  evbuf[sbitosend][evnum[sbitosend]].flags=0xFFFF;
  evbuf[sbitosend][evnum[sbitosend]].mac5=0xFFFF;
  evbuf[sbitosend][evnum[sbitosend]].ts0=MAGICWORD32;
  evbuf[sbitosend][evnum[sbitosend]].ts1=MAGICWORD32;
  evbuf[sbitosend][evnum[sbitosend]].coinc=MAGICWORD32;
  //printf("p2\n");
  bptr=&(evbuf[sbitosend][evnum[sbitosend]].adc[0]); //pointer to start of ADC field
  memcpy(bptr,&(evnum[sbitosend]),sizeof(int)); bptr=bptr+sizeof(int);
  memcpy(bptr,&mstime0,sizeof(struct timeb)); bptr=bptr+sizeof(struct timeb); //start of poll time
  memcpy(bptr,&mstime1,sizeof(struct timeb)); bptr=bptr+sizeof(struct timeb);  //end of poll time
  //printf("p3\n");
  evnum[sbitosend]++;
  zmq_msg_t msg;
  zmq_msg_init_data (&msg, evbuf[sbitosend], evnum[sbitosend]*EVLEN , free_subbufer, (void*)sbitosend);
  //printf("Scheduling subbuf %d for sending..\n",sbitosend);
  zmq_msg_send (&msg, publisher, ZMQ_DONTWAIT);
  //printf("done..\n");
  zmq_msg_close (&msg);
  //printf("exit sendata()\n");
  return 0;
}

int sendstats2() //send statistics in binary packet format
{
  // float Rate;
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
  memcpy(ptr,&ds,sizeof(ds)); ptr+=sizeof(ds);
  // Get event rates
  for(int f=0; f<nclients;f++) //loop on all connected febs : macs[f][6]
    {
      sendcommand(macs[f],FEB_GET_RATE,0,buf);
      memset(&fs,0,sizeof(fs));
      fs.connected=1;
      if(recvfromfeb(10000)==0) fs.connected=0;  
      //   sprintf(str+strlen(str), "Timeout for FEB %02x:%02x:%02x:%02x:%02x:%02x !\n",macs[f][0],macs[f][1],macs[f][2],macs[f][3],macs[f][4],macs[f][5]);
      if(rpkt.CMD!=FEB_OK)   fs.error=1;  
      //  sprintf(str+strlen(str), "no FEB_OK for FEB %02x:%02x:%02x:%02x:%02x:%02x !\n",macs[f][0],macs[f][1],macs[f][2],macs[f][3],macs[f][4],macs[f][5]);
      
      fs.evtrate=*((float*)(rpkt.Data));
      //   sprintf(str+strlen(str), "FEB %02x:%02x:%02x:%02x:%02x:%02x ",macs[f][0],macs[f][1],macs[f][2],macs[f][3],macs[f][4],macs[f][5]);
      //   sprintf(str+strlen(str), "%s Conf: %d Bias: %d ",verstr[f],FEB_configured[macs[f][5]],FEB_biason[macs[f][5]]);
      fs.configured=FEB_configured[macs[f][5]];
      fs.biason=FEB_biason[macs[f][5]];
      sprintf(fs.fwcpu,"%s",verstr[f]);
      sprintf(fs.fwfpga,"rev3.1");
      memcpy(fs.mac,macs[f],6);
      
      if(GLOB_daqon)
	{
	  fs.evtperpoll=evtsperpoll[macs[f][5]];
	  fs.lostcpu=lostperpoll_cpu[macs[f][5]];
	  fs.lostfpga=lostperpoll_fpga[macs[f][5]];
	  fs.ts0ok=ts0ok[macs[f][5]];
	  fs.ts1ok=ts1ok[macs[f][5]];
	  //    sprintf(str+strlen(str), "Per poll: %d ",evtsperpoll[macs[f][5]]);
	  //   sprintf(str+strlen(str), "Lost CPU: %d ",lostperpoll_cpu[macs[f][5]]);
	  //    sprintf(str+strlen(str), "FPGA: %d ",lostperpoll_fpga[macs[f][5]]);
	}
      //   sprintf(str+strlen(str), "rate %5.1f Hz\n",Rate);
      memcpy(ptr,&fs,sizeof(fs)); ptr+=sizeof(fs);
    }
  //if(GLOB_daqon) sprintf(str+strlen(str), "Poll %d ms.\n",msperpoll);
  zmq_msg_send (&msg, pubstats2, ZMQ_DONTWAIT);
  //printf("done..\n");
  zmq_msg_close (&msg);
  return 1; 
}

int sendstats()
{
  //printf("enter sendtats()\n");
  char str[8192]; //stats string
  float Rate;
  memset(str,0,sizeof(str));
  
  time_t result=time(NULL);
  if(GLOB_daqon) sprintf(str+strlen(str), "\nDAQ ON; System time: %s", asctime(gmtime(&result))); 
  else sprintf(str+strlen(str), "\nDAQ OFF; System time: %s", asctime(gmtime(&result))); 
  //for(int mac5=0;mac5<255;mac5++) printf("%02x %d ",mac5,FEB_configured[mac5]);    
  
  // Get event rates
  for(int f=0; f<nclients;f++) //loop on all connected febs : macs[f][6]
    {
      sendcommand(macs[f],FEB_GET_RATE,0,buf); 
      if(recvfromfeb(10000)==0) 
	sprintf(str+strlen(str), "Timeout for FEB %02x:%02x:%02x:%02x:%02x:%02x !\n",macs[f][0],macs[f][1],macs[f][2],macs[f][3],macs[f][4],macs[f][5]);
      if(rpkt.CMD!=FEB_OK)    
	sprintf(str+strlen(str), "no FEB_OK for FEB %02x:%02x:%02x:%02x:%02x:%02x !\n",macs[f][0],macs[f][1],macs[f][2],macs[f][3],macs[f][4],macs[f][5]);
      
      Rate=*((float*)(rpkt.Data));
      sprintf(str+strlen(str), "FEB %d %02x:%02x:%02x:%02x:%02x:%02x ",macs[f][5], macs[f][0],macs[f][1],macs[f][2],macs[f][3],macs[f][4],macs[f][5]);
      sprintf(str+strlen(str), "%s Conf: %d Bias: %d ",verstr[f],FEB_configured[macs[f][5]],FEB_biason[macs[f][5]]);
      if(GLOB_daqon)
	{
	  sprintf(str+strlen(str), "Per poll: %d ",evtsperpoll[macs[f][5]]);
	  sprintf(str+strlen(str), "Lost CPU: %d ",lostperpoll_cpu[macs[f][5]]);
	  sprintf(str+strlen(str), "FPGA: %d ",lostperpoll_fpga[macs[f][5]]);
	  sprintf(str+strlen(str), "GPS_OK: %d Beam_OK:%d ",ts0ok[macs[f][5]],ts1ok[macs[f][5]]);
	}
      sprintf(str+strlen(str), "rate %5.1f Hz\n",Rate);
      //  printf("%s",str);
    }
  if(GLOB_daqon) sprintf(str+strlen(str), "Poll %d ms.\n",msperpoll);
  
  zmq_msg_t msg;
  zmq_msg_init_size (&msg, strlen(str)+1);
  memcpy(zmq_msg_data (&msg), str, strlen(str)+1);
  zmq_msg_send (&msg, pubstats, ZMQ_DONTWAIT);
  //printf("done..\n");
  zmq_msg_close (&msg);
  //printf("exit sendtats()\n");
  return 1; 
}
//###############################################
void print_binary(int number, int num_digits)
{
  int digit;
  int q = 0;
  for(digit = num_digits - 1; digit >= 0; digit--)
    {
      printf("%c", number & (1<< digit) ? '1' : '0');
      q++;
      if(q == 4)
	{
	  printf(" ");
	  q = 0;
	}
    }
}

//###############################################
int polldata() // poll data from daysy-chain and send it to the publisher socket
{  
  //printf("enter polldata()\n");
  bool NOts0,NOts1;
  bool REFEVTts0,REFEVTts1;
  uint32_t tt0,tt1;
  uint32_t ts0,ts1;
  uint32_t ts0_ref,ts1_ref;
  uint8_t ls2b0,ls2b1; //least sig 2 bits
  uint16_t adc;
  uint64_t tmp;
  // umut
  uint32_t coinc;
  //
  ftime(&mstime0);
  //
  sbndaq::BernCRTZMQEvent *evt=0;
  int rv=0;
  int jj;
  int numbytes; //number of received bytes
  int datalen;  //data buffer
  msperpoll=0;
  overwritten=0;
  lostinfpga=0;
  //evtsperpoll=0;
  memset(lostperpoll_cpu,0,sizeof(lostperpoll_cpu));
  memset(lostperpoll_fpga,0,sizeof(lostperpoll_fpga));
  memset(evtsperpoll,0,sizeof(evtsperpoll));
  //
  if(GLOB_daqon==0) {sleep(1); return 0;} //if no DAQ running - just delay for not too fast ping
  for(int f=0; f<nclients;f++) //loop on all connected febs : macs[f][6]
  {
    ts0ok[macs[f][5]]=1;
    ts1ok[macs[f][5]]=1;
    sendcommand(macs[f],FEB_RD_CDR,0,buf);
    numbytes=1;
    rpkt.CMD=0; //just to start somehow
    while(numbytes>0 && rpkt.CMD!=FEB_EOF_CDR) //loop on messages from one FEB
    {
      numbytes=recvfromfeb(5000);
      if(numbytes==0) continue;       
      if(rpkt.CMD!=FEB_DATA_CDR) continue; //should not happen, but just in case..
      datalen=numbytes-18;
      //printf("feb->host: received data packet %d bytes CMD=%04x from  %02x:%02x:%02x:%02x:%02x:%02x.\n", numbytes,rpkt.CMD,rpkt.src_mac[0],rpkt.src_mac[1],rpkt.src_mac[2],rpkt.src_mac[3],rpkt.src_mac[4],rpkt.src_mac[5]);
			
      jj=0;
      while(jj<datalen)
      {
	//printf(" Remaining events: %d\n",(t->gpkt).REG);
	//printf("Flags: 0x%08x ",*(UInt_t*)(&(t->gpkt).Data[jj]));
	overwritten=*(uint16_t*)(&(rpkt).Data[jj]); 
	jj=jj+2;
	lostinfpga=*(uint16_t*)(&(rpkt).Data[jj]); 
	jj=jj+2;
	total_lost+=lostinfpga;
	lostperpoll_fpga[rpkt.src_mac[5]]+=lostinfpga;
	ts0=*(uint32_t*)(&(rpkt).Data[jj]); jj=jj+4; 
	ts1=*(uint32_t*)(&(rpkt).Data[jj]); jj=jj+4; 
	//	printf("T0=%u ns, T1=%u ns \n",ts0,ts1);
	ls2b0=ts0 & 0x00000003;
	ls2b1=ts1 & 0x00000003;
	tt0=(ts0 & 0x3fffffff) >>2; // and bit, right shift operators
	tt1=(ts1 & 0x3fffffff) >>2;
	tt0=(GrayToBin(tt0) << 2) | ls2b0; // lleft shift operator
	tt1=(GrayToBin(tt1) << 2) | ls2b1;
	tt0=tt0+5;//IK: correction based on phase drift w.r.t GPS
	tt1=tt1+5; //IK: correction based on phase drift w.r.t GPS
	NOts0=((ts0 & 0x40000000)>0); // check overflow bit
	NOts1=((ts1 & 0x40000000)>0);
	//        if((ts0 & 0x80000000)>0) {ts0=0x0; ts0_ref=tt0; ts0_ref_MEM[rpkt.src_mac[5]]=tt0;} 
	//        else { ts0=tt0; ts0_ref=ts0_ref_MEM[rpkt.src_mac[5]]; }
	//        if((ts1 & 0x80000000)>0) {ts1=0x0; ts1_ref=tt1; ts1_ref_MEM[rpkt.src_mac[5]]=tt1;} 
	//        else { ts1=tt1; ts1_ref=ts1_ref_MEM[rpkt.src_mac[5]]; }
				
	if((ts0 & 0x80000000)>0) 
	{
	  REFEVTts0=1; 
	  ts0=tt0; 
	  ts0_ref=tt0; 
	  ts0_ref_MEM[rpkt.src_mac[5]]=tt0;
	}
	else 
	{
	  REFEVTts0=0; 
	  ts0=tt0; 
	  ts0_ref=ts0_ref_MEM[rpkt.src_mac[5]]; 
	}
	if((ts1 & 0x80000000)>0) 
	{
	  REFEVTts1=1; 
	  ts1=tt1; 
	  ts1_ref=tt1; 
	  ts1_ref_MEM[rpkt.src_mac[5]]=tt1;
	}
	else
	{
	  REFEVTts1=0; 
	  ts1=tt1; 
	  ts1_ref=ts1_ref_MEM[rpkt.src_mac[5]]; 
	}
				
	//	 printf("T0=%u ns, T1=%u ns T0_ref=%u ns  T1_ref=%u ns \n",ts0,ts1,ts0_ref,ts1_ref);
	//         tmp=ts0*1e9/(ts0_ref-1); ts0=tmp;
	//         tmp=ts1*1e9/(ts0_ref-1); ts1=tmp;
	//         tmp=ts1_ref*1e9/(ts0_ref-1); ts1_ref=tmp;
	//	 printf("Corrected T0=%u ns, T1=%u ns T0_ref=%u ns  T1_ref=%u ns \n",ts0,ts1,ts0_ref,ts1_ref);
	evt=getnextevent();
	if(evt==0) 
	{
	  driver_state=DRV_BUFOVERRUN; 
	  printdate();  
	  printf("Buffer overrun for FEB S/N %d !! Aborting.\n",macs[f][5]); 
	  continue;
	} 
	evt->ts0=ts0;
	evt->ts1=ts1;
	evt->lostcpu=overwritten;
	evt->lostfpga=lostinfpga;  
	evt->flags=0;
	evt->mac5=rpkt.src_mac[5];
	// the "|" operator is the bitwise OR operator.
	if(NOts0==0) evt->flags|=0x0001; else ts0ok[macs[f][5]]=0;    //opposite logic! 1 if TS is present, 0 if not!    
	if(NOts1==0) evt->flags|=0x0002; else ts1ok[macs[f][5]]=0;
	if(REFEVTts0==1) evt->flags|=0x0004; //bit indicating TS0 reference event
	if(REFEVTts1==1) evt->flags|=0x0008; //bit indicating TS1 reference event
	//
	if(DEBUG_umut) printf("Mac5: %d Flag: %d TS0: %u ns TS1: %u ns\n",evt->mac5, evt->flags, evt->ts0, evt->ts1);
	for(int kk=0; kk<32; kk++) 
	{ 
	  evt->adc[kk]=*(uint16_t*)(&(rpkt).Data[jj]); 
	  if(DEBUG_umut) printf("%d ",evt->adc[kk]);
	  jj++; 
	  jj++;
	}
	//
	coinc = *(uint32_t*)(&(rpkt).Data[jj]); jj=jj+4;
	if(DEBUG_umut) print_binary(coinc,16);
	if(DEBUG_umut) printf("\n");
	//umut
	evt->coinc = coinc;
	//
	total_acquired++;
	evtsperpoll[rpkt.src_mac[5]]++;
      }   //loop on event buffer in one L2 packet     
    }	// loop on messages from 1 FEB
    total_lost+=overwritten;
    lostperpoll_cpu[rpkt.src_mac[5]]+=overwritten;
  } //loop on FEBS

  //if(evtsperpoll>0) printf(" %d events per poll.\n",evtsperpoll); 
  ftime(&mstime1);
  msperpoll=(mstime1.time-mstime0.time)*1000+(mstime1.millitm-mstime0.millitm);
  //printf("exit polldata()\n");
  return rv;
}

int polldelay=1;
int main (int argc, char **argv)
{
  
  //printf("WARNING!! The poll duration is set to minimum %d ms.\n",polldelay);
  int rv;
  char cmd[32]; //command string
  if(argc<3 || argc > 4) { usage(); return 0;}

  int listening_port;
  if(argc == 3) listening_port = 5555;
  else listening_port = atoi(argv[3]);

  char listening_socket[32], publishing_socket[32], stats_socket[32], stats2_socket[32];
  sprintf(listening_socket,  "tcp://*:%d", listening_port);
  sprintf(publishing_socket, "tcp://*:%d", listening_port+1);
  sprintf(stats_socket,      "tcp://*:%d", listening_port+2);
  sprintf(stats2_socket,     "tcp://*:%d", listening_port+3);

  polldelay=atoi(argv[2]);
  printf("WARNING!! The poll duration is set to minimum %d ms.\n",polldelay);
  memset(evbuf,0, sizeof(evbuf));
  memset(evnum,0, sizeof(evnum));
  memset(evbufstat,0, sizeof(evbufstat));
  
  memset(FEB_configured,0,256);
  memset(FEB_daqon,0,256);
  memset(FEB_evrate,0,256);
  memset(FEB_biason,0,256);
  memset(FEB_present,0,256);
  memset(FEB_lastheard,0,256*sizeof(time_t)); //number of seconds since the board is heard
  memset(FEB_VCXO,0,256*sizeof(uint16_t));
  // network interface to febs
  rv=initif(argv[1]);
  if(rv==0) 
    {
      printdate(); 
      printf("Can't initialize network interface %s! Exiting.\n",argv[1]); 
      return 0;
    }
  context = zmq_ctx_new();
  
  //  Socket to respond to clients
  responder = zmq_socket (context, ZMQ_REP);
  rv=zmq_bind (responder, listening_socket);
  if(rv<0) 
    {
      printdate(); 
      printf("Can't bind socket %s for command! Exiting.\n", listening_socket); 
      return 0;
    }
  //rv=zmq_bind (responder, "ipc://command");
  //if(rv<0) {printdate(); printf("Can't bind ipc socket for command! Exiting.\n"); return 0;}
  printdate(); printf ("febdrv: listening at %s\n", listening_socket);
  //printdate(); printf ("febdrv: listening at ipc://command\n");
  
  //  Socket to send data to clients
  publisher = zmq_socket (context, ZMQ_PUB);
  rv = zmq_bind (publisher, publishing_socket);
  if(rv<0) 
    {
      printdate(); 
      printf("Can't bind socket %s for data! Exiting.\n", publishing_socket); 
      return 0;
    }
  //rv = zmq_bind (publisher, "ipc://data");
  //if(rv<0) {printdate(); printf("Can't bind ipc socket for data! Exiting.\n"); return 0;}
  printdate(); printf ("febdrv: data publisher at %s\n", publishing_socket);
  //printdate(); printf ("febdrv: data publisher at ipc://data\n");
  
  //  Socket to send statistics to clients
  pubstats = zmq_socket (context, ZMQ_PUB);
  rv = zmq_bind (pubstats, stats_socket);
  if(rv<0) 
    {
      printdate(); 
      printf("Can't bind socket %s for stats! Exiting.\n", stats_socket); 
      return 0;
    }
  //rv = zmq_bind (pubstats, "ipc://stats");
  //if(rv<0) {printdate(); printf("Can't bind ipc socket for stats! Exiting.\n"); return 0;}
  printdate(); printf ("febdrv: stats publisher at %s\n", stats_socket);
  //printdate(); printf ("febdrv: stats publisher at ipc://stats\n");
  
  //  Socket to send binary packed statistics to clients
  pubstats2 = zmq_socket (context, ZMQ_PUB);
  rv = zmq_bind (pubstats2, stats2_socket);
  if(rv<0) 
    {
      printdate(); 
      printf("Can't bind socket %s for stats2! Exiting.\n", stats2_socket); 
      return 0;
    }
  //rv = zmq_bind (pubstats2, "ipc://stats2");
  //if(rv<0) {printdate(); printf("Can't bind ipc socket for stats2! Exiting.\n"); return 0;}
  printdate(); printf ("febdrv: stats2 publisher at %s\n", stats2_socket);
  //printdate(); printf ("febdrv: stats2 publisher at ipc://stats2\n");
  
  //initialising FEB daisy chain
  //scanclients();
  pingclients();
  
  driver_state=DRV_OK;
  
  zmq_msg_t request;
  
  while (1) 
    {  // main loop
      pingclients();
      driver_state=DRV_OK;
      //Check next request from client
      zmq_msg_init (&request);
      //if(zmq_msg_recv (&request, responder, ZMQ_DONTWAIT)==-1) {zmq_msg_close (&request);  polldata(); senddata(); sendstats(); sendstats2();sendstats3(); continue;} 
      if(zmq_msg_recv (&request, responder, ZMQ_DONTWAIT)==-1) 
	{
	  zmq_msg_close (&request);  
	  polldata(); 
	  senddata(); 
	  sendstats(); 
	  sendstats2(); 
	  usleep(polldelay*1000); 
	  continue;
	} 
      sprintf(cmd,"%s",(char*)zmq_msg_data(&request));
      printdate(); printf ("Received Command %s %02x  ",cmd, *(uint8_t*)(zmq_msg_data(&request)+8)); 
      rv=0;
      if(nclients>0)
	{
	  if(strcmp(cmd, "BIAS_ON")==0) rv=biasON(*(uint8_t*)(zmq_msg_data(&request)+8)); //get 8-th byte of message - mac of target board
	  else if (strcmp(cmd, "BIAS_OF")==0) rv=biasOFF(*(uint8_t*)(zmq_msg_data(&request)+8));
	  else if (strcmp(cmd, "DAQ_BEG")==0) rv=startDAQ(*(uint8_t*)(zmq_msg_data(&request)+8));
	  else if (strcmp(cmd, "DAQ_END")==0) rv=stopDAQ();
	  else if (strcmp(cmd, "SETCONF")==0) rv=configu(*(uint8_t*)(zmq_msg_data(&request)+8), (uint8_t*)(zmq_msg_data(&request)+9), zmq_msg_size (&request)-9); 
	  else if (strcmp(cmd, "GET_SCR")==0) rv=getSCR(*(uint8_t*)(zmq_msg_data(&request)+8),buf); 
	  else if (strcmp(cmd, "GETINFO")==0) rv=getInfo();
	}
      //  Send reply back to client
      zmq_msg_t reply;
      if (strcmp(cmd, "GET_SCR")==0) 
	{
	  zmq_msg_init_size (&reply, 1144/8);
	  memcpy(zmq_msg_data (&reply),buf,1144/8);
	}
      else if (strcmp(cmd, "GETINFO")==0) 
	{
	  zmq_msg_init_size (&reply, infoLength+1);
	  memcpy(zmq_msg_data (&reply),buf,infoLength+1);
	}
      else
	{ 
	  zmq_msg_init_size (&reply, 5);
	  /* The following is really weird, right? */
	  if(rv>0) sprintf((char *)zmq_msg_data (&reply), "OK");
	  else sprintf((char *)zmq_msg_data (&reply), "ERR");
	}
      printf("Sending reply %s\n",(char*)zmq_msg_data (&reply));
      zmq_msg_send (&reply, responder, 0);
      zmq_msg_close (&reply);
      
    } //end main loop
  //  We never get here but if we did, this would be how we end
  zmq_close (responder);
  zmq_ctx_destroy (context);
  return 1;
}
