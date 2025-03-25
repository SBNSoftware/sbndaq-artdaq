/// prints out statistics from FEB driver
#include <zmq.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>


void usage()
{
 printf("FEB statistics monitor. Connects to running febdrv at given socket \nUsage: ");
 printf("febmon <socket>\n");
 printf("Socket string, example: tcp://localhost:5557 \n");

}

int main (int argc, char **argv)
{
time_t t0,t1;
int dt,dt0;
int rv=0;
if(argc!=2) { usage(); return 0;}
void * context = zmq_ctx_new ();
//  Socket to talk to server
printf ("Connecting to febdrv...\n");
void *subscriber = zmq_socket (context, ZMQ_SUB);
rv=zmq_connect (subscriber, argv[1]); if(rv<0) {printf("Connection to %s failed!\n",argv[1]); return 0;}
//zmq_connect (subscriber, "ipc://stats");
zmq_setsockopt (subscriber, ZMQ_SUBSCRIBE, NULL, 0);
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
                      printf("No reply from driver for %d seconds!\n",dt); 
                      dt0=dt;
  }
 };
printf ("%s", (char*)zmq_msg_data (&reply));
zmq_msg_close (&reply);
}
zmq_close (subscriber);
zmq_ctx_destroy (context);
return 0;
}
