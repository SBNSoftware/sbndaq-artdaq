#include "sbndaq-artdaq/Generators/SBND/SPECTDC/SPECTDC_Interface.hh"
#define TRACE_NAME "TDCChanCfg"
#include "sbndaq-artdaq/Generators/SBND/SPECTDC/StringLiterals.hh"

// clang-format off
#include "fmctdc-lib/fmctdc-lib.h"
#include "fmctdc-lib/fmctdc-lib-private.h"
// clang-format on

using namespace sbndaq::SPECTDCInterface;
namespace lit = sbndaq::literal;

bool TDCChan::configure_termination() {
  auto err = fmctdc_set_termination(fmctdc.tdcdevice, id, terminated ? 1 : 0);
  if (err) {
    TLOG(TLVL_ERROR) << "Cannot set the 50 Ohm termination for channel=" << int{id} << " to "
                     << (terminated ? lit::termination_on : lit::termination_off) << ".";
    return false;
  }

  auto status = fmctdc_get_termination(fmctdc.tdcdevice, id);
  if (status < 0) {
    TLOG(TLVL_ERROR) << "Cannot get the 50 Ohm termination status of channel=" << int{id} << ".";
    return false;
  }

  TLOG(TLVL_INFO) << "Channel " << int{id} << ": 50 Ohm termination is "
                  << (status == 1 ? lit::termination_on : lit::termination_off) << ".";

  return true;
}

bool TDCChan::configure_timeoffset() {
  auto err = fmctdc_set_offset_user(fmctdc.tdcdevice, id, time_offset_ps);
  if (err) {
    TLOG(TLVL_ERROR) << "Cannot set the time offset for channel=" << int{id} << " to " << time_offset_ps << " ps.";
    return false;
  }
  decltype(time_offset_ps) offset;

  err = fmctdc_get_offset_user(fmctdc.tdcdevice, id, &offset);
  if (err) {
    TLOG(TLVL_ERROR) << "Cannot get the time offset for channel=" << int{id} << ".";
    return false;
  }

  TLOG(TLVL_INFO) << "Channel " << int{id} << ": time offset is " << offset << " ps.";

  return true;
}

bool TDCChan::configure_buffer_mode() {
  auto err = fmctdc_set_buffer_mode(
      fmctdc.tdcdevice, id, (buffer_mode == chan_buffer_mode_t::circular ? FMCTDC_BUFFER_CIRC : FMCTDC_BUFFER_FIFO));
  if (err) {
    TLOG(TLVL_ERROR) << "Cannot set the FIFO/circular buffer mode for channel=" << id << " to "
                     << (buffer_mode == chan_buffer_mode_t::fifo ? lit::buffer_mode_fifo : lit::buffer_mode_circular);
    return false;
  }

  auto status = fmctdc_get_buffer_mode(fmctdc.tdcdevice, id);
  if (status < 0) {
    TLOG(TLVL_ERROR) << "Cannot get the FIFO/circular buffer mode for channel=" << id << ".";
    return false;
  }

  TLOG(TLVL_INFO) << "Channel " << int{id} << ": buffer mode is "
                  << (status == 0 ? lit::buffer_mode_fifo : lit::buffer_mode_circular) << ".";

  return true;
}

bool TDCChan::configure_buffer_length() {
  auto err = fmctdc_set_buffer_len(fmctdc.tdcdevice, id, buffer_length);
  if (err) {
    TLOG(TLVL_ERROR) << "Cannot set the buffer length for channel=" << int{id} << " to " << buffer_length
                     << " samples.";
    return false;
  }

  auto status = fmctdc_get_buffer_len(fmctdc.tdcdevice, id);
  if (status < 0) {
    TLOG(TLVL_ERROR) << "Cannot get the buffer length for channel=" << int{id} << ".";
    return false;
  }

  TLOG(TLVL_INFO) << "Channel " << int{id} << ": buffer length is " << status << " samples.";
  return true;
}

bool TDCChan::configure_flush_buffer() {
  auto err = fmctdc_flush(fmctdc.tdcdevice, id);
  if (err) {
    TLOG(TLVL_ERROR) << "Cannot flush data buffers for channel=" << int{id} << ".";
    return false;
  }

  TLOG(TLVL_INFO) << "Flushed data buffers for channel=" << int{id} << ".";

  return true;
}

bool TDCChan::configure_channel_status() {
  auto err = fmctdc_channel_status_set(fmctdc.tdcdevice, id, enabled ? FMCTDC_STATUS_ENABLE : FMCTDC_STATUS_DISABLE);
  if (err) {
    TLOG(TLVL_ERROR) << "Cannot set the status for  channel=" << int{id} << " to "
                     << (enabled ? lit::enabled : lit::disabled) << ".";
    return false;
  }

  auto status = fmctdc_channel_status_get(fmctdc.tdcdevice, id);
  if (status < 0) {
    TLOG(TLVL_ERROR) << "Cannot read the enabled/disabled status for channel=" << int{id} << ".";
    return false;
  }

  TLOG(TLVL_INFO) << "Channel " << int{id} << ": status is " << (status == 0 ? lit::disabled : lit::enabled) << ".";
  return true;
}

bool TDCChan::configure_channel_disable() {
  auto err = fmctdc_channel_disable(fmctdc.tdcdevice, id);
  if (err) {
    TLOG(TLVL_ERROR) << "Cannot set the status for  channel=" << int{id} << " to disabled.";
    return false;
  }

  auto status = fmctdc_channel_status_get(fmctdc.tdcdevice, id);
  if (status < 0) {
    TLOG(TLVL_ERROR) << "Cannot read the enabled/disabled status for channel=" << int{id} << ".";
    return false;
  }

  TLOG(TLVL_INFO) << "Channel " << int{id} << ": status is " << (status == 0 ? lit::disabled : lit::enabled) << ".";
  return true;
}

