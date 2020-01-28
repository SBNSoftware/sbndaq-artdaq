#include "Cudpux.h"
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "common.h"

#ifdef __OLDAPPLE__
typedef int socklen_t;
#endif
/*
Cudp::Cudp()
{
   Cudp(0);
}
*/
Cudp::Cudp(int localport)
{
//  struct sockaddr_in myaddr;
 
    connected=false;

    bzero ((char *) &myaddr, sizeof(myaddr));

   sock = socket (AF_INET, SOCK_DGRAM, 0);

   if (sock < 0)
      throw;

    myaddr.sin_family       = AF_INET;
    myaddr.sin_port         = htons (localport);
    //myaddr.sin_port         = localport;
 
    int blen = 65536;

    if(setsockopt(sock,SOL_SOCKET,SO_SNDBUF,(char *)&blen,sizeof(blen))<0)
      throw;
    if(setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,(char *)&blen,sizeof(blen))<0)
      throw;

    port = localport;

//    if (port)
    int retval = bind(sock,(struct sockaddr *)&myaddr,sizeof(myaddr));
    if(retval<0)
        { 
	  TRACEN("Cudpx.cc",TLVL_ERROR, "bind failed");

          throw;
        }

	  TRACEN("Cudpx.cc",TLVL_INFO, "Bound socket %d (%d)", myaddr.sin_port,ntohs(myaddr.sin_port));
        
    
     sockAddrSize = sizeof (struct sockaddr_in);

    TRACEN("Cudpx.cc",TLVL_ERROR, "Sock addr size %d", sockAddrSize);

}

Cudp::Cudp(int snew, int)
{

   connected = true;
   port =0;

   sockAddrSize = sizeof (struct sockaddr_in);            
   sock = snew;
}

Cudp::~Cudp()
{
  close (sock);
}


void 
Cudp::Connect(char *host,int toport)
{
    clientAddr.sin_family      = AF_INET;
    clientAddr.sin_addr.s_addr = inet_addr (host);
    clientAddr.sin_port        = htons (toport);
    //clientAddr.sin_port        = toport;
  
 if (connect (sock, (struct sockaddr  *)&clientAddr, sizeof (clientAddr)) < 0)
{
       	  TRACEN("Cudpx.cc",TLVL_ERROR, "Connect failed");
  throw;
}
  connected = true;
}

int
Cudp::Receive(char *buffer,int size,int timeout)
{
   // printf ("waiting message %d\n",size);
         fd_set rfds;
		struct timeval tv;
  
    FD_ZERO(&rfds);
    FD_SET(sock,&rfds);

    tv.tv_sec = timeout;
    tv.tv_usec = 0;

     int retva;
	  int nfd=sock;
	  nfd++;
     if (timeout==0)
       retva = select (nfd,&rfds,0,&rfds,NULL);
     else
       retva = select (nfd,&rfds,0,&rfds,&tv);

       	  TRACEN("Cudpx.cc",TLVL_ERROR, "select return: %d isset %d",retva,FD_ISSET(sock,&rfds));
     
     if (retva)
     {
             return  recvfrom (sock, buffer,size,0,
                 (struct sockaddr *) &clientAddr,(socklen_t *)&sockAddrSize) ;
     }
     else return 0;
}

int
Cudp::Receive(char *buffer,int size)
{
//    return  read (sock, buffer,size) ;
//    printf ("waiting message %d\n",size);

     int mres=  recvfrom (sock, buffer,size,0,
                 (struct sockaddr *) &clientAddr,(socklen_t  *)&sockAddrSize) ;
     if (mres<0) throw(errno);
     else return mres;

/* 
    int howmany = 0;
    int toberead = size;
    do
    {
      //int readnow = recv (sock, &buffer[howmany], toberead,MSG_WAITALL) ;
      int readnow = recvfrom (sock, &buffer[howmany], toberead,0,
           (struct sockaddr *) &clientAddr,&sockAddrSize) ;
      if (readnow < 0 ) {printf("some rrorrs...%d\n",errno);throw(errno);}
      else { howmany+=readnow; toberead-=readnow;}
    } while (toberead>0);
    return howmany;
*/
}

int
Cudp::Answer( char *buffer,int size)
{
  //char *inetAddr;

  //inetAddr = inet_ntoa(clientAddr.sin_addr);
 
TRACEN("Cudpx.cc",TLVL_ERROR, "Trying to answer message size %d",size);

//   if (size > 1024) size = 1024;

//  printf ("MESSAGE FROM: Internet Address %s, port %d\n%s\n",
//                inetAddr, ntohs (clientAddr.sin_port));

    return  sendto (sock, buffer, size,0,
            (struct sockaddr *) &clientAddr,sockAddrSize) ;
}

int
Cudp::SendTo(char *host, int toport, char *buffer,int size)
{
    clientAddr.sin_family      = AF_INET;
    clientAddr.sin_addr.s_addr = inet_addr (host);
    clientAddr.sin_port        = htons (toport);
    //clientAddr.sin_port        = toport;

    return  sendto (sock, buffer, size,0,
            (struct sockaddr *) &clientAddr,sockAddrSize) ;
}
