#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <unistd.h>
#include <string>
#include <sstream>
#include <iostream>
#include "CRT.h"
#include <sys/types.h>
#include <signal.h>

//using namespace std;
using std::cout;
using std::endl;
using std::string;
using std::istringstream;

int stopallboards(const char *argv, const char *online_path){ 
  string fname;
  int PMTINI,PMTFIN; 
  fname = argv;
  int disk = 1;                 //Disk number (1 or 2)
    
  //string mode = "fcl";
  //loadconfig(mode,0,0,0,filename);                       //read fcl file and load PMT data

  PMTINI = 1;
  PMTFIN = Bottom::getnumpmt();

  Bottom::stoptakedata(PMTINI,PMTFIN,0,0,online_path);               //Stops taking data

  //string cmd = "killall bottomCRTreadout";
  //system(cmd.c_str());

  char pidline[1024];
  char *pid;
  int sig = 9;
  int i =0;
  int pidno[2];
  FILE *fp = popen("pidof bottomCRTreadout","r");
  fgets(pidline,1024,fp);
  pid = strtok(pidline," ");
  while (pid != NULL) {
    pidno[i] = atoi(pid);
    pid = strtok(NULL , " ");
    i++;
  }     
  pclose(fp);

  for (int id : pidno) {
    kill(id,sig);
  }

  return 0;
}
