/**
 *
 * \brief CRT Hit Info
 *
 *
 */

#ifndef AFRAME_Hit_H
#define AFRAME_Hit_H

#include <cstdint>
#include <vector>
#include <map>
#include <string>
#include <utility>

namespace sbndaq {

    struct Hit{
      
      int       feb_h;
      int	feb_v;

      uint64_t	time; // average time
      uint64_t	time_h;
      uint64_t	time_v;

      int adcA_h; int adcB_h;
      int adcA_v; int adcB_v;

      //uint64_t       ts0_s; ///< Second-only part of timestamp T0.
      //double    ts0_s_corr; ///< [Honestly, not sure at this point, it was there since long time (BB)]
      //double        ts0_ns; ///< Timestamp T0 (from White Rabbit), in UTC absolute time scale in nanoseconds from the Epoch.
      //double   ts0_ns_corr; ///< [Honestly, not sure at this point, it was there since long time (BB)]

      //double        ts1_ns; ///< Timestamp T1 ([signal time w.r.t. Trigger time]), 

      float	x_pos; ///< position in x-direction (cm).
      float     x_err; ///< position uncertainty in x-direction (cm).
      float     y_pos; ///< position in y-direction (cm).
      float     y_err; ///< position uncertainty in y-direction (cm).
      float     z_pos; ///< position in z-direction (cm).
      float     z_err; ///< position uncertainty in z-direction (cm).

      // Add the ChannelMap positional info as well. These are the values that are well constrained by a strip width
      float	x_vert;
      float	y_horiz;     
      float	z_horiz;     

      Hit() {}

      //int64_t ts0() const { return static_cast<int64_t>(ts0_s) * 1'000'000'000LL + static_cast<int64_t>(ts0_ns); }
      // T1 is a relative time not a timestamp, so we don't need second-part.
      // nano-second part is enough and we saved entire time there.
      //int64_t ts1() const { return static_cast<int64_t>(ts1_ns); }

    };

} // end namespace sbndaq

#endif
