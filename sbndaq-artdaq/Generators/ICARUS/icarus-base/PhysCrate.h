#ifndef _PhysCrate_
#define _PhysCrate_

#include <stdio.h>
#include <netinet/in.h>


#include "structures.h"
#include "A2795Board.h"

#include <vector>

class PhysCrate
{
private:
    A2795Board** boards;

    static const int headSize = 28;

    int nBoards;
    char * tilebuf;
    int presBoard;
    int mip;
    std::vector< int > BoardIDs;
public:
    PhysCrate();
    ~PhysCrate();

    void initialize( std::vector<int> busVec=std::vector<int>());
    void configure ( BoardConf conf );
    void configureTrig ( TrigConf conf );
    void waitData();
    void start();
    void ArmTrigger();
    bool dataAvail();
    DataTile* getData();
    int getId();

    int NBoards() const {
        return nBoards;
    }

    const std::vector< int >& getBoardIDs() const {
        return BoardIDs;
    }
    int BoardStatus ( int i );
    int BoardHandle ( int i );

    int BoardTemps( int i , uint8_t&, uint8_t&);
};

#endif
