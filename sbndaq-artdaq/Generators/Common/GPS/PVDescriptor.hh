//
//
//

#ifndef H_PVDESCRIPTOR
#define H_PVDESCRIPTOR

#include <cadef.h>
#include <string.h>

class PVDescriptor
{
public:
  PVDescriptor(std::string name_,
	       int dataType_,
	       chid channel_)
  {
    name = name_;
    dataType = dataType_;
    channel  = channel_;
  };

  std::string name;
  int dataType;
  chid channel;
};

#endif

