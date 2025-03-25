#define TRACE_NAME "StringUtils"
#include <cstdlib>
#include <iostream>
#include <numeric>
#include "sbndaq-artdaq/Generators/SBND/SPECTDC/SPECTDC_Interface.hh"
#include "sbndaq-artdaq/Generators/SBND/SPECTDC/StringLiterals.hh"

using namespace sbndaq::SPECTDCInterface;
namespace lit = sbndaq::literal;

std::string TDCChan::as_fhicl() const {
  std::stringstream os;
  os << "{";
  os << " " << lit::id << ":" << int{id};
  os << " " << lit::name << ":";
  std::copy(name.begin(), name.end(), std::ostream_iterator<char>(os, ""));
  os << std::string(SPECTDCInterface::TDCChan::name_length -
                        std::accumulate(name.begin(), name.end(), 0,
                                        [](long count, char c) { return isalpha(c) ? count + 1 : count; }),
                    ' ');
  os << " " << lit::enabled << ":" << std::boolalpha << enabled;
  os << " " << lit::terminated << ":" << std::boolalpha << terminated;
  os << " " << lit::time_offset_ps << ":" << time_offset_ps;
  os << " " << lit::buffer_mode << ":"
     << (buffer_mode == chan_buffer_mode_t::fifo ? lit::buffer_mode_fifo : lit::buffer_mode_circular);
  os << " " << lit::buffer_length << ":" << buffer_length;

  os << " }";
  return os.str();
}

std::string TDCCard::as_fhicl() const {
  std::stringstream os;
  os << " {";
  os << "\n  " << lit::timesync_source << ":"
     << (timesync_source == SPECTDCInterface::timesync_source_t::wr ? lit::timesync_wr : lit::timesync_host);
  if (tai2utc) {
    os << "\n  " << lit::time_adjustment << ":" << tai2utc->as_fhicl();
  }
  os << "\n  " << lit::calibration_data << ":" << calibration_data;
  os << "\n  " << lit::monitor << ":{";
  os << "\n   " << lit::temperature << ":" << std::boolalpha << monitor_temperature;
  os << "\n   " << lit::timestamps << ": " << std::boolalpha << monitor_timestamps;
  os << "\n  },";
  os << "\n  " << lit::channels << ":[";
  for (auto& ch : chans) os << "\n   " << ch.as_fhicl() << ",";
  os << "\n  ]";
  os << "\n }";
  return os.str();
}

std::string SPECCard::as_fhicl() const {
  std::stringstream os;
  os << "{";
  os << "\n " << lit::deviceid << ":" << deviceid;
  os << "\n " << lit::fmctdc << ":" << fmctdc.as_fhicl();
  os << "\n}\n";
  return os.str();
}

std::string TAI2UTCAdjustment::as_fhicl() const {
  std::stringstream os;
  os << "{";
  os << " " << lit::make_adjustment << ":" << std::boolalpha << make_adjustment;
  os << " " << lit::add_leap_seconds << ":" << int{add_leap_seconds};
  os << " " << lit::next_leap_second_tai << ":" << next_leap_second_tai;
  os << " }";
  return os.str();
}

TDCChan::name_t TDCChan::to_array(std::string v) {
  auto rv = name_t();
  std::copy(v.begin(), v.end(), rv.data());
  if (v.length() > SPECTDCInterface::TDCChan::name_length) {
    TLOG(TLVL_ERROR) << "Truncated TDC channel name \"" << v << "\" to \"" << std::string(std::begin(rv), std::end(rv))
                     << "\".";
  }
  return rv;
}

std::string as_hexstring(std::string const& s) {
  if (s.find_first_not_of("0123456789") != std::string::npos) return lit::autodetect;
  std::stringstream os;
  os << "0x" << std::hex << stoi(s);
  return os.str();
}
