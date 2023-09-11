//
//
// sbndaq-artdaq/Generators/Common/CAEN/CAENStatus.cc  (wfb)
//
//
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string> 
#include <chrono>
#include <ctime>
#include <iomanip>
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

  auto now = std::chrono::system_clock::now();
  auto in_time_t = std::chrono::system_clock::to_time_t(now);
  std::stringstream datetime;
  datetime << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d_%X");
  
  std::ofstream outfile ("/home/nfs/sbnd/lnguyen/CAEN_Temperature_Log/Temperature_Log_"+datetime.str()+".txt");
  outfile << "Channel Temperature[C] Date Time" << std::endl;

  board = 0; // Can be 0...7, but we only use one per optical chain

  bool goodTemp = true;
 
  while(goodTemp){

    //Open digitizer in link 3
    link = 3; 
    retcod = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_OpticalLink, link, board, 0, &handle);
    if ( retcod != CAEN_DGTZ_Success ) continue; 
    printf("\n Link %1.1d ", link);

    //Get digitizer info
    retcod = CAEN_DGTZ_GetInfo(handle,&info);
    if ( retcod != CAEN_DGTZ_Success ) continue; //Check if can get info
        
    printf("  %s Serial %d\n ", info.ModelName, info.SerialNumber ); //%d

    //Get Time
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream datetime;
    datetime << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");

    //Read Temperature
    uint32_t readback;
    retcod = CAEN_DGTZ_ReadRegister(handle,0x10A8,&readback);
    if ( retcod == CAEN_DGTZ_Success )
    {
    printf("      Temperature ch 0-1: %d",readback);
      outfile << "0" << " " << readback << " " << datetime.str() << std::endl;
      outfile << "1" << " " << readback << " " << datetime.str() << std::endl;
      if(readback > 85) goodTemp = false;
    }
    else
    {
      printf("       Temperature 0:E");
      outfile << "0" << " " << "-999" << " " << datetime.str() << std::endl;
      outfile << "1" << " " << "-999" << " " << datetime.str() << std::endl;
    } 
      
    retcod = CAEN_DGTZ_ReadRegister(handle,0x12A8,&readback);
    if ( retcod == CAEN_DGTZ_Success )
    {
    printf("      Temperature ch 2-3: %d",readback);
      outfile << "2" << " " << readback << " " << datetime.str() << std::endl;
      outfile << "3" << " " << readback << " " << datetime.str() << std::endl;
      if(readback > 85) goodTemp = false;
    }
    else
    {
      printf("       Temperature 2:E");
      outfile << "2" << " " << "-999" << " " << datetime.str() << std::endl;
      outfile << "3" << " " << "-999" << " " << datetime.str() << std::endl;
    } 
      
    retcod = CAEN_DGTZ_ReadRegister(handle,0x14A8,&readback);
    if ( retcod == CAEN_DGTZ_Success )
    {
    printf("      Temperature ch 4-5: %d",readback);
      outfile << "4" << " " << readback << " " << datetime.str() << std::endl;
      outfile << "5" << " " << readback << " " << datetime.str() << std::endl;
      if(readback > 85) goodTemp = false;
    }
    else
    {
      printf("       Temperature 4:E");
      outfile << "4" << " " << "-999" << " " << datetime.str() << std::endl;
      outfile << "5" << " " << "-999" << " " << datetime.str() << std::endl;
    } 
      
    retcod = CAEN_DGTZ_ReadRegister(handle,0x16A8,&readback);
    if ( retcod == CAEN_DGTZ_Success )
    {
      printf("      Temperature ch 6-7: %d",readback);
      outfile << "6" << " " << readback << " " << datetime.str() << std::endl;
      outfile << "7" << " " << readback << " " << datetime.str() << std::endl;
      if(readback > 85) goodTemp = false;
    }
    else
    {
      printf("       Temperature 6:E");
      outfile << "6" << " " << "-999" << " " << datetime.str() << std::endl;
      outfile << "7" << " " << "-999" << " " << datetime.str() << std::endl;
    } 
      
    retcod = CAEN_DGTZ_ReadRegister(handle,0x18A8,&readback);
    if ( retcod == CAEN_DGTZ_Success )
    { 
      printf("\n      Temperature ch 8-9: %d",readback);
      outfile << "8" << " " << readback << " " << datetime.str() << std::endl;
      outfile << "9" << " " << readback << " " << datetime.str() << std::endl;
      if(readback > 85) goodTemp = false;
    }
    else
    {
      printf("\n       Temperature 8:E");
      outfile << "8" << " " << "-999" << " " << datetime.str() << std::endl;
      outfile << "9" << " " << "-999" << " " << datetime.str() << std::endl;
    } 
      
    retcod = CAEN_DGTZ_ReadRegister(handle,0x1AA8,&readback);
    if ( retcod == CAEN_DGTZ_Success )
    {
      printf("      Temperature ch 10-11: %d",readback);
      outfile << "10" << " " << readback << " " << datetime.str() << std::endl;
      outfile << "11" << " " << readback << " " << datetime.str() << std::endl;
      if(readback > 85) goodTemp = false;
    }
    else
    {
      printf("       Temperature 10:E");
      outfile << "10" << " " << "-999" << " " << datetime.str() << std::endl;
      outfile << "11" << " " << "-999" << " " << datetime.str() << std::endl;
    } 
      
    retcod = CAEN_DGTZ_ReadRegister(handle,0x1CA8,&readback);
    if ( retcod == CAEN_DGTZ_Success )
    {
    printf("      Temperature ch 12-13: %d",readback);
      outfile << "12" << " " << readback << " " << datetime.str() << std::endl;
      outfile << "13" << " " << readback << " " << datetime.str() << std::endl;
      if(readback > 85) goodTemp = false;
    }
    else
    {
      printf("       Temperature 12:E");
      outfile << "12" << " " << "-999" << " " << datetime.str() << std::endl;
      outfile << "13" << " " << "-999" << " " << datetime.str() << std::endl;
    } 
      
    retcod = CAEN_DGTZ_ReadRegister(handle,0x1EA8,&readback);
    if ( retcod == CAEN_DGTZ_Success )
    {
    printf("      Temperature ch 14-15: %d",readback);
      outfile << "14" << " " << readback << " " << datetime.str() << std::endl;
      outfile << "15" << " " << readback << " " << datetime.str() << std::endl;
      if(readback > 85) goodTemp = false;
    }
    else
    {
    printf("       Temperature 14:E");
      outfile << "14" << " " << "-999" << " " << datetime.str() << std::endl;
      outfile << "15" << " " << "-999" << " " << datetime.str() << std::endl;
    } 
    //End of read temperature      
    
    //Close digitizer
    retcod = CAEN_DGTZ_CloseDigitizer(handle); 
 
    //Throw some errors if temperature goes above 85 
    if(!goodTemp){
      outfile << "BAD TEMPERATURE, STOP RECORDING" << std::endl;
      std::cout << "\nBAD TEMPERATURE, STOP RECORDING" << std::endl;
    }

    //Get some sleep
    unsigned int sleep_duration = 30;  
    sleep(sleep_duration);

  } //End of while loop
      
  outfile.close();
  return;
}


int main()
{
  readTemperature();
}

