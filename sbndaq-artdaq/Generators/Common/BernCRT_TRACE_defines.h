#include "trace.h"

/*****
 * TRACE levels used in the code
 * 
 * 0 - ERROR (likely fatal) conditions
 * 1 - WARNING (likely non-fatal but still bad) conditions
 * 2 - LOGGING (general logging/messaging)
 * 3 - DEBUG (general debugging)
 * 4 - GetData LOG

 *****/

#define TR_ERROR 0
#define TR_WARNING 1
#define TR_LOG 2
#define TR_DEBUG 3


#define TR_GD_LOG 4
#define TR_GD_DEBUG 5

#define TR_FF_LOG 8
#define TR_FF_DEBUG 9
