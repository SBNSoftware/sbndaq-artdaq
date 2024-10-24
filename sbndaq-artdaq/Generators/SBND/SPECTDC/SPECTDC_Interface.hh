#ifndef sbndaq_artdaq_Generators_SPECTDC_SPECTDCInterface_hh
#define sbndaq_artdaq_Generators_SPECTDC_SPECTDCInterface_hh

#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "sbndaq-artdaq-core/Overlays/SBND/TDCTimestampFragment.hh"

#include "fhiclcpp/fwd.h"
//#define GENERATE_FAKE_DATA 1
struct fmctdc_board;

namespace sbndaq {
using PoolBufferUPtr_t = std::unique_ptr<class PoolBuffer>;
namespace SPECTDCInterface {

enum class timesync_source_t { wr, host };
enum class chan_buffer_mode_t { fifo, circular };

struct TAI2UTCAdjustment {
 public:
  explicit TAI2UTCAdjustment(fhicl::ParameterSet const&);
  ~TAI2UTCAdjustment() = default;

 public:
  std::string as_fhicl() const;
  uint64_t adjust(uint64_t) const;
  uint8_t count_leap_seconds(uint64_t) const;

 private:
  TAI2UTCAdjustment(TAI2UTCAdjustment const&) = delete;
  TAI2UTCAdjustment(TAI2UTCAdjustment&&) = delete;
  TAI2UTCAdjustment& operator=(TAI2UTCAdjustment const&) = delete;
  TAI2UTCAdjustment& operator=(TAI2UTCAdjustment&&) = delete;

 private:
  uint8_t add_leap_seconds = 37;
  uint64_t next_leap_second_tai = 1672531199;
  bool make_adjustment = true;
};

struct Device {
  virtual ~Device(){};
  virtual bool open() = 0;
  virtual bool configure() = 0;
  virtual void close() = 0;
  virtual void read() = 0;
  virtual void monitor() = 0;
  virtual bool start() = 0;
  virtual bool stop() = 0;
  virtual std::string as_fhicl() const = 0;
};

class TDCChan;
class TDCCard;
class SPECCard;

namespace TDCTimeUtils {
constexpr auto onesecond = uint64_t{1};
constexpr auto onesecond_ms = uint64_t{1'000};
constexpr auto onesecond_us = uint64_t{1'000'000};
constexpr auto onesecond_ns = uint64_t{1'000'000'000};
constexpr auto as_seconds = uint64_t{1'000'000'000};
constexpr auto as_milliseconds = uint64_t{1'000'000};
constexpr auto as_microseconds = uint64_t{1'000};
uint64_t hosttime();
uint64_t hosttime_us();
uint64_t hosttime_ns();
double elapsed_time_ns(uint64_t);
void thread_sleep(uint64_t);
void thread_sleep_us(uint64_t);
void thread_sleep_ms(uint64_t);
std::tuple<int, std::string> wr_time_summary(uint64_t);
std::string to_localtime(uint64_t);
template <uint64_t scale>
inline uint64_t convert_time_ns(uint64_t sample_time_ns) {
  return uint64_t{sample_time_ns / scale};
}
}  // namespace TDCTimeUtils

class TDCChan : public Device {
 private:
  typedef bool (TDCChan::*fn_configure_t)();
  static constexpr size_t name_length = TDCTimestamp::name_length;
  using name_t = TDCTimestamp::name_t;
  name_t to_array(std::string);

 public:
  explicit TDCChan(fhicl::ParameterSet const&, PoolBufferUPtr_t&, TDCCard const&);
  ~TDCChan() override{};
  TDCChan(TDCChan&&) = default;

 public:
  std::string as_fhicl() const override;
  bool open() override;
  bool configure() override;
  void close() override;
  void read() override;
  void monitor() override;
  bool start() override;
  bool stop() override;

 private:
  TDCChan(TDCChan const&) = delete;
  TDCChan& operator=(TDCChan const&) = delete;
  TDCChan& operator=(TDCChan&&) = delete;

 private:
  bool configure_timeoffset();
  bool configure_termination();
  bool configure_buffer_length();
  bool configure_buffer_mode();
  bool configure_flush_buffer();
  bool configure_drain_buffer();
  bool configure_channel_status();
  bool configure_channel_disable();
  void monitor_timestamp(uint64_t ts, int id) const;

 private:
  uint8_t id = 0;
  name_t name = {'u', 'n', 'k', 'n', 'o', 'w', 'n'};
  bool enabled = false;
  bool terminated = false;
  bool monitor_only = false;
  int32_t time_offset_ps = 0;
  chan_buffer_mode_t buffer_mode = chan_buffer_mode_t::circular;
  uint32_t buffer_length = 64;

  // used internally
  PoolBufferUPtr_t& buffer;
  std::unique_ptr<uint8_t[]> fmctdc_buffer;
  static constexpr uint8_t fmctdc_buffer_size = 64;
  TDCCard const& fmctdc;
  int fd = -1;
  std::vector<fn_configure_t> configtasks;
  std::vector<fn_configure_t> starttasks;
  std::vector<fn_configure_t> stoptasks;
  uint64_t bytes_read = 0;
  uint64_t sample_read_count = 0;
  uint64_t sample_drain_count = 0;
  uint64_t missed_sample_count = 0;
  uint64_t sample_drop_count = 0;
  uint64_t last_seen_sample_seq = 0;
  bool inhibit = true;
  std::string metric_prefix;
  friend class TDCCard;
};

class TDCCard : public Device {
 public:
  explicit TDCCard(fhicl::ParameterSet const&, PoolBufferUPtr_t&);
  ~TDCCard();

 public:
  std::string as_fhicl() const override;
  bool open() override;
  bool configure() override;
  void close() override;
  void reportTimeTemp();
  void read() override;
  void monitor() override;
  bool start() override;
  bool stop() override;

 private:
  TDCCard(TDCCard const&) = delete;
  TDCCard(TDCCard&&) = delete;
  TDCCard& operator=(TDCCard const&) = delete;
  TDCCard& operator=(TDCCard&&) = delete;

 private:
  bool monitor_temperature = true;
  bool monitor_timestamps = true;
  std::string calibration_data = "none";
  timesync_source_t timesync_source = timesync_source_t::host;
  unsigned int deviceid = 0;
  int polltime_ms = 50;
  bool blocking_reads = true;
  uint64_t max_sample_time_lag_ns = 100'000'000;

  // used internally
  uint64_t total_bytes_read = 0;
  uint64_t total_sample_read_count = 0;
  std::vector<TDCChan> chans;
  std::unique_ptr<TAI2UTCAdjustment> tai2utc = nullptr;

  fmctdc_board* tdcdevice = nullptr;
  friend class TDCChan;
  friend class SPECCard;
};

class SPECCard : public Device {
 public:
  explicit SPECCard(fhicl::ParameterSet const&, PoolBufferUPtr_t&);
  ~SPECCard();

 public:
  std::string as_fhicl() const override;
  bool open() override;
  bool configure() override;
  void close() override;
  void read() override;
  void monitor() override;
  bool start() override;
  bool stop() override;

 private:
  SPECCard(SPECCard const&) = delete;
  SPECCard(SPECCard&&) = delete;
  SPECCard& operator=(SPECCard const&) = delete;
  SPECCard& operator=(SPECCard&&) = delete;

 private:
  std::string deviceid = "auto";
  bool verbose = false;

  // used internally
  bool isOpen = false;
  bool isConfigured = false;
  TDCCard fmctdc;
};
}  // namespace SPECTDCInterface
}  // namespace sbndaq
#endif  // sbndaq_artdaq_Generators_SPECTDC_SPECTDCInterface_hh
