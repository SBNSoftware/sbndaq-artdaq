// 0MQ data backend structure
typedef struct {
  uint16_t mac5;
  uint16_t flags;
  uint16_t lostcpu;
  uint16_t lostfpga;
  uint32_t ts0;
  uint32_t ts1;
  uint16_t adc[32];
  uint32_t coinc;
} EVENT_t; 

//#define EVLEN 80
#define EVLEN 84
#define MAGICWORD8 0xa5 //marker for the buffer start in the file 
#define MAGICWORD16 0xaa55 //marker for the buffer start in the file 
#define MAGICWORD32 0xaa55aa55 //marker for the buffer start in the file 
//#define MAGICWORD32 0x01020255 //marker for the buffer start in the file 

typedef struct {
  int status;		
  int daqon;
  time_t datime;
  int nfebs;
  int msperpoll;
  //		char string[64];
} DRIVER_STATUS_t;

typedef struct {
  uint8_t mac[6];		
  char fwcpu[64];
  char fwfpga[64];
  int connected;
  int configured;
  int biason;
  int error;
  uint16_t evtperpoll;
  uint16_t lostcpu;
  uint16_t lostfpga;
  float evtrate;
  uint8_t ts0ok;
  uint8_t ts1ok;
  //	char string[128];
} FEB_STATUS_t;
