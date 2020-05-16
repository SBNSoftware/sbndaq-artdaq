#ifndef sbndaq_artdaq_Generators_Common_febdrv_hh
#define sbndaq_artdaq_Generators_Common_febdrv_hh

#include "fhiclcpp/ParameterSet.h"


#include "sbndaq-artdaq-core/Overlays/Common/BernCRTZMQFragment.hh"

#include <string.h>
#include <time.h>
// #include <sys/timeb.h>
// #include "febevt.h"
#include <net/if.h>

// Ethernet switch register r/w
#define FEB_RD_SR 0x0001
#define FEB_WR_SR 0x0002
#define FEB_RD_SRFF 0x0003
#define FEB_WR_SRFF 0x0004
#define FEB_OK_SR 0x0000
#define FEB_ERR_SR 0x00FF

// Broadcast receiver MAC and general reset (init)
#define FEB_SET_RECV 0x0101
#define FEB_GEN_INIT 0x0102
#define FEB_GEN_HVON 0x0103
#define FEB_GEN_HVOF 0x0104
#define FEB_GET_RATE 0x0105 //get event rate
#define FEB_OK 0x0100
#define FEB_ERR 0x01FF

//CITIROC slow control register
#define FEB_RD_SCR 0x0201
#define FEB_WR_SCR 0x0202
#define FEB_OK_SCR 0x0200
#define FEB_ERR_SCR 0x02FF

//CITIROC probe MUX register
#define FEB_RD_PMR 0x0401
#define FEB_WR_PMR 0x0402
#define FEB_OK_PMR 0x0400
#define FEB_ERR_PMR 0x04FF


//CITIROC Data control register
#define FEB_RD_CDR 0x0301
#define FEB_WR_CDR 0x0302
#define FEB_DATA_CDR 0x0300
#define FEB_EOF_CDR 0x0303
#define FEB_ERR_CDR 0x03FF

//Firmware read-write transmission
#define FEB_RD_FW 0x0501
#define FEB_WR_FW 0x0502
#define FEB_OK_FW 0x0500
#define FEB_ERR_FW 0x05FF
#define FEB_EOF_FW 0x0503
#define FEB_DATA_FW 0x0504


#define DRV_OK         0
#define DRV_SENDERROR  1
#define DRV_RECVERROR  2
#define DRV_BUFOVERRUN 3
#define DRV_INITERR    4

#define MAXPACKLEN 1500
#define MAXPAYLOAD (1500-14-4)


// Ethernet L2 Packets
typedef struct {
  uint8_t   dst_mac[6];  /*!< Destination MAC address */
  uint8_t   src_mac[6];  /*!< Source MAC address */
  uint16_t  iptype;          /*!< IP type*/
  uint16_t  CMD;  /*!< FEBDTP command field*/
  uint16_t  REG;  /*!< FEBDTP register field*/
  uint8_t   Data[MAXPAYLOAD]; /*!< FEBDTP data field, 50-8=42 bytes*/

} FEBDTP_PKT_t; // packet total length 64 bytes, 42 useful data

namespace sbndaq {
class FEBDRV {
public:
  void Init(std::string ethernet_port);
public:
  FEBDRV();

  //void Init(std::string iface) { Init(iface.c_str()); }

  void Init(const char *iface);

  int startDAQ(uint8_t mac5);
  int stopDAQ(uint8_t);

  void SetDriverState(int state) { driver_state=state; }
  int NClients() { return nclients; }

  void ConfigSetBit(uint8_t *buffer, uint16_t bitlen, uint16_t bit_index, bool value);
  bool ConfigGetBit(uint8_t *buffer, uint16_t bitlen, uint16_t bit_index);

  bool initif();

  int sendcommand(const uint8_t *mac, uint16_t cmd, uint16_t reg, uint8_t * buf);

  int pingclients();
  uint64_t MACAddress(int client) const;

  int sendconfig(uint8_t mac5);

  int biasON(uint8_t mac5);
  int biasOFF(uint8_t mac5);

  int configu(uint8_t mac5, const uint8_t *buf1, int len);
  int getSCR(uint8_t mac5, uint8_t *buf1);

  int sendstats();
  int sendstats2();

  int polldata();
  int recvL2pack();
  void processL2pack(int,const uint8_t*);
  int recvandprocessL2pack(const uint8_t*);
  int polldata_setup();

  void pollfeb(const uint8_t* mac);
  void updateoverwritten();
  void polldata_complete();

  static void free_subbufer (void*, void *hint); //call back from ZMQ sent function, hint points to subbufer index

  void* GetResponder() { return responder; }
  void* GetContext() { return context; }

  int IncrementTotalLost(int i) { total_lost += i; return total_lost; }
  int IncrementTotalAcquired(int i) { total_acquired += i; return total_acquired; }

  FEBDTP_PKT_t const& ReceivedPkt() { return rpkt; }

private:

  void *context = NULL;
  
  //  Socket to respond to clients
  void *responder = NULL;
  //  Socket to send data to clients
  void *publisher = NULL;
  //  Socket to send statistics to clients
  void *pubstats = NULL;
  void *pubstats2 = NULL;
  
  FEBDTP_PKT_t spkt, rpkt; //send and receive packets
  std::map <uint8_t, std::vector<sbndaq::BernCRTZMQEvent>> feb_buffer;
  std::map <uint8_t, uint16_t> feb_buffer_counter;
  
  std::map <uint8_t, uint16_t> evtsperpoll;
  int msperpoll=0;
  std::map <uint8_t, uint32_t> lostperpoll_cpu;
  std::map <uint8_t, uint32_t> lostperpoll_fpga;
  
  uint8_t GLOB_daqon;
  int nclients;
  int macs[256][6]; //list of detected clients
  char verstr[256][32]; //list of version strings of clients
  uint8_t hostmac[6];
  char ifName[IFNAMSIZ];
  //int sockfd;
  int sockfd_w; 
  int sockfd_r;
  struct timeval tv;
  struct ifreq if_idx;
  struct ifreq if_mac;
  int driver_state;
  uint8_t dstmac[6];
  uint8_t brcmac[6];

  std::map<uint8_t, uint32_t> ts0_ref_MEM;
  std::map<uint8_t, uint32_t> ts1_ref_MEM;
  
  uint32_t overwritten;
  uint32_t lostinfpga;
  uint32_t total_lost;
  uint32_t total_acquired;

  uint32_t GrayToBin(uint32_t n);
  

  int sendtofeb(int len, FEBDTP_PKT_t const& spkt);  //sending spkt
  int recvfromfeb(int timeout_us, FEBDTP_PKT_t & rcvrpkt); //result is in rpkt

  int flushlink();

  int numbytes;
  bool NOts0,NOts1;
  bool REFEVTts0,REFEVTts1;
  uint32_t tt0,tt1;
  uint32_t ts0,ts1;
  uint8_t ls2b0,ls2b1; //least sig 2 bits
  sbndaq::BernCRTZMQEvent *evt;

  static uint8_t bufPMR[256][1500];
  static uint8_t bufSCR[256][1500];
  std::map<uint8_t, uint8_t > FEB_present;
  std::map<uint8_t, time_t  > FEB_lastheard; //number of seconds since the board is heard, -1= never
  std::map<uint8_t, uint8_t > FEB_configured;
  std::map<uint8_t, uint8_t > FEB_daqon;
  std::map<uint8_t, uint8_t > FEB_evrate;
  std::map<uint8_t, uint8_t > FEB_biason;
  std::map<uint8_t, uint16_t>  FEB_VCXO;
  
  static uint8_t buf[MAXPACKLEN];

}; //class FEBDRV
} //namespace sbndaq
#endif
