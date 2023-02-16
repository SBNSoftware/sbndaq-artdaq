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



int main(int argc, char *argv[])
{ 
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
    //string DataPath = "/OVDAQ/DATA/Run_0001086";
    //string DataFolder = "/e/h.0/localdev/readout/data" + to_string(disk) + DataPath;

    string online_path = "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ";
    string DataPath2 = "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/DCOV";
//    string online_path = "/e/h.0/localdev/ICARUS_DAQ";
//    string DataPath2 = "/e/h.0/localdev/ICARUS_DAQ/DAQ_CPP_v1/readout/DCOV";

    //Direct this to wherever the readout is compiled
    //To be noted that the main DAQ code in /readout/src/main.c also contains the link to the location where the data is written
    //DAQ is also expecting to write a logfile in $(DataPath2)/log (must exist or DAQ will fail)
    string cmd = online_path + "\\/readout\\/script\\/start_readout.sh \"readout_old\" \"1\" \"1\" \"none\"";
    //string cmd = "/e/h.0/localdev/readout/script/start_readout.sh \"readout\" \"1\" \"1\" \"none\"";
    system(cmd.c_str());

    //Data may be written wherever, but the code assumes there is a symbolic link $(online_path)/readout/data1 
    //to the folder where the DAQ puts the data
    //Output folder for the DAQ is /e/h.0/localdev/readout/data1
    //Folder where the DAQ lives is /e/h.0/localdev/readout/bin
    ////////////////////////////////////////////////////////////////////////

    //For artdaq: this code creates a link to the latest data run at $(online_path)/readout/DataFolder

    //string mode = "mysql";
    //string mode = "debug";
    string mode = "fcl";

    loadconfig(mode,0,0,0,filename);                       //read fcl file and load PMT data

    //getpmtdata(17,0);
    //cout << "board_number: " << board_number << " gate: " << gate << " force_trig: " << force_trig << " trigger_mode: " << trigger_mode << endl;
    
    PMTINI = 1;
    PMTFIN = getnumpmt();
    cout << "PMTFIN: " << PMTFIN << endl;


    initializeboard("auto",500, PMTINI, PMTFIN,online_path);   //Takes baseline data and prepares USBs for writing
    // TODO create symbolic link between Run_000** and DataFolder


    //EBbaseline;
    //write function in CRT.cpp
	//cout << "Datapath: " << DataPath << " totalusb: " << totalusb << " totalpmt: " << totalpmt << endl;
    
    eventbuilder("auto", PMTINI, PMTFIN,online_path);          //Calculates baseline data and writes it to a file

    sleep(2.0);

    starttakedata(PMTINI,PMTFIN,0,0);              //Starts taking data
    

    //To debug directly, uncomment the following  
    
    //sleep(5); //How long to take data
    //stoptakedata(PMTINI,PMTFIN,0,0);
    //generatecsv(PMTINI,PMTFIN,online_path);                  //Prepares directories and generates plots with signal()

 //     sleep(0.5);

 //     getavg(PMTINI,PMTFIN,7);                     //Reads csv and gets the average of specified col

 //     getavg(PMTINI,PMTFIN,1);                                                                                                                           
	//col 1: pulse height
	//col 2: pulse height sigma
	//col 3: hits per channel
	//col 4: hits per packet
	//col 5: number of photo-electrons
	//col 6: gain
	//col 7: rate per channel


     // checkeff(33,3);                              //Writes to Timestamps.txt file

}
