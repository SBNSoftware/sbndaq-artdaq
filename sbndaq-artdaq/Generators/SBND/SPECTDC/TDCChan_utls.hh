#ifndef sbndaq_artdaq_Generators_SPECTDC_TDCChan_utils_hh
#define sbndaq_artdaq_Generators_SPECTDC_TDCChan_utils_hh
#include "sbndaq-artdaq/Generators/SBND/SPECTDC/SPECTDC_Interface.hh"

using name_t = sbndaq::TDCTimestamp::name_t;
template <typename T>
inline sbndaq::TDCTimestamp make_timestamp(uint8_t id, name_t const& name, T const& time) {
  sbndaq::TDCTimestamp ts;
  ts.vals.seconds = time.seconds;
  ts.vals.coarse = time.coarse;
  ts.vals.frac = time.frac;
  ts.vals.tai2epoch_offset = 0;
  ts.vals.name = name;
  ts.vals.channel = id;
  return ts;
}

inline sbndaq::TDCTimestamp adjust_tai2utc(sbndaq::TDCTimestamp& ts,
                                           std::unique_ptr<sbndaq::SPECTDCInterface::TAI2UTCAdjustment> const& adj) {
  auto leap_seconds = adj->count_leap_seconds(ts.vals.seconds);
  if (leap_seconds) {
    ts.vals.tai2epoch_offset = leap_seconds;
    ts.vals.seconds -= leap_seconds;
  }
  return ts;
}

inline std::string as_string(sbndaq::TDCTimestamp const& ts) {
  std::stringstream os;
  os << ts;
  return os.str();
}

#endif  // sbndaq_artdaq_Generators_SPECTDC_TDCChan_utils_hh¬
