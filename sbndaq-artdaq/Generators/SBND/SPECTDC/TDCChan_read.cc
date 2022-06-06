#include "sbndaq-artdaq/Generators/SBND/SPECTDC/SPECTDC_Interface.hh"
#define TRACE_NAME "TDCChanRead"
#include "artdaq/DAQdata/Globals.hh"
#include "sbndaq-artdaq/Generators/Common/PoolBuffer.hh"
#include "sbndaq-artdaq/Generators/SBND/SPECTDC/StringLiterals.hh"
#include "sbndaq-artdaq/Generators/SBND/SPECTDC/TDCChan_utls.hh"

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

void TDCChan::read() {
  if (!enabled) return;

  fmctdc_time(&tdcts_arr)[] = *reinterpret_cast<fmctdc_time(*)[]>(&fmctdc_buffer[0]);
  auto read_count =
      fmctdc_read(fmctdc.tdcdevice, id, &tdcts_arr[0], fmctdc_buffer_size, fmctdc.blocking_reads ? 0 : O_NONBLOCK);
  if (read_count < 1) {
    TLOG(TLVL_DEBUG + 1) << "No samples in channel=" << int{id};
    return;
  }
  auto sample_info = std::string("Unknown");
  if (read_count > 3) {
    TLOG(TLVL_DEBUG) << "Channel " << int{id} << " received a birst of " << read_count << " samples.";
  }

  auto gap = uint64_t{0};
  for (decltype(read_count) i = 0; i < read_count; i++) {
    auto ts = make_timestamp(id, name, tdcts_arr[i]);
    if (fmctdc.tai2utc) adjust_tai2utc(ts, fmctdc.tai2utc);

    if (fmctdc.monitor_timestamps) monitor_timestamp(ts.timestamp_ns());
    sample_info = as_string(ts);

    if (monitor_only) {
      TLOG(TLVL_DEBUG + 2) << sample_info;
    } else {
      TLOG(TLVL_DEBUG + 3) << sample_info;
      if (inhibit) {
        ++sample_drain_count;
      } else {
        auto size = buffer->write(PoolBuffer::DataRange<TDCTimestamp>{ts.data.begin(), ts.data.end()});

        if (!size) {
          ++sample_drop_count;
          TLOG(TLVL_WARNING) << "Failed adding " << sample_info << " to PoolBuffer; PoolBuffer is "
                             << (buffer->isEmpty() ? "full" : "empty") << ".";
        } else {
          ++sample_read_count;
        }
      }
    }
    gap = tdcts_arr[i].seq_id - last_seen_sample_seq;
    if (gap > 1) {
      missed_sample_count += gap;
      TLOG(TLVL_WARNING) << "Detected a gap in the channel " << int{id}
                         << " sample sequence; last_seen_sample_seq/gap=" << last_seen_sample_seq << "/" << gap;
      if (metricMan) {
        metricMan->sendMetric(metric_prefix + lit::tdc_sequence_gap, gap, lit::unit_sample_count, 1,
                              MetricMode::Accumulate);
      }
    }
    last_seen_sample_seq = tdcts_arr[i].seq_id;
  }

  if (metricMan) {
    metricMan->sendMetric(metric_prefix + lit::tdc_sample_rate, uint64_t(read_count), lit::unit_samples_per_second, 1,
                          MetricMode::Rate);
  }
  bytes_read += read_count * sizeof(fmctdc_time);
}
