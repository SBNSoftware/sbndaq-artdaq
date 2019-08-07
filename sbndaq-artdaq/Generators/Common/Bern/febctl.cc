// sends literal commands without parameters to feb

#include <zmq.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>


void usage()
{
 printf("Usage: ");
 printf("febctl <socket> <CMD> <FEB S/N>\n");
 printf("Socket string, example: tcp://localhost:5555 \n");
 printf("<CMD> can be chosen from:\nBIAS_OF\nBIAS_ON\nDAQ_BEG\nDAQ_END\nGETINFO\n");
 printf("<FEB S/N> can be 255 or omitted, in this case command is broadcast for all connected FEBs\n");
}


int main (int argc, char **argv)
{
  int rv=0;
  if(argc<3 || argc>4) { usage(); return 0;}
  char cmd[32];
  uint8_t mac5;
  if(argc==4) mac5=atoi(argv[3]); else mac5=255;
  void * context = zmq_ctx_new ();

  //  Socket to talk to server
  printf ("Connecting to febdrv...\n");
  void *requester = zmq_socket (context, ZMQ_REQ);
  const int linger = 0; //time in ms to keep pending messages after closing the socket
  zmq_setsockopt(requester, ZMQ_LINGER, &linger, sizeof(linger));
  rv=zmq_connect (requester, argv[1]); 
  if(rv<0) 
  { printf("Connection to %s failed!\n",argv[1]); return -1;}

  //zmq_connect (requester, "ipc://command");
  int request_nbr;
  zmq_msg_t request;
  zmq_msg_init_size (&request, 9);
  memcpy(zmq_msg_data (&request), argv[2],7);
  memcpy(cmd, argv[2],7); cmd[7]=0;
  ((uint8_t*)zmq_msg_data (&request))[7]=0;
  ((uint8_t*)zmq_msg_data (&request))[8]=mac5;
  printf ("Sending command %s...\n", cmd);
  zmq_msg_send (&request, requester, 0);
  zmq_msg_close (&request);
  zmq_msg_t reply;
  const int timeout = 5*1000*1000; //µs
  zmq_msg_init (&reply);
  rv = zmq_msg_recv (&reply, requester, ZMQ_DONTWAIT);
  for(int i = 0; i < 10 && rv < 0; i++) {
    printf("\twaiting for answer...\n");
    usleep(timeout/10);
    rv = zmq_msg_recv (&reply, requester, ZMQ_DONTWAIT);
  }
  if(rv>=0) printf ("Received reply: %s\n", (char*)zmq_msg_data (&reply));
  else printf ("Error: received no reply after waiting for %d seconds\n", timeout / 1000000);
  zmq_msg_close (&reply);
  zmq_close (requester);
  zmq_ctx_destroy (context); 
  return 0;
}
