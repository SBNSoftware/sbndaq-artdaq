#include <string.h>
#include <stdio.h>

#include "artdaq/DAQdata/Globals.hh"
#include "Cudpux.h"

Cudp mytrig(9888);
Cudp mysock(11001);

int totp=0;
//extern "C"
int vetoOn()
{
  char mhost[128];
  char buf[128] __attribute__ ((nonstring));
  //char myrec[256];
  char st[256];
  int retval=0;

  strcpy(mhost,"192.168.2.152");
  //strcpy(mhost,"172.16.4.169");
  strcpy(buf,"12on");

  do {
    sprintf(st,"%2d",totp++%100);
    strncpy(buf,st,2);
    mytrig.SendTo(mhost,10000,buf,14);  
//    retval = mysock.Receive(myrec,2,1); 
    //retval = mysock.Receive(myrec,2); 
//    printf("sent received %s %s\n",buf, myrec);
    if (retval<0) {TRACEN("veto.cc",TLVL_INFO, "Lost udp packet");}
  } while (retval<0);
   
   TRACEN("veto.cc",TLVL_INFO, "veto ON");

return 0;
}

//extern "C"
int vetoOff()
{
  char mhost[128];
  char buf[128];
  //char myrec[256];
  int retval=0;

  strcpy(mhost,"192.168.2.152");
  //strcpy(mhost,"172.16.4.169");
  strcpy(buf,"12of");

  do {
    mytrig.SendTo(mhost,10000,buf,14);  
   // retval = mysock.Receive(myrec,2,1); 
    //retval = mysock.Receive(myrec,2); 
    if (retval<0) {TRACEN("veto.cc",TLVL_INFO, "Lost udp packet");}
  } while (retval<0);

    TRACEN("veto.cc",TLVL_INFO, "veto OFF");

return 0;
}

