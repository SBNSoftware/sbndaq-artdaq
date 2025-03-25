#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <unistd.h>
#include <string>
#include <iostream>
#include "CRT.h"
//#include "eventbuilder.h"
#include <tracemf.h>

//using namespace std;
//using std::cout;
using std::endl;
using std::string;

int startallboards(const char *argv, const char *online_path)
{ 
  string filename;
  int PMTINI,PMTFIN; 
  filename = argv;
  int disk = 1;                 //Disk number (1 or 2)

    //string online_path = "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ";
    //log_path = online_path;
    //string DataPath2 = online_path;//"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/bottomCRTreadout/DCOV";

    //Direct this to wherever the bottomCRTreadout is compiled
    //To be noted that the main DAQ code in /bottomCRTreadout/src/main.c also contains the link to the location where the data is written
    //DAQ is also expectiing to write a logfile in $(DataPath2)/log (must exist or DAQ will fail)

    //TODO Change this to work with hardware library
  //string dir_path = "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ";
  //string cmd = dir_path + "\\/bottomCRTreadout\\/script\\/start_bottomCRTreadout.sh \"bottomCRTreadout\" \"1\" \"1\" \"none\"";
    string cmd = "bottomCRTreadout -d 1 &";
    system(cmd.c_str());

    //Data may be written wherever, but the code assumes there is a symbolic link $(online_path)/bottomCRTreadout/data1 
    //to the folder where the DAQ puts the data

    ////////////////////////////////////////////////////////////////////////

    //For artdaq: this code creates a link to the latest data run at $(online_path)/bottomCRTreadout/DataFolder

    string mode = "fcl";
    Bottom::loadconfig(mode,0,0,filename);                       //read fcl file and load PMT data

    PMTINI = 1;
    PMTFIN = Bottom::getnumpmt();
    //cout << "PMTFIN: " << PMTFIN << endl;


    Bottom::initializeboard("auto",1000,PMTINI,PMTFIN,online_path);   //Takes baseline data and prepares USBs for writing

    int res = Bottom::eventbuilder("auto",PMTINI,PMTFIN,online_path);

    int trycounter = 0;
    while(res == 0){  //Calculates baseline data and writes it to a file
      trycounter++;
      TRACE(TLVL_INFO,"Baseline taking failed, trying to initialize again. Attempt %d",trycounter);
      sleep(1.0);
      Bottom::initializeboard("auto",1000,PMTINI,PMTFIN,online_path); //Re-initialize the boards
      res = Bottom::eventbuilder("auto",PMTINI,PMTFIN,online_path);
      if(trycounter>2){
        TRACE(TLVL_ERROR,"No baselines taken after 4 attempts, giving up.");
        break;
      }
    }
    Bottom::starttakedata(PMTINI,PMTFIN);              //Starts taking data

    return 0;
}
