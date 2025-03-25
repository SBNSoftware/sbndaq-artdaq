
#ifndef _structures_
#define _structures_

//static int nSamples ;
//static int preSamples;
//static int dataArraySize = nSamples+32;
// See the Acqiris programmer's reference: size of the array needed by each board depends on the acquisition mode.

typedef struct {
    double sampInterval;
    double delayTime;
    int nbrSegments;
    int coupling;
    int bandwidth;
    double fullScale;
    double offset;
    int offsetadc;
    int thresh;
    double coff[16];
    double cthre[16];
} BoardConf;

typedef struct {
    int trigClass;
    int sourcePattern;
    int trigCoupling;

    int channel;
    int trigSlope;
    double trigLevel1;
    double trigLevel2;
    int nsamples;
    int presamples;
} TrigConf;

typedef struct {
    struct Header {
        int token;
        int info1;
        int info2;
        int info3;
        unsigned int timeinfo;
        int chID;
        int packSize;
    } Header;
    char data[1];
} DataTile;

#endif
