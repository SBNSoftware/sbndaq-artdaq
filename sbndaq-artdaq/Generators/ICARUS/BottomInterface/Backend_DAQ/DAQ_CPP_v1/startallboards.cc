#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <unistd.h>
#include <string>
#include <iostream>
#include "CRT.h"

//using namespace std;
using std::cout;
using std::endl;
using std::string;

int readout_exe(int argc, char **argv);

int startallboards(const char *argv, const char *online_path)
{ 
  string filename;
  int PMTINI,PMTFIN; 
  filename = argv;

  //Direct this to wherever the readout is compiled
  //To be noted that the main DAQ code in /readout/src/main.c also contains the link to the location where the data is written
  //DAQ is also expectiing to write a logfile in $(DataPath2)/log (must exist or DAQ will fail)


  //TODO Change this to work with hardware library
  string arg = "-d 1";
  char* carray = new char[4];
  strcpy(carray,arg.c_str());
  readout_exe(1, &carray);

  //string dir_path = "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ";
  //string cmd = dir_path + "\\/readout\\/script\\/start_readout.sh \"readout\" \"1\" \"1\" \"none\"";
  //system(cmd.c_str());

  //Data may be written wherever, but the code assumes there is a symbolic link $(online_path)/readout/data1 
  //to the folder where the DAQ puts the data

  ////////////////////////////////////////////////////////////////////////

  //For artdaq: this code creates a link to the latest data run at $(online_path)/readout/DataFolder

  string mode = "fcl";
  loadconfig(mode,0,0,0,filename);                       //read fcl file and load PMT data

  PMTINI = 1;
  PMTFIN = getnumpmt();

  initializeboard("auto",500, PMTINI, PMTFIN,online_path);   //Takes baseline data and prepares USBs for writing
  eventbuilder("auto", PMTINI, PMTFIN,online_path);          //Calculates baseline data and writes it to a file
  starttakedata(PMTINI,PMTFIN,0,0);              //Starts taking data

  return 0;
}
