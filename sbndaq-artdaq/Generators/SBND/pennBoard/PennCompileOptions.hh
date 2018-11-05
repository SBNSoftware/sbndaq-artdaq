#ifndef PENNCOMPILEOPTIONS_HH_
#define PENNCOMPILEOPTIONS_HH_

// JCF, Jul-29-2015

// As of this writing, the Penn boardreader code is only designed to
// support REBLOCK_PENN_USLICE being defined (i.e.,
// PENN_DONT_REBLOCK_USLICES being undefined)


//are we using the Penn emulator?
/// when off, force the sequence id repeat check to be off (safety check)
/// when on, send a second xml config file with emulator options
//#define PENN_EMULATOR

//does the millislice contain:
//  N complete uslices
//or
//  reblocked in a set time-width
#ifndef PENN_DONT_REBLOCK_USLICES
//// WARNING: OFF not been tested for ages
#define REBLOCK_PENN_USLICE
#endif
/// note having REBLOCK_PENN_USLICE turned OFF is not fully implemented including:
///   overlap region
///   millislice header timing information
///   

//  1) the 4 bytes header
//  2) the rest of the uslice in one go
// Developer mode...with extra verbosity
//#define __PTB_BOARD_READER_DEVEL_MODE__
//#define __PTB_DEBUG__  


#endif //PENNCOMPILEOPTIONS_HH_
