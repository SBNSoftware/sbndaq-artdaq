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
  printf("Connects to data stream from running febdrv at a given data socket and prints data in terminal.\n Usage: ");
  printf("febpeek <socket>\n");
  printf("Interface example:  tcp://localhost:5556 \n");
  // printf("<max events> is maximum number of polls per file. If reached, the index in file name increments. Optional, set to 10000 by default. \n");

}

void print_data(unsigned char * data, int size) {
  int event_size = 84;
  printf("MAC FLG LC  LF  T0      T1      ADC     2       4       6       8       10      12      14      16      18      20      22      24      26      28      30      COINC..\n");
  for(int iByte = 0; iByte < size; iByte++) {
    switch(iByte%event_size%8) {
      case 1:
      case 3:
        printf("\033[0;31m");
        break;
      case 0:
      case 2:
        printf("\033[1;31m");
        break;
      case 5:
      case 7:
        printf("\033[0;33m");
        break;
      case 4:
      case 6:
        printf("\033[1;33m");
        break;
    }

    printf("%02x", data[iByte]);
    if((iByte+1)%event_size == 0) {
      printf("\n");
    }
  }
  printf("MAC FLG magic0  magic1  #events poll_start      poll_end        start_d end_d   zeros...................................................................................\n");
  printf("\n");
}

int main (int argc, char **argv)
{
  int rv;
  char * iface;
  time_t t0;
  int dt,dt0;
  if(argc<2) { usage(); return 0;}
  iface=argv[1];
  void * context = zmq_ctx_new ();
  //  Socket to talk to server
  printf ("Connecting to febdrv at %s...\n",iface);
  void *subscriber = zmq_socket (context, ZMQ_SUB);
  rv=zmq_connect (subscriber, iface);
  if(rv<0) { printf("Can't connect to the socket!\n"); return 0;}
  rv=zmq_setsockopt (subscriber, ZMQ_SUBSCRIBE, NULL, 0);
  if(rv<0) { printf("Can't set SUBSCRIBE option to the socket!\n"); return 0;}
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
    print_data((unsigned char*)zmq_msg_data(&reply), zmq_msg_size(&reply));
    zmq_msg_close (&reply);
  }
  zmq_close (subscriber);
  zmq_ctx_destroy (context);
  return 0;
}
