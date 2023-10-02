#include "sbndaq-artdaq/Generators/SBND/SPECTDC/SPECTDC_Interface.hh"
#define TRACE_NAME "TDCChan"
#include "artdaq/DAQdata/Globals.hh"
#include "fhiclcpp/ParameterSet.h"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "sbndaq-artdaq-core/Overlays/SBND/TDCTimestampFragment.hh"
#include "sbndaq-artdaq/Generators/Common/PoolBuffer.hh"
#include "sbndaq-artdaq/Generators/SBND/SPECTDC/StringLiterals.hh"

// clang-format off
#include "fmctdc-lib/fmctdc-lib.h"
#include "fmctdc-lib/fmctdc-lib-private.h"
// clang-format on

using namespace sbndaq::SPECTDCInterface;
namespace lit = sbndaq::literal;
namespace utls = sbndaq::SPECTDCInterface::TDCTimeUtils;
using artdaq::MetricMode;
using sbndaq::PoolBuffer;
using sbndaq::TDCTimestamp;
struct fmctdc_time;
//
// TDCChan
//
TDCChan::TDCChan(fhicl::ParameterSet const& ps, PoolBufferUPtr_t& b, TDCCard const& tdc)
    : id{ps.get<decltype(id)>(lit::id)},
      name{to_array(ps.get<std::string>(lit::name))},
      enabled{ps.get<bool>(lit::enabled, false)},
      terminated{ps.get<bool>(lit::terminated, false)},
      monitor_only{ps.get<bool>(lit::monitor_only, false)},
      time_offset_ps{ps.get<decltype(time_offset_ps)>(lit::time_offset_ps, 0)},
      buffer_length{ps.get<decltype(buffer_length)>(lit::buffer_length, 64)},
      buffer(b),
      fmctdc_buffer{new uint8_t[fmctdc_buffer_size * sizeof(fmctdc_time)]},
      fmctdc{tdc} {
  // configuration tasks
  configtasks.emplace_back(&TDCChan::configure_channel_disable);
  configtasks.emplace_back(&TDCChan::configure_flush_buffer);
  configtasks.emplace_back(&TDCChan::configure_buffer_length);
  configtasks.emplace_back(&TDCChan::configure_buffer_mode);
  configtasks.emplace_back(&TDCChan::configure_termination);
  configtasks.emplace_back(&TDCChan::configure_timeoffset);

  // start tasks
  starttasks.emplace_back(&TDCChan::configure_flush_buffer);
  starttasks.emplace_back(&TDCChan::configure_channel_status);
  starttasks.emplace_back(&TDCChan::configure_drain_buffer);

  // stop tasks
  stoptasks.emplace_back(&TDCChan::configure_channel_disable);
  stoptasks.emplace_back(&TDCChan::configure_flush_buffer);
  // metric_prefix = std::string(std::begin(name), std::end(name));
  metric_prefix = std::string("TDC Chan").append(std::to_string(int{id})).append(" ");
}

bool TDCChan::open() {
  TLOG(TLVL_DEBUG_1) << "Opening channel=" << int{id} << ".";
  auto err = fmctdc_channel_disable(fmctdc.tdcdevice, id);
  if (err) {
    TLOG(TLVL_ERROR) << "Cannot disable channel=" << int{id} << ".";
    return false;
  }
  TLOG(TLVL_DEBUG_1) << "Opened channel=" << int{id} << ".";
  fd = fmctdc_fileno_channel(fmctdc.tdcdevice, id);
  return true;
}

void TDCChan::close() {
  TLOG(TLVL_DEBUG_2) << "Closing channel=" << int{id} << ".";
  auto err = fmctdc_channel_disable(fmctdc.tdcdevice, id);
  if (err) {
    TLOG(TLVL_ERROR) << "Cannot disable channel=" << int{id} << ".";
  }

  err = fmctdc_flush(fmctdc.tdcdevice, id);
  if (err) {
    TLOG(TLVL_ERROR) << "Cannot flush data buffers for channel=" << int{id} << ".";
  }
  TLOG(TLVL_DEBUG_2) << "Closed channel=" << int{id} << ".";
}

bool TDCChan::configure() {
  TLOG(TLVL_DEBUG_3) << "Configuring channel=" << int{id} << ".";
  inhibit = true;
  for (auto task : configtasks)
    if (!(this->*task)()) return false;
  TLOG(TLVL_DEBUG_3) << "Configured channel=" << int{id} << ".";
  return true;
}

bool TDCChan::start() {
  TLOG(TLVL_DEBUG_4) << "Starting channel=" << int{id} << ".";
  if (metricMan) {
    metricMan->sendMetric(metric_prefix + lit::tdc_laggy_samples, uint64_t{0}, lit::unit_samples_per_second, 11,
                          MetricMode::Rate);
    metricMan->sendMetric(metric_prefix + lit::tdc_sample_time_lag, uint64_t{0}, lit::unit_nanoseconds, 11,
                          MetricMode::Average);
    metricMan->sendMetric(metric_prefix + lit::tdc_sequence_gap, uint64_t{0}, lit::unit_sample_count, 11,
                          MetricMode::Accumulate);
    metricMan->sendMetric(metric_prefix + lit::tdc_sample_rate, uint64_t{0}, lit::unit_samples_per_second, 11,
                          MetricMode::Rate);
    metricMan->sendMetric(metric_prefix + lit::tdc_bytes_read, uint64_t{0}, lit::unit_bytes, 11, MetricMode::LastPoint);
    metricMan->sendMetric(metric_prefix + lit::tdc_read_count, uint64_t{0}, lit::unit_sample_count, 11,
                          MetricMode::LastPoint);
    metricMan->sendMetric(metric_prefix + lit::tdc_drain_count, uint64_t{0}, lit::unit_sample_count, 11,
                          MetricMode::LastPoint);
    metricMan->sendMetric(metric_prefix + lit::tdc_missed_count, uint64_t{0}, lit::unit_sample_count, 11,
                          MetricMode::LastPoint);
    metricMan->sendMetric(metric_prefix + lit::tdc_dropped_count, uint64_t{0}, lit::unit_sample_count, 11,
                          MetricMode::LastPoint);
    metricMan->sendMetric(metric_prefix + lit::tdc_last_sequence, uint64_t{0}, lit::unit_sample_count, 11,
                          MetricMode::LastPoint);
  }
  for (auto task : starttasks)
    if (!(this->*task)()) return false;
  inhibit = false;
  TLOG(TLVL_DEBUG_4) << "Started channel=" << int{id} << ".";
  return true;
}

bool TDCChan::stop() {
  TLOG(TLVL_DEBUG_5) << "Stopping channel=" << int{id} << ".";
  inhibit = true;
  for (auto task : stoptasks)
    if (!(this->*task)()) return false;
  TLOG(TLVL_DEBUG_5) << "Stopped channel=" << int{id} << ".";
  return true;
}

void TDCChan::monitor_timestamp(uint64_t timestamp_ns) const {
  auto lag_ns = utls::elapsed_time_ns(timestamp_ns);

  if (metricMan) {
    metricMan->sendMetric(metric_prefix + lit::tdc_sample_time_lag, lag_ns, lit::unit_nanoseconds, 11,
                          MetricMode::Average);
  }
  if (lag_ns < fmctdc.max_sample_time_lag_ns) return;

  if (lag_ns <= utls::onesecond_ns) {
    TLOG(TLVL_DEBUG_10) << "Wrong TDC sample time, check the NTP and WhiteRabbit timing systems; host_time-sample_time="
                        << lag_ns << " ns.";

  } else {
    TLOG(TLVL_WARN) << "Wrong TDC sample time, check the NTP and WhiteRabbit timing system; host_time-sample_time="
                    << lag_ns / utls::onesecond_ns << " seconds.";
    if (metricMan) {
      metricMan->sendMetric(metric_prefix + lit::tdc_laggy_samples, uint64_t{1}, lit::unit_samples_per_second, 11,
                            MetricMode::Rate);
    }
  }
}

void TDCChan::monitor() {
  if (!enabled) return;

  if (metricMan) {
    metricMan->sendMetric(metric_prefix + lit::tdc_bytes_read, uint64_t{bytes_read}, lit::unit_bytes, 11,
                          MetricMode::LastPoint);
    metricMan->sendMetric(metric_prefix + lit::tdc_read_count, uint64_t{sample_read_count}, lit::unit_sample_count, 11,
                          MetricMode::LastPoint);
    metricMan->sendMetric(metric_prefix + lit::tdc_drain_count, uint64_t{sample_drain_count}, lit::unit_sample_count, 11,
                          MetricMode::LastPoint);
    metricMan->sendMetric(metric_prefix + lit::tdc_missed_count, uint64_t{missed_sample_count}, lit::unit_sample_count,
                          11, MetricMode::LastPoint);
    metricMan->sendMetric(metric_prefix + lit::tdc_dropped_count, uint64_t{sample_drop_count}, lit::unit_sample_count,
                          11, MetricMode::LastPoint);
    metricMan->sendMetric(metric_prefix + lit::tdc_last_sequence, uint64_t{last_seen_sample_seq},
                          lit::unit_sample_count, 11, MetricMode::LastPoint);
  }
}
