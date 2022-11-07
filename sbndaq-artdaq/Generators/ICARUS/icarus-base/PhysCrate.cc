#define TRACE_NAME "PhysCrate_cc"
#include "PhysCrate.h"
#include "veto.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>

#include "artdaq/DAQdata/Globals.hh"
#include "TRACE/tracemf.h"

#define kMaxSize 950000
#define kMaxBoards 32 
#define kMaxBoardsonLink 5
#define kMaxLinks 4 


PhysCrate::PhysCrate() {}

PhysCrate::~PhysCrate() 
{ 
	delete(tilebuf);
}

#ifndef _simulate_
// Initializes the crate detecting the boards
void
PhysCrate::initialize(std::vector<int> busVec) 
{
   TRACEN("PhysCrate_cc",TLVL_INFO, "PhysCrate::initialize(): Initializing crate.");

    //int status;
    int boardId(0);
    int nDev;
    
    boards = new A2795Board*[kMaxBoards];
    
    nBoards=0;
    nDev=0;
    if(busVec.size()==0){
      int nBus=0;
      do {
        do {        
  	  TRACEN("PhysCrate_cc",TLVL_INFO, "trying bus=%d,dev=%d",nBus,nDev);
	
          boards[nBoards]=new A2795Board(nDev,nBus);
          boardId = boards[nBoards]->boardId;
          nDev++;
          if (boardId>-1)
          { 
	      TRACEN("PhysCrate_cc",TLVL_INFO, "PhysCrate::initialize(): Created board (%d, %d, %d, %d)",nBus,nDev,nBoards,boardId);
	      nBoards++;
	  }
	  //else break;
	} while (nDev<kMaxBoardsonLink );
	nBus++;
	nDev=0;
      } while (nBus < kMaxLinks);
      
    }
    else{
      for(int nBus : busVec){

	do {        
	  TRACEN("PhysCrate_cc", TLVL_INFO, "trying bus=%d,dev=%d",nBus,nDev);
	  
	  boards[nBoards]=new A2795Board(nDev,nBus);
          boardId = boards[nBoards]->boardId;
	  nDev++;
	  if (boardId>-1)
	    { 
	      TRACEN("PhysCrate_cc", TLVL_INFO, "PhysCrate::initialize(): Created board (%d, %d, %d, %d)",nBus,nDev,nBoards,boardId);
	      
	      nBoards++;
	    }
	  //else break;
	} while (nDev<kMaxBoardsonLink );
	nDev=0;
      }
    }

    BoardIDs.resize( nBoards );
    for ( int iBoard = 0; iBoard < nBoards; ++iBoard ) BoardIDs[iBoard] = boards[iBoard]->boardId;

    TRACEN("PhysCrate_cc",TLVL_INFO,  "PhysCrate::initialize(): %d A2795 boards found.", nBoards);

    char mhost[128];
    gethostname(mhost,128);

    struct hostent *hp = gethostbyname(mhost);

    TRACEN("PhysCrate_cc",TLVL_INFO,  std::string("Got local name ").append(mhost));
	     
    if (hp == NULL) {
             TRACEN("PhysCrate_cc",TLVL_INFO,  std::string("Host").append(mhost).append(" not found"));

             //exit (3);
      mip=158;
     }
    else {
       char ** p = hp->h_addr_list;
       in_addr in;
       memcpy(&in.s_addr, *p, sizeof (in.s_addr));
       
       TRACEN("PhysCrate_cc",TLVL_INFO,  std::string("inet_ntoa").append(inet_ntoa(in)));
       
       sscanf(inet_ntoa(in),"%d",&mip);
    }

    TRACEN("PhysCrate_cc",TLVL_INFO, "Set localID.");

  int size = sizeof(DataTile)+2*kMaxSize*nBoards;
  tilebuf = new char[size];

}
#endif

int
PhysCrate::getId()
{
 return mip;
}

#ifdef _simulate_
void
PhysCrate::initialize()
{
    nBoards = 8;
    boards = new A2795Board*[nBoards];
    for (int i = 0; i < nBoards; i++)
    {
        boards[i] = new A2795Board(i,0);
	TRACEN("PhysCrate_cc",TLVL_DEBUG, "PhysCrate::initialize(): Created board.");
    }
  int size = sizeof(DataTile)+2*kMaxSize*nBoards;
  tilebuf = new char[size];
}
#endif

// Configures the boards
void
PhysCrate::configure(BoardConf conf)
{
    for (int i = 0; i < nBoards; i++)
    {
        boards[i]->configure(conf);
    }
    TRACEN("PhysCrate_cc",TLVL_DEBUG, "PhysCrate::configure(): Board parameters set.");
    
} // configure()

// Configures the trigger parameters
void 
PhysCrate::configureTrig(TrigConf conf)
{
    for (int i = 0; i < nBoards; i++)
    {
        boards[i]->configureTrig(conf);
    }
    
    TRACEN("PhysCrate_cc",TLVL_DEBUG, "PhysCrate::configureTrig(): Trigger parameters set.");
}

void
PhysCrate::start()
{
    //for (int i = 0; i < nBoards; i++)
    //{
    //    boards[i]->startDPU();
    //}
   //boards[0]->write(A_Signals, SIGNALS_TTLINK_ALIGN);
   boards[0]->write(A_Signals, SIGNALS_TTLINK_COMMA);
   sleep(3);
   boards[0]->write(A_Signals, SIGNALS_TTLINK_SOR);


  //vetoOn();
//  for (int i = 0; i< nBoards; i++)
//    boards[i]->ArmTrigger();
  //vetoOff();
  
}


// Waits until the boards give a signal of Data Ready
void
PhysCrate::waitData()
{
      TRACEN("PhysCrate_cc",20, "PhysCrate::waitData(): on %d boards", nBoards);
  
#ifdef AUTO_TRIGGER
  //boards[0]->write(A_Signals, SIGNALS_TTLINK_GTRG);
#endif

    // for (int i = 0; i < nBoards; i++)
    for (int i = 0; i < 1; i++)
    // for (int i = nBoards-1; i < nBoards; i++)
    {

      TRACEN("PhysCrate_cc", 20, "PhysCrate::waitData(): waiting for board %d", i);
	
        while(!(boards[i]->isDataRdy()));

      TRACEN("PhysCrate_cc", 20, "Board %d ready.", i);
    }
} // waitData()


bool
PhysCrate::dataAvail()
{
 if (presBoard<nBoards) {
   TRACEN("PhysCrate_cc", 20, "PhysCrate::waitData(): waiting for board %d", presBoard);
   while(!(boards[presBoard]->isDataRdy()));
   TRACEN("PhysCrate_cc", 20, "Board %d ready.", presBoard);
   TLOG(20) << "Board " << presBoard << " has data ready.";
   return true;
  }
  return false;
}
// Returns a pointer to a dynamically allocated memory area with the data of the event.
DataTile*
PhysCrate::getData()
{
  //int size = sizeof(DataTile)+2*nSamples*nBoards;
  //char* tilebuf = new char[size];

  DataTile* tile = (DataTile*) tilebuf;

  // read all boards
   int i=presBoard;

  //for (int i = 0; i < nBoards; i++)
  //{
    //boards[i]->getData(1,tile->data + 2*i*nSamples);
    //boards[i]->getData(2,tile->data + (2*i+1)*nSamples);

//printf ("nSamp %d \n",boards[i]->nSamples);

    int nSamples=boards[i]->getData(1,tile->data  );
//    TRACEN("PhysCrate_cc",TLVL_DEBUG+1, "nSamp %d ",nSamples);
    TLOG(TLVL_DEBUG+1) << "PhysCrate::getData: tile->data: " << &(tile->data);

//    boards[i]->getData(2,tile->data + boards[i]->nSamples);

  //}

    //tile->Header.packSize = htonl(size);
    //tile->Header.packSize = htonl(2*nSamples+sizeof(DataTile));
    tile->Header.packSize = htonl(nSamples+sizeof(DataTile));
    TLOG(TLVL_DEBUG+1) << "PhysCrate::getData: nSamples: " << nSamples << ", sizeof(DataTile): "
             << sizeof(DataTile);
    //tile->Header.packSize = htonl(2*boards[i]->nSamples+sizeof(DataTile));
    //tile->Header.chID = htonl(mip+i*256); 
    boards[i]->fillHeader(tile);
    presBoard++ ;
    return tile;
}


void
PhysCrate::ArmTrigger()
{
  vetoOn();
  for (int i = 0; i< nBoards; i++)
    boards[i]->ArmTrigger();
  //vetoOff();

  presBoard=0;

} 

int PhysCrate::BoardStatus(int i)
{
  if(i>=nBoards)
    return -2;

  return boards[i]->Status();
}

int PhysCrate::BoardHandle(int i)
{
  if(i>=nBoards)
    return -2;

  return boards[i]->BoardHandle();
}

int PhysCrate::BoardTemps(int i,uint8_t& t1, uint8_t& t2)
{
  if(i>=nBoards)
    return -2;
  uint32_t temps = boards[i]->Temperatures();
  t1 = (temps & 0x000000ff);
  t2 = ((temps & 0x00ff0000) >> 16);

  return 0;
}

#if FALSE
// Returns a pointer to a dynamically allocated memory area with the data of the event.
DataTile*
PhysCrate::getData()
{
  int size = sizeof(DataTile)+2*nSamples*nBoards;
  char* buf = new char[size];
  DataTile* tile = (DataTile*) buf;
  // read all boards
  for (int i = 0; i < nBoards; i++)
  {
    boards[i]->getData(1,tile->data + 2*i*(sizeof(DataTile)+nSamples));
    tile->Header.packSize = htonl(sizeof(DataTile)+nSamples);


    boards[i]->getData(2,tile->data + (2*i+1)*(sizeof(DataTile)+nSamples));
    tile->Header.packSize = htonl(sizeof(DataTile)+nSamples);
  
  }
  //for (int i = 0; i< nBoards; i++)
  //  boards[i]->startDPU();

  vetoOn();
  for (int i = 0; i< nBoards; i++)
    boards[i]->ArmTrigger();
  //vetoOff();
  
  return tile;    
} // getData()
#endif
