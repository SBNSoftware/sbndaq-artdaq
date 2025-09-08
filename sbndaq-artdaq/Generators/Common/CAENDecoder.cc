//
//  CAENDecoder class converts CAENDigitizer enumerated 
//  codes into human readable text   (W.Badgett)
//

#define TRACE_NAME "CAENDecoder"
#include "artdaq/DAQdata/Globals.hh"

#include "CAENDecoder.hh"
#include "CAENException.hh"

#include <iostream>
#include <sstream>
#include <ctime>


void sbndaq::CAENDecoder::commError(CAENComm_ErrorCode retcod, const std::string label, const int fragID)
{
  if ( retcod != CAENComm_Success)
  {
    char buffer[256];
    CAENComm_DecodeError(retcod,buffer);
    TLOG_ERROR("CAENDecoder") << "(fragID=" << fragID << ") " << label 
			      << " error: " << buffer << TLOG_ENDL;
  }
}

void sbndaq::CAENDecoder::vmeError(CVErrorCodes retcod, const std::string label, const int fragID)
{
  if ( retcod != cvSuccess )
  {
    char buffer[256];
    CAENComm_DecodeError(retcod,buffer);
    TLOG_ERROR("CAENDecoder") << "(fragID=" << fragID << ") " << label 
			      << " error: " << buffer << TLOG_ENDL;
  }
}

void sbndaq::CAENDecoder::checkError(CAEN_DGTZ_ErrorCode err, const std::string label, const int fragID)
{
  if ( err != CAEN_DGTZ_Success )
  {
    std::stringstream text;
    sbndaq::CAENException e(err, label, fragID);
    TLOG_ERROR("CAENDecoder") << e.what() << " [" << err << "]" << TLOG_ENDL;
    throw(e);
  }
}

