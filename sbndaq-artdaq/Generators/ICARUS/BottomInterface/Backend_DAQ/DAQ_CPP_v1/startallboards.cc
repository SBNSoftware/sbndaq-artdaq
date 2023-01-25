#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <unistd.h>
#include <string>
#include <iostream>
#include "CRT.h"
//#include "eventbuilder.h"

//using namespace std;
using std::cout;
using std::endl;
using std::string;


int startallboards(const char *argv, const char *online_path)
{ 
  string filename;
  int PMTINI,PMTFIN; 
  filename = argv;
  int disk = 1;                 //Disk number (1 or 2)

    //string online_path = "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ";
    data_path = online_path+"/runs";
    //log_path = online_path;
    string DataPath2 = online_path;//"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/DCOV";

    //Direct this to wherever the readout is compiled
    //To be noted that the main DAQ code in /readout/src/main.c also contains the link to the location where the data is written
    //DAQ is also expectiing to write a logfile in $(DataPath2)/log (must exist or DAQ will fail)

    //TODO Change this to work with hardware library
    string file_path = __FILE__;
    string dir_path = file_path.substr(0,file_path.rfind("\\");
    string cmd = dir_path + "\\/readout\\/script\\/start_readout.sh \"readout\" \"1\" \"1\" \"none\"";
    system(cmd.c_str());

    //Data may be written wherever, but the code assumes there is a symbolic link $(online_path)/readout/data1 
    //to the folder where the DAQ puts the data

    ////////////////////////////////////////////////////////////////////////

    //For artdaq: this code creates a link to the latest data run at $(online_path)/readout/DataFolder

    string mode = "fcl";
    loadconfig(mode,0,0,0,filename);                       //read fcl file and load PMT data

    PMTINI = 1;
    PMTFIN = getnumpmt();
    cout << "PMTFIN: " << PMTFIN << endl;


    initializeboard("auto",500, PMTINI, PMTFIN,online_path);   //Takes baseline data and prepares USBs for writing

    eventbuilder("auto", PMTINI, PMTFIN,online_path);          //Calculates baseline data and writes it to a file

    starttakedata(PMTINI,PMTFIN,0,0);              //Starts taking data

    return 0;
}
