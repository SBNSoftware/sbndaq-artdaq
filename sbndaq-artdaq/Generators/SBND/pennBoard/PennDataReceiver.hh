/*
 * PennDataReceiver.hh
 *
 *  Created on: Dec 15, 2014
 *      Author: tdealtry (based on tcn45 rce code)
 */

#ifndef PENNDATARECEIVER_HH_
#define PENNDATARECEIVER_HH_

#include <memory>
#include <thread>
#include <atomic>
#include <chrono>
#include <ctime>
#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <boost/thread.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>

#include "SafeQueue.hh"
#include "sbndaq-artdaq/Generators/SBND/pennBoard/PennRawBuffer.hh"
#include "sbndaq-artdaq-core/Overlays/SBND/PennMicroSlice.hh"
#include "sbndaq-artdaq/Generators/SBND/pennBoard/PennCompileOptions.hh"
#include "messagefacility/MessageLogger/MessageLogger.h"

using boost::asio::ip::tcp;

namespace sbnddaq {

class PennDataReceiver {

public:


    struct Word_warning {
      typedef uint32_t data_t;
      typedef uint16_t data_size_t;

      typedef uint8_t  warning_type_t;
      typedef uint8_t  data_packet_type_t;
      typedef uint32_t short_nova_timestamp_t;

      // The order of the data packet type and the timestamp have been
      // swapped to reflect that it's the MOST significant three bits in
      // the payload header which contain the type. I've also added a
      // 1-bit pad to reflect that the least significant bit is unused.

      uint32_t padding : 24;
      warning_type_t warning_type : 5;
      data_packet_type_t     data_packet_type     : 3;

      static size_t const size_words = sizeof(data_t);
      static data_size_t const num_bits_padding     = 24;
      static data_size_t const num_bits_warning  = 5;
      static data_size_t const num_bits_packet_type = 3;
    };


  PennDataReceiver(int debug_level, uint32_t tick_period_usecs, uint16_t udp_receive_port, uint32_t millislice_size, uint16_t millislice_overlap_size, bool rate_test);
	virtual ~PennDataReceiver();

	void start();
	void stop();

	void commit_empty_buffer(PennRawBufferPtr& buffer);
	size_t empty_buffers_available(void);
	size_t filled_buffers_available(void);
	bool retrieve_filled_buffer(PennRawBufferPtr& buffer, unsigned int timeout_us=0);
	void release_empty_buffers(void);
	void release_filled_buffers(void);

  bool exception() const { return exception_.load(); }   // GBcopy

  void set_stop_delay(uint32_t delay_us) {sleep_on_stop_ = delay_us;};
  uint32_t get_stop_delay() {return sleep_on_stop_;};

private:

	enum DeadlineIoObject { None, Acceptor, DataSocket };

	void validate_microslice_header(void);
	void validate_microslice_payload(void);

	void run_service(void);
	void do_accept(void);
	void do_read(void);
	void handle_received_data(std::size_t length);
	void suspend_readout(bool await_restart);

	void set_deadline(DeadlineIoObject io_object, unsigned int timeout_us);
	void check_deadline(void);
    void set_exception( bool exception ) { exception_.store( exception ); }  // GBcopy

	int debug_level_;

	/// -- Ethernet connection management variables
	boost::asio::io_service io_service_;
	tcp::acceptor           acceptor_;
	tcp::socket	        accept_socket_;
	tcp::socket		data_socket_;

	boost::asio::deadline_timer deadline_;
	DeadlineIoObject deadline_io_object_;
	uint32_t tick_period_usecs_;

	// Port that should be listened for connection from the PTB (conf param)
	uint16_t receive_port_;
	// Size of the millislice (units?)
  uint32_t millislice_size_;

	std::atomic<bool> run_receiver_;
	std::atomic<bool> suspend_readout_;
	std::atomic<bool> readout_suspended_;
        std::atomic<bool> exception_;    // GBcopy

	int recv_socket_;

	std::unique_ptr<std::thread> receiver_thread_;

	SafeQueue<sbnddaq::PennRawBufferPtr> empty_buffer_queue_;
	SafeQueue<sbnddaq::PennRawBufferPtr> filled_buffer_queue_;
	PennRawBufferPtr current_raw_buffer_;
	void*            current_write_ptr_;
  void*            tmp_write_ptr_;

  void*            receiver_state_start_ptr_;
  std::size_t      state_nbytes_recvd_;

  enum NextReceiveState { ReceiveMicrosliceHeader, ReceiveMicroslicePayload, ReceiveMicroslicePayloadHeader, ReceiveMicroslicePayloadCounter, ReceiveMicroslicePayloadTrigger, ReceiveMicroslicePayloadTimestamp };
  std::string nextReceiveStateToString(NextReceiveState val);
  std::vector<std::string> const next_receive_state_names_ 
  { "ReceiveMicrosliceHeader", "ReceiveMicroslicePayload", "ReceiveMicroslicePayloadHeader", "ReceiveMicroslicePayloadCounter", "ReceiveMicroslicePayloadTrigger", "ReceiveMicroslicePayloadTimestamp" };
  NextReceiveState next_receive_state_;
  size_t           next_receive_size_;
  size_t           nextReceiveStateToExpectedBytes(NextReceiveState val);

  enum MillisliceState { MillisliceEmpty, MillisliceIncomplete, MicrosliceIncomplete, MillisliceComplete };
  std::string millisliceStateToString(MillisliceState val);
  std::vector<std::string> const millislice_state_names_ 
  { "MillisliceEmpty", "MillisliceIncomplete", "MicrosliceIncomplete", "MillisliceComplete" };
  MillisliceState  millislice_state_;
  size_t           millislice_size_recvd_;
  uint32_t         microslices_recvd_;
  uint32_t         payloads_recvd_;
  uint32_t         payloads_recvd_counter_;
  uint32_t         payloads_recvd_trigger_;
  uint32_t         payloads_recvd_timestamp_;
  uint32_t         payloads_recvd_warning_;
  uint32_t         payloads_recvd_checksum_;
  sbnddaq::PennMicroSlice::Header::block_size_t microslice_size_;
  size_t           microslice_size_recvd_;
  uint32_t         millislices_recvd_;
  
  // FIXME: NFB - This might not be needed
  uint32_t microslices_recvd_timestamp_; //counts the number of microslices received that contain a timestamp word
  bool microslice_seen_timestamp_word_;
  bool last_microslice_was_fragment_;

  bool                                           microslice_version_initialised_;
  sbnddaq::PennMicroSlice::Header::format_version_t microslice_version_;
  bool                                           sequence_id_initialised_;
  sbnddaq::PennMicroSlice::Header::sequence_id_t    last_sequence_id_;
  
  std::chrono::high_resolution_clock::time_point start_time_;
  
  bool rate_test_;

  size_t           overlap_size_;
  static const int overlap_buffer_size_ = 65536;
  uint8_t          overlap_ptr_[sbnddaq::PennDataReceiver::overlap_buffer_size_];
  uint16_t         millislice_overlap_size_;

  sbnddaq::PennMicroSlice::sample_count_t overlap_payloads_recvd_;
  sbnddaq::PennMicroSlice::sample_count_t overlap_payloads_recvd_counter_;
  sbnddaq::PennMicroSlice::sample_count_t overlap_payloads_recvd_trigger_;
  sbnddaq::PennMicroSlice::sample_count_t overlap_payloads_recvd_timestamp_;
  sbnddaq::PennMicroSlice::sample_count_t overlap_payloads_recvd_warning_;
  sbnddaq::PennMicroSlice::sample_count_t overlap_payloads_recvd_checksum_;

  size_t           current_microslice_;
  static const int current_microslice_buffer_size_ = 65536;
//  uint8_t          current_microslice_ptr_[sbnddaq::PennDataReceiver::current_microslice_buffer_size_];

  size_t           remaining_size_;
  static const int remaining_buffer_size = 65536;
  uint8_t          remaining_ptr_[sbnddaq::PennDataReceiver::remaining_buffer_size];

  sbnddaq::PennMicroSlice::sample_count_t remaining_payloads_recvd_;
  sbnddaq::PennMicroSlice::sample_count_t remaining_payloads_recvd_counter_;
  sbnddaq::PennMicroSlice::sample_count_t remaining_payloads_recvd_trigger_;
  sbnddaq::PennMicroSlice::sample_count_t remaining_payloads_recvd_timestamp_;
  sbnddaq::PennMicroSlice::sample_count_t remaining_payloads_recvd_warning_;
  sbnddaq::PennMicroSlice::sample_count_t remaining_payloads_recvd_checksum_;

  uint64_t boundary_time_;    //unit is 64MHz NOvA clock ticks
  uint64_t run_start_time_;   //unit is 64MHz NOvA clock ticks

  uint64_t overlap_time_; //unit is 64MHz NOvA clock ticks

  uint32_t sleep_on_stop_; // time (us) to sleep before stopping

};


} /* namespace sbnddaq */

#endif /* PENNDATARECEIVER_HH_ */
