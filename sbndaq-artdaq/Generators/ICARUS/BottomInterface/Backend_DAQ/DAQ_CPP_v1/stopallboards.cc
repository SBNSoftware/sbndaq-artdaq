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
#include "TRACE/tracemf.h"

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

  sleep(2);

  char pidline[1024];
  char *pid;
  int sig = 9;
  unsigned pids_found= 0;
  int pidno[2];
  FILE *fp = popen("/sbin/pidof bottomCRTreadout","r");
  if (fp) {
    pidline[0]='x'; pidline[1]='\0';
    char *ret= fgets(pidline,sizeof(pidline),fp);
    if (ret) {
      TLOG_DEBUG(1)<<"fp="<<(void*)fp<<" pidline is "<<pidline;
      pid = strtok(pidline," ");
      TLOG_DEBUG(1) << "pid is " << pid;
      while (pid != NULL && pids_found < (sizeof(pidno)/sizeof(pidno[0]))) {
	pidno[pids_found] = atoi(pid);
	pid = strtok(NULL , " ");
	++pids_found;
      }
    } else TLOG_DEBUG(1) <<"ret="<<(void*)ret <<" NO (stdout) OUTPUT; pidline[0]="<<pidline[0];
    for (unsigned id=0; id<pids_found; ++id) {
      if ( pidno[id] > 0 ) {
	TLOG_DEBUG()<<"killing pid: "<<pidno[id];
	kill(pidno[id],sig);
      }
    }
    pclose(fp);
  } else TLOG_ERROR() <<"attempt to get pids of bottomCRTreadout processes via popen failed. popen(...) returned: "<<(void*)fp;
  TLOG_DEBUG() << "pids found: " << pids_found;

  return 0;
}
