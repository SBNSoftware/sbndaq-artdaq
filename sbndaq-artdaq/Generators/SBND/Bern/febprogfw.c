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

#include "febdrv.h"


FEBDTP_PKT_t spkt,rpkt; //send and receive packets


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


int initif(char *iface)
{
        spkt.iptype=0x0108; // IP type 0x0801
	tv.tv_sec = 0;  /* 0 Secs Timeout */
	tv.tv_usec = 500000;  // 500ms
        sprintf(ifName,"%s",iface); 

 		/* Open RAW socket to send on */
	if ((sockfd_w = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1) {
	    perror("sender: socket");
            return 0;
	}
	/* Open PF_PACKET socket, listening for EtherType ETHER_TYPE */
	if ((sockfd_r = socket(PF_PACKET, SOCK_RAW, spkt.iptype)) == -1) {
		perror("listener: socket");	
		return 0;
	}

        if (setsockopt(sockfd_r, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval))== -1){
		perror("SO_SETTIMEOUT");
		return 0;
	}
        	/* Bind listener to device */
	if (setsockopt(sockfd_r, SOL_SOCKET, SO_BINDTODEVICE, ifName, IFNAMSIZ-1) == -1){
		perror("SO_BINDTODEVICE");
		return 0;
	}

	/* Get the index of the interface to send on */
	memset(&if_idx, 0, sizeof(struct ifreq));
	strncpy(if_idx.ifr_name, ifName, IFNAMSIZ-1);
	if (ioctl(sockfd_w, SIOCGIFINDEX, &if_idx) < 0)
	    {perror("SIOCGIFINDEX"); return 0;}
	/* Get the MAC address of the interface to send on */
	memset(&if_mac, 0, sizeof(struct ifreq));
	strncpy(if_mac.ifr_name, ifName, IFNAMSIZ-1);
	if (ioctl(sockfd_w, SIOCGIFHWADDR, &if_mac) < 0)
	    {perror("SIOCGIFHWADDR"); return 0;}
        memcpy(&hostmac,((uint8_t *)&if_mac.ifr_hwaddr.sa_data),6);
        memcpy(&(spkt.src_mac),hostmac,6);
        printdate(); printf("febdrv: initialized %s with MAC  ", ifName);
        printmac(hostmac);
        printf("\n");
        return 1;
}

void CMD_stoa(unsigned short cmd, char* str)
{
 switch ( cmd ) {

  case FEB_RD_SR : 
    sprintf(str,"FEB_RD_SR");
    break;
  case FEB_WR_SR : 
    sprintf(str,"FEB_WR_SR");
    break;
  case FEB_RD_SRFF : 
    sprintf(str,"FEB_RD_SRFF");
    break;
  case FEB_WR_SRFF : 
    sprintf(str,"FEB_WR_SRFF");
    break;
  case FEB_OK_SR : 
    sprintf(str,"FEB_OK_SR");
    break;
  case FEB_ERR_SR : 
    sprintf(str,"FEB_ERR_SR");
    break;

  case FEB_SET_RECV : 
    sprintf(str,"FEB_SET_RECV");
    break;
  case FEB_GEN_INIT : 
    sprintf(str,"FEB_GEN_INIT");
    break;
  case FEB_GET_RATE : 
    sprintf(str,"FEB_GET_RATE");
    break;
  case FEB_GEN_HVON : 
    sprintf(str,"FEB_GEN_HVON");
    break;
  case FEB_GEN_HVOF : 
    sprintf(str,"FEB_GEN_HVOF");
    break;
  case FEB_OK : 
    sprintf(str,"FEB_OK");
    break;
  case FEB_ERR : 
    sprintf(str,"FEB_ERR");
    break;

  case FEB_RD_SCR : 
    sprintf(str,"FEB_RD_SCR");
    break;
  case FEB_WR_SCR  : 
    sprintf(str,"FEB_WR_SCR");
    break;
  case FEB_OK_SCR : 
    sprintf(str,"FEB_OK_SCR");
    break;
  case FEB_ERR_SCR : 
    sprintf(str,"FEB_ERR_SCR");
    break;

  case FEB_RD_PMR : 
    sprintf(str,"FEB_RD_PMR");
    break;
  case FEB_WR_PMR  : 
    sprintf(str,"FEB_WR_PMR");
    break;
  case FEB_OK_PMR : 
    sprintf(str,"FEB_OK_PMR");
    break;
  case FEB_ERR_PMR : 
    sprintf(str,"FEB_ERR_PMR");
    break;

  case FEB_RD_CDR : 
    sprintf(str,"FEB_RD_CDR");
    break;
  case FEB_WR_CDR : 
    sprintf(str,"FEB_WR_CDR");
    break;
  case FEB_DATA_CDR : 
    sprintf(str,"FEB_DATA_CDR");
    break;
  case FEB_ERR_CDR : 
    sprintf(str,"FEB_ERR_CDR");
    break;
  case FEB_EOF_CDR : 
    sprintf(str,"FEB_EOF_CDR");
    break;

  case FEB_RD_FW : 
    sprintf(str,"FEB_RD_FW");
    break;
  case FEB_WR_FW : 
    sprintf(str,"FEB_WR_FW");
    break;
  case FEB_DATA_FW : 
    sprintf(str,"FEB_DATA_FW");
    break;
  case FEB_ERR_FW : 
    sprintf(str,"FEB_ERR_FW");
    break;
  case FEB_EOF_FW : 
    sprintf(str,"FEB_EOF_FW");
    break;
  case FEB_OK_FW : 
    sprintf(str,"FEB_OK_FW");
    break;

  default : 
    sprintf(str,"Unrecognized code");
 }
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
	    {perror("SIOCGIFINDEX"); return 0;}
	/* Get the MAC address of the interface to send on */
	memset(&if_mac, 0, sizeof(struct ifreq));
	strncpy(if_mac.ifr_name, ifName, IFNAMSIZ-1);
	if (ioctl(sockfd_w, SIOCGIFHWADDR, &if_mac) < 0)
	    {perror("SIOCGIFHWADDR");return 0;}

	/* Index of the network device */
	socket_address.sll_ifindex = if_idx.ifr_ifindex;
	/* Address length*/
	socket_address.sll_halen = ETH_ALEN;
	/* Destination MAC */
	memcpy(socket_address.sll_addr,spkt.dst_mac,6);
	/* Send packet */
	 if (sendto(sockfd_w, (char*)&spkt, len, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0)
	    {driver_state=DRV_SENDERROR; return 0;}
//	printf("host->feb: sent packet %d bytes from  %02x:%02x:%02x:%02x:%02x:%02x ", len,spkt.src_mac[0],spkt.src_mac[1],spkt.src_mac[2],spkt.src_mac[3],spkt.src_mac[4],spkt.src_mac[5]);
//	printf("to %02x:%02x:%02x:%02x:%02x:%02x.\n", spkt.dst_mac[0],spkt.dst_mac[1],spkt.dst_mac[2],spkt.dst_mac[3],spkt.dst_mac[4],spkt.dst_mac[5]);
        return 1; 
}

int recvfromfeb(int timeout_us) //result is in rpkt
{
	int numbytes;
    char repstr[54];
            // set receive timeout
       tv.tv_usec=timeout_us;
       if (setsockopt(sockfd_r, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval))== -1){
		perror("SO_SETTIMEOUT");
		return 0;
	}
 	numbytes = recvfrom(sockfd_r, &rpkt, MAXPACKLEN, 0, NULL, NULL);
        if(numbytes<=0) {driver_state=DRV_RECVERROR; return 0;} //timeout
    CMD_stoa(rpkt.CMD,repstr);
//	printf("feb->host: received packet %d bytes from  %02x:%02x:%02x:%02x:%02x:%02x  CMD= %s\n", numbytes,rpkt.src_mac[0],rpkt.src_mac[1],rpkt.src_mac[2],rpkt.src_mac[3],rpkt.src_mac[4],rpkt.src_mac[5],repstr);
    
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
        packlen=sendtofeb(packlen);
        if(packlen<=0) return 0; 
}

void usage()
{
 printf("Usage: to program FEB with <S/N> on eth0 interface type with firmware file <FW_filename>:\n");
 printf("febprogfw eth0 <S/N> <FW_filename>\n");
}

int pingclient(uint8_t mac5)
{
  int retval=0;
  dstmac[5]=mac5; 
  sendcommand(dstmac,FEB_SET_RECV,500,hostmac);
  printf("Pinging requested FEB ");
      for(int i=0;i<5;i++) printf("%02x:",dstmac[i]);
       printf("%02x \n",dstmac[5]);
  
  while(recvfromfeb(10000)) if(rpkt.CMD==FEB_OK) 
  {
      printdate(); printf("Replied FEB: ");
      for(int i=0;i<5;i++) printf("%02x:",rpkt.src_mac[i]);
      printf("%02x ",rpkt.src_mac[5]);
      printf("%s\n",rpkt.Data); // new client reply received
      if(rpkt.src_mac[5]==mac5) {printf("Match to the requested one.\n"); retval=1;};
  }
  return retval;
}

int sendconfig(uint8_t mac5)
{
//t->SendCMD(t->dstmac,FEB_SET_RECV,fNumberEntry75->GetNumber(),t->srcmac);
  int nreplies=0;
  int PrevConfigured=FEB_configured[mac5];
dstmac[5]=mac5;
FEB_configured[mac5]=0;
sendcommand(dstmac,FEB_WR_SCR,0x0000,bufSCR[mac5]);
  while(recvfromfeb(50000)) { 
  if(rpkt.CMD!=FEB_OK_SCR) return 0;
  nreplies++;
  }
  if(nreplies==0) return 0;
  if(nreplies!=nclients && mac5==255) return 0;

sendcommand(dstmac,FEB_WR_PMR,0x0000,bufPMR[mac5]);
if(!recvfromfeb(50000))  return 0;
  while(recvfromfeb(50000)) { 
  if(rpkt.CMD!=FEB_OK_PMR) return 0;
  nreplies++;
  }
  if(nreplies==0) return 0;
  if(nreplies!=nclients && mac5==255) return 0;

FEB_configured[mac5]=PrevConfigured+1;

return 1;
}




int biasON(uint8_t mac5)
{
  int nreplies=0;
  dstmac[5]=mac5; 
  sendcommand(dstmac,FEB_GEN_HVON,0,buf); //reset buffer
  
  while(recvfromfeb(10000)) { 
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
  
  while(recvfromfeb(10000)) { 
  if(rpkt.CMD!=FEB_OK) {printf("biasOFF(): no OK reply from the FEB.\n"); return 0;}
  nreplies++;
  }
  if(nreplies!=1) return 0;
  return 1;
}

int stopDAQ(uint8_t mac5)
{
  int nreplies=0;
  dstmac[5]=mac5; 
  sendcommand(dstmac,FEB_GEN_INIT,0,buf); //stop DAQ on the FEB
  
  while(recvfromfeb(10000)) { 
  if(rpkt.CMD!=FEB_OK) {printf("stopDAQ(): no OK reply from the FEB.\n"); return 0;}
  nreplies++;
  }
  if(nreplies!=1) return 0;
  return 1;
}

int FW_ptr=0;
uint8_t fwbuf[1024*1024]; //Firmware buffer
uint8_t crc7;

uint8_t getCRC(uint8_t message[], uint32_t length)
{
	uint32_t i, j;
	uint8_t crc = 0;

  for (i = 0; i < length; i++)
  {
    crc ^= message[i];
    for (j = 0; j < 8; j++)
    {
      if (crc & 1)
        crc ^=  0x91; //CRC7_POLY
      crc >>= 1;
    }
  }
  return crc;
}


bool ProgramFW(uint32_t startaddr, uint16_t blocks) 
{
  bool retval=1;
  buf[0]=startaddr & 0x000000ff; 
  buf[1]=(startaddr & 0x0000ff00)>>8; 
  buf[2]=(startaddr & 0x00ff0000)>>16; 
  buf[3]=(blocks & 0x00FF);
  buf[4]=00; //(blocks & 0xFF00) >>8;
  if( sendcommand(dstmac,FEB_WR_FW,00,buf)==0) retval=0; //initiate flash programming
    recvfromfeb(10000);    if(rpkt.CMD!=FEB_OK_FW) {printf("ProgramFW(): no OK reply received to FEB_WR_FW\n"); retval=0;}
    else printf("FEB_WR_FW command OK.\n");
  for( int i=0;i<blocks;i++) {
    if(sendcommand(dstmac,FEB_DATA_FW,getCRC(fwbuf+i*1024,1024),fwbuf+i*1024)==0) retval=0;
    recvfromfeb(10000);    if(rpkt.CMD!=FEB_OK_FW) {printf("ProgramFW(): no OK reply received to FEB_DATA_FW\n");retval=0;}
    else printf("Block %d : FEB_DATA_FW command OK.\n",i+1);
  }
  if(retval==0) printf("Error in ProgramFW!!\n");
  return retval;

}

bool UpdateFW(char *fname)
{
  bool retval=1;
  memset(fwbuf,1024*1024,00);
  printf("Opening FW file %s..\n",fname);
   FILE *fp=fopen(fname,"r");
   if(!fp) return 0;
  fread(fwbuf,1024,1024,fp);
  fclose(fp);
 printf("Programming first 64 kB into FLASH addr 0x20000..\n");
  retval=ProgramFW(0x20000,64); //put FW into safe area
  if(retval==0) return 0;
 printf("Copying 64 kB FLASH from 0x20000 to 0x0..\n");
  buf[0]=0;buf[1]=0;buf[2]=0x02;buf[3]=64;buf[4]=0; //prepare for copying 64 kB from 0x20000 to 0x0
  sendcommand(dstmac,FEB_WR_FW,0x0101,buf); //perform copy
  
 printf("Programming complete.\n");
 // gSystem->Sleep(1000);
 // RescanNet();
  return retval;
}



int main (int argc, char **argv)
{

uint8_t mac5;

int rv;
char cmd[32]; //command string
if(argc!=4) { usage(); return 0;}


memset(FEB_configured,0,256);
memset(FEB_daqon,0,256);
memset(FEB_evrate,0,256);
memset(FEB_biason,0,256);
memset(FEB_present,0,256);
memset(FEB_lastheard,0,256); //number of seconds since the board is heard
// network interface to febs
rv=initif(argv[1]);
mac5=atoi(argv[2]);
 if(mac5==255){
   usage();
   printf("0xFF is not allowed as the FEB address! This is reserved for broadcast messages. Exiting.\n");
   return 0;}
if(rv==0) {printdate(); printf("Can't initialize network interface %s! Exiting.\n",argv[1]); return 0;}
if( pingclient(mac5)==0) {printf("FEB with S/N=%d is not replying! Check connection and power on FEB. Exiting. \n",mac5); return 0;};
if(biasOFF(mac5)==0) {printf("FEB %d: Can't turn off the bias, no OK reply received. Exiting. \n",mac5); return 0;};
if(stopDAQ(mac5)==0) {printf("FEB %d: Can't stop DAQ, no OK reply received.  Exiting. \n",mac5); return 0;};

if(UpdateFW(argv[3])==0){printf("FEB %d: Updating Firmware failed!  Exiting. \n",mac5); return 0;}
/*
while (1) {  // main loop
pingclients();
driver_state=DRV_OK;
//Check next request from client
zmq_msg_init (&request);
//if(zmq_msg_recv (&request, responder, ZMQ_DONTWAIT)==-1) {zmq_msg_close (&request);  polldata(); senddata(); sendstats(); sendstats2();sendstats3(); continue;} 
if(zmq_msg_recv (&request, responder, ZMQ_DONTWAIT)==-1) {zmq_msg_close (&request);  polldata(); senddata(); sendstats(); sendstats2(); continue;} 
sprintf(cmd,"%s",(char*)zmq_msg_data(&request));
printdate(); printf ("Received Command %s %02x  ",cmd, *(uint8_t*)(zmq_msg_data(&request)+8) );
rv=0;
if(nclients>0)
 {
 if(strcmp(cmd, "BIAS_ON")==0) rv=biasON(*(uint8_t*)(zmq_msg_data(&request)+8)); //get 8-th byte of message - mac of target board
 else if (strcmp(cmd, "BIAS_OF")==0) rv=biasOFF(*(uint8_t*)(zmq_msg_data(&request)+8));
 else if (strcmp(cmd, "DAQ_BEG")==0) rv=startDAQ(*(uint8_t*)(zmq_msg_data(&request)+8));
 else if (strcmp(cmd, "DAQ_END")==0) rv=stopDAQ(*(uint8_t*)(zmq_msg_data(&request)+8));
 else if (strcmp(cmd, "SETCONF")==0) rv=configu(*(uint8_t*)(zmq_msg_data(&request)+8), (uint8_t*)(zmq_msg_data(&request)+9), zmq_msg_size (&request)-9); 
 else if (strcmp(cmd, "GET_SCR")==0) rv=getSCR(*(uint8_t*)(zmq_msg_data(&request)+8),buf); 
 }
//  Send reply back to client
zmq_msg_t reply;
if (strcmp(cmd, "GET_SCR")==0) 
 {
  zmq_msg_init_size (&reply, 1144/8);
  memcpy(zmq_msg_data (&reply),buf,1144/8);
 }
else
 { 
 zmq_msg_init_size (&reply, 5);
 if(rv>0) sprintf(zmq_msg_data (&reply), "OK");
 else sprintf(zmq_msg_data (&reply), "ERR");
 }
printf("Sending reply %s\n",(char*)zmq_msg_data (&reply));
zmq_msg_send (&reply, responder, 0);
zmq_msg_close (&reply);

} //end main loop

*/
return 1;
}
