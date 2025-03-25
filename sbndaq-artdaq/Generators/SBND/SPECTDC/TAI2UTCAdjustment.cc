#include "sbndaq-artdaq/Generators/SBND/SPECTDC/SPECTDC_Interface.hh"
#define TRACE_NAME "TAI2UTCAdjustment"
#include "fhiclcpp/ParameterSet.h"
#include "sbndaq-artdaq/Generators/SBND/SPECTDC/StringLiterals.hh"

using namespace sbndaq::SPECTDCInterface;
namespace lit = sbndaq::literal;
//
// TAI2UTCAdjustment
//
TAI2UTCAdjustment::TAI2UTCAdjustment(fhicl::ParameterSet const& ps)
    : add_leap_seconds{ps.get<decltype(add_leap_seconds)>(lit::add_leap_seconds)},
      next_leap_second_tai{ps.get<decltype(next_leap_second_tai)>(lit::next_leap_second_tai)},
      make_adjustment{ps.get<bool>(lit::make_adjustment, true)} {}

uint64_t TAI2UTCAdjustment::adjust(uint64_t sec) const {
  uint64_t epoch = sec;
  if (!make_adjustment || (add_leap_seconds == 0 && sec < next_leap_second_tai)) return epoch;
  epoch -= add_leap_seconds;
  if (sec >= next_leap_second_tai) --epoch;
  return epoch;
}

uint8_t TAI2UTCAdjustment::count_leap_seconds(uint64_t sec) const {
  uint8_t result = 0;
  if (!make_adjustment || (add_leap_seconds == 0 && sec < next_leap_second_tai)) return result;
  result = add_leap_seconds;
  if (sec >= next_leap_second_tai) ++result;
  return result;
}
