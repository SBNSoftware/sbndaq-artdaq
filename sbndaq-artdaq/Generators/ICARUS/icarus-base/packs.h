//Packet format t600 run
//version 0.0   May 2 2001
//
// ****** all contents are big endian *****
#ifndef _packs_
#define _packs_

typedef struct {
    int token;
    int Run;
    int Event;
    int ToD;
    int AbsTime;
    int Conf;
    int Size;
} evHead;

typedef struct {
    int mclass;
    int status;
    char hurdstatus[80];
    int param[42];
} udpBrd;

typedef struct {
    int token;
    int memstat1;
    int memstat2;
    int abstime;
    int mintime;
    int crateid;
    int size;
} statpack;

#endif
