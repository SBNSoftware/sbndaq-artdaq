//
//  CAENException.cc   (W.Badgett)
//

#include <iostream>
#include <sstream>
#include "CAENException.hh"

sbndaq::CAENException::CAENException(CAEN_DGTZ_ErrorCode error_, 
				      std::string label_,
				      int fragID_):
  error(error_),
  label(label_),
  fragID(fragID_)
{
}

std::string sbndaq::CAENException::what()
{ 
  std::stringstream hello;
  hello << *this;
  return(hello.str());
}

void sbndaq::CAENException::print(std::ostream & os) 
{ 
  os << *this;
}

std::ostream& operator<<(std::ostream& s, const sbndaq::CAENException& e)
{
  if ( e.fragID >= 0 ) { s << "(fragID=" << e.fragID << ") ";}
  s << "ERROR: " << e.label << " " << sbndaq::CAENDecoder::CAENError(e.error);
  s << std::endl;
  return(s);
}
