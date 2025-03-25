#ifndef _TIMEUTILS_H
#define _TIMEUTILS_H 1

#include "sys/time.h"

namespace nevistpc {
  
  long diff_time_microseconds(struct timeval, struct timeval);
  float diff_time_milliseconds(struct timeval, struct timeval);

} // end of namespace nevistpc

#endif //_TIMEUTILS_H

