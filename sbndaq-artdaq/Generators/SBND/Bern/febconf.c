/// sends literal command without parameters to feb
#include <zmq.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define MAXPACKLEN 1500


void usage()
{
 printf("Usage: ");
 printf("febconf tcp://localhost:5555 SCRfile.txt PMRfile.txt <FEB S/N>\n");
 printf("tcp://localhost:5555 should be socket to send conf command to...\n");
 printf("SCRfile is a bitstream .txt file for SC register.\n");
 printf("PMRfile is a bitstream .txt file for PM register.\n"); 
 printf("<FEB S/N> is the mac address (lowest byte) of FEB to be configured. Cannot braodcast (no 255!)\n");
 //printf("<FEB S/N> can be 255 or omitted, in this case command is broadcast for all connected FEBs\n");

}

uint8_t bufSCR[MAXPACKLEN];
uint8_t bufPMR[MAXPACKLEN];

int readbitstream(char * fname, uint8_t *buf)
{
  FILE *file = fopen(fname, "r");
       if(file<=0) return 0;
       char line[128];
       char bits[128];
       char comment[128];
       char bit; 
       int ptr, byteptr;
       int bitlen=0;
       char ascii[MAXPACKLEN];
	while (fgets(line, sizeof(line), file)) {
	  bit=1; ptr=0; byteptr=0;
//	  	printf("%d: %s",bitlen,line);

	  while(bit!=0x27 && bit!=0 && ptr<sizeof(line) && bitlen<MAXPACKLEN) // ASCII(0x27)= '
	  {
	    bit=line[ptr];
	    ptr++;
	    if(bit==0x20 || bit==0x27) continue; //ignore spaces and apostrophe
//	     printf("%c",bit);
	    ascii[bitlen]=bit;
	    bitlen++;
	  }
//	printf("\n");
	}
  fclose(file);
  memset(buf,0,MAXPACKLEN); //reset buffer
// Now encode ASCII bitstream into binary
  for(ptr=bitlen-1;ptr>=0;ptr--)
  {
    byteptr=(bitlen-ptr-1)/8;
    if(ascii[ptr]=='1')  buf[byteptr] |= (1 << (7-ptr%8)); 
 //   if((ptr%8)==0) printf("bitpos=%d buf[%d]=%02x\n",ptr,byteptr,buf[byteptr]);
  }

  for (ptr=0;ptr<bitlen/8;++ptr){
    printf("%02x", buf[ptr]);
  }
  puts("\n");

  return bitlen;
}

int main (int argc, char **argv)
{
if(argc!=5) { usage(); return 0;}
int bitlen=0;
char cmd[32];
uint8_t mac5;
uint8_t buf[MAXPACKLEN];
mac5=atoi(argv[4]);
 if(mac5==255){
   usage();
   printf("SHAME!\n");
   return 0;
 }

bitlen=readbitstream(argv[2], bufSCR);
if(bitlen==1144) printf("FEBDTP::ReadBitStream: %d bits read from SCR config file %s.\n",bitlen,argv[2]);
else { printf("FEBDTP::ReadBitStream: %d bits read from unrecognized type file %s. Aborting\n",bitlen,argv[2]);
return 0; }
bitlen=readbitstream(argv[3], bufPMR);
if(bitlen==224) printf("FEBDTP::ReadBitStream: %d bits read from PMR config file %s.\n",bitlen,argv[3]);
else { printf("FEBDTP::ReadBitStream: %d bits read from unrecognized type file %s. Aborting\n",bitlen,argv[3]);
return 0; }

void * context = zmq_ctx_new ();
//  Socket to talk to server
printf ("Connecting to febdrv...\n");
void *requester = zmq_socket (context, ZMQ_REQ);
zmq_connect (requester, argv[1]);
//zmq_connect (requester, "ipc://command");
//int request_nbr;
//zmq_msg_t request;
//zmq_msg_init_size (&request, 9);
//sprintf(zmq_msg_data (&request),"CONFIGU");
sprintf(cmd,"SETCONF");
//((uint8_t*)zmq_msg_data (&request))[7]=0;
//((uint8_t*)zmq_msg_data (&request))[8]=mac5;
cmd[8]=mac5;
printf ("Sending command %s...", cmd);
//zmq_msg_send (&request, requester, ZMQ_SNDMORE);
//zmq_msg_close (&request);

memcpy(buf,cmd,9);
memcpy(buf+9,bufSCR,1144/8);
memcpy(buf+9+1144/8,bufPMR,224/8);
zmq_send ( requester, buf, (1144+224)/8+9, 0);

printf ("waiting for reply..\n");


zmq_msg_t reply;
zmq_msg_init (&reply);
zmq_msg_recv (&reply, requester, 0);
printf ("Received reply: %s\n", (char*)zmq_msg_data (&reply));
zmq_msg_close (&reply);
zmq_close (requester);
zmq_ctx_destroy (context);
return 1;
}
