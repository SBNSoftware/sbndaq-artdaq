#include "sbndaq-artdaq/Generators/SBND/SPECTDC/SPECTDC_Interface.hh"
#define TRACE_NAME "TDCTimeUtils"
namespace utls = sbndaq::SPECTDCInterface::TDCTimeUtils;
#include <chrono>
#include <thread>
#include <tuple>
using std::chrono::microseconds;
using std::chrono::milliseconds;
using std::chrono::nanoseconds;
using std::chrono::seconds;

using clk = std::chrono::system_clock;

uint64_t utls::elapsed_time_ns(uint64_t sample_time_ns) {
  uint64_t host_time_ns =
      std::chrono::duration_cast<nanoseconds>(clk::time_point{clk::now()}.time_since_epoch()).count();

  //pull window is +-10ms so what do I expect of host_time to server_time?
  if (sample_time_ns > host_time_ns){

    //TLOG(TLVL_WARNING) << "Wrong TDC sample time, check the NTP and WhiteRabbit timing systems; sample_time-host_time="
    //                   << sample_time_ns - host_time_ns << " ns.";

    TLOG(TLVL_WARNING) << "Sample time > host time; sample_time-host_time="
                       << sample_time_ns - host_time_ns << " ns.";

  }else{
    //debug warning of fragment is not too far back in the past?
    if( (host_time_ns - sample_time_ns) > 1e7 ){
    TLOG(TLVL_WARNING) << "Host time < sample time; host_time-sample_time="
                       << host_time_ns - sample_time_ns << " ns.";
	}
  }

  //if host_time_ns - sample_time_ns is negative then most likely give bogus number: 18446744073
  //expect host_time > server_time
  
  return host_time_ns - sample_time_ns;
}

uint64_t utls::hosttime() {
  return uint64_t(std::chrono::duration_cast<seconds>(clk::time_point{clk::now()}.time_since_epoch()).count());
}

uint64_t utls::hosttime_us() {
  return uint64_t(std::chrono::duration_cast<microseconds>(clk::time_point{clk::now()}.time_since_epoch()).count());
}

uint64_t utls::hosttime_ns() {
  return uint64_t(std::chrono::duration_cast<nanoseconds>(clk::time_point{clk::now()}.time_since_epoch()).count());
}
void utls::thread_sleep(uint64_t s) { std::this_thread::sleep_until(clk::now() + seconds(s)); }
void utls::thread_sleep_us(uint64_t us) { std::this_thread::sleep_until(clk::now() + microseconds(us)); }
void utls::thread_sleep_ms(uint64_t ms) { std::this_thread::sleep_until(clk::now() + milliseconds(ms)); }

std::string utls::to_localtime(uint64_t wr_time) {
  time_t epoch = wr_time;
  return asctime(localtime(&epoch));
}

std::tuple<int, std::string> utls::wr_time_summary(uint64_t wr_time) {
  auto host_now = clk::now();
  auto wr_now = clk::time_point{seconds{wr_time}};
  auto time_wr = clk::to_time_t(wr_now);
  auto time_host = clk::to_time_t(host_now);
  std::string str_host{std::ctime(&time_host)};
  str_host.pop_back();
  std::string str_wr{std::ctime(&time_wr)};
  str_wr.pop_back();
  int lvl = TLVL_ERROR;
  std::stringstream ss;
  ss << "Wrong TDC time, check the NTP and WhiteRabbit timing systems; abs(host_time-wr_time)=";
  auto delta_seconds = std::chrono::duration_cast<seconds>(host_now - wr_now).count();
  auto lag_seconds = std::abs(delta_seconds);
  if (lag_seconds > 5 && lag_seconds < 300) {
    lvl = TLVL_WARNING;
    ss << lag_seconds << " seconds. Host time / WhiteRabbit time => " << str_host << " / " << str_wr;
  } else if (lag_seconds >= 300 && lag_seconds < 3600) {
    ss << lag_seconds / 60 << " minutes. Host time / WhiteRabbit time => " << str_host << " / " << str_wr;
  } else if (lag_seconds >= 3600 && lag_seconds < 86400) {
    ss << lag_seconds / 3600 << " hours. Host time / WhiteRabbit time => " << str_host << " / " << str_wr;
  } else if (lag_seconds >= 86400) {
    ss << lag_seconds / 86400 << " days. Host time / WhiteRabbit time => " << str_host << " / " << str_wr;
  } else {
    lvl = TLVL_DEBUG;
    ss.str("");
    ss.clear();
    ss << "Host time / WhiteRabbit time => " << str_host << " / " << str_wr
       << ", abs(host_time-wr_time)=" << lag_seconds;
  }

  return std::make_tuple(lvl, ss.str());
}
