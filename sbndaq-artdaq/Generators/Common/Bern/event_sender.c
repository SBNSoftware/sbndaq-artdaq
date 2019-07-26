/// prints out statistics from FEB driver
#include <zmq.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "febstore.h"

#define EVLEN 76

#define events_per_pack 2000  //number of events send per package
#define send_duration 1       //period of sending in seconds

void free_bufer (void *data, void *hint);

int main (){
  void * publisher =0;
  int size_ev;
  void * context = zmq_ctx_new();
  int rv;
  //  Socket to send data to clients
  publisher = zmq_socket (context, ZMQ_PUB);
  if(publisher<=0) {printf("Can't initialize the socket!\n"); return 0;}
  rv = zmq_bind (publisher, "tcp://*:5556");
  if(rv<0) {printf("Can't bind tcp socket for data! Exiting.\n"); return 0;}
  printf ("febdrv: data publisher at tcp://5556\n");

  printf("\n");
 
  FILE *data = 0;
  //give path of source file
  data = fopen("/home/thmettler/Desktop/MuTOF/fermi_data/MuTOF_54-58-28-35.dat","r");	//eigene Maschinen

	fseek(data, 0, SEEK_END); // seek to end of file
	int size = ftell(data); // get current file pointer
	fseek(data, 0, SEEK_SET); // seek back to beginning of file
	
	size_ev=size/sizeof(EVENT_t);		//number of total events
	printf("Total Number of events: %d\n",size_ev);
	
  EVENT_t evbuf[events_per_pack+1];
  
  int ev_counter=0;
  
  while(ev_counter<size_ev){
  
	//Fill the buffer
	for (int counter=0; counter < events_per_pack; counter++)
		{
			fread(&evbuf[counter],sizeof(EVENT_t),1,data);
		}
	
    
  zmq_msg_t msg;
  zmq_msg_init_data (&msg, evbuf, events_per_pack*EVLEN , free_bufer, NULL);
  zmq_msg_send (&msg, publisher, ZMQ_DONTWAIT);
  zmq_msg_close (&msg);
  ev_counter+=events_per_pack;
    printf("sent events: %d\n", ev_counter);
  sleep(send_duration);
  
  }
 zmq_close (publisher);
 zmq_ctx_destroy (context);
 return 0; 
}

void free_bufer (void *data, void *hint) //call back from ZMQ sent function, hint points to subbufer index
{
 data=0;
}

