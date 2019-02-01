//
//   GPSReceiver.cc
//
//   Comms between server and SpectraTime GPS Receiver device
//   (W.Badgett)
//

#include <GPSReceiver.hh>
#include <iostream>
#include <string>
#include <fcntl.h>
#include <errno.h>

GPSReceiver::GPSReceiver(std::string ipAddress,
			 std::string port)
  : NetworkReceiver(ipAddress,port)
{
  int retcod;
  daqMQ = 0xCDF;
  dcsMQ = 0xAD0;

  daqID = msgget(daqMQ,IPC_CREAT|0666);
  if ( daqID < 0 )
  {
    printf("Error msgget %X [%s]\n", daqMQ,strerror(errno));
    exit(errno); 
  }

  dcsID = msgget(dcsMQ,IPC_CREAT|0666);
  if ( dcsID < 0 )
  {
    printf("Error msgget %X [%s]\n", dcsMQ,strerror(errno));
    exit(errno); 
  }

  write("bt0");
  usleep(5000);
  write("maw0b00");
  usleep(5000);
  write("maw0c00");
  usleep(5000);
  write("st");
  usleep(5000);
  write("MAW0BBA");
  usleep(5000);
  write("MAW0C01");
  sleep(1);
}

void GPSReceiver::processLine(char * line)
{
  bool complete = gpsInfo.processLine(line);
  if ( complete ) 
  {
    int retcod;
    gpsInfo.print();

    retcod = msgsnd(daqID,(const void *)&gpsInfo.gps,sizeof(gpsInfo.gps),
		    IPC_NOWAIT);
    if (( retcod < 0 ) && ( errno != EAGAIN))
    {
      printf("Error msgsnd 0x%X %d [%s]\n", daqMQ,daqID,strerror(errno));
    }

    retcod = msgsnd(dcsID,(const void *)&gpsInfo.gps,sizeof(gpsInfo.gps),
		    IPC_NOWAIT);
    if (( retcod < 0 ) && ( errno != EAGAIN))
    {
      printf("Error msgsnd 0x%X %d [%s]\n", dcsMQ,dcsID,strerror(errno));
    }
  }
}

int main(int argc, char* argv[])
{
  if ( argc < 3 ) 
  { 
    std::cout << "Expected two arguments:" << std::endl;
    std::cout << "  gps <IP address> <port>" << std::endl << std::endl;
    exit(-1);
  }
  GPSReceiver gps(argv[1],argv[2]);
  bool ok = true;
  while ( ok )
  {
    clock_gettime(CLOCK_REALTIME,&gps.gpsInfo.gps.systemTimer); // heart beat
    sleep(1);
    int32_t zeit = gps.gpsInfo.gps.timeStamp;
    if ( std::abs( zeit - gps.gpsInfo.gps.systemTimer.tv_sec ) > 10 )
    {
      std::cout << "Suspects comms hangup, exiting" << std::endl;
      ok = false;
    }
  }
  exit(0);
}


