#include "sbndaq-artdaq/Generators/SBND/SPECTDC/SPECTDCFragmentPreProcessor.hh"

#define TRACE_NAME "SPECTDCFragmentPreProcessor"

#include <chrono>
#include <iostream>
#include <numeric>
#include <unordered_map>
#include "tracemf.h"
#include "sbndaq-artdaq-core/Overlays/SBND/TDCTimestampFragment.hh"

using sbndaq::SPECTDCFragmentPreProcessor;

/*
The SPECTDCFragmentPreProcessor takes incoming TDC data fragments and orders them by timestamp while buffering 
fragments that arrive too early. The main algorithm first validates all fragments, merges any previously pending 
fragments with new incoming ones, and sorts them by timestamp. It then determines a partition point using 
the "oldest-most-recent" approach - for each TDC channel, it finds the most recent timestamp and then takes 
the oldest timestamp from this set of recent timestamps. This partition point, combined with a timeout threshold, 
determines which fragments are processed immediately versus held back. Fragments older than the partition point 
move forward for processing while newer ones are stored in a pending queue. This buffering prevents out-of-order 
processing while the timeout mechanism (default 1.25s) ensures fragments aren't held indefinitely if a channel 
stops receiving data. The processor assigns sequential IDs to fragments that pass through and maintains counters 
to track processed and pending fragments. The system handles thread safety through mutex locking and includes 
error handling for failed processing attempts.
*/

SPECTDCFragmentPreProcessor::~SPECTDCFragmentPreProcessor() {
  if (fragmentCounter_ + pendingFragments_.size() != seenFragmentCount_) {
    TLOG(TLVL_INFO) << "Processed fragments (" << fragmentCounter_ << " processed + " << pendingFragments_.size()
                    << " pending) does not equal total fragments seen (" << seenFragmentCount_ << ")\n";
  }

  pendingFragments_.clear();
}

bool SPECTDCFragmentPreProcessor::processFragments(artdaq::FragmentPtrs& fragments) noexcept {
  std::lock_guard<std::mutex> lock(processingMutex_);
  try {
    if (fragments.empty() && pendingFragments_.empty()) return true;
    seenFragmentCount_ += fragments.size();
    validateFragments(fragments);
    mergePendingFragments(fragments);
    qsortFragmentsByTimestamp(fragments);
    const auto currentTime = std::chrono::duration_cast<std::chrono::nanoseconds>(
                                 std::chrono::system_clock::now().time_since_epoch())
                                 .count();
    acceptBeforeTimestamp_ = currentTime - releaseFragmentTimeout_;
    filterFragmentsByTimestamp(fragments, acceptBeforeTimestamp_);
    assignSequentialIDs(fragments, true);
    return true;
  } catch (const std::exception& e) {
    TLOG(TLVL_ERROR) << "Caught exception in processFragments: " << e.what() << "\n";
    return false;
  }
}

void SPECTDCFragmentPreProcessor::filterFragmentsByTimestamp(artdaq::FragmentPtrs& fragments,
                                                             std::uint64_t acceptBeforeTimestamp) {
  if (fragments.empty()) return;
  /*This function implements the oldest-most-recent algorithm, which retains artdaq fragments based
    on the timestamp up to the oldest sample among recent samples across all active TDC channels.
    Fragments older then this timestamp are moved to the pendingFragments list. Additionally, artdaq
    fragments are retained if they remain pending beyond a configurable timeout period, which may
    occur when certain channels stop receiving data.*/
  using sbndaq::TDCTimestampFragment;
  artdaq::FragmentPtrs acceptedFragments;
  struct ChannelInfo {
    uint64_t timestamp{0};
    artdaq::FragmentPtr* fragment{nullptr};
  };
  std::unordered_map<std::uint32_t, ChannelInfo> channelLatest;
  channelLatest.reserve(32);
  for (auto it = fragments.rbegin(); it != fragments.rend(); ++it) {
    auto& fragment = *it;
    if (!fragment) continue;
    auto& info = channelLatest[TDCTimestampFragment(*fragment).getTDCTimestamp()->vals.channel];
    const auto timestamp = fragment->timestamp();
    if (timestamp > info.timestamp) {
      info.timestamp = timestamp;
      info.fragment = &fragment;
    }
  }
  oldestMostRecentPoint_ = std::transform_reduce(
      channelLatest.begin(), channelLatest.end(), std::numeric_limits<uint64_t>::max(),
      [](uint64_t a, uint64_t b) { return std::min(a, b); },
      [](const auto& pair) { return pair.second.timestamp; });
  partitionPoint_ = std::max(acceptBeforeTimestamp, oldestMostRecentPoint_);
  for (auto& fragment : fragments) {
    if (!fragment) continue;
    if (fragment->timestamp() <= partitionPoint_) {
      acceptedFragments.push_back(std::move(fragment));
    } else {
      pendingFragments_.push_back(std::move(fragment));
    }
  }
  fragments = std::move(acceptedFragments);
}
