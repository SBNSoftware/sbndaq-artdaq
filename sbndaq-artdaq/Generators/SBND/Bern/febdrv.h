#include <time.h>

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


#define DRV_OK 0
#define DRV_SENDERROR 1
#define DRV_RECVERROR 2
#define DRV_BUFOVERRUN 3
#define MAXPACKLEN 1500
#define MAXPAYLOAD (1500-14-4)


// Ethernet L2 Packets
typedef struct {
	uint8_t   dst_mac[6];	/*!< Destination MAC address */
	uint8_t   src_mac[6];	/*!< Source MAC address */
	uint16_t  iptype;	        /*!< IP type*/
	uint16_t  CMD;	/*!< FEBDTP command field*/
	uint16_t  REG;	/*!< FEBDTP register field*/
	uint8_t   Data[MAXPAYLOAD]; /*!< FEBDTP data field, 50-8=42 bytes*/

} FEBDTP_PKT_t; // packet total length 64 bytes, 42 useful data

uint8_t bufPMR[256][1500];
uint8_t bufSCR[256][1500];
uint8_t FEB_present[256];
time_t FEB_lastheard[256]; //number of seconds since the board is heard, -1= never
uint8_t FEB_configured[256];
uint8_t FEB_daqon[256];
uint8_t FEB_evrate[256];
uint8_t FEB_biason[256];
uint16_t FEB_VCXO[256];

uint8_t buf[MAXPACKLEN];

uint8_t GLOB_daqon=0;

