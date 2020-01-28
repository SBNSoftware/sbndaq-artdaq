#ifndef _ICARUSDAQ_BASE_COMMON_H_
#define _ICARUSDAQ_BASE_COMMON_H_

#include "trace.h"
#define TLVL_ERROR        0
#define TLVL_WARNING      1
#define TLVL_INFO         2
#define TLVL_DEBUG        3
#define TLVL_TRACE        4

//#define TRACEN(...)

#ifdef MUST_USE_TRACE
#include <sstream>
using std::stringstream;
#undef printf
#define printf(...) \
    static_assert(false,"Using printf\'s is not recommended, use TRACE macros instead.")

#define cout stringstream ss;\
    static_assert(false,"Using std::cout\'s is not recommended, use TRACE macros instead.");\
    ss

#define cerr stringstream ss;\
    static_assert(false,"Using std::cerr\'s is not recommended, use TRACE macros instead.");\
    ss

#endif // MUST_USE_TRACE

#endif /* _ICARUSDAQ_BASE_COMMON_H_ */
