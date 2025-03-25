#include "VetoUDP.h"

int main(void)
{
    VetoUDP myudp("137.138.194.204",7654);

    while (1)
        myudp.TestRun(1e7);

    return 0;
}

