#include "sbndaq-artdaq/Generators/SBND/SPECTDC/SPECTDC_Interface.hh"
#define TRACE_NAME "TDCCard"
#include "artdaq/DAQdata/Globals.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "sbndaq-artdaq-core/Overlays/SBND/TDCTimestampFragment.hh"
#include "sbndaq-artdaq/Generators/SBND/SPECTDC/StringLiterals.hh"

// clang-format off
#include "fmctdc-lib/fmctdc-lib.h"
#include "fmctdc-lib/fmctdc-lib-private.h"
// clang-format on

#include <fstream>
#include <poll.h>

using namespace sbndaq::SPECTDCInterface;
namespace lit = sbndaq::literal;
namespace utls = sbndaq::SPECTDCInterface::TDCTimeUtils;
using artdaq::MetricMode;
using sbndaq::PoolBuffer;
using sbndaq::TDCTimestamp;

namespace sbndaq::detail {
pollfd g_pollfd[FMCTDC_NUM_CHANNELS];
}
using sbndaq::detail::g_pollfd;
//
// TDCCard
//
TDCCard::TDCCard(fhicl::ParameterSet const& ps, PoolBufferUPtr_t& b)
    : calibration_data{ps.get<std::string>(lit::calibration_data, lit::nofile)},
      timesync_source{ps.get<std::string>(lit::timesync_source, lit::timesync_host) == lit::timesync_wr
                          ? timesync_source_t::wr
                          : timesync_source_t::host},
      polltime_ms{ps.get<decltype(polltime_ms)>(lit::polltime_ms, 50)},
      blocking_reads{ps.get<bool>(lit::blocking_reads, false)} {
  if (ps.has_key(lit::monitor)) {
    auto ps1 = ps.get<fhicl::ParameterSet>(lit::monitor, fhicl::ParameterSet());
    monitor_timestamps = ps1.get<bool>(lit::timestamps, false);
    monitor_temperature = ps1.get<bool>(lit::temperature, false);
  }
  if (ps.has_key(lit::channels) && ps.is_key_to_sequence(lit::channels)) {
    for (int c = 0; c < FMCTDC_NUM_CHANNELS; c++) {
      std::stringstream ss;
      ss << lit::channels << "[" << c << "]";
      if (!ps.has_key(ss.str())) break;
      auto ps1 = ps.get<fhicl::ParameterSet>(ss.str());
      chans.emplace_back(ps1, b, *this);
    }
  }
  if (calibration_data != lit::nofile) {
    std::ifstream calibration(calibration_data);
    if (!calibration.is_open()) {
      TLOG(TLVL_ERROR) << "Missing TDC calibration file \"" << calibration_data << "\".";
      calibration_data = lit::nofile;
    }
  }
  if (timesync_source == timesync_source_t::wr) {
    if (ps.has_key(lit::time_adjustment) && ps.is_key_to_table(lit::time_adjustment)) {
      auto ps1 = ps.get<fhicl::ParameterSet>(lit::time_adjustment);
      auto tmp = std::make_unique<TAI2UTCAdjustment>(ps1);
      std::swap(tai2utc, tmp);
    }
  }
}

TDCCard::~TDCCard() { close(); }

bool TDCCard::open() {
  TLOG(TLVL_DEBUG + 1) << "Opening TDC device id=0x" << std::hex << deviceid << ".";
  tdcdevice = fmctdc_open(-1, deviceid);
  if (!tdcdevice) {
    TLOG(TLVL_ERROR) << "Cannot open TDC device id=" << deviceid
                     << ", fmctdc_open returned error: " << fmctdc_strerror(errno) << ".";
    return false;
  }

  TLOG(TLVL_DEBUG + 1) << "Opened TDC device id=0x" << std::hex << deviceid << ", handle=" << std::hex << tdcdevice
                       << ".";

  for (auto& chan : chans)
    if (!chan.open()) return false;

  int i = 0;
  for (auto& chan : chans) {
    g_pollfd[i].fd = chan.fd;
    g_pollfd[i].events = POLLIN | POLLERR;
    ++i;
  }

  return true;
}

void TDCCard::close() {
  if (!tdcdevice) return;
  TLOG(TLVL_DEBUG + 2) << "Closing TDC device id=0x" << std::hex << deviceid << ", handle=" << std::hex << tdcdevice
                       << ".";
  stop();
  for (auto& chan : chans) chan.close();

  auto err = fmctdc_close(tdcdevice);
  if (err) {
    TLOG(TLVL_ERROR) << "Cannot close TDC device id=" << deviceid
                     << ", fmctdc_close returned error: " << fmctdc_strerror(errno) << ".";
  }

  TLOG(TLVL_DEBUG + 2) << "Closed TDC device id=0x" << std::hex << deviceid << ", handle=" << std::hex << tdcdevice
                       << ".";

  tdcdevice = nullptr;
}

bool TDCCard::configure() {
  TLOG(TLVL_DEBUG + 3) << "Configuring TDC device id=" << deviceid;
  if (timesync_source == timesync_source_t::wr) {
    TLOG(TLVL_INFO) << "Enabling the WhiteRabbit timing system on a TDC device.";
    auto err = fmctdc_wr_mode(tdcdevice, 1);
    if (err == ENOTSUP) {
      TLOG(TLVL_INFO) << "Gateware has no support for WhiteRabbit, try reloading SPEC/TDC card drivers.";
    }
    int exitcode = 10;
    for (; exitcode > 0; --exitcode) {
      utls::thread_sleep(utls::onesecond);
      auto status = fmctdc_check_wr_mode(tdcdevice);

      switch (status) {
        case ENODEV:
          TLOG(TLVL_INFO) << "WhiteRabbit Status: disabled.";
          exitcode = -1;
          break;
        case ENOLINK:
          TLOG(TLVL_INFO) << "WhiteRabbit Status: link down.";
          break;
        case EAGAIN:
          TLOG(TLVL_INFO) << "WhiteRabbit Status: synchronization in progress.";
          break;
        case 0:
          TLOG(TLVL_INFO) << "WhiteRabbit Status: synchronized.";
          exitcode = -9;
          break;
        case -1:
          TLOG(TLVL_INFO) << "WhiteRabbit Status: link down or synchronization in progress.";
          break;
        default:
          TLOG(TLVL_ERROR) << "Cannot enable WhiteRabbit timing system on a TDC device id=" << deviceid
                           << ", fmctdc_check_wr_mode returned error: " << fmctdc_strerror(errno) << ".";
          return false;
      }
    }

    if (exitcode != -10) return false;
  } else {
    TLOG(TLVL_INFO) << "Disabling the WhiteRabbit timing system on a TDC device.";
    fmctdc_wr_mode(tdcdevice, 0);
    utls::thread_sleep(utls::onesecond);
    auto err = fmctdc_set_host_time(tdcdevice);
    if (err) {
      TLOG(TLVL_ERROR) << "Cannot set the TDC base-time according to the host time on device id=" << deviceid
                       << ", fmctdc_set_host_time returned error: " << fmctdc_strerror(errno) << ".";
      return false;
    }
    TLOG(TLVL_INFO) << "Updated the TDC base-time according to the host time.";
  }
  reportTimeTemp();

  for (auto& chan : chans)
    if (!chan.configure()) return false;

  TLOG(TLVL_DEBUG + 3) << "Configured TDC device id=" << deviceid;
  return true;
}

bool TDCCard::start() {
  if (!tdcdevice) return false;

  TLOG(TLVL_DEBUG + 4) << "Starting TDC device id=0x" << std::hex << deviceid << ".";

  uint8_t error_count = 0;
  for (auto& chan : chans)
    if (!chan.start()) ++error_count;

  if (metricMan) {
    metricMan->sendMetric(lit::monitor_wr_synced, uint8_t{0}, lit::unit_bool, 1, MetricMode::LastPoint);
    metricMan->sendMetric(lit::monitor_temperature, float{0}, lit::unit_temperature, 1, MetricMode::Average);
  }
  TLOG(TLVL_DEBUG + 4) << "Started TDC device id=0x" << std::hex << deviceid << ".";
  return 0 == error_count;
}

bool TDCCard::stop() {
  if (!tdcdevice) return false;

  TLOG(TLVL_DEBUG + 5) << "Stopping TDC device id=0x" << std::hex << deviceid << ".";

  uint8_t error_count = 0;
  for (auto& chan : chans)
    if (!chan.stop()) ++error_count;

  TLOG(TLVL_DEBUG + 5) << "Stopped TDC device id=0x" << std::hex << deviceid << ", error_count=" << error_count << ".";
  return 0 == error_count;
}

void TDCCard::reportTimeTemp() {
  fmctdc_time ts;
  auto err = fmctdc_get_time(tdcdevice, &ts);
  if (err) {
    TLOG(TLVL_ERROR) << "Cannot get the base-time of a TDC device id=" << deviceid
                     << ", fmctdc_get_time returned error: " << fmctdc_strerror(errno) << ".";
    return;
  }

  auto wr_seconds = tai2utc ? tai2utc->adjust(ts.seconds) : ts.seconds;
  TLOG(TLVL_INFO) << "Current TDC time is " << (unsigned long long)ts.seconds << ", or "
                  << utls::to_localtime(wr_seconds);

  int lvl;
  std::string msg;
  std::tie(lvl, msg) = utls::wr_time_summary(wr_seconds);
  TLOG(lvl) << msg;
  auto temperature = fmctdc_read_temperature(tdcdevice);
  TLOG(TLVL_INFO) << "Current TDC temperature is " << temperature << " deg C.";

  auto status = fmctdc_check_wr_mode(tdcdevice);
  switch (status) {
    case ENODEV:
      TLOG(TLVL_INFO) << "WhiteRabbit Status: disabled.";
      break;
    case ENOLINK:
      TLOG(TLVL_INFO) << "WhiteRabbit Status: link down.";
      break;
    case EAGAIN:
      TLOG(TLVL_INFO) << "WhiteRabbit Status: synchronization in progress.";
      break;
    case -1:
      TLOG(TLVL_INFO) << "WhiteRabbit Status: disabled.";
      break;
    case 0:
      TLOG(TLVL_INFO) << "WhiteRabbit Status: synchronized.";
      break;
    default:
      TLOG(TLVL_ERROR) << "Cannot check WhiteRabbit status for TDC device id=" << deviceid
                       << ", fmctdc_check_wr_mode returned error: " << fmctdc_strerror(errno) << ".";
  }

  if (metricMan) {
    metricMan->sendMetric(lit::monitor_wr_synced, uint8_t{status == 0}, lit::unit_bool, 1, MetricMode::LastPoint);
    metricMan->sendMetric(lit::monitor_temperature, float{temperature}, lit::unit_temperature, 1, MetricMode::Average);
  }
}

void TDCCard::read() {
  TLOG(TLVL_DEBUG + 8) << "Reading (begin) TDC device id=0x" << std::hex << deviceid << ".";
  if (poll(g_pollfd, FMCTDC_NUM_CHANNELS, polltime_ms) < 1) return;
  uint8_t i = 0;
  for (auto& chan : chans) {
    if (g_pollfd[i].revents & POLLIN) chan.read();
    ++i;
  }
  TLOG(TLVL_DEBUG + 8) << "Reading (end) TDC device id=0x" << std::hex << deviceid << ".";
}

void TDCCard::monitor() {
  if (!tdcdevice) return;
  TLOG(TLVL_DEBUG + 9) << "Monitoring(begin) TDC device id=0x" << std::hex << deviceid << ".";
  reportTimeTemp();
  for (auto& chan : chans) chan.monitor();
  TLOG(TLVL_DEBUG + 9) << "Monitoring(end) TDC device id=0x" << std::hex << deviceid << ".";
}
