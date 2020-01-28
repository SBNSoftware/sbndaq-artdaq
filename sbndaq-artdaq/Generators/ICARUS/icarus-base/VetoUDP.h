#include <cstring>
#include <stdio.h>
#include <unistd.h>
#include "Cudpux.h"
#include <iostream>

class VetoUDP
{

public:
    VetoUDP ( const char*,int );
    int TestRun ( unsigned int );
    int VetoOn();
    int VetoOff();

private:

    int port;

    Cudp myudp;

    int totp;

    char mhost[128];
    char buf[3];

};
