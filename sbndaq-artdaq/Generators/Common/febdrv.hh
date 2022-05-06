#ifndef sbndaq_artdaq_Generators_Common_febdrv_hh
#define sbndaq_artdaq_Generators_Common_febdrv_hh

#include "fhiclcpp/ParameterSet.h"
#include "sbndaq-artdaq-core/Overlays/Common/BernCRTFragmentV2.hh"

#include <string.h>
#include <net/if.h>

#include <chrono>

// #include <mutex>
// #include <boost/circular_buffer.hpp>

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


//TODO remove? needed only by driver_state
#define DRV_OK         0
#define DRV_SENDERROR  1
#define DRV_RECVERROR  2
#define DRV_BUFOVERRUN 3
#define DRV_INITERR    4

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
    bool Init(std::string ethernet_port);
    FEBDRV();

    bool startDAQ(uint8_t mac5 = 255);

    void SetDriverState(int state) { driver_state=state; }

    void ConfigSetBit(uint8_t *buffer, uint16_t bitlen, uint16_t bit_index, bool value);
    bool ConfigGetBit(uint8_t *buffer, uint16_t bitlen, uint16_t bit_index);

    void pingclients();

    

    void biasON(uint8_t mac5);
    void biasOFF(uint8_t mac5 = 255);


    bool sendconfig(uint8_t mac5, uint8_t * bufSCR, uint16_t lenSCR, uint8_t * bufPMR, uint16_t lenPMR);

    void processSingleHit(int & jj, sbndaq::BernCRTHitV2& hit);
    void processSingleHit(int & jj, sbndaq::BernCRTHitV2& hit, int ifirmwareFLAG);
    int GetData();

    void pollfeb(uint8_t mac);
    
    std::vector<uint8_t> GetMACs();
    
  private:
    


    bool initif();
    
    

    int sendcommand(const uint8_t *mac, uint16_t cmd, uint16_t reg, uint8_t * buf);
    
    FEBDTP_PKT_t rpkt; //receive packets //TODO does it have to be global?!
    
    std::map <uint8_t, uint32_t> lostperpoll_cpu;
    
    std::vector<std::array<uint8_t, 6> > macs;
    uint8_t hostmac[6]; //TODO do we need it? Same data is stored in spkt
    char ifName[IFNAMSIZ];

    //ethernet communication sockets
    int sockfd_w;
    int sockfd_r;
    
    struct ifreq if_idx; //TODO why these two need to be global?
    struct ifreq if_mac;
    
    int driver_state; //TODO do we need it? it's set but never checked
    uint8_t dstmac[6];

    uint32_t GrayToBin(uint32_t n);
    
    bool sendtofeb(int len, FEBDTP_PKT_t const& spkt);  //sending spkt
    int recvfromfeb(int timeout_us, FEBDTP_PKT_t & rcvrpkt); //result is in rpkt

    int flushlink();

    uint8_t buf[1500]; //TODO does the buffer need to be global?
    
  }; //class FEBDRV
} //namespace sbndaq
#endif
