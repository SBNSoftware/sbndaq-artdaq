/// prints out statistics from FEB driver
#include <zmq.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "febevt.h"


void usage()
{
 printf("Connects to data stream from running febdrv at a given data socket and prints out missing GPS and Beam trigs.\n Usage: ");
 printf("febmontrigs <socket>\n");
 printf("Example:  fenmontrigs tcp://localhost:5556 \n");
// printf("<max events> is maximum number of polls per file. If reached, the index in file name increments. Optional, set to 10000 by default. \n");

}

int main (int argc, char **argv)
{
int rv;
char * iface;
char filename[128];
char sim[4]={'|','/','-','\\'};
int isim=0;
time_t t0,t1;
int dt,dt0;
int polls, maxpolls, findex;
if(argc!=2) { usage(); return 0;}
iface=argv[1];
int evsinpoll;
int gps[256];
int beam[256];
EVENT_t * event;
void * context = zmq_ctx_new ();
//  Socket to talk to server
printf ("Connecting to febdrv at %s...\n",iface);
void *subscriber = zmq_socket (context, ZMQ_SUB);
if(subscriber<=0) { printf("Can't initialize the socket!\n"); return 0;}
rv=zmq_connect (subscriber, iface);
if(rv<0) { printf("Can't connect to the socket!\n"); return 0;}
//zmq_connect (subscriber, "ipc://stats");
rv=zmq_setsockopt (subscriber, ZMQ_SUBSCRIBE, NULL, 0);
if(rv<0) { printf("Can't set SUBSCRIBE option to the socket!\n"); return 0;}
printf("0"); fflush(stdout);
while(1)
{
zmq_msg_t reply;
zmq_msg_init (&reply);
t0=time(NULL);
dt=0; dt0=0;
while(zmq_msg_recv (&reply, subscriber, ZMQ_DONTWAIT)==-1)
 {
  dt=time(NULL)-t0; 
  if(dt>2 && dt!=dt0) {
                      printf("No data from driver for %d seconds!\n",dt); 
                      dt0=dt;
  }
 };
printf("\b%c",sim[isim]); fflush(stdout); if(isim<3) isim++; else isim=0;
//fwrite((char*)zmq_msg_data(&reply),zmq_msg_size(&reply) , 1, fp);
for(int i=0;i<256;i++) {gps[i]=1; beam[i]=1;} //all OK by default
evsinpoll=zmq_msg_size(&reply)/EVLEN;
for(int i=0;i<evsinpoll-1;i++) //loop through all but the special event
{
  event=(EVENT_t*)(zmq_msg_data(&reply)+i*EVLEN);
  if((event->flags & 0x1)==0) gps[event->mac5]=0;
  if((event->flags & 0x2)==0) beam[event->mac5]=0;
}
//Now check if there were missing
for(int i=0;i<256;i++) 
   {
      if(gps[i]==0) printf("Missed GPS PPS at FEB %d\n",i);
      if(beam[i]==0) printf("Missed Beam pulse at FEB %d\n",i);
   }

zmq_msg_close (&reply);
}
zmq_close (subscriber);
zmq_ctx_destroy (context);
return 0;
}
