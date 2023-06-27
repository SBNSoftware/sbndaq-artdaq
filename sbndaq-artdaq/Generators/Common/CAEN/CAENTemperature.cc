//
//
// sbndaq-artdaq/Generators/Common/CAEN/CAENStatus.cc  (wfb)
//
//
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <chrono>
#include <ctime>
#include "CAENDigitizer.h"
#include "CAENDigitizerType.h"

//#include "CAENDecoder.hh"

enum
{
  N_LINKS = 4  // 4 link per A3818 PCIe board
};

void readTemperature()
{

  int link, retcod, board, handle;
  uint32_t data;
  CAEN_DGTZ_BoardInfo_t info;

  board = 0; // Can be 0...7, but we only use one per optical chain

  printf("Link Model\n"); 
  for ( link = 0; link<N_LINKS; link++)
  {
    printf("\n %1.1d ", link);
    retcod = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_OpticalLink,
				     link, board, 0, &handle);
    if ( retcod == CAEN_DGTZ_Success )
    {
      retcod = CAEN_DGTZ_GetInfo(handle,&info);
      if ( retcod == CAEN_DGTZ_Success )
      {
	printf("  %s Serial %d\n ", info.ModelName, info.SerialNumber ); //%d

        uint32_t readback8;
        retcod = CAEN_DGTZ_ReadRegister(handle,0x10A8,&readback8);
        if ( retcod == CAEN_DGTZ_Success )
        {
    	  printf("      Temperature ch 0-1: %d",readback8);
        }
        else
        {
    	  printf("       Temperature 0:E");
        } 
    
        retcod = CAEN_DGTZ_ReadRegister(handle,0x12A8,&readback8);
        if ( retcod == CAEN_DGTZ_Success )
        {
    	  printf("      Temperature ch 2-3: %d",readback8);
        }
        else
        {
    	  printf("       Temperature 2:E");
        } 
    
        retcod = CAEN_DGTZ_ReadRegister(handle,0x14A8,&readback8);
        if ( retcod == CAEN_DGTZ_Success )
        {
    	  printf("      Temperature ch 4-5: %d",readback8);
        }
        else
        {
    	  printf("       Temperature 4:E");
        } 
    
        retcod = CAEN_DGTZ_ReadRegister(handle,0x16A8,&readback8);
        if ( retcod == CAEN_DGTZ_Success )
        {
    	  printf("      Temperature ch 6-7: %d",readback8);
        }
        else
        {
    	  printf("       Temperature 6:E");
        } 
    
        retcod = CAEN_DGTZ_ReadRegister(handle,0x18A8,&readback8);
        if ( retcod == CAEN_DGTZ_Success )
        { 
    	  printf("\n      Temperature ch 8-9: %d",readback8);
        }
        else
        {
    	  printf("\n       Temperature 8:E");
        } 
    
        retcod = CAEN_DGTZ_ReadRegister(handle,0x1AA8,&readback8);
        if ( retcod == CAEN_DGTZ_Success )
        {
    	  printf("      Temperature ch 10-11: %d",readback8);
        }
        else
        {
    	  printf("       Temperature 10:E");
        } 
    
        retcod = CAEN_DGTZ_ReadRegister(handle,0x1CA8,&readback8);
        if ( retcod == CAEN_DGTZ_Success )
        {
    	  printf("      Temperature ch 12-13: %d",readback8);
        }
        else
        {
    	  printf("       Temperature 12:E");
        } 
    
        retcod = CAEN_DGTZ_ReadRegister(handle,0x1EA8,&readback8);
        if ( retcod == CAEN_DGTZ_Success )
        {
    	  printf("      Temperature ch 14-15: %d",readback8);
        }
        else
        {
    	  printf("       Temperature 14:E");
        } 
    
        retcod = CAEN_DGTZ_ReadRegister(handle,CAEN_DGTZ_ACQ_STATUS_ADD,&data);
        if ( retcod == CAEN_DGTZ_Success )
        {
    	  bool run  = data & 0x0004;
    	  bool drdy = data & 0x0008;
    	  bool full = data & 0x0010;
    	  bool clk  = data & 0x0020;
    	  bool pll  = data & 0x0080;
    	  bool rdy  = data & 0x0100;
    	  printf("\n CLK:%d  PLL:%d  RUN:%d  DRDY:%d  FULL:%d  RDY:%d\n",
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

void read_temperature_loop()
{

  std::cout << "Read out temperature of each channel...Ctrl+C to kill" << std::endl;
 
  auto start = std::chrono::system_clock::now();
 
  while(true){

    auto now = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = now-start;
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);

    std::cout << std::endl
  	      << "---> Time now is " << std::ctime(&now_time)
              << "---> Time elapsed is " << elapsed_seconds.count() << " s"
              << std::endl;
    
    readTemperature();
 
    unsigned int sleep_duration = 1;  

    std::cout << "Sleep for " <<  sleep_duration << " s...zzz" << std::endl; 
    sleep(sleep_duration);
    
//    std::cout << "Sleep for " << sleep_duration << " us...zzz" << std::endl; 
//    usleep(sleep_duration);
  }
}

int main( int argc, char* argv[] ) //int argc, char **argv)
{
   printf("You have entered %d arguments:\n", argc);
 
    for (int i = 0; i < argc; i++) {
        printf("%s\n", argv[i]);
    }
   
  read_temperature_loop();
  
}

