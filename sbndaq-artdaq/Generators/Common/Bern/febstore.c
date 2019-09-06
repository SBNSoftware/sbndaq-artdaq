/// prints out statistics from FEB driver
#include <zmq.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

FILE *fp;

void usage()
{
 printf("Connects to data stream from running febdrv at a given data socket and stores (adds) data in a file.\n Usage: ");
 printf("febstore <socket> <filename>  <max events>\n");
 printf("Interface example:  tcp://localhost:5556 \n");
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
if(argc<3 || argc>4) { usage(); return 0;}
iface=argv[1];
//filename=argv[2];
findex=1; polls=0;
sprintf(filename,"%s.%d",argv[2],findex);
if(argc==4) maxpolls=atoi(argv[3]); else maxpolls=10000;
fp=fopen(filename,"a"); 
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
//printf ("%s", (char*)zmq_msg_data (&reply));
printf("\b%c",sim[isim]); fflush(stdout); if(isim<3) isim++; else isim=0;
fwrite((char*)zmq_msg_data(&reply),zmq_msg_size(&reply) , 1, fp);
fflush(fp);
zmq_msg_close (&reply);
polls++;
/*
if(polls>maxpolls) 
  {
    polls=0;
    findex++;
    fclose(fp);
    sprintf(filename,"%s.%d",argv[2],findex);
    fp=fopen(filename,"a"); 
  }
  */
}
zmq_close (subscriber);
zmq_ctx_destroy (context);
fclose(fp);
return 0;
}
