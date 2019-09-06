/// prints out statistics from FEB driver
#include <zmq.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "febevt.h"

#define BUFLEN 512
#define PORT 8089 
#define SERVER "127.0.0.1"

DRIVER_STATUS_t ds;
FEB_STATUS_t fs;
char query[2048];
 // MYSQL *con=NULL;
time_t lastseen=0;

struct sockaddr_in si_me, si_other;
int s,i,slen=sizeof(si_other), recv_len;
char buf[BUFLEN];



void die(char *s)
{
 perror(s);
 exit(1);
}

void usage()
{
 printf("FEB statistics logger. Sends data by UDP to influxdb at port 8089 of the indicated IP address. Connects to running febdrv at given stats2 socket \nUsage: ");
 printf("febinfluxdb <febdrv:socket> <influxdb:host> [period(optional)]\n");
 printf("Optional period parameter defines periodicity of DB update in seconds. If ommited, every message from febdrv triggers the update (a lot of data!)\n");
 printf("Example: \n febinfluxdb tcp://localhost:5558 127.0.0.1 1\n");
}

//'9999-12-31 23:59:59.999999'
void sendudp(char *message)
{
// printf("Sending %d bytes\n",strlen(message));
  if(sendto(s, message, strlen(message), 0, (struct sockaddr*) &si_other, slen)==-1) die("sendto()");
}

void updatedb_dstat()
{
  char dtm[32];
  if(ds.status>-1) lastseen=ds.datime;
  strftime(dtm, 20, "%Y-%m-%d %H:%M:%S", localtime(&lastseen));
//  printf("ds. datime=%s daqon=%d status=%d nfebs=%d msperpoll=%d\n",asctime(localtime(&ds.datime)) ,ds.daqon,ds.status,ds.nfebs,ds.msperpoll);
  printf("(`status`, `lastseen`, `nfebs`, `pollperiod`, `daqon`) = (%d,'%s',%d,%d,%d)\n",ds.status,dtm,ds.nfebs,ds.msperpoll,ds.daqon);

 // printf("drvstats datime=%ldi,daqon=%di,status=%di,nclients=%di,msperpoll=%di\n",ds.datime,ds.daqon,ds.status,ds.nfebs,ds.msperpoll);
  sprintf(query,"drvstats datime=%ldi,daqon=%di,status=%di,nclients=%di,msperpoll=%di",ds.datime,ds.daqon,ds.status,ds.nfebs,ds.msperpoll);
  sendudp(query);
//if (mysql_query(con, query)) {
//     printf("%s\n", mysql_error(con));
//  }
}

void updatedb_febstat()
{
 // printf("fs. %02x:%02x:%02x:%02x:%02x:%02x %s %s connected=%d configured=%d biason=%d error=%d evtsperpoll=%d lostcpu=%d lostfpga=%d evtrate=%f\n",fs.mac[0],fs.mac[1],fs.mac[2],fs.mac[3],fs.mac[4],fs.mac[5],fs.fwcpu,fs.fwfpga,fs.connected, fs.configured, fs.biason,fs.error,fs.evtperpoll,fs.lostcpu,fs.lostfpga,fs.evtrate); 
 // printf("febstats,host=\"feb%d\" configd=%di,connectd=%di,evrate=%f,lostcpu=%di,lostfpga=%di,biason=%di,evtsperpoll=%di,error=%di\n",fs.mac[5],fs.configured, fs.connected,fs.evtrate,fs.lostcpu,fs.lostfpga,fs.biason,fs.evtperpoll,fs.error); 
  sprintf(query,"febstats,host=\"feb%d\" configd=%di,connectd=%di,evrate=%f,lostcpu=%di,lostfpga=%di,biason=%di,evtsperpoll=%di,error=%di,ts0ok=%di,ts1ok=%di",fs.mac[5],fs.configured, fs.connected,fs.evtrate,fs.lostcpu,fs.lostfpga,fs.biason,fs.evtperpoll,fs.error,fs.ts0ok,fs.ts1ok); 
  sendudp(query);
  char mac[20];
  //sprintf(mac,"%02x:%02x:%02x:%02x:%02x:%02x",fs.mac[0],fs.mac[1],fs.mac[2],fs.mac[3],fs.mac[4],fs.mac[5]);
  // sprintf(query,"INSERT INTO `febs_status`(`FebSN`, `MAC`, `configured`, `connected`, `evtrate`, `lostcpu`, `lostfpga`, `biason`, `evsperpoll`, `error`) VALUES (%d,'%s',%d,%d,%f,%d,%d,%d,%d,%d)", fs.mac[5],mac,fs.configured,fs.connected,fs.evtrate,fs.lostcpu,fs.lostfpga,fs.biason,fs.evtperpoll,fs.error);

//if (mysql_query(con, query)) {
//     printf("%s\n", mysql_error(con));
//  }

}

int main (int argc, char **argv)
{
   char *token;
   unsigned int dummy;
time_t t0,t1;
int dt,dt0;
int rv=0;
unsigned int mac[6];
if(argc<3 || argc>4) { usage(); return 0;}
int period=0;
if(argc==4) {period=atoi(argv[3]); printf("DB update period %d s.\n", period);}
void * context = zmq_ctx_new ();
//  Socket to talk to server
printf ("Connecting to febdrv...\n");
void *subscriber = zmq_socket (context, ZMQ_SUB);
rv=zmq_connect (subscriber, argv[1]); if(rv<0) {printf("Connection to %s failed!\n",argv[1]); return 0;}
//zmq_connect (subscriber, "ipc://stats");
zmq_setsockopt (subscriber, ZMQ_SUBSCRIBE, NULL, 0);




if((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1) die("socket");
memset((char*)&si_other,0,sizeof(si_other));
si_other.sin_family = AF_INET;
si_other.sin_port = htons(PORT);

if(inet_aton(argv[2], &si_other.sin_addr)==0) die("inet_aton()");
printf("UDP socket initialized.\n");
//printf("UDP c.\n");

time_t secs=time(NULL);

while(1)
{
zmq_msg_t reply;
zmq_msg_init (&reply);
t0=time(NULL);
dt=0; dt0=0;
int rv=0;
char str[256*100];
void *ptr=0;
memset(&ds, 0, sizeof(ds));
memset(&fs, 0, sizeof(fs));
while(zmq_msg_recv (&reply, subscriber, ZMQ_DONTWAIT)==-1)
 {
  dt=time(NULL)-t0; 
  if(dt>2 && dt!=dt0) {
                      printf("No reply from driver for %d seconds!\n",dt); 
                      dt0=dt;
                      ds.status=-1;
                      //printf("No reply from driver for %d seconds!\n",dt); 
		              updatedb_dstat();
  }
 };
//printf ("%s", (char*)zmq_msg_data (&reply));
//sprintf (str,"%s", (char*)zmq_msg_data (&reply));
if((time(NULL)-secs)>=period) //perform DB update only once a second
 { 
  secs=time(NULL);
  ptr=(void*)zmq_msg_data (&reply);
  memcpy(&ds,ptr, sizeof(ds)); ptr+=sizeof(ds);
  updatedb_dstat();
  for(int i=0; i<ds.nfebs; i++)
  {
   memcpy(&fs,ptr, sizeof(fs));ptr+=sizeof(fs);
   updatedb_febstat();
  }
 }
zmq_msg_close (&reply);


}
zmq_close (subscriber);
zmq_ctx_destroy (context);
close(s);
return 0;
}


