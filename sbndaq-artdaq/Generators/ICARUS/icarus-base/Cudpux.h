#ifndef _Cudp
#define _Cudp_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>

class Cudp
{
protected:

    int port;
    int sock;

    int connected;
    struct sockaddr_in myaddr;

    unsigned int sockAddrSize;

public:

    struct sockaddr_in clientAddr;

    //Cudp();
    Cudp ( int port );
    Cudp ( int sock,int opt );
    ~Cudp();

    void Connect ( char *hostaddr,int port );
    void Disconnect();

    int Answer ( char * buffer,int size );
    int SendTo ( char * host, int port,char * buffer,int size );
    int Receive ( char *buffer,int size );
    int Receive ( char *buffer,int size,int timeout );

};

#endif
