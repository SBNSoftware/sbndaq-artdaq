#include "sbndaq-artdaq/Generators/SBND/SPECTDCTimestampReader.hh"
#define TRACE_NAME "SPECTDCTimestampReader"
#include "artdaq/DAQdata/Globals.hh"
#include "artdaq/Generators/GeneratorMacros.hh"
#include "fhiclcpp/ParameterSet.h"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "sbndaq-artdaq-core/Overlays/SBND/TDCTimestampFragment.hh"
#include "sbndaq-artdaq/Generators/Common/PoolBuffer.hh"
#include "sbndaq-artdaq/Generators/Common/workerThread.hh"

uint64_t* aa = nullptr;
using artdaq::MetricMode;
using sbndaq::PoolBuffer;
using sbndaq::SPECTDCTimestampReader;
using sbndaq::TDCTimestamp;
using sbndaq::SPECTDCInterface::SPECCard;
namespace utls = sbndaq::SPECTDCInterface::TDCTimeUtils;

SPECTDCTimestampReader::SPECTDCTimestampReader(fhicl::ParameterSet const& ps)
    : CommandableFragmentGenerator{ps},
      fragment_id_{ps.get<decltype(fragment_id_)>("fragment_id")},
      buffer_{new PoolBuffer()},
      hardware_{new SPECCard{ps.get<fhicl::ParameterSet>("spec"), buffer_}},
      data_thread_{nullptr},
      metadata_{},
      configured_{configure(ps)},
      sleep_on_no_data_us_{ps.get<decltype(sleep_on_no_data_us_)>("sleep_on_no_data_us", 10000)},
      events_to_generate_{ps.get<decltype(events_to_generate_)>("events_to_generate", 0)},
      separate_data_thread_{ps.get<decltype(separate_data_thread_)>("separate_data_thread", 0)} {
  aa = new uint64_t[1024];
  std::fill_n(aa, 1024, 1);
}

SPECTDCTimestampReader::~SPECTDCTimestampReader() {}

bool SPECTDCTimestampReader::configure(fhicl::ParameterSet const& ps) {
  TLOG(TLVL_DEBUG + 3) << "Configuring.";
  auto frag_size = sizeof(TDCTimestamp);
  PoolBuffer tmpBuff;
  tmpBuff.allocate(frag_size, 2048, true);
  auto buff_size = ps.get<decltype(frag_size)>("data_buffer_depth_fragments", 1024) *
                   (tmpBuff.poolBufferSize() / tmpBuff.blockCount());

  buffer_->allocate(frag_size, buff_size, true);
  TLOG(TLVL_DEBUG + 3) << "Buffer" << buffer_->debugInfo();
  hardware_->configure();
  TLOG(TLVL_DEBUG + 3) << "Configured.";

  return true;
}

void SPECTDCTimestampReader::start() {
  TLOG(TLVL_DEBUG + 4) << "Starting";
  stop_requested_ = false;
  hardware_->start();

  if (!separate_data_thread_) {
    TLOG(TLVL_DEBUG + 4) << "Started";
    return;
  }

  auto usec_delay_if_failed = uint64_t{sleep_on_no_data_us_ < 10'000 ? 10'000 : sleep_on_no_data_us_};
  auto max_stop_tries = uint64_t{2'000'000 / usec_delay_if_failed};

  TLOG(TLVL_INFO) << "Starting getData() worker thread with usec_delay_if_failed=" << usec_delay_if_failed
                  << ", max_stop_tries=" << max_stop_tries << ".";

  share::ThreadFunctor functor = std::bind(&SPECTDCTimestampReader::getData_, this);
  auto worker_functor = std::make_unique<share::WorkerThreadFunctor>(functor, "getData() worker");
  auto worker_thread = share::WorkerThread::createWorkerThread(worker_functor, usec_delay_if_failed, max_stop_tries);
  data_thread_.swap(worker_thread);
  data_thread_->start();

  TLOG(TLVL_DEBUG + 4) << "Started with getData() worker thread.";
}

void SPECTDCTimestampReader::stop() {
  TLOG(TLVL_DEBUG + 5) << "Stopping hardware readout and getdata threads.";
  stop_requested_ = true;
  hardware_->stop();
  if (data_thread_) data_thread_->stop();
  TLOG(TLVL_DEBUG + 5) << "Stopped hardware readout and getdata threads.";
}

void SPECTDCTimestampReader::requestStop() {
  TLOG(TLVL_INFO) << "Stopping boardreader process after " << ev_counter() - 1
                  << " events, timestamp=" << utls::hosttime() << ".";
  stop_requested_ = true;
}

bool SPECTDCTimestampReader::getNext_(artdaq::FragmentPtrs& fragments) {
  if (should_stop() || stop_requested_) {
    return false;
  }

  if (!separate_data_thread_) {
    if (!getData_()) {
      TLOG(TLVL_ERROR) << " Stopping boardreader process after " << ev_counter() << " events.";
      return false;
    }
  }

  if (buffer_->activeBlockCount() == 0) {
    TLOG(TLVL_WARNING) << "Buffer has no data.  Last seen fragment seq=" << ev_counter() << "; Sleep for "
                       << sleep_on_no_data_us_ << " us and return.";
    utls::thread_sleep_us(sleep_on_no_data_us_);
    return true;
  }

  while (buffer_->activeBlockCount()) {
    std::unique_ptr<artdaq::Fragment> fragment_uptr(artdaq::Fragment::FragmentBytes(
        TDCTimestamp::size_bytes, ev_counter(), fragment_id_, sbndaq::detail::FragmentType::TDCTIMESTAMP, metadata_));

    using sbndaq::PoolBuffer;
    PoolBuffer::DataRange<decltype(artdaq::Fragment())> range{fragment_uptr->dataBegin(), fragment_uptr->dataEnd()};

    if (!buffer_->read(range)) break;

    const auto ts = TDCTimestampFragment(*fragment_uptr);
    fragment_uptr->setTimestamp(ts.getTDCTimestamp()->timestamp_ns());
    fragments.emplace_back(std::move(fragment_uptr));

    ev_counter_inc();

    auto const& last_frag = *fragments.back();
    auto const last_frag_ovl = TDCTimestampFragment(last_frag);
    auto create_time_ms = utls::convert_time_ns<utls::as_milliseconds>(utls::elapsed_time_ns(last_frag.timestamp()));
    TLOG(TLVL_DEBUG + 3) << "Fragment seq=" << last_frag.sequenceID() << ", timestamp=" << std::setw(20)
                         << last_frag.timestamp() << ", create_time=" << std::setw(5) << create_time_ms << " ms, "
                         << last_frag_ovl;
    if (metricMan) {
      metricMan->sendMetric("Fragments Sent", 1, "Events", 1, MetricMode::Rate);
      metricMan->sendMetric("Fragment Create Time", create_time_ms, "ms", 1, MetricMode::Average | MetricMode::Maximum);
    }

    aa[1] = ev_counter();
    if (0 != events_to_generate_ && ev_counter() > events_to_generate_) {
      requestStop();
      return false;
    }
  }

  return true;
}

bool SPECTDCTimestampReader::getData_() {
  try {
    hardware_->read();
  } catch (...) {
    TLOG(TLVL_ERROR) << "Critical error in getData_(); stopping boardreader process....";
    utls::thread_sleep_us(sleep_on_no_data_us_);
    return false;
  }
  return true;
}

bool SPECTDCTimestampReader::checkHWStatus_() {
  if (metricMan) {
    metricMan->sendMetric("PoolBuffer Free Block Count", buffer_->freeBlockCount(), "Samples", 1,
                          MetricMode::LastPoint);
    metricMan->sendMetric("PoolBuffer Fully Drained Count", buffer_->fullyDrainedCount(), "Times", 1,
                          MetricMode::LastPoint);
    metricMan->sendMetric("PoolBuffer Low Watermark", buffer_->lowWaterMark(), "Samples", 1, MetricMode::LastPoint);
  }
  try {
    hardware_->monitor();
  } catch (...) {
    TLOG(TLVL_ERROR) << "Critical error in checkHWStatus_(); stopping boardreader process....";
    utls::thread_sleep_us(sleep_on_no_data_us_);
    return false;
  }
  return true;
}

DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(SPECTDCTimestampReader)
