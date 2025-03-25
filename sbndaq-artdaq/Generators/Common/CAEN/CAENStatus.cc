//
//
// sbndaq-artdaq/Generators/Common/CAEN/CAENStatus.cc  (wfb)
//
//

#include <stdio.h>
#include "CAENDigitizer.h"
#include "CAENDigitizerType.h"

//#include "CAENDecoder.hh"

enum
{
  N_LINKS = 8  // 4 link per A3818 PCIe board
};

int main(int argc, char **argv)
{
  int link, retcod, board, handle;
  uint32_t data;
  CAEN_DGTZ_BoardInfo_t info;

  board = 0; // Can be 0...7, but we only use one per optical chain

  printf("Lnk Model\n"); 
  for ( link = 0; link<N_LINKS; link++)
  {
    printf("%1.1d ", link);
    retcod = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_OpticalLink,
				     link, board, 0, &handle);
    if ( retcod == CAEN_DGTZ_Success )
    {
      retcod = CAEN_DGTZ_GetInfo(handle,&info);
      if ( retcod == CAEN_DGTZ_Success )
      {
	printf("  %s Serial %d\n      %s\n      %s %d\n", info.ModelName, 
	       info.SerialNumber, info.ROC_FirmwareRel, info.AMC_FirmwareRel);

	uint32_t mode;
	mode = 999;
	retcod = CAEN_DGTZ_ReadRegister(handle,CAEN_DGTZ_MON_MODE_ADD,&mode);
	if ( retcod == CAEN_DGTZ_Success )
	{
	  printf("      AMode:%d", mode);
	}
	else
	{
	  printf("      AMode:E", mode);
	}

    uint32_t readback;
    for(uint32_t ch=0; ch<16; ++ch)
    {    
      retcod = CAEN_DGTZ_ReadRegister(handle,0x1084+(ch<<8),&readback);
      if ( retcod == CAEN_DGTZ_Success )
	{
	  printf("      Channel:%d SLFTRG:%d", ch, readback);
	}
	else
	{
	  printf("     Channel:%d SLFTRG:E", ch);
	}
     }

    uint32_t readback2;
      retcod = CAEN_DGTZ_ReadRegister(handle,0x810C,&readback2);
      if ( retcod == CAEN_DGTZ_Success )
	{
	  printf("      GLBMSK:%x",readback2);
	}
	else
	{
	  printf("       GLBMSK:E", readback2);
	}  
     
      uint32_t readback3;
      retcod = CAEN_DGTZ_ReadRegister(handle,0x8110,&readback3);
      if ( retcod == CAEN_DGTZ_Success )
	{
	  printf("      TRGOUT:%x",readback3);
	}
	else
	{
	  printf("       TRGOUT:E", readback3);
	} 
	//retcod = CAEN_DGTZ_GetTriggerPolarity(handle,CAEN_DGTZ_ACQ_STATUS_ADD,(CAEN_DGTZ_TriggerPolarity_t *)&data);

	retcod = CAEN_DGTZ_ReadRegister(handle,CAEN_DGTZ_ACQ_STATUS_ADD,&data);
	if ( retcod == CAEN_DGTZ_Success )
	{
	  bool run  = data & 0x0004;
	  bool drdy = data & 0x0008;
	  bool full = data & 0x0010;
	  bool clk  = data & 0x0020;
	  bool pll  = data & 0x0080;
	  bool rdy  = data & 0x0100;
	  printf(" CLK:%d  PLL:%d  RUN:%d  DRDY:%d  FULL:%d  RDY:%d\n",
		 clk, pll, run, drdy, full, rdy);
	}
	else
	{
	  printf("    *** [Status,%d]  suspect PLL is not locked\n", retcod);
	}
      }
      else
      {
	printf("  *** [Info,%d]\n", retcod);
      }
      retcod = CAEN_DGTZ_CloseDigitizer(handle); // We don't care if this fails
    }
    else
    {
      printf("  *** [Open,%d]\n", retcod); // Empty link
    }
  }
}

