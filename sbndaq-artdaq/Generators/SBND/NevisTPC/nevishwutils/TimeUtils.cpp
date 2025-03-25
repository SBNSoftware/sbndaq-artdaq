#include "TimeUtils.h"

namespace nevistpc {

  long diff_time_microseconds(struct timeval t2, struct timeval t1){
    return (t2.tv_sec*1e6 + t2.tv_usec - t1.tv_sec*1e6 - t1.tv_usec);
  }
  
  float diff_time_milliseconds(struct timeval t2, struct timeval t1){
    float micro = (float)(t2.tv_sec*1e6 + t2.tv_usec - t1.tv_sec*1e6 - t1.tv_usec);
    return micro/1000.;
  }

} // end of namespace nevistpc

