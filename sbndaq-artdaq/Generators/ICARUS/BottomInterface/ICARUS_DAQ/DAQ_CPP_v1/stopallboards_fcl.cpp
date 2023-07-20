#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <unistd.h>
#include <string>
#include <iostream>
#include "CRT_fcl.h"
//#include "eventbuilder.h"


using namespace std;

int main(int argc, char *argv[]){ 
  string filename;
  int PMTINI,PMTFIN; 
  if (argc==2){
    filename = argv[1];
  }
  else{
    filename = "";
    cout<<"Please enter the name of the FHiCL file to read from"<<'\n';
    return 0;
  }
  int disk = 1;                 //Disk number (1 or 2)

    string online_path = "/home/nfs/icarus/DAQ_DevAreas/DAQ_17Feb2023_MM/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ";
    string DataPath2 = "/home/nfs/icarus/DAQ_DevAreas/DAQ_17Feb2023_MM/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/DCOV";
    //string online_path = "/e/h.0/localdev/ICARUS_DAQ";
    //string DataPath2 = "/e/h.0/localdev/ICARUS_DAQ/DAQ_CPP_v1/readout/DCOV";

    //Direct this to wherever the readout is compiled
    //To be noted that the main DAQ code in /readout/src/main.c also contains the link to the location where the data is written
    //DAQ is also expecting to write a logfile in $(DataPath2)/log (must exist or DAQ will fail)

    //Data may be written wherever, but the code assumes there is a symbolic link $(online_path)/readout/data1 
    //to the folder where the DAQ puts the data
    //Output folder for the DAQ is /e/h.0/localdev/readout/data1
    //Folder where the DAQ lives is /e/h.0/localdev/readout/bin
    ////////////////////////////////////////////////////////////////////////

    //For artdaq: this code creates a link to the latest data run at $(online_path)/readout/DataFolder
 
  string mode = "fcl";
  loadconfig(mode,0,0,0,filename);                       //read fcl file and load PMT data

  PMTINI = 1;
  PMTFIN = getnumpmt();

  //stoptakedata(PMTINI,PMTFIN,0,0,online_path);               //Stops taking data
  stoptakedata_new(PMTINI,PMTFIN,0,0,online_path);               //Stops taking data

  string cmd = online_path + "\\/readout\\/script\\/stop_readout.sh \"readout_old\"";
  system(cmd.c_str()); 

 
  //For checking rates and producing histograms, uncomment the following:
  generatecsv(PMTINI,PMTFIN,online_path);
  //   sleep(0.5);
  //   getavg(PMTINI,PMTFIN,7,online_path);                     //Reads csv and gets the average of specified col

 //     getavg(PMTINI,PMTFIN,1);                                                                                                                           
	//col 1: pulse height
	//col 2: pulse height sigma
	//col 3: hits per channel
	//col 4: hits per packet
	//col 5: number of photo-electrons
	//col 6: gain
	//col 7: rate per channel


     //checkeff(39,6,online_path);                              //Writes to Timestamps.txt file
}
