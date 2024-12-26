#include "sbndaq-artdaq/Generators/SBND/SPECTDC/SPECTDCFragmentPreProcessor.hh"

#define TRACE_NAME "SPECTDCFragmentPreProcessor_utils"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "trace.h"
#include "sbndaq-artdaq-core/Overlays/SBND/TDCTimestampFragment.hh"

namespace {
constexpr std::size_t kMaxDisplayEntries = 8;
constexpr std::size_t kMaxChanDigits = 2;
constexpr std::size_t kMinSeqDigits = 8;
std::size_t kSeqDigits = kMinSeqDigits;
}  // namespace

namespace sbndaq {

void updateSeqPaddingSize(uint64_t num) noexcept {
  if (num == 0) kSeqDigits = kMinSeqDigits;
  kSeqDigits = std::max<std::size_t>(kMinSeqDigits,
                                     static_cast<std::size_t>(std::log10(num)) + 2);
}

std::string formatTimestamp(const std::uint64_t timestamp, const std::uint64_t reference, const bool delta) {
  std::stringstream ss;
  uint64_t seconds = timestamp / 1000000000;
  uint64_t last_two = (seconds % 100);
  seconds = seconds / 100;
  uint64_t subsecA = (timestamp % 1000000000) / 1000000;
  uint64_t subsecB = (timestamp % 1000000) / 1000;
  uint64_t subsecC = timestamp % 1000;
  if (reference == 0) {
    ss << " ";
  } else if (timestamp < reference) {
    ss << "-";
  } else {
    ss << "+";
  }
  ss << seconds << '.' << std::setfill('0') << std::setw(2) << last_two
     << '\'' << std::setfill('0') << std::setw(3) << subsecA
     << '.' << std::setfill('0') << std::setw(3) << subsecB
     << '.' << std::setfill('0') << std::setw(3) << subsecC;
  if (delta) {
    int64_t delta_time = (timestamp >= reference) ? timestamp - reference : reference - timestamp;
    uint64_t delta_seconds = delta_time / 1000000000;
    uint64_t delta_last_two = (delta_seconds % 100);
    delta_seconds = delta_seconds / 100;
    uint64_t delta_subsecA = (delta_time % 1000000000) / 1000000;
    uint64_t delta_subsecB = (delta_time % 1000000) / 1000;
    uint64_t delta_subsecC = delta_time % 1000;

    ss << " [" << ((timestamp < reference) ? "-" : "+")
       << delta_seconds << '.' << std::setfill('0') << std::setw(2) << delta_last_two
       << '\'' << std::setfill('0') << std::setw(3) << delta_subsecA
       << '.' << std::setfill('0') << std::setw(3) << delta_subsecB
       << '.' << std::setfill('0') << std::setw(3) << delta_subsecC << "]";
  }
  return ss.str();
}

void reportFistLastFragments(const artdaq::FragmentPtrs& fragments, const std::string& label) {
  using sbndaq::TDCTimestampFragment;
  std::stringstream buffer;
  buffer << label << ":";
  TLOG(TLVL_DEBUG + 10) << buffer.str() << '\n';
  buffer.str("");

  if (fragments.empty()) {
    buffer << "No fragments to report";
    TLOG(TLVL_DEBUG + 10) << buffer.str() << '\n';
    buffer.str("");
  } else {
    buffer << "Total fragments: " << fragments.size() << " "
           << "First: (seq,chan,timestamp)=("
           << std::setfill(' ') << std::setw(kSeqDigits) << fragments.front()->sequenceID()
           << "," << std::setfill(' ') << std::setw(kMaxChanDigits) << TDCTimestampFragment(*fragments.front()).getTDCTimestamp()->vals.channel
           << "," << formatTimestamp(fragments.front()->timestamp()) << ") "
           << "Last: (seq,chan,timestamp)=("
           << std::setfill(' ') << std::setw(kSeqDigits) << fragments.back()->sequenceID()
           << "," << std::setfill(' ') << std::setw(kMaxChanDigits) << TDCTimestampFragment(*fragments.front()).getTDCTimestamp()->vals.channel
           << "," << formatTimestamp(fragments.back()->timestamp()) << ")";
    TLOG(TLVL_DEBUG + 10) << buffer.str() << '\n';
    buffer.str("");
  }
}

void reportAllFragments(const artdaq::FragmentPtrs& fragments, const std::string& label, const std::uint64_t partition, const bool delta) {
  using sbndaq::TDCTimestampFragment;
  std::stringstream buffer;
  buffer << label << ":";
  TLOG(TLVL_DEBUG + 11) << buffer.str() << '\n';
  buffer.str("");

  if (fragments.empty()) {
    buffer << "No fragments";
    TLOG(TLVL_DEBUG + 11) << buffer.str() << '\n';
    buffer.str("");
    return;
  }

  buffer << "Total fragments: " << fragments.size();
  TLOG(TLVL_DEBUG + 11) << buffer.str() << '\n';
  buffer.str("");

  auto it = fragments.begin();
  if (fragments.size() < 3 * kMaxDisplayEntries) {
    for (; it != fragments.end(); ++it) {
      if (*it) {
        buffer << "(seq,chan,timestamp)=(" << std::setfill(' ') << std::setw(kSeqDigits) << (*it)->sequenceID()
               << "," << std::setfill(' ') << std::setw(kMaxChanDigits) << TDCTimestampFragment(*(it->get())).getTDCTimestamp()->vals.channel
               << "," << formatTimestamp((*it)->timestamp(), partition, delta) << ")";
        TLOG(TLVL_DEBUG + 11) << buffer.str() << '\n';
        buffer.str("");
      } else {
        buffer << "Invalid fragment pointer";
        TLOG(TLVL_DEBUG + 11) << buffer.str() << '\n';
        buffer.str("");
      }
    }
  } else {
    for (std::size_t i = 0; i < kMaxDisplayEntries && it != fragments.end(); ++i, ++it) {
      if (*it) {
        buffer << "(seq,chan,timestamp)=(" << std::setfill(' ') << std::setw(kSeqDigits) << (*it)->sequenceID()
               << "," << std::setfill(' ') << std::setw(kMaxChanDigits) << TDCTimestampFragment(*(it->get())).getTDCTimestamp()->vals.channel
               << "," << formatTimestamp((*it)->timestamp(), partition, delta) << ")";
        TLOG(TLVL_DEBUG + 11) << buffer.str() << '\n';
        buffer.str("");
      } else {
        buffer << "Invalid fragment pointer";
        TLOG(TLVL_DEBUG + 11) << buffer.str() << '\n';
        buffer.str("");
      }
    }

    buffer << "...";
    TLOG(TLVL_DEBUG + 11) << buffer.str() << '\n';
    buffer.str("");

    auto total = fragments.size();
    it = std::next(fragments.begin(), total - 2 * kMaxDisplayEntries);
    for (; it != fragments.end(); ++it) {
      if (*it) {
        buffer << "(seq,chan,timestamp)=(" << std::setfill(' ') << std::setw(kSeqDigits) << (*it)->sequenceID()
               << "," << std::setfill(' ') << std::setw(kMaxChanDigits)
               << TDCTimestampFragment(*(it->get())).getTDCTimestamp()->vals.channel
               << "," << formatTimestamp((*it)->timestamp(), partition, delta) << ")";
        TLOG(TLVL_DEBUG + 11) << buffer.str() << '\n';
        buffer.str("");
      } else {
        buffer << "Invalid fragment pointer";
        TLOG(TLVL_DEBUG + 11) << buffer.str() << '\n';
        buffer.str("");
      }
    }
  }
}

bool runVerboseFragmentProcessing(SPECTDCFragmentPreProcessor& processor, artdaq::FragmentPtrs& fragments) {
  const auto initailEventCount = fragments.size();
  const auto now =
      std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

  reportAllFragments(fragments, "Before Processing", now, true);

  const auto startTime = std::chrono::high_resolution_clock::now();
  const bool processingSucceeded = processor.processFragments(fragments);
  const auto stopTime = std::chrono::high_resolution_clock::now();

  const auto currentTime =
      std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

  reportAllFragments(fragments, "After Processing", processor.oldestMostRecentPoint(), true);

  auto padding = kSeqDigits - kMinSeqDigits + 3;
  TLOG(TLVL_DEBUG + 12) << std::setw(padding) << "" << "       partition/acceptBefore: " << formatTimestamp(processor.partitionPoint(), processor.acceptBeforeTimestamp(), true) << '\n';
  TLOG(TLVL_DEBUG + 12) << std::setw(padding) << "" << "   oldestMostRecent/partition: " << formatTimestamp(processor.oldestMostRecentPoint(), processor.partitionPoint(), true) << '\n';
  TLOG(TLVL_DEBUG + 12) << std::setw(padding) << "" << "acceptBefore/oldestMostRecent: " << formatTimestamp(processor.acceptBeforeTimestamp(), processor.oldestMostRecentPoint(), true) << '\n';
  TLOG(TLVL_DEBUG + 12) << std::setw(padding) << "" << "        currentTime/partition: " << formatTimestamp(currentTime, processor.partitionPoint(), true) << '\n';
  TLOG(TLVL_DEBUG + 12) << std::setw(padding) << "" << "    expect/have pending frags: " << (processor.oldestMostRecentPoint() <= processor.acceptBeforeTimestamp() ? "no" : "yes")
                        << "/" << (processor.getPendingFragments().empty() ? "no" : "yes") << '\n';

  reportAllFragments(processor.getPendingFragments(), "Pending Fragments", processor.oldestMostRecentPoint(), true);

  const auto processingDuration = std::chrono::duration_cast<std::chrono::microseconds>(stopTime - startTime);

  std::stringstream ss;
  ss << "Performance Results:";
  TLOG(TLVL_DEBUG + 12) << ss.str() << '\n';
  ss.str("");

  ss << "- Processed (" << std::boolalpha << processingSucceeded << ") " << initailEventCount << " frags in " << processingDuration.count() << " usecs";
  TLOG(TLVL_DEBUG + 12) << ss.str() << '\n';
  ss.str("");

  ss << "- Accepted/Penfing counts: " << fragments.size() << "/" << processor.getPendingFragments().size();
  TLOG(TLVL_DEBUG + 12) << ss.str() << '\n';

  updateSeqPaddingSize(processor.fragmentCounter());

  return processingSucceeded;
}

}  // namespace sbndaq
