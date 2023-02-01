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

int stopallboards(const char *argv, const char *online_path){ 
  string filename;
  int PMTINI,PMTFIN; 
  filename = argv;
  int disk = 1;                 //Disk number (1 or 2)
    
  //string online_path = "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ";
  //log_path = online_path;
  string DataPath2 = online_path;//"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/DCOV";

  //Direct this to wherever the readout is compiled
  //To be noted that the main DAQ code in /readout/src/main.c also contains the link to the location where the data is written
  //DAQ is also expecting to write a logfile in $(DataPath2)/log (must exist or DAQ will fail)

  //Data may be written wherever, but the code assumes there is a symbolic link $(online_path)/readout/data1 
  //to the folder where the DAQ puts the data

  ////////////////////////////////////////////////////////////////////////

  //For artdaq: this code creates a link to the latest data run at $(online_path)/readout/DataFolder
 
  string mode = "fcl";
  loadconfig(mode,0,0,0,filename);                       //read fcl file and load PMT data

  PMTINI = 1;
  PMTFIN = getnumpmt();

  stoptakedata(PMTINI,PMTFIN,0,0,online_path);               //Stops taking data

  //TODO Change this to work with hardware library
  //string dir_path = "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ";
  //string cmd = dir_path + "\\/readout\\/script\\/stop_readout.sh \"readout\"";
  //system(cmd.c_str());
/*
  my @marks = (
    "\\.\\/readout"
  );



  open IN, "ps -eo pid,cmd |" or die $!;
  while(<IN>) {
      if(/^\s*(\d+)\s+(.*)/) {
          my $pid = $1;
          my $cmd = $2;
          my $kill_it = 0;
          for (@marks) { if ($cmd =~ /$_/) { $kill_it = 1; } }
          if ($kill_it) {
              #print "Killing process $pid...\n";
              kill 9, $pid or print "Error killing: $!\n";
          }
      }
  }
  
  open IN, "ipcs -q |" or die $!;
  while(<IN>) {
      if(/^([0-9a-fx]+)\s+/) {
          my $tmp = hex($1);
          #print "Destroying queue $tmp...\n";
          #system "ipcrm -Q $tmp";
      }
  }
  */
  return 0;
}
