#ifndef sbndaq_artdaq_Generators_SPECTDC_StringLiterals_hh
#define sbndaq_artdaq_Generators_SPECTDC_StringLiterals_hh
namespace sbndaq {
namespace literal {
constexpr auto id = "id";
constexpr auto enabled = "enabled";
constexpr auto disabled = "disabled";
constexpr auto termination_on = "on";
constexpr auto termination_off = "off";
constexpr auto terminated = "terminated";
constexpr auto monitor_only = "monitor_only";
constexpr auto time_offset_ps = "time_offset_ps";
constexpr auto buffer_length = "buffer_length";
constexpr auto name = "name";
constexpr auto buffer_mode = "buffer_mode";
constexpr auto buffer_mode_fifo = "FIFO";
constexpr auto buffer_mode_circular = "circular";
constexpr auto tdc_sample_time_lag = "Sample Time Lag";
constexpr auto tdc_laggy_samples = "Laggy Samples";
constexpr auto tdc_sequence_gap = "Sequence Gap";
constexpr auto tdc_sample_rate = "Sample Rate";
constexpr auto tdc_bytes_read = "Sample Bytes Read";
constexpr auto tdc_read_count = "Sample Read Total";
constexpr auto tdc_drain_count = "Sample Drained Total";
constexpr auto tdc_missed_count = "Sample Missed Total";
constexpr auto tdc_dropped_count = "Sample Dropped Total";
constexpr auto tdc_last_sequence = "Sample Last Sequence";
constexpr auto unit_samples_per_second = "samples";
constexpr auto unit_sample_count = "sample count";
constexpr auto unit_nanoseconds = "ns";
constexpr auto unit_bytes = "bytes";
constexpr auto monitor = "monitor";
constexpr auto timestamps = "timestamps";
constexpr auto temperature = "temperature";
constexpr auto channels = "channels";
constexpr auto calibration_data = "calibration_data";
constexpr auto nofile = "none";
constexpr auto autocalib = "auto";
constexpr auto time_adjustment = "time_adjustment";
constexpr auto timesync_source = "timesync_source";
constexpr auto timesync_host = "host";
constexpr auto timesync_wr = "wr";
constexpr auto blocking_reads = "blocking_reads";
constexpr auto polltime_ms = "polltime_ms";
constexpr auto monitor_temperature = "SPEC Temperature";
constexpr auto monitor_time_lag = "SPEC Time Delta";
constexpr auto monitor_wr_synced = "WR Time Synced";
constexpr auto unit_temperature = "C";
constexpr auto unit_seconds = "seconds";
constexpr auto unit_bool = "bool";
constexpr auto deviceid = "deviceid";
constexpr auto verbose = "verbose";
constexpr auto fmctdc = "fmctdc";
constexpr auto autodetect = "auto";
constexpr auto rc_success = "success";
constexpr auto rc_failure = "failure";
constexpr auto add_leap_seconds = "add_leap_seconds";
constexpr auto next_leap_second_tai = "next_leap_second_tai";
constexpr auto make_adjustment = "make_adjustment";
}  // namespace literal
}  // namespace sbndaq

#endif  // sbndaq_artdaq_Generators_SPECTDC_StringLiterals_hh
