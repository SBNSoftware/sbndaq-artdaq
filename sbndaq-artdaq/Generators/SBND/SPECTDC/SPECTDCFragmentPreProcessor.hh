#ifndef sbndaq_artdaq_Generators_SPECTDCFragmentPreProcessor_hh
#define sbndaq_artdaq_Generators_SPECTDCFragmentPreProcessor_hh

#include <cstddef>
#include <cstdint>
#include <list>
#include <mutex>
#include <stdexcept>

#include "artdaq-core/Data/Fragment.hh"

namespace sbndaq {
class SPECTDCFragmentPreProcessor {
 public:
  explicit SPECTDCFragmentPreProcessor(std::uint64_t timeout_us = 1250000 )
      : releaseFragmentTimeout_{timeout_us * 1000} {};

  ~SPECTDCFragmentPreProcessor();

  [[nodiscard]] bool processFragments(artdaq::FragmentPtrs& fragments) noexcept;
  [[nodiscard]] const artdaq::FragmentPtrs& getPendingFragments() const noexcept { return pendingFragments_; };
  [[nodiscard]] std::uint64_t oldestMostRecentPoint() const noexcept { return oldestMostRecentPoint_; }
  [[nodiscard]] std::uint64_t partitionPoint() const noexcept { return partitionPoint_; }
  [[nodiscard]] std::uint64_t acceptBeforeTimestamp() const noexcept { return acceptBeforeTimestamp_; }
  [[nodiscard]] std::uint64_t seenFragmentCount() const noexcept { return seenFragmentCount_; }
  [[nodiscard]] std::uint64_t fragmentCounter() const noexcept { return fragmentCounter_; }

 private:
  [[nodiscard]] std::uint64_t fragmentCounter_inc() noexcept { return fragmentCounter_++; }
  std::uint64_t releaseFragmentTimeout_{0};
  std::uint64_t fragmentCounter_{0};
  std::uint64_t oldestMostRecentPoint_{0};
  std::uint64_t partitionPoint_{0};
  std::uint64_t acceptBeforeTimestamp_{0};
  std::uint64_t seenFragmentCount_{0};
  artdaq::FragmentPtrs pendingFragments_;
  mutable std::mutex processingMutex_;

  // private methods
  void filterFragmentsByTimestamp(artdaq::FragmentPtrs& fragments, std::uint64_t acceptBeforeTimestamp);

  void mergePendingFragments(artdaq::FragmentPtrs& fragments) {
    if (pendingFragments_.empty()) return;
    fragments.splice(fragments.end(), std::move(pendingFragments_));
  }

  void qsortFragmentsByTimestamp(artdaq::FragmentPtrs& fragments) {
    if (fragments.size() <= 1) return;
    fragments.sort([](const auto& a, const auto& b) { return a->timestamp() < b->timestamp(); });
  }

  void assignSequentialIDs(artdaq::FragmentPtrs& fragments, bool useFragmentCounterInc = false) {
    if (fragments.empty()) return;
    auto it = fragments.begin();
    for (std::size_t i = 0; it != fragments.end(); ++i, ++it) {
      (*it)->setSequenceID(useFragmentCounterInc ? fragmentCounter_inc() : fragmentCounter_inc() + i);
    }
  }

  void validateFragments(const artdaq::FragmentPtrs& fragments) const {
    for (const auto& fragment : fragments) {
      if (!fragment) throw std::invalid_argument("Null fragment pointer detected");
    }
  }
};

void reportFistLastFragments(const artdaq::FragmentPtrs& fragments, const std::string& label);
void reportAllFragments(const artdaq::FragmentPtrs& fragments, const std::string& label, std::uint64_t partition = 0, bool delta = false);
std::string formatTimestamp(std::uint64_t timestamp, std::uint64_t reference = 0, bool delta = false);
bool runVerboseFragmentProcessing(SPECTDCFragmentPreProcessor& processor, artdaq::FragmentPtrs& fragments);

}  // namespace sbndaq

#endif  // sbndaq_artdaq_Generators_SPECTDCFragmentPreProcessor_hh
