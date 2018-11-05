/*
 * PennDataReceiver.cpp - PENN data recevier classed based on the Boost asynchronous IO network library
 *
 *  Created on: Dec 15, 2014
 *      Author: tdealtry (based on tcn45 rce code)
 */

#include "PennDataReceiver.hh"

#include <iostream>
#include <unistd.h>
#include <stdexcept>
#include <bitset>
#include <boost/crc.hpp>

#include "sbndaq-artdaq-core/Overlays/SBND/PennMicroSlice.hh"
#include "sbndaq-artdaq-core/Overlays/SBND/PennMilliSlice.hh"

#include "sbndaq-artdaq-core/Overlays/Common/Utilities.hh"

//#define __PTB_BOARD_READER_DEVEL_MODE__


// Lower level means more verbosity
#define RECV_DEBUG(level) if (level <= debug_level_) mf::LogInfo("PennDataReceiver")


sbnddaq::PennDataReceiver::PennDataReceiver(int debug_level, uint32_t tick_period_usecs,
    uint16_t receive_port, uint32_t millislice_size,
    uint16_t millislice_overlap_size, bool rate_test ) :
    debug_level_(debug_level),
    acceptor_(io_service_, tcp::endpoint(tcp::v4(), (short)receive_port)),
    accept_socket_(io_service_),
    data_socket_(io_service_),
    deadline_(io_service_),
    deadline_io_object_(None),
    tick_period_usecs_(tick_period_usecs),
    receive_port_(receive_port),
    millislice_size_(millislice_size),
    run_receiver_(true),
    suspend_readout_(false),
    readout_suspended_(false),
    exception_(false),  // GBcopy
    recv_socket_(0),
    rate_test_(rate_test),
    millislice_overlap_size_(millislice_overlap_size),
    run_start_time_(0)
{
  

  RECV_DEBUG(1) << "sbnddaq::PennDataReceiver constructor";

  //in this instance millislice_size_ is the number of bits that need to rollover to start a new microslice

  // JCF, Jul-29-2015
  // In other sections of the code, it looks as if
  // millislice_size_ was intended to be the length itself, in
  // timestamp units, of a millislice - as opposed to the length
  // being millislice_size_ . Until I figure out what was
  // intended, for now the "pow()" expression below is commented
  // out

//  //	if((pow(millislice_size_, 2) - 1) > sbnddaq::PennMicroSlice::ROLLOVER_LOW_VALUE) {
//  if (millislice_size_ > sbnddaq::PennMicroSlice::ROLLOVER_LOW_VALUE) {
//
//    // JCF, Jul-30-2015
//
//    // I've upgraded this from a warning to an error
//
//    mf::LogError("PennDataReceiver") << "sbnddaq::PennDataReceiver ERROR millislice_size_ " << millislice_size_
//        << " is greater than sbnddaq::PennMicroSlice::ROLLOVER_LOW_VALUE " << (uint32_t)sbnddaq::PennMicroSlice::ROLLOVER_LOW_VALUE
//        << " 28-bit timestamp rollover will not be handled correctly";
//    //TODO handle error cleanly
//  }

  // Initialise and start the persistent deadline actor that handles socket operation
  // timeouts. Timeout is set to zero (no timeout) to start

  this->set_deadline(None, 0);
  this->check_deadline();

  // Start asynchronous accept handler
  this->do_accept();

  // Launch thread to run receiver IO service
  receiver_thread_ = std::unique_ptr<std::thread>(new std::thread(&sbnddaq::PennDataReceiver::run_service, this));
}

sbnddaq::PennDataReceiver::~PennDataReceiver()
{
  RECV_DEBUG(1) << "sbnddaq::PennDataReceiver destructor";

  // Flag receiver as no longer running
  run_receiver_.store(false);

  // Cancel any currently pending IO object deadlines and terminate timer
  deadline_.expires_at(boost::asio::deadline_timer::traits_type::now());

  // Wait for thread running receiver IO service to terminate
  receiver_thread_->join();

  RECV_DEBUG(3) << "sbnddaq::PennDataReceiver destructor: receiver thread joined OK";

}

void sbnddaq::PennDataReceiver::start(void)
{
  RECV_DEBUG(1) << "sbnddaq::PennDataReceiver::start called";

  start_time_ = std::chrono::high_resolution_clock::now();

	// If the data receive socket is open, flush any stale data off it
	if (deadline_io_object_ == DataSocket)
	{

		std::size_t flush_length = 0;

		while (data_socket_.available() > 0)
		{
			boost::array<char, 65536> buf;
			boost::system::error_code ec;

			size_t len = data_socket_.read_some(boost::asio::buffer(buf), ec);
			flush_length += len;
			RECV_DEBUG(3) << "Flushed: " << len << " total: " << flush_length
					<< " available: " << data_socket_.available();

			if (ec == boost::asio::error::eof)
			{
				mf::LogInfo("PennDataReceiver") << "sbnddaq::PennDataReceiver:start: client closed data socket connection during flush operation";
				break;
			}
			else if (ec)
			{
				mf::LogWarning("PennDataReceiver") << "sbnddaq::PennDataReceiver::start: got unexpected socket error flushing data socket: " << ec;
				break;
			}
		}
		if (flush_length > 0)
		{
		  // JCF, Feb-4-2016

		  // I've observed that when the RceDataReceiver has
		  // to flush bytes of stale data it pretty much
		  // guarantees an error later in the run; given that
		  // this code is largely based on the
		  // RceDataReceiver, then, I've promoted this message
		  // from "Info" level to "Warning" level

			mf::LogWarning("PennDataReceiver") << "sbnddaq::PennDataReceiver::start: flushed " << flush_length << " bytes stale data off open socket";
		}
		else
		{
			mf::LogDebug("PennDataReceiver") << "sbnddaq::PennDataReceiver::start: no stale data to flush off socket";
		}
	}

  // Clean up current buffer state of any partially-completed readouts in previous runs
  if (current_raw_buffer_ != nullptr)
  {
    mf::LogInfo("PennDataReceiver") << "::start: dropping unused or partially filled buffer containing "
        << microslices_recvd_ << " microslices";
    current_raw_buffer_.reset();
    millislice_state_ = MillisliceEmpty;
  }

  // Initialise millislice state
  millislice_state_ = MillisliceEmpty;
  millislices_recvd_ = 0;

  // Initialise sequence ID latch
  sequence_id_initialised_ = false;
  last_sequence_id_ = 0;

  // Initialise microslice version latch
  microslice_version_initialised_ = false;
  microslice_version_ = 0;
  microslice_size_recvd_ = 0;

  // Initialise receive state to read a microslice header first
  next_receive_state_ = ReceiveMicrosliceHeader;
  next_receive_size_  = sizeof(sbnddaq::PennMicroSlice::Header);

  RECV_DEBUG(2) << "::start: Next receive state : " << nextReceiveStateToString(ReceiveMicrosliceHeader);

  // Initialise this to make sure we can count number of 'full' microslices
  microslice_seen_timestamp_word_ = false;
  //... and aren't shocked by repeated sequence IDs
  last_microslice_was_fragment_   = false;

  // Initalise to make sure we wait for the full Header
  state_nbytes_recvd_ = 0;
  receiver_state_start_ptr_ = 0;

  // Clear the times used for calculating millislice boundaries
  run_start_time_ = 0;
  boundary_time_  = 0;
  overlap_time_   = 0;

  // Clear the counters used for the remains of split uslices
  remaining_size_ = 0;
  remaining_payloads_recvd_ = 0;
  remaining_payloads_recvd_counter_ = 0;
  remaining_payloads_recvd_trigger_ = 0;
  remaining_payloads_recvd_timestamp_ = 0;
  remaining_payloads_recvd_warning_ = 0;
  remaining_payloads_recvd_checksum_ = 0;

  // Clear the counters used for the overlap period
  overlap_size_ = 0;
  overlap_payloads_recvd_ = 0;
  overlap_payloads_recvd_counter_ = 0;
  overlap_payloads_recvd_trigger_ = 0;
  overlap_payloads_recvd_timestamp_ = 0;
  overlap_payloads_recvd_warning_ = 0;
  overlap_payloads_recvd_checksum_ = 0;

  // Clear suspend readout handshake flags
  suspend_readout_.store(false);
  readout_suspended_.store(false);

}

void sbnddaq::PennDataReceiver::stop(void)
{
  RECV_DEBUG(1) << "sbnddaq::PennDataReceiver::stop called";

  // Suspend readout and wait for receiver thread to respond accordingly
  suspend_readout_.store(true);

//  const uint32_t stop_timeout_usecs = 5000000;
 // uint32_t max_timeout_count = stop_timeout_usecs / tick_period_usecs_;
  uint32_t max_timeout_count = sleep_on_stop_ / tick_period_usecs_;
  uint32_t timeout_count = 0;
  while (!readout_suspended_.load())
  {
    usleep(tick_period_usecs_);
    timeout_count++;
    if (timeout_count > max_timeout_count)
    {
      // GBcopy: In the RCEs, JCF (Oct-24, 2015) recommends we either 
      // downgrade this to a warning or swallow the exception this automatically throws
      try {    // GBcopy
           mf::LogError("PennDataReceiver") << "ERROR - timeout waiting for PennDataReceiver thread to suspend readout";
      } catch (...) {  //GBcopy
      }  // GBcopy
      break;
    }
  }

  auto elapsed_msecs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time_).count();
  double elapsed_secs = ((double)elapsed_msecs) / 1000;
  double rate = ((double)millislices_recvd_) / elapsed_secs;

  RECV_DEBUG(1) << "sbnddaq::PennDataRecevier::stop : last sequence id was " << (unsigned int)last_sequence_id_;

  mf::LogInfo("PennDataReceiver")  << "sbnddaq::PennDataReceiver::stop : received " << millislices_recvd_ << " millislices in "
      << elapsed_secs << " seconds, rate "
      << rate << " Hz";

}

void sbnddaq::PennDataReceiver::commit_empty_buffer(PennRawBufferPtr& buffer)
{
  empty_buffer_queue_.push(std::move(buffer));
}

size_t sbnddaq::PennDataReceiver::empty_buffers_available(void)
{
  return empty_buffer_queue_.size();
}

size_t sbnddaq::PennDataReceiver::filled_buffers_available(void)
{
  return filled_buffer_queue_.size();
}

bool sbnddaq::PennDataReceiver::retrieve_filled_buffer(PennRawBufferPtr& buffer, unsigned int timeout_us)
{
  bool buffer_available = true;

  // If timeout is specified, try to pop the buffer from the queue and time out, otherwise
  // block waiting for the queue to contain a buffer
  if (timeout_us == 0)
  {
    buffer = filled_buffer_queue_.pop();
  }
  else
  {
    buffer_available = filled_buffer_queue_.try_pop(buffer, std::chrono::microseconds(timeout_us));
  }

  return buffer_available;
}

void sbnddaq::PennDataReceiver::release_empty_buffers(void)
{

  while (empty_buffer_queue_.size() > 0)
  {
    empty_buffer_queue_.pop();
  }
}

void sbnddaq::PennDataReceiver::release_filled_buffers(void)
{

  if (filled_buffer_queue_.size() > 0) {
    mf::LogWarning("PennDataReceiver") << "Releasing " << filled_buffer_queue_.size() << " filled buffers";
  }
  while (filled_buffer_queue_.size() > 0)
  {
    filled_buffer_queue_.pop();
  }
}

void sbnddaq::PennDataReceiver::run_service(void)
{
  RECV_DEBUG(1) << "sbnddaq::PennDataReceiver::run_service starting";

  io_service_.run();

// GBcopy:   Make this a warning if we are not officially stopping the run
  if (suspend_readout_.load())  // Normal situation, it should stop after a bit when we stop the run
  {
     RECV_DEBUG(1) << "sbnddaq::PennDataReceiver::run_service stopping";
  } else {
     mf::LogWarning("PennDataReceiver") << "boost::asio::io_service has stopped running mysteriously inside the run.  Continuing the run from here is probably not useful.";
  }
}

// NFB Dec-02-2015
//
//do_accept simply keeps track of accepting the ethernet connection from the PTB
// Then redirects the handling of the received data to do_read
void sbnddaq::PennDataReceiver::do_accept(void)
{
  RECV_DEBUG(5) << "sbnddaq::PennDataReceiver::do_accept starting";

  // JCF, Jul-29-2015

  // The "Timeout on async_accept" message appears many, many times --
  // O(1000) -- before the connection is made; to reduce clutter, I'm
  // only going to have it display every nth_timeout times

  const size_t print_nth_timeout = 200;
  static size_t nth_timeout = 0;

  // Suspend readout and cleanup any incomplete millislices if stop has been called
  if (suspend_readout_.load())
  {
    RECV_DEBUG(4) << "Suspending readout at do_accept entry";
    this->suspend_readout(false);
  }

  // Exit if shutting receiver down
  if (!run_receiver_.load()) {
    RECV_DEBUG(4) << "Stopping do_accept() at entry";
    return;
  }

  this->set_deadline(Acceptor, tick_period_usecs_);

  acceptor_.async_accept(accept_socket_,
      [this](boost::system::error_code ec)
      {
    if (!ec)
    {
      RECV_DEBUG(1) << "Accepted new data connection from source " << accept_socket_.remote_endpoint();
      data_socket_ = std::move(accept_socket_);
      this->do_read();
    }
    else
    {
      if (ec == boost::asio::error::operation_aborted)
      {
        if (nth_timeout % print_nth_timeout == 0) {
          RECV_DEBUG(4) << "Timeout on async_accept";
        }

        nth_timeout++;
      }
      else
      {
	try {
	  mf::LogError("PennDataReceiver") << "Got error on asynchronous accept: " << ec << " " << ec.message();
	} catch (...) {
	  set_exception(true);
	}
      }
      this->do_accept();
    }
      }
  );
}

// NFB Dec-02-2015

// do_read handles the data received from the PTB, casts them into RawBuffer,
// converts them into MicroSlices and
// and pushes them into millislices
void sbnddaq::PennDataReceiver::do_read(void)
{

  // Suspend readout and cleanup any incomplete millislices if stop has been called
  if (suspend_readout_.load())
  {
    RECV_DEBUG(3) << "Suspending readout at do_read entry";
    this->suspend_readout(true);
  }

  // Terminate receiver read loop if required
  if (!run_receiver_.load())
  {
    RECV_DEBUG(1) << "Stopping do_read at entry";
    return;
  }

  // Set timeout on read from data socket
  this->set_deadline(DataSocket, tick_period_usecs_);

  if (millislice_state_ == MillisliceEmpty)
  {
    // NFB Dec-2-2015
    //
    // The code below is ambiguous when it fails. It is impossible to know whether
    // it failed because there are no empty buffers available
    // or whether a lock on the queue has not been released.
    // Changed the code to discern the different problems

// OLD_DOWHILE should be set off so we use the first bit of code here, but the old stuff 
// is still in the second bit incase we need to go back.  The new stuff is copied from the RCE
#ifndef OLD_DOWHILE
// This new code causes the run to stop in a gentle way if there is a memory allocation problem, so that the root file can be closed properly
    
    // Attempt to obtain a raw buffer to receive data into
    unsigned int buffer_retries = 0;
    bool buffer_available;
    const unsigned int max_retries = 100;     // Used in the new code only
    const unsigned int buffer_retry_report_interval = 10;  // Used in the new code only

    do
    {
        buffer_available = empty_buffer_queue_.try_pop(current_raw_buffer_,std::chrono::milliseconds(100));
        if (!buffer_available)
        {
           if ((buffer_retries > 0) && ((buffer_retries % buffer_retry_report_interval) == 0)) {
            mf::LogWarning("PennDataReceiver") << "sbnddaq::PennDataReceiver::receiverLoop: no buffers available on commit queue due to more data entering the PennDataReceiver than leaving it, retrying ("
              << buffer_retries << " attempts so far)";
           }
           buffer_retries++;
	   
           if (buffer_retries > max_retries) {
              if (!suspend_readout_.load()) {
                 if (!exception_.load() ) {
                   try {
                       mf::LogError("PennDataReceiver") << "sbnddaq::PennDataReceiver::receiverLoop: too many buffer retries, signalling an exception";
                   } catch (...) {
                     set_exception(true);
                     // FIXME: This return might be causing troubles.
                     return;
                   }
                 }
              }
           }
        } else {
           if (buffer_retries > buffer_retry_report_interval) {
              RECV_DEBUG(1) << "sbnddaq::PennDataReceiver::receiverLoop: obtained new buffer after " << buffer_retries << " retries";
           }
        }

        // Check if readout is being suspended/terminated and handle accordingly
        if (suspend_readout_.load()) {
           mf::LogInfo("PennDataReceiver") << "Suspending readout during buffer retry loop";
           this->suspend_readout(true);

           // When we come out of suspended at this point, the main thread will 
           // have invalidated the current buffer, so set
           // buffer_available to false so that we get a fresh one for the next read
           buffer_available = false;
        }

        // Terminate receiver read loop if required
        if (!run_receiver_.load()) {
           RECV_DEBUG(1) << "Terminating receiver thread in buffer retry loop";
           return;    // Returning from do_read() without queueing another read operation will cause the boost::asio::io_service to return, which terminates the thread.
        }
    } while (!buffer_available);

#else
// This is the older code, which we hopefully won't want to go back to when the new stuff above works

    // Attempt to obtain a raw buffer to receive data into
    unsigned int buffer_retries = 0;
    bool buffer_available;
    const unsigned int max_buffer_retries = 1000;   // Used in the old code only

    do
    {
      // Try to grab an empty buffer to work on with 1s timeout
      buffer_available = empty_buffer_queue_.try_pop(current_raw_buffer_, std::chrono::milliseconds(1000));
      if (!buffer_available)
      {
        buffer_retries++;
        try {
          mf::LogError("PennDataReceiver") << "sbnddaq::PennDataReceiver::receiverLoop no buffers available on commit queue (buffers available : " << empty_buffers_available() << ")";
        } catch(mf::LogError::ExceptClass &e) {
          // do nothing. Just don't let LogError stop the retries
          continue;
        }
      }
    } while (!buffer_available && (buffer_retries < max_buffer_retries));
#endif

    // There is a buffer available. Therefore, regardless of its contents
    // we now have a Millislice. Probably incomplete, but a Millislice nonetheless
    if (buffer_available)
    {
      //RECV_DEBUG(4) << "::do_read::Received raw buffer at " << (void*)current_raw_buffer_;
      millislice_state_ = MillisliceIncomplete;
      millislice_size_recvd_ = 0;
      microslices_recvd_ = 0;
      microslices_recvd_timestamp_ = 0;
      payloads_recvd_ = 0;
      payloads_recvd_counter_ = 0;
      payloads_recvd_trigger_ = 0;
      payloads_recvd_timestamp_ = 0;
      payloads_recvd_warning_  = 0;
      payloads_recvd_checksum_  = 0;

      // Set the write pointer to the body of the raw buffer.
      // This is the location where data is being written.
      current_write_ptr_ = (void*)(current_raw_buffer_->dataPtr());
      RECV_DEBUG(2) << "Receiving new millislice into raw buffer at address " << current_write_ptr_;

      //add the overlap period with the previous millislice
      // to the start of this millislice
      if(overlap_size_) {
        //move the overlap period to the start of the new millislice
        memmove(current_write_ptr_, overlap_ptr_, overlap_size_);
        current_write_ptr_ = (void*)((uint8_t*)current_write_ptr_ + overlap_size_);
        // NFB : Dec-02-2015
        // I would have thought that the checksum was not passed into the microslice
        // The checksum received from the PTB should be discarded when the buffer is converted into
        // a PennRawBuffer
        // The same would happen for selftests, which should be handled directly at the board
        RECV_DEBUG(2) << "Overlap period of " << overlap_size_ << " bytes added to this millislice. "
            << "Payload contains "  << overlap_payloads_recvd_
            << " total words ("     << overlap_payloads_recvd_counter_
            << " counter + "        << overlap_payloads_recvd_trigger_
            << " trigger + "        << overlap_payloads_recvd_timestamp_
            << " timestamp + "      << overlap_payloads_recvd_warning_
            << " warning+ "       << overlap_payloads_recvd_checksum_
            << "checksum)";

        //increment size & payload counters
        millislice_size_recvd_    += overlap_size_;
        payloads_recvd_           += overlap_payloads_recvd_;
        payloads_recvd_counter_   += overlap_payloads_recvd_counter_;
        payloads_recvd_trigger_   += overlap_payloads_recvd_trigger_;
        payloads_recvd_timestamp_ += overlap_payloads_recvd_timestamp_;
        // These should never be incremented!!
        // the selftests are now caught at the board level and the checksums
        // are supposed to be filtered out when collected
        payloads_recvd_warning_  += overlap_payloads_recvd_warning_;
        payloads_recvd_checksum_  += overlap_payloads_recvd_checksum_;
        //reset 'overlap' counters
        overlap_size_                     = 0;
        overlap_payloads_recvd_           = 0;
        overlap_payloads_recvd_counter_   = 0;
        overlap_payloads_recvd_trigger_   = 0;
        overlap_payloads_recvd_timestamp_ = 0;
        overlap_payloads_recvd_warning_  = 0;
        overlap_payloads_recvd_checksum_  = 0;
      }

      //add any remains of the previous microslice (due to millislice boundary
      // occurring within it) to the start of this millislice
      if(remaining_size_) {
        memmove(current_write_ptr_, remaining_ptr_, remaining_size_);
        current_write_ptr_ = (void*)((uint8_t*)current_write_ptr_ + remaining_size_);
        RECV_DEBUG(2) << "Added the last "   << remaining_size_ << " bytes of previous microslice to this millislice. "
            << "Payload contains " << remaining_payloads_recvd_
            << " total words ("    << remaining_payloads_recvd_counter_
            << " counter + "       << remaining_payloads_recvd_trigger_
            << " trigger + "       << remaining_payloads_recvd_timestamp_
            << " timestamp + "     << remaining_payloads_recvd_warning_
            << " warning + "      << remaining_payloads_recvd_checksum_
            << "checksum)";
        //increment size & payload counters
        millislice_size_recvd_    += remaining_size_;
        payloads_recvd_           += remaining_payloads_recvd_;
        payloads_recvd_counter_   += remaining_payloads_recvd_counter_;
        payloads_recvd_trigger_   += remaining_payloads_recvd_trigger_;
        payloads_recvd_timestamp_ += remaining_payloads_recvd_timestamp_;
        // This should not go into the millislice
        payloads_recvd_warning_  += remaining_payloads_recvd_warning_;
        payloads_recvd_checksum_  += remaining_payloads_recvd_checksum_;
        //reset 'remaining' counters
        remaining_size_                     = 0;
        remaining_payloads_recvd_           = 0;
        remaining_payloads_recvd_counter_   = 0;
        remaining_payloads_recvd_trigger_   = 0;
        remaining_payloads_recvd_timestamp_ = 0;
        remaining_payloads_recvd_warning_  = 0;
        remaining_payloads_recvd_checksum_  = 0;
      }
    }
    else // else... if buffer_available
    {    // If we are using the new DOWHILE above, this clause should never be 
         //possible, but leave it here for now
      // There is no clean way to handle unavailable memory
      try {
       mf::LogError("PennDataReceiver") << "Failed to obtain new raw buffer for millislice, terminating receiver loop";
      } catch (...) {
        // Swallow the exception here, but if we are running, set exception_ flag so getNext will 
        // know to stop the run gracefully
        if (! suspend_readout_.load()) {
          mf::LogInfo("PennDataReceiver") << "Setting exception to true; suspend_readout is false";
          set_exception(true);
        }
      }

      return;
    }

  } // millislice_state_ == MillisliceEmpty
  /// Overlaps and remainings are now dealt with.
  /// -- Process the new data

  // NFB Dec-06-2015
  // There are only two types of data loaded from the socket: headers and payloads
  RECV_DEBUG(5) << "\nreceive state "     << (unsigned int)next_receive_state_
                << " " << nextReceiveStateToString(next_receive_state_)
                << "\nmslice state "   << (unsigned int)millislice_state_
                << " " << millisliceStateToString(millislice_state_)
                << "\nuslices received "         << microslices_recvd_
                << "\nuslice size received "    << microslice_size_recvd_
                << "\nmslice size received "    << millislice_size_recvd_
                << "\ncurrent write ptr "           << current_write_ptr_
                << "\nnext recv size " << next_receive_size_;

  // The data should not go
  // Start the asynchronous receive operation into the (existing) current raw buffer.

  // NFB -- Jan, 14 2016
  // According to boost documentation:
  /**
   * The receive operation may not receive all of the requested number of bytes.
   * Consider using the async_read function if you need to ensure that the requested
   * amount of data is received before the asynchronous operation completes.
   */
  //data_socket_.async_receive(
  boost::asio::async_read(data_socket_,
			  boost::asio::buffer(current_write_ptr_, next_receive_size_),
			  [this](boost::system::error_code ec, std::size_t length)
			  {
    if (!ec)
    {
      RECV_DEBUG(4) << "Received " << length << " bytes on socket";

      this->handle_received_data(length);

      this->do_read();
    }
    else
    {
      if (ec == boost::asio::error::eof)
      {
        mf::LogInfo("PennDataReceiver") << "Client socket closed the connection";
        this->do_accept();
      }
      else if (ec == boost::asio::error::operation_aborted)
      {
        // NFB : Shoudln't this be a warning?  GDB: I think this is OK, this is the
        // usual thing to do if no data arrives in a certain time, which should
        // happen when we are waiting for the run to start, and if we are 
        // checking often enough during the run.
        RECV_DEBUG(5) << "Timeout on read from data socket";
        this->do_read();
      }
      else
      {
	try {
	  mf::LogError("PennDataReceiver") << "Got error on aysnchronous read from the socket: " << ec << " " << ec.message();
	} catch (...) {
	  set_exception(true);
	}
      }

    }
      }
  );
}

void sbnddaq::PennDataReceiver::handle_received_data(std::size_t length)
{

  RECV_DEBUG(2) << "sbnddaq::PennDataReceiver::handle_received_data: Handling "
		<< " data with size " << (unsigned int)length << " at status " << nextReceiveStateToString(next_receive_state_);

#ifdef __PTB_BOARD_READER_DEVEL_MODE__
  display_bits(current_write_ptr_, length, "PennDataReceiver");
#endif

  // The way this is working is by stitching a microslice and then roll back the
  // pointers to subtract stuff that should not be going there.
  // The problem with this approach is that it might fail if
  // there are packets that should not be sent inside the microslice.

  //update size of uslice component, uslice & mslice
  state_nbytes_recvd_    += length;
  // We should handle this in a way a bit more special
  microslice_size_recvd_ += length;
  millislice_size_recvd_ += length;



  //make pointer to the start of the current object (Header, Payload_Header, Payload*)
  if(!receiver_state_start_ptr_)
    receiver_state_start_ptr_ = current_write_ptr_;

  //now we can update the current_write_ptr_
  // Set it to the end of the received data
  current_write_ptr_ = static_cast<void*>(sbndaq::reinterpret_cast_checked<uint8_t*>(current_write_ptr_) + length );

  //check to see if we have all the bytes for the current object
  std::size_t nbytes_expected = nextReceiveStateToExpectedBytes(next_receive_state_);
  if(state_nbytes_recvd_ < nbytes_expected) {
    millislice_state_   = MicrosliceIncomplete;
    //next_receive_state_ = //already set correctly
    next_receive_size_ = nbytes_expected - state_nbytes_recvd_;

    // JCF, Jul-30-2015

    // I upgraded this circumstance to an error. Microslices should never be incomplete.
    // The TCP stack takes care of it.

    // JCF, Jan-6-2015

    // I've downgraded it back to a warning- there's no guarantee you
    // get all the bytes necessarily in one shot. However, I should
    // now implement a more sophisticated test which throws an
    // exception if we don't ULTIMATELY get all the bytes

    // NFB, Jan-14-2016

    // I've upgraded back to an error. Replacing boost::async_receive by boost::async_read
    // should force the right amount of bytes to be collected from the socket.

    try{
    mf::LogError("PennDataReceiver") << "Incomplete " << nextReceiveStateToString(next_receive_state_)
						      << " received for microslice " << microslices_recvd_
						      << " (got " << state_nbytes_recvd_
						      << " bytes, expected " << nbytes_expected << ")";
    } catch(...) {
      set_exception(true);
    }
    return;
  }

  // JCF, Jul-28-2015

  // I've replaced Tom Dealtry's use of the
  // boost::crc_32_type calculation for the checksum with
  // Nuno's BSD method
  // (https://en.wikipedia.org/wiki/BSD_checksum),
  // implemented in the ptb_runner program

  // Note this code relies on the only two possible receive
  // states being "ReceiveMicrosliceHeader" and
  // "ReceiveMicroslicePayload"

#ifdef DO_CHECKSUM
  static uint16_t software_checksum = 0; // "static" means this
#endif /*DO_CHECKSUM*/
  // is the same variable
  // across calls to this
  // function

  size_t bytes_to_check = 0;

  // Reset the checksum to zero when we're expecting a new
  // microslice, and if we're looking at the microslice payload,
  // don't factor the contents of the checksum word into the
  // checksum itself
  if (next_receive_state_ == ReceiveMicrosliceHeader) {
#ifdef DO_CHECKSUM
    software_checksum = 0;
#endif /*DO_CHECKSUM*/
    bytes_to_check = length;
  } else {
    // If the next state is the payload we want to discount the header from the size to be checked
    bytes_to_check = length - sizeof(sbnddaq::PennMicroSlice::Header);
  }

  RECV_DEBUG(2) << "Calculating checksum with status " << nextReceiveStateToString(next_receive_state_)
					      << " on length  " << length
					      << " bytes_to_check " << bytes_to_check;


#ifdef DO_CHECKSUM
  uint8_t* byte_ptr = sbndaq::reinterpret_cast_checked<uint8_t*>(receiver_state_start_ptr_);
  //	RECV_DEBUG(4) << "JCF: current value of checksum is " << software_checksum <<
  //	  ", will look at " << bytes_to_check << " bytes starting at " <<
  //	  static_cast<void*>(byte_ptr) << " in the checksum calculation";
  for (size_t i_byte = 0; i_byte < bytes_to_check; ++i_byte) {
    software_checksum = (software_checksum >> 1) + ((software_checksum & 0x1) << 15) ;
    software_checksum += *(byte_ptr + i_byte);
    software_checksum &= 0xFFFF;
  }
#endif /*DO_CHECKSUM*/
  // -- Finished checking checksum
  
  switch (next_receive_state_)
  {
  case ReceiveMicrosliceHeader:
    {
      // Just testing that it does not go belly up
      // validate_microslice_header();

      {
	// Capture the microslice version, length and sequence ID from the header
	sbnddaq::PennMicroSlice::Header* header = sbndaq::reinterpret_cast_checked<sbnddaq::PennMicroSlice::Header*>( receiver_state_start_ptr_ );
	sbnddaq::PennMicroSlice::Header::format_version_t local_microslice_version = header->format_version;
	sbnddaq::PennMicroSlice::Header::sequence_id_t    sequence_id = header->sequence_id;
	
	microslice_size_   = header->block_size;
	
	RECV_DEBUG(2) << "Got header for microslice version 0x" 
		      << std::hex << (unsigned int)local_microslice_version << std::dec
		      << " with size " << (unsigned int)microslice_size_
		      << " sequence ID " << (unsigned int)sequence_id
		      << " (previous ID " << (unsigned int)last_sequence_id_ << ")";
	// Validate the version - it shouldn't change in a run
	// Once the version is set to anything different than zero, 
	// then it is just a matter of making a comparison
	if (microslice_version_) {
	  if (microslice_version_ != local_microslice_version) {

	    try {
	    mf::LogError("PennDataReceiver") 
	      << "ERROR: Latest microslice version 0x" 
	      << std::hex << (unsigned int)local_microslice_version
	      << " is different to previous microslice version 0x" 
	      << (unsigned int)microslice_version_ << std::dec;
	    } catch (...) {
	      set_exception(true);
	    }
	  }
	} else {
	  microslice_version_ = local_microslice_version;
	}
  
  
	// Validate the version in the packet
	uint8_t version            =  local_microslice_version & 0x0F;
	uint8_t version_complement = (local_microslice_version & 0xF0) >> 4;
	if( ! ((version ^ version_complement) << 4) ) {
	  try {
	    mf::LogError("PennDataReceiver") 
	      << "ERROR: Microslice version and version complement do not agree 0x"
	      << std::hex << (unsigned int)version << ", 0x" 
	      << (unsigned int)version_complement << std::dec;
	  } catch (...) {
	    set_exception(true);
	  }
	}
  

      // Validate the sequence ID - should be incrementing
      // monotonically (or identical to previous if it was
      // fragmented)

	if (sequence_id_initialised_ && (sequence_id != uint8_t(last_sequence_id_+1))) {
	  if (last_microslice_was_fragment_ && (sequence_id == uint8_t(last_sequence_id_))) {
	    // do nothing - we're in a normal fragmented microslice
	  }
	  else if (last_microslice_was_fragment_ && (sequence_id != uint8_t(last_sequence_id_))) {
	    try {
	      mf::LogError("PennDataReceiver") << "WARNING: mismatch in microslice sequence IDs! Got " << (unsigned int)sequence_id << " expected " << (unsigned int)(uint8_t(last_sequence_id_));
	    } catch (...) {
	      set_exception(true);
	    }

	  }
	  else if (rate_test_ && (sequence_id == uint8_t(last_sequence_id_))) {
	    // NFB Dec-02-2015
	    // No idea of what this is
	    
	    // do nothing - all microslices in the rate test have the same sequence id
	  }
	  else {
	    try {
	      mf::LogError("PennDataReceiver") << "WARNING: mismatch in microslice sequence IDs! Got " << (unsigned int)sequence_id << " expected " << (unsigned int)(uint8_t(last_sequence_id_+1));
	    } catch (...) {
	      set_exception(true);
	    }
	  }
	}
	else {
	  sequence_id_initialised_ = true;
	}
	last_sequence_id_ = sequence_id;
	
      }




      //update states ready to be for next call
      millislice_state_ = MicrosliceIncomplete;

      next_receive_state_ = ReceiveMicroslicePayload;
      next_receive_size_  = microslice_size_ - sizeof(sbnddaq::PennMicroSlice::Header);

      // NFB: Do we actually need two variables?
      // If there is not timestamp then it is fragmented...

      //reset this since we're in a new uslice now
      //microslice_seen_timestamp_word_ = false;
      last_microslice_was_fragment_ = false;

      // NFB Dec-02-2015

      // The code below overwrites the microslice header since it is of no use to the Millislice
      //and roll back the current_write_ptr_, as to overwrite the Header in the next recv
      current_write_ptr_ = static_cast<void*>(sbndaq::reinterpret_cast_checked<uint8_t*>(current_write_ptr_) - sizeof(sbnddaq::PennMicroSlice::Header));
      millislice_size_recvd_ -= sizeof(sbnddaq::PennMicroSlice::Header);

      // NFB Dec-06-2015 : This doesn't seem to be used anywhere else
      //copy the microslice header to some other place in memory, for checksum tests
      //memcpy(current_microslice_ptr_, state_start_ptr_, sizeof(sbnddaq::PennMicroSlice::Header));

      break;
    } //case ReceiveMicrosliceHeader

  case ReceiveMicroslicePayload:
    {
      //got a full microslice (complete size checks already done)
      RECV_DEBUG(2) << "Complete payload received for microslice " << microslices_recvd_ << " length " << state_nbytes_recvd_;
      microslices_recvd_++;
      try{
	validate_microslice_payload();
      }catch(...) {
	// payload didn't validate for some reason. Send an error and print the whole thing
	mf::LogInfo("PennDataReceiver") << "Error was caught validating a run. Dumping the culprit microslice";
	//	display_bits(receiver_state_start_ptr_,length,"PennDataReceiver");
        set_exception(true);    // GB+JM-A	
      }
      // NFB : The very first packet from the PTB should have been a timestamp word.
      // NFB Dec-02-2015

      if(!run_start_time_) {
        mf::LogInfo("PennDataReceiver") << "This is the first MicroSlice. Estimating run start time from the first payload.";
	
        // NFB Dec-06-2015
        // This is tricky. The easiest way would be to drop the data until a timestamp was found.

        // Actually, the best way is to do a multiphase approach:
        //1. grab the first timestamp from the first payload_header
        //2. Walk the payloads until a full timestamp is found.
        //3. Calculate the difference between the rollovers and subtract from the full TS
        //4. Set the start run time to that value
	
        uint8_t *current_data_ptr = static_cast<uint8_t*>(receiver_state_start_ptr_);

        // I know that the first microslice sent by the PTB
        // is a timestamp. Just grab it

        // 1. -- Grab the first timestamp -- confirm it is indeed a timestamp
        sbnddaq::PennMicroSlice::Payload_Header *payload_header = static_cast<sbnddaq::PennMicroSlice::Payload_Header *>(receiver_state_start_ptr_);

        if (payload_header->data_packet_type != sbnddaq::PennMicroSlice::DataTypeTimestamp) {
          mf::LogWarning("PennDataReceiver") << "Expected the first word to be a timestamp.  ";
        }
        current_data_ptr+= sizeof(sbnddaq::PennMicroSlice::Payload_Header);
        sbnddaq::PennMicroSlice::Payload_Timestamp *ts_word = sbndaq::reinterpret_cast_checked<sbnddaq::PennMicroSlice::Payload_Timestamp*>(current_data_ptr);
	
        run_start_time_ = ts_word->nova_timestamp;
        boundary_time_  = (run_start_time_ + millislice_size_ - 1);
        overlap_time_   = (boundary_time_  - millislice_overlap_size_);
        mf::LogInfo("PennDataReceiver") << "start run time estimated to be " << run_start_time_
						  << " boundary_time " << boundary_time_ 
						  << " overlap time " << overlap_time_;
      } // if !run_start_time_

      //form a microslice
      // This microslice will only have the payload (including checksum)
      sbnddaq::PennMicroSlice uslice(((uint8_t*)receiver_state_start_ptr_));

      //count the number of different types of payload word
      sbnddaq::PennMicroSlice::sample_count_t n_counter_words(0);
      sbnddaq::PennMicroSlice::sample_count_t n_trigger_words(0);
      sbnddaq::PennMicroSlice::sample_count_t n_timestamp_words(0);
      sbnddaq::PennMicroSlice::sample_count_t n_warning_words(0);
      sbnddaq::PennMicroSlice::sample_count_t n_checksum_words(0);
      sbnddaq::PennMicroSlice::sample_count_t n_words(0);

      //also check to see if the millislice boundary is inside this microslice

      uint32_t hardware_checksum(0);
      std::size_t this_overlap_size(0);
      uint8_t* this_overlap_ptr = nullptr;

      RECV_DEBUG(2) << "Boundary time == " << boundary_time_ << ", overlap time == " << overlap_time_ ;

      // JCF, Jul-19-2015

      // I set the second-to-last argument to false, telling
      // sampleTimeSplitAndCountTwice NOT to reverse the bytes
      // in the header - because I've added this feature already

      // JCF, Jul-28-2015

      // The argument remains set to "false", although in fact
      // it turns out the bytes didn't need to be reversed

      RECV_DEBUG(2) << "Processing the microslice.  " << nextReceiveStateToString(next_receive_state_)
                     << " on length  " << length
                     << " bytes_to_check " << bytes_to_check;

      /// NFB -- Continue revieweing here
      uint8_t* split_ptr =
          uslice.sampleTimeSplitAndCountTwice(boundary_time_, remaining_size_,
              overlap_time_,  this_overlap_size, this_overlap_ptr,
              n_words, n_counter_words, n_trigger_words, n_timestamp_words,
              n_warning_words, n_checksum_words,
              remaining_payloads_recvd_, remaining_payloads_recvd_counter_,
              remaining_payloads_recvd_trigger_, remaining_payloads_recvd_timestamp_,
              remaining_payloads_recvd_warning_, remaining_payloads_recvd_checksum_,
              overlap_payloads_recvd_, overlap_payloads_recvd_counter_,
              overlap_payloads_recvd_trigger_, overlap_payloads_recvd_timestamp_,
              overlap_payloads_recvd_warning_, overlap_payloads_recvd_checksum_,
              hardware_checksum,
              false, microslice_size_);

      // check they agree
#ifdef DO_CHECKSUM
      if(hardware_checksum != software_checksum) {

	try {
	  mf::LogError("PennDataReceiver") << "ERROR: Microslice checksum mismatch! Hardware: " << hardware_checksum << " Software: " << software_checksum;
	} catch (...) {
	  set_exception(true);
	}
      }
      else {
        RECV_DEBUG(4) << "Microslice checksums... Hardware: " << hardware_checksum << " Software: " << software_checksum;
      }
#endif /*DO_CHECKSUM*/

      size_t sizeof_checksum_frame = sizeof(sbnddaq::PennMicroSlice::Payload_Header) + sbnddaq::PennMicroSlice::payload_size_checksum;
      
      // NFB: Nov-18-2015
      // This should now be correct. If not then the offsets are still being calculated wrong.
      current_write_ptr_ = static_cast<void*>(sbndaq::reinterpret_cast_checked<uint8_t*>(current_write_ptr_) - sizeof_checksum_frame);
      millislice_size_recvd_ -= sizeof_checksum_frame;


      ///
      /// Microslice is split between millislices
      ///
      if (split_ptr == nullptr) {
        // The whole microslice goes into the millislice and there were no words received
        // Not sure about this logic.
        if (n_checksum_words == 0 || n_words == 0) {
	  try {
	    mf::LogError("PennDataReceiver") << "Code is about to try to decrement a uint32_t variable which has a value of 0";
	  } catch (...) {
	    set_exception(true);
	  }
        }

        n_checksum_words--;
        n_words--;
      } else {

        RECV_DEBUG(2) << "split_ptr is non-null with value " << static_cast<void*>(split_ptr);
        // Wasn't this already done before?
        if(remaining_payloads_recvd_checksum_) {
          remaining_size_   -= sizeof(sbnddaq::PennMicroSlice::Payload_Header) - sbnddaq::PennMicroSlice::payload_size_checksum;
          remaining_payloads_recvd_ -= remaining_payloads_recvd_checksum_;
          remaining_payloads_recvd_checksum_ = 0;
        }
      }

      ///
      /// Overlap
      ///
      if(this_overlap_ptr != nullptr) {
        RECV_DEBUG(2) << "this_overlap_ptr is non-null with value " << static_cast<void*>(this_overlap_ptr);

        if(overlap_payloads_recvd_checksum_) {
          this_overlap_size -= sizeof(sbnddaq::PennMicroSlice::Payload_Header) - sbnddaq::PennMicroSlice::payload_size_checksum;
          overlap_payloads_recvd_ -= overlap_payloads_recvd_checksum_;
          overlap_payloads_recvd_checksum_ = 0;
        }
      }

      //stash the microslice data that's for the next millislice
      if(split_ptr != nullptr) {
        if(remaining_size_ > sbnddaq::PennDataReceiver::remaining_buffer_size) {
	  try {
	    mf::LogError("PennDataReceiver") << "ERROR buffer overflow for 'remaining bytes of microslice, after the millislice boundary'";
	  } catch (...) {
	    set_exception(true);
	  }
        }
        RECV_DEBUG(2) << "Millislice boundary found within microslice " << microslices_recvd_timestamp_
            << ". Storing " << remaining_size_ << " bytes for next millislice";
        memmove(remaining_ptr_, split_ptr, remaining_size_);
        millislice_size_recvd_ -= remaining_size_;
      }

      //stash the microslice data that's for the overlap period at the start of the next millislice
      if(this_overlap_ptr != nullptr) {
        if(overlap_size_ + this_overlap_size > sbnddaq::PennDataReceiver::overlap_buffer_size_) {
	  try {
	    mf::LogError("PennDataReceiver") << "ERROR buffer overflow for 'overlap bytes of microslice, after the millislice boundary'";
	  } catch (...) {
	    set_exception(true);
	  }
        }
        RECV_DEBUG(2) << "Overlap period found within microslice " << microslices_recvd_timestamp_
            << ". Storing " << overlap_size_ << " bytes for start of next millislice";
        memcpy(overlap_ptr_ + overlap_size_, this_overlap_ptr, this_overlap_size);
        overlap_size_ += this_overlap_size;
      }

      RECV_DEBUG(2) << "Payload contains " << n_words
          << " total words ("    << n_counter_words
          << " counter + "       << n_trigger_words
          << " trigger + "       << n_timestamp_words
          << " timestamp + "     << n_warning_words
          << " warning + "      << n_checksum_words
          << "checksum)"
          << " before the millislice boundary";

      //check if we're inside a fragmented microslice
      if(n_timestamp_words) {
        microslices_recvd_timestamp_++;
        last_microslice_was_fragment_ = false;
      }
      else
        last_microslice_was_fragment_ = true;

      //increment payload counters
      payloads_recvd_           += n_words;
      payloads_recvd_counter_   += n_counter_words;
      payloads_recvd_trigger_   += n_trigger_words;
      payloads_recvd_timestamp_ += n_timestamp_words;
      payloads_recvd_warning_  += n_warning_words;
      payloads_recvd_checksum_  += n_checksum_words;

      // JCF, Jul-30-2015

      // Setting microslice_size_recvd_ = 0 here implies that we
      // can't (or shouldn't) fragment a microslice across
      // reads, correct?

      microslice_size_recvd_ = 0;
      millislice_state_ = MillisliceIncomplete;
      next_receive_state_ = ReceiveMicrosliceHeader;
      next_receive_size_ = sizeof(sbnddaq::PennMicroSlice::Header);

      RECV_DEBUG(2) << "JCF: at end of \"case ReceiveMicroslicePayload\"";
      break;
  }//case ReceiveMicroslicePayload

    default:
    {
      // Should never happen - bug or data corruption
      try {
	mf::LogError("PennDataReceiver") << "FATAL ERROR after async_recv - unrecognised next receive state: " << next_receive_state_;
      } catch (...) {
	set_exception(true);
      }
      return;
      break;
    }
  }//switch(next_receive_state_)

  //reset counters & ptrs for the next call
  state_nbytes_recvd_ = 0;
  receiver_state_start_ptr_ = 0;

  // If correct number of microslices (with timestamps) have been received, flag millislice as complete

  if(remaining_size_)
  {
    RECV_DEBUG(1) << "Millislice " << millislices_recvd_
        << " complete with " << microslices_recvd_timestamp_
        << " microslices (" << microslices_recvd_
        << " including fragments), total size " << millislice_size_recvd_
        << " bytes and "      << payloads_recvd_
        << " payload words (" << payloads_recvd_counter_
        << " counter + "      << payloads_recvd_trigger_
        << " trigger + "      << payloads_recvd_timestamp_
        << " timestamp + "    << payloads_recvd_warning_
        << " warning + "     << payloads_recvd_checksum_
        << " checksum)";
    millislices_recvd_++;
    millislice_state_ = MillisliceComplete;
  }

  // If the millislice is complete, place the buffer to the filled queue and set the state accordingly
  if (millislice_state_ == MillisliceComplete)
  {
    current_raw_buffer_->setSize(millislice_size_recvd_);
    current_raw_buffer_->setCount(microslices_recvd_);
    current_raw_buffer_->setSequenceID(millislices_recvd_ & 0xFFFF); //lowest 16 bits
    current_raw_buffer_->setCountPayload(payloads_recvd_);
    current_raw_buffer_->setCountPayloadCounter(payloads_recvd_counter_);
    current_raw_buffer_->setCountPayloadTrigger(payloads_recvd_trigger_);
    current_raw_buffer_->setCountPayloadTimestamp(payloads_recvd_timestamp_);
    current_raw_buffer_->setEndTimestamp(boundary_time_);
    current_raw_buffer_->setWidthTicks(millislice_size_);
    current_raw_buffer_->setOverlapTicks(millislice_overlap_size_);
    current_raw_buffer_->setFlags(0);

    //update the times

//    boundary_time_ = (boundary_time_ + millislice_size_)         & 0xFFFFFFF; //lowest 28 bits
//    overlap_time_  = (boundary_time_ - millislice_overlap_size_) & 0xFFFFFFF; //lowest 28 bits
//    filled_buffer_queue_.push(std::move(current_raw_buffer_));
    boundary_time_ = boundary_time_ + millislice_size_;
    overlap_time_ = boundary_time_ - millislice_overlap_size_;
    filled_buffer_queue_.push(std::move(current_raw_buffer_));
    millislice_state_ = MillisliceEmpty;
  }
}

void sbnddaq::PennDataReceiver::suspend_readout(bool await_restart)
{
  readout_suspended_.store(true);

  if (await_restart)
  {
    RECV_DEBUG(2) << "::suspend_readout: awaiting restart or shutdown";
    while (suspend_readout_.load() && run_receiver_.load())
    {
      usleep(tick_period_usecs_);
    }
    RECV_DEBUG(2) << "::suspend_readout: restart or shutdown detected, exiting wait loop";
  }

}

void sbnddaq::PennDataReceiver::set_deadline(DeadlineIoObject io_object, unsigned int timeout_usecs)
{

  // Set the current IO object that the deadline is being used for
  deadline_io_object_ = io_object;

  // Set the deadline for the asynchronous write operation if the timeout is set, otherwise
  // revert back to +ve infinity to stall the deadline timer actor
  if (timeout_usecs > 0)
  {
    deadline_.expires_from_now(boost::posix_time::microseconds(timeout_usecs));
  }
  else
  {
    deadline_.expires_from_now(boost::posix_time::pos_infin);
  }
}

void sbnddaq::PennDataReceiver::check_deadline(void)
{

  // Handle deadline if expired
  if (deadline_.expires_at() <= boost::asio::deadline_timer::traits_type::now())
  {

    // Cancel pending operation on current specified object
    switch (deadline_io_object_)
    {
      case Acceptor:
        acceptor_.cancel();
        break;
      case DataSocket:
        data_socket_.cancel();
        break;
      case None:
        // Do nothing
        break;
      default:
        // Shouldn't happen
        break;
    }

    // No longer an active deadline to set the expiry to positive inifinity
    deadline_.expires_at(boost::posix_time::pos_infin);
  }

  // Put the deadline actor back to sleep if receiver is still running
  if (run_receiver_.load())
  {
    deadline_.async_wait(boost::bind(&sbnddaq::PennDataReceiver::check_deadline, this));
  }
  else
  {
    mf::LogInfo("PennDataReceiver")  << "Deadline actor terminating";
  }
}

std::string sbnddaq::PennDataReceiver::nextReceiveStateToString(NextReceiveState val)
{
  try {
    return next_receive_state_names_.at(val);
  }
  catch(const std::out_of_range& e) {
    return "INVALID/UNKNOWN";
  }
  return "INVALID/UNKNOWN";
}

std::string sbnddaq::PennDataReceiver::millisliceStateToString(MillisliceState val)
{
  try {
    return millislice_state_names_.at(val);
  }
  catch(const std::out_of_range& e) {
    return "INVALID/UNKNOWN";
  }
  return "INVALID/UNKNOWN";
}

std::size_t sbnddaq::PennDataReceiver::nextReceiveStateToExpectedBytes(NextReceiveState val)
{
  switch(val)
  {
    case ReceiveMicrosliceHeader:
      return sizeof(sbnddaq::PennMicroSlice::Header);
      break;
    case ReceiveMicroslicePayload:
      return microslice_size_ - sizeof(sbnddaq::PennMicroSlice::Header);
      break;
    default:
      return 0;
      break;
  }
}

void sbnddaq::PennDataReceiver::validate_microslice_header(void) {
  // Capture the microslice version, length and sequence ID from the header
  sbnddaq::PennMicroSlice::Header* header = sbndaq::reinterpret_cast_checked<sbnddaq::PennMicroSlice::Header*>( receiver_state_start_ptr_ );
  sbnddaq::PennMicroSlice::Header::format_version_t local_microslice_version = header->format_version;
  sbnddaq::PennMicroSlice::Header::sequence_id_t    sequence_id = header->sequence_id;

  microslice_size_   = header->block_size;

  RECV_DEBUG(2) << "Got header for microslice version 0x" 
		<< std::hex << (unsigned int)local_microslice_version << std::dec
		<< " with size " << (unsigned int)microslice_size_
		<< " sequence ID " << (unsigned int)sequence_id
		<< " (previous ID " << last_sequence_id_ << ")";
  // Validate the version - it shouldn't change in a run
  // Once the version is set to anything different than zero, 
  // then it is just a matter of making a comparison
  if (microslice_version_) {
    if (microslice_version_ != local_microslice_version) {
      try {
	mf::LogError("PennDataReceiver") 
	  << "ERROR: Latest microslice version 0x" 
	  << std::hex << (unsigned int)local_microslice_version
	  << " is different to previous microslice version 0x" 
	  << (unsigned int)microslice_version_ << std::dec;
      } catch (...) {
	set_exception(true);
      }
    }
  } else {
    microslice_version_ = local_microslice_version;
  }
  
  
  // Validate the version in the packet
  uint8_t version            =  local_microslice_version & 0x0F;
  uint8_t version_complement = (local_microslice_version & 0xF0) >> 4;
  if( ! ((version ^ version_complement) << 4) ) {
    try {
      mf::LogError("PennDataReceiver") 
	<< "ERROR: Microslice version and version complement do not agree 0x"
	<< std::hex << (unsigned int)version << ", 0x" 
	<< (unsigned int)version_complement << std::dec;
    } catch (...) {
      set_exception(true);
    }
  }
  
  // Validate the sequence ID -- should always be incrementing
  // except for fragmented blocks
  if (sequence_id_initialised_) {
    if (last_microslice_was_fragment_) {
      if (sequence_id != uint8_t(last_sequence_id_)) {
	try {
	  mf::LogError("PennDataReceiver") 
	    << "Mismatch in microslice sequence IDs! Got " 
	    << (unsigned int)sequence_id << " expected " 
	    << (unsigned int)(uint8_t(last_sequence_id_));
	} catch (...) {
	  set_exception(true);
	}
      }
    } else if (sequence_id != uint8_t(last_sequence_id_+1)) {
      try {
	mf::LogError("PennDataReceiver") << "Mismatch in microslice sequence IDs! Got " << (unsigned int)sequence_id << " expected " << (unsigned int)(uint8_t(last_sequence_id_+1));
      } catch (...) {
	set_exception(true);
      }
    }
  } else {
    sequence_id_initialised_ = true;
  }
  
  last_sequence_id_ = sequence_id;
}

void sbnddaq::PennDataReceiver::validate_microslice_payload(void) {
  
  // Check that the last word is a checksum
  //uint8_t* current_ptr = sbndaq::reinterpret_cast_checked<uint8_t*>(current_write_ptr_);
  uint8_t* current_ptr = static_cast<uint8_t*>(current_write_ptr_);
  static const size_t sizeof_checksum_frame = sizeof(sbnddaq::PennMicroSlice::Payload_Header) + sbnddaq::PennMicroSlice::payload_size_checksum;

  current_ptr -= sizeof_checksum_frame;
  sbnddaq::PennMicroSlice::Payload_Header *payload_header = sbndaq::reinterpret_cast_checked<sbnddaq::PennMicroSlice::Payload_Header *>(current_ptr);
  if (payload_header->data_packet_type != sbnddaq::PennMicroSlice::DataTypeChecksum) {
    
    //    // display_bits(current_ptr,4);
    try {
      mf::LogError("PennDataReceiver") << "Microslice received has no checksum word. Last packet has header " << std::bitset<3>(payload_header->data_packet_type);
    } catch (...) {
      set_exception(true);
    }
  }
  // Check that there is a timestamp word
  static const size_t sizeof_timestamp_frame = sizeof(sbnddaq::PennMicroSlice::Payload_Header) + sbnddaq::PennMicroSlice::payload_size_timestamp;
  current_ptr -= sizeof_timestamp_frame;
  payload_header = sbndaq::reinterpret_cast_checked<sbnddaq::PennMicroSlice::Payload_Header *>(current_ptr);
  // The second to last packet should be a timestamp
  if (payload_header->data_packet_type != sbnddaq::PennMicroSlice::DataTypeTimestamp) {
    //    display_bits(current_ptr,4);
   try { 
       mf::LogError("PennDataReceiver") << "Microslice received as no timestamp word. Fragmented packets are no yet supported. Second to last packet has header " << std::bitset<3>(payload_header->data_packet_type);
    } catch (...) {
       // Swallow the exception here, but if we are running, set exception_ flag so getNext will 
       // know to stop the run gracefully
       if (! suspend_readout_.load()) {
         mf::LogInfo("PennDataReceiver") << "Setting exception to true; suspend_readout is false";
         set_exception(true);     // This may not be needed here (but can't harm), because the call to validate_microslice_payload catches
       }
    }

  }
}
