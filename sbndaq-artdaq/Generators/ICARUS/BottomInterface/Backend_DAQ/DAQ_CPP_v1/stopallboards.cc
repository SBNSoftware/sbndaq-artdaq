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
  string filename;
  int PMTINI,PMTFIN; 
  filename = argv;
  int disk = 1;                 //Disk number (1 or 2)
    
  string mode = "fcl";
  loadconfig(mode,0,0,0,filename);                       //read fcl file and load PMT data

  PMTINI = 1;
  PMTFIN = getnumpmt();

  stoptakedata(PMTINI,PMTFIN,0,0,online_path);               //Stops taking data

  string cmd = "killall bottomCRTreadout";
  system(cmd.c_str());

/*
  char pidline[1024];
  char *pid;
  int sig = 9;
  int i =0;
  int pidno[5];
  FILE *fp = popen("pidof bottomCRTreadout","r");
  fgets(pidline,1024,fp);

  printf("%s",pidline);
  pid = strtok(pidline," ");

  while (pid != NULL) {
    pidno[i] = atoi(pid);
    printf("%d\n",pidno[i]);
    pid = strtok(NULL , " ");
    kill(atoi(pid),sig);
    i++;
  }     
  pclose(fp);
*/
  return 0;
}
