#define EVLEN sizeof(sbndaq::BernCRTZMQEvent)
#define MAGICWORD8 0xa5 //marker for the buffer start in the file 
#define MAGICWORD16 0xaa55 //marker for the buffer start in the file 
#define MAGICWORD32 0xaa55aa55 //marker for the buffer start in the file 

typedef struct {
  int status;		
  int daqon;
  time_t datime;
  int nfebs;
  int msperpoll;
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
} FEB_STATUS_t;
