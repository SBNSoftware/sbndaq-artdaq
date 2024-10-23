#include "sbndaq-artdaq/Generators/SBND/SPECTDC/SPECTDC_Interface.hh"
#define TRACE_NAME "TDCChanDrain"
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

bool TDCChan::configure_drain_buffer() {
  if (!enabled) return true;

  TLOG(TLVL_DEBUG_1) << "Draining data in channel=" << int{id} << ".";
  auto drained_count = uint64_t{0};
  auto sample_info = std::string("Unknown");
  while (true) {
    fmctdc_time(&tdcts_arr)[] = *reinterpret_cast<fmctdc_time(*)[]>(&fmctdc_buffer[0]);
    auto read_count = fmctdc_read(fmctdc.tdcdevice, id, &tdcts_arr[0], fmctdc_buffer_size, O_NONBLOCK);
    if (read_count < 1) {
      TLOG(TLVL_INFO) << "Drained " << drained_count << " samples in channel=" << int{id} << ".";
      sample_drain_count += drained_count;
      return true;
    }
    if (read_count > 5) {
      TLOG(TLVL_DEBUG) << "Channel " << int{id} << " received a birst of " << read_count << " timestamps.";
    }

    auto host_time = utls::hosttime();

    for (decltype(read_count) i = 0; i < read_count; i++) {
      auto ts = make_timestamp(id, name, tdcts_arr[i]);
      if (fmctdc.tai2utc) adjust_tai2utc(ts, fmctdc.tai2utc);
      last_seen_sample_seq = tdcts_arr[i].seq_id;
      auto sample_time = uint64_t{ts.vals.seconds};

      sample_info = as_string(ts);
      if (sample_time > host_time) {
        TLOG(TLVL_WARNING)
            << "Wrong TDC sample time, check the NTP and WhiteRabbit timing systems; sample_time-host_time="
            << sample_time - host_time << " ns, " << sample_info;
        ++drained_count;
        continue;
      }

      if (host_time == sample_time) {
        if (monitor_only) {
          TLOG(TLVL_DEBUG_2) << sample_info;
        } else {
          TLOG(TLVL_DEBUG_3) << sample_info;
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
        bytes_read += sizeof(fmctdc_time);
      } else {
        ++drained_count;
      }
    }
    utls::thread_sleep_us(2);
  }
}
