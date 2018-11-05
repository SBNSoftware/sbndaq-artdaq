#include "sbndaq-artdaq/Generators/SBND/PennReceiver.hh"

#include "sbndaq-artdaq-core/Overlays/SBND/PennMilliSliceWriter.hh"
#include "sbndaq-artdaq-core/Overlays/SBND/PennMilliSliceFragment.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"

#include "canvas/Utilities/Exception.h"
#include "artdaq/Application/GeneratorMacros.hh"
#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "artdaq-core/Utilities/SimpleLookupPolicy.hh"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <fstream>
#include <iomanip>
#include <iterator>
#include <iostream>
#include <sstream>
#include <thread>
#include <unistd.h>

sbnddaq::PennReceiver::PennReceiver(fhicl::ParameterSet const & ps)
:
CommandableFragmentGenerator(ps),
run_receiver_(false),
data_timeout_usecs_(ps.get<uint32_t>("data_timeout_usecs", 60000000))
{

  int fragment_id = ps.get<int>("fragment_id");
  fragment_ids_.push_back(fragment_id);

  instance_name_for_metrics_ = "PennReceiver";

  mf::LogInfo("PennReceiver") << "Starting up";

////////////////////////////
  // HARDWARE OPTIONS
  // config stream connection parameters

  client_sleep_on_stop_ = ps.get<uint32_t>("PTB.sleep_on_stop_us",5000000);
  penn_client_host_addr_ =
      ps.get<std::string>("PTB.penn_client_host_addr", "192.168.1.205");
  penn_client_host_port_ =
      ps.get<std::string>("PTB.penn_client_host_port", "8991");
  penn_client_timeout_usecs_ =
      ps.get<uint32_t>("PTB.penn_client_timeout_usecs", 500000);


  ////////////////////////////
  // BOARDREADER OPTIONS

  // time that the board reader waits for an answer before considering that a timeout occurred.
  receiver_tick_period_usecs_ =
      ps.get<uint32_t>("PTB.receiver_tick_period_usecs", 10000);

  // millislice size parameters
  // the size is calculated in clock ticks. Up to the configuration user to
  // make sure that the right amount of ticks are set.

  // NOTE: Keep in mind that the clock is running at 32 MHz but the timestamp register is
  // updated at 64 MHz. What we care is not the effective clock ticks but the
  // timestamp ticks
  millislice_size_ =
      ps.get<uint32_t>("PTB.millislice_size",640000); // -- default: 10 ms

  millislice_overlap_size_ = 
      ps.get<uint16_t>("PTB.millislice_overlap_size", 0); // -- no overlap by default

  // boardreader printout options

  int receiver_debug_level =
      ps.get<int>("PTB.receiver_debug_level", 0);
#ifdef __PTB_DEBUG__  
 mf::LogInfo("PennReceiver") << "Debug level set to " << receiver_debug_level;
#endif
  reporting_interval_fragments_ =
      ps.get<uint32_t>("PTB.reporting_interval_fragments", 100);
  reporting_interval_time_ = 
      ps.get<uint32_t>("PTB.reporting_interval_time", 0);

  // boardreader buffer sizes -- These might need some tuning

  raw_buffer_size_ =
      ps.get<size_t>("PTB.raw_buffer_size", 100000); // 100 kB buffer
  // Number of raw buffers that are added to the stack
  raw_buffer_precommit_ =
      ps.get<uint32_t>("PTB.raw_buffer_precommit", 2000);
  filled_buffer_release_max_ =                                 // GBcopy
    ps.get<uint32_t>("PTB.filled_buffer_release_max", 2000);         // GBcopy
  // NFB -- This means that artDAQ fragments are used instead of PennRawBuffers
  // Not really sure what is effectively the difference between one and the other here
  use_fragments_as_raw_buffer_ =
      ps.get<bool>("PTB.use_fragments_as_raw_buffer", true);
#ifdef REBLOCK_PENN_USLICE
  if(use_fragments_as_raw_buffer_ == false) {
    mf::LogError("PennReceiver") << "use_fragments_as_raw_buffer == false has not been implemented";
  } else {
    mf::LogInfo("PennReceiver") << "Using artdaq::Fragment as raw buffers";
  }

#endif
  /////////////////////////////////////////////////////////////////////////////////
  ///
  /// Penn board options. This part is only pertinent for the PTB configuration.
  ///
  /////////////////////////////////////////////////////////////////////////////////

  penn_dry_run_ = ps.get<bool>("PTB.dry_run_mode",false);

  if (penn_dry_run_) {
    mf::LogWarning("PennReceiver") << "Dry run mode requested for PTB. No data will be produced.";
  }

  // -- data stream connection parameters
  penn_data_dest_host_ =
      ps.get<std::string>("PTB.penn_data_buffer.daq_host", "192.168.1.1");
  penn_data_dest_port_ =
      ps.get<uint16_t>("PTB.penn_data_buffer.daq_port", 8992);

  penn_data_dest_rollover_ = 
      ps.get<uint32_t>("PTB.penn_data_buffer.daq_rollover",1500);

  // Penn microslice duration (in NOvA clock ticks)
  penn_data_microslice_size_ =
      ps.get<uint32_t>("PTB.penn_data_buffer.daq_microslice_size", 64000);

  if (penn_data_microslice_size_ >= millislice_size_) {
      mf::LogError("PennReceiver") << "Microslice size (" << penn_data_microslice_size_
          << ") must be smaller than the millislice (" << millislice_size_ << ")";

  }

  if (penn_data_microslice_size_ > ((1<<27)-1)) {
    mf::LogError("PennReceiver") << "Microslice size ( "  << penn_data_microslice_size_
        << " ) must fit in 27 bits  [ max : " << (1<<27)-1 << "]";
  }


  // -- Channel masks
  penn_channel_mask_bsu_ =
      ps.get<uint64_t>("PTB.channel_mask.BSU", 0x3FFFFFFFFFFFF);
  penn_channel_mask_tsu_ =
      ps.get<uint64_t>("PTB.channel_mask.TSU", 0xFFFFFFFFFFFF);

  // -- Pulse width
  ptb_pulse_width_ = ps.get<uint32_t>("PTB.hardware.pulse_width",5);

  if (ptb_pulse_width_ > ((1<<6)-1)) {
    mf::LogError("PennReceiver") << "Pulse width ( "  << ptb_pulse_width_
        << " ) must fit in 6 bits  [ max : " << (1<<6)-1 << "]";
  }

  // -- How to deal with external triggers
  penn_ext_triggers_mask_ = ps.get<uint8_t>("PTB.external_triggers.mask",0xF);

  penn_ext_triggers_echo_ = ps.get<bool>("PTB.external_triggers.echo_triggers",false);

  penn_ext_triggers_gate_ = ps.get<uint32_t>("PTB.external_triggers.gate",5);
  if (penn_ext_triggers_gate_ > ((1<<11)-1)) {
    mf::LogError("PennReceiver") << "External trigger gate width ( "  << penn_ext_triggers_gate_
        << " ) must fit in 11 bits  [ max : " << (1<<11)-1 << "]";
  }

  penn_ext_triggers_prescale_ = ps.get<uint32_t>("PTB.external_triggers.prescale",0);
  if (penn_ext_triggers_prescale_ > ((1<<8)-1)) {
    mf::LogError("PennReceiver") << "External trigger prescale ( "  << penn_ext_triggers_prescale_
        << " ) must fit in 8 bits  [ max : " << (1<<8)-1 << "]";
  }

  // -- Calibrations
  for (uint32_t i = 1; i <= penn_num_calibration_channels_; ++i) {
    CalibChannelConfig channel;
    std::ostringstream channel_name;
    channel_name << "PTB.calibrations.C";
    channel_name << i;

    channel.id           = ps.get<std::string>(channel_name.str() + ".id");
    channel.id_mask   = ps.get<std::string>(channel_name.str() + ".id_mask");
    channel.enabled      = ps.get<bool>(channel_name.str() + ".enabled");
    channel.period       = ps.get<uint32_t>(channel_name.str() + ".period");

    calib_channels_.push_back(channel);
  }

  // -- Muon triggers
  // This is the more elaborated part:
  // First grab the global parameters
  penn_muon_num_triggers_ = ps.get<uint32_t>("PTB.muon_triggers.num_triggers",4);
  penn_trig_in_window_ = ps.get<uint32_t>("PTB.muon_triggers.trig_window",12);
  penn_trig_lockdown_window_ = ps.get<uint32_t>("PTB.muon_triggers.trig_lockdown",0);

  // And now grab the individual trigger mask configuration
  for (uint32_t i = 0; i < penn_muon_num_triggers_; ++i) {
    MuonTriggerMaskConfig mask;
    std::ostringstream trig_name;
    trig_name << "PTB.muon_triggers.trigger_";
    trig_name << i;

    std::string first_param = trig_name.str() + ".id";

    mask.id           = ps.get<std::string>(trig_name.str() + ".id");
    mask.id_mask      = ps.get<std::string>(trig_name.str() + ".id_mask");
    mask.prescale     = ps.get<uint8_t>(trig_name.str() + ".prescale");
    mask.logic        = ps.get<uint8_t>(trig_name.str() + ".logic");
    mask.g1_logic     = ps.get<uint8_t>(trig_name.str() + ".group1.logic");
    mask.g1_mask_bsu  = ps.get<uint64_t>(trig_name.str() + ".group1.BSU");
    mask.g1_mask_tsu  = ps.get<uint64_t>(trig_name.str() + ".group1.TSU");
    mask.g2_logic     = ps.get<uint8_t>(trig_name.str() + ".group2.logic");
    mask.g2_mask_bsu  = ps.get<uint64_t>(trig_name.str() + ".group2.BSU");
    mask.g2_mask_tsu  = ps.get<uint64_t>(trig_name.str() + ".group2.TSU");

    muon_triggers_.push_back(mask);

  }

  ////////////////////////////
  // EMULATOR OPTIONS
  ////////////////////////////

  // amount of data to generate
  penn_data_num_millislices_ =
      ps.get<uint32_t>("PTB.penn_data_num_millislices", 10);
  penn_data_num_microslices_ =
      ps.get<uint32_t>("PTB.penn_data_num_microslices", 10);
  penn_data_frag_rate_ =
      ps.get<float>("PTB.penn_data_frag_rate", 10.0);

  // type of data to generate
  penn_data_payload_mode_ =
      ps.get<uint16_t>("PTB.penn_data_payload_mode", 0);
  penn_data_trigger_mode_ =
      ps.get<uint16_t>("PTB.penn_data_trigger_mode", 0);
  penn_data_fragment_microslice_at_ticks_ =
      ps.get<int32_t>("PTB.penn_data_fragment_microslice_at_ticks", 0);

  // special debug options
  penn_data_repeat_microslices_ =
      ps.get<bool>("PTB.penn_data_repeat_microslices", false);
  penn_data_debug_partial_recv_ =
      ps.get<bool>("PTB.penn_data_debug_partial_recv", false);


  ///
  /// -- Configuration loaded.
  ///


  // Create an PENN client instance
  penn_client_ = std::unique_ptr<sbnddaq::PennClient>(new sbnddaq::PennClient(
      penn_client_host_addr_, penn_client_host_port_, penn_client_timeout_usecs_));

  // What does this actually do? FLushes the registers?
  //  penn_client_->send_command("HardReset");
  penn_client_->send_command("HardReset");
  if (penn_client_->exception()) {
    set_exception(true);
    mf::LogError("PennReceiver") << "sbnddaq::PennReceiver::constructor_ : found penn client in exception state";
  }
  sleep(1);
  std::ostringstream config_frag;
  this->generate_config_frag(config_frag);

#ifndef PENN_EMULATOR
  bool rate_test = false;
#else
  bool rate_test = penn_data_repeat_microslices_;

  config_frag.str(std::string());
  this->generate_config_frag_emulator(config_frag);
  penn_client_->send_config(config_frag.str());
#endif //!PENN_EMULATOR

  // Create a PennDataReceiver instance
  // This should be where the PTB connects. 
  //#ifdef __PTB_BOARD_READER_DEVEL_MODE__
  mf::LogInfo("PennReceiver") << "Creating data receiver with parameters :receiver_tick_period_usecs_ ["
        << receiver_tick_period_usecs_
        << "] millislice_size_ : [" << millislice_size_
				     << "] millislice_overlap_size_ : [" << millislice_overlap_size_ << "]";
  //#endif


  //-- Dump the configuration as received:
  // FIXME: Continue here

  data_receiver_ =
      std::unique_ptr<sbnddaq::PennDataReceiver>(new sbnddaq::PennDataReceiver(receiver_debug_level,
                                                                         receiver_tick_period_usecs_,
                                                                         penn_data_dest_port_,
                                                                         millislice_size_,
                                                                         millislice_overlap_size_,
                                                                         rate_test));

data_receiver_->set_stop_delay(client_sleep_on_stop_);

// Sleep for a short while to give time for the DataReceiver to be ready to
  // receive connections
  usleep(500000);

#ifdef __PTB_DEBUG__
  mf::LogDebug("PennReceiver") << "Sending the configuration to the PTB";
#endif
  // Can I send the coonfiguration after creating the receiver?
  penn_client_->send_config(config_frag.str());
#ifdef __PTB_DEBUG__
  mf::LogDebug("PennReceiver") << "Configuration sent to the PTB";
#endif
  if (penn_client_->exception()) {
    set_exception(true);
    mf::LogError("PennReceiver") << "sbnddaq::PennReceiver::constructor_ : found control thread in exception state";
  }
}

void sbnddaq::PennReceiver::start(void)
{
#ifdef __PTB_DEBUG__
  mf::LogDebug("PennReceiver") << "start() called";
#endif
  // Tell the data receiver to drain any unused buffers from the empty queue
  data_receiver_->release_empty_buffers();

  // Tell the data receiver to release any unused filled buffers
  data_receiver_->release_filled_buffers();

  // Clear the fragment map of any pre-allocated fragments still present
  raw_to_frag_map_.clear();

  // Pre-commit buffers to the data receiver object - creating either fragments or raw buffers depending on raw buffer mode
  // What is effectively the difference between the modes?
#ifdef __PTB_DEBUG__
  mf::LogDebug("PennReceiver") << "Pre-committing " << raw_buffer_precommit_ << " buffers of size " << raw_buffer_size_ << " to receiver";
#endif
  empty_buffer_low_mark_ = 0;
  for (unsigned int i = 0; i < raw_buffer_precommit_; i++)
  {
    sbnddaq::PennRawBufferPtr raw_buffer;
    // Use artDAQ fragments to build the raw buffer
    if (use_fragments_as_raw_buffer_)
    {
      //mf::LogWarning("PennReceiver") << "getNext_ : Using fragments as raw buffers. This does not remove warning words from the Millislices.";

      raw_buffer = this->create_new_buffer_from_fragment();
    }
    else
    {
      // Is this mode really needed?
      raw_buffer = sbnddaq::PennRawBufferPtr(new PennRawBuffer(raw_buffer_size_));
    }
#ifdef __PTB_DEBUG__
    mf::LogDebug("PennReceiver") << "Pre-commiting raw buffer " << i << " at address " << (void*)(raw_buffer->dataPtr());
#endif
    data_receiver_->commit_empty_buffer(raw_buffer);
    empty_buffer_low_mark_++;
  }
  filled_buffer_high_mark_ = 0;  //GBcopy

  // Initialise data statistics
  millislices_received_ = 0;
  total_bytes_received_ = 0;
  start_time_ = std::chrono::high_resolution_clock::now();
  report_time_ = start_time_;

  // Start the data receiver
  data_receiver_->start();

  // Send start command to PENN
  // The soft-reset kills the connection.
  //penn_client_->send_command("SoftReset");
  penn_client_->send_command("StartRun");
  
  // check if there was no problem starting the run. If there was then fail.
  if (penn_client_->exception()) {
    set_exception(true);
    mf::LogError("PennReceiver") << "sbnddaq::PennReceiver::start_ : found control thread in exception state";
  }
  
  // if (xml_answer.size() == 0) {
  //   mf::LogWarning("PennReceiver") << "PTB didn't send a start of run timestamp. Will estimate from data flow.";
  // } else {
  //   std::stringstream tmpVal;
  //   tmpVal << xml_answer;
  //   uint64_t start_time;
  //   tmpVal >> start_time;
  //   data_receiver_->set_run_start_time(start_time);
  // }
}

// GBcopy  stopNoMutex() is called before stop() by how much?  Probably not much,
// GBcopy  but the RCEs tell the hardware to stop sending data here, so I have moved it?
void sbnddaq::PennReceiver::stopNoMutex(void)
{
        mf::LogInfo("PennReceiver") << "In stopNoMutex - instructing PTB to stop";

        // Instruct the PTB to stop
        penn_client_->send_command("StopRun");  // GBcopy
        // no need to look for exception state....is there?
}

void sbnddaq::PennReceiver::stop(void)
{
  // GBcopy (done in stopNoMutex instead): mf::LogInfo("PennReceiver") << "stop() called";

  // Instruct the PENN to stop
  // NFB Dec-02-2015

  // Introduced a answer mode to the PTB which sends run statistics at the end of the
  // run.
  // Collect them here and compare to the statistics accumulated in the board reader
  //std::string statistics;
  // GBcopy (done in stopNoMutex instead): penn_client_->send_command("StopRun");


  mf::LogInfo("PennReceiver") << "stop() called: Low water mark on empty buffer queue is " << empty_buffer_low_mark_;
  mf::LogInfo("PennReceiver") << "stop() called: High water mark on filled buffer queue is " << filled_buffer_high_mark_;

  auto elapsed_msecs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time_).count();
  double elapsed_secs = ((double)elapsed_msecs) / 1000;
  double rate = ((double)millislices_received_) / elapsed_secs;
  double data_rate_mbs = ((double)total_bytes_received_) / ((1024*1024) * elapsed_secs);

  mf::LogInfo("PennReceiver") << "Received " << millislices_received_ << " millislices in "
      << elapsed_secs << " seconds, rate "
      << rate << " Hz, total data " << total_bytes_received_ << " bytes, rate " << data_rate_mbs << " MB/s";

  // Stop the data receiver.
  data_receiver_->stop();

  // NFB: The low water mark inicates if a run has had too few millislices being collected
  // For all purposes it seems that this value could take any form.
  mf::LogInfo("PennReceiver") << "Low water mark on empty buffer queue is " << empty_buffer_low_mark_;
  mf::LogInfo("PennReceiver") << "High water mark on filled buffer queue is " << filled_buffer_high_mark_;

  /*
  auto elapsed_msecs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time_).count();
  double elapsed_secs = ((double)elapsed_msecs) / 1000;
  double rate = ((double)millislices_received_) / elapsed_secs;
  double data_rate_mbs = ((double)total_bytes_received_) / ((1024*1024) * elapsed_secs);

  mf::LogInfo("PennReceiver") << "Received " << millislices_received_ << " millislices in "
      << elapsed_secs << " seconds, rate "
      << rate << " Hz, total data " << total_bytes_received_ << " bytes, rate " << data_rate_mbs << " MB/s";
  */
  // std::ostringstream msg;
  // msg << "PTB data collection statistics : \n";
  // for (std::map<std::string,std::string>::iterator it = statistics.begin(); it != statistics.end(); ++it) {
  //   msg.width(20);
  //   msg << it->first;
  //   msg << " : ";
  //   msg << it->second;
  // }

  // mf::LogInfo("PennReceiver") << msg;

}

// GBcopy - OLD_GETNEXT is not set, so we should be using the first one of these (the newer one), when it is tested.
#ifndef OLD_GETNEXT
bool sbnddaq::PennReceiver::getNext_(artdaq::FragmentPtrs & frags) {

	uint32_t buffers_released = 0;
	PennRawBufferPtr recvd_buffer;
	const unsigned int buffer_recv_timeout_us = 500000;

	// Before releasing any fragments, capture buffer state metrics
	size_t empty_buffers_available = data_receiver_->empty_buffers_available();
	if (empty_buffers_available < empty_buffer_low_mark_)
	{
	  empty_buffer_low_mark_ = empty_buffers_available;
	}
	size_t filled_buffers_available = data_receiver_->filled_buffers_available();
	if (filled_buffers_available > filled_buffer_high_mark_)
	{
	  filled_buffer_high_mark_ = filled_buffers_available;
	}

	// JCF, Dec-11_2015

	// If it's the start of data taking, we pretend we received a
	// buffer at the start time in order to begin the clock used
	// to determine later whether or not we're timed out (this
	// code is required if no buffers ever arrive and we can't
	// therefore literally say that a buffer was received at some
	// point in time)

	if ( startOfDatataking() ) {
	  mf::LogInfo("PennReceiver") << "Penn: start of datataking";
	  last_buffer_received_time_ = std::chrono::high_resolution_clock::now();
	}

	// If we find buffers in the while loop, then at the bottom of
	// this function we'll skip the timeout check...
	bool buffers_found_in_while_loop = false;  

	// Loop to release fragments if filled buffers available, up to the maximum allowed
	while ((data_receiver_->filled_buffers_available() > 0) && (buffers_released <= filled_buffer_release_max_))
	{
		bool buffer_available = data_receiver_->retrieve_filled_buffer(recvd_buffer, buffer_recv_timeout_us);

		if (!buffer_available)
		{
			mf::LogWarning("PennReceiver") << "sbnddaq::PennReceiver::getNext_ : "
					<< "receiver indicated buffers available but unable to retrieve one within timeout";
			continue;
		}

		if (recvd_buffer->size() == 0)
		{
			mf::LogWarning("PennReceiver") << "sbnddaq::PennReceiver::getNext_ : no data received in raw buffer";
			continue;
		}

		// We now know we have a legitimate buffer which we
		// can save in an artdaq::Fragment, so record the time
		// it occurred as a point of reference to later
		// determine if there's been a data timeout

		buffers_found_in_while_loop = true;
		last_buffer_received_time_ = std::chrono::high_resolution_clock::now();

		// Get a pointer to the data in the current buffer and create a new fragment pointer
		uint8_t* data_ptr = recvd_buffer->dataPtr();
		std::unique_ptr<artdaq::Fragment> frag;
		uint32_t millislice_size = 0;

		// If using fragments as raw buffers, process accordingly
		if (use_fragments_as_raw_buffer_)
		{
			// Map back onto the fragment from the raw buffer data pointer we have just received
			if (raw_to_frag_map_.count(data_ptr))
			{
				// Extract the fragment from the map
				frag = std::move(raw_to_frag_map_[data_ptr]);

				// Validate and finalize the fragment received
				millislice_size = validate_millislice_from_fragment_buffer(frag->dataBeginBytes(), 
                                  recvd_buffer->size(),
#ifndef REBLOCK_PENN_USLICE
		                  recvd_buffer->count(),
#endif
			          recvd_buffer->sequenceID(),
			          recvd_buffer->countPayload(), recvd_buffer->countPayloadCounter(),
			          recvd_buffer->countPayloadTrigger(), recvd_buffer->countPayloadTimestamp(),
			          recvd_buffer->endTimestamp(), recvd_buffer->widthTicks(), 
                                  recvd_buffer->overlapTicks()  );

				// Clean up entry in map to remove raw fragment buffer
				raw_to_frag_map_.erase(data_ptr);

				// Create a new raw buffer pointing at a new fragment and replace the received buffer
				// pointer with it - this will be recycled onto the empty queue later
				recvd_buffer =create_new_buffer_from_fragment();

			}
			else
			{
				mf::LogError("PennReceiver") << "sbnddaq::PennReceiver::getNext_ : cannot map raw buffer with data address"
						<< (void*)recvd_buffer->dataPtr() << " back onto fragment";
				continue;
			}
		}
		// Otherwise format the raw buffer into a new fragment
		else
		{
		  mf::LogError("PennReceiver") << "You must use_fragments_as_raw_buffer, the other option is not implemented, change PTB configuration please";
		}

		// Recycle the raw buffer onto the commit queue for re-use by the receiver.
		data_receiver_->commit_empty_buffer(recvd_buffer);

		// Set fragment fields appropriately
		frag->setSequenceID(ev_counter());
		frag->setFragmentID(fragmentIDs()[0]);
		frag->setUserType(sbndaq::detail::PENNTRIGGER);

		// Resize fragment to final millislice size
	        frag->resizeBytes(millislice_size);

		// Add the fragment to the list
		frags.emplace_back(std::move (frag));

		// Increment the event counter
		ev_counter_inc();

		// Update counters
		millislices_received_++;
		total_bytes_received_ += millislice_size;
		buffers_released++;

	} // End of loop over available buffers

	// Report on data received so far
	if (reporting_interval_time_ != 0)
	{
		std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();

		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - report_time_).count() >
			(reporting_interval_time_ * 1000))
		{
			auto elapsed_msecs = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time_).count();
			double elapsed_secs = ((double)elapsed_msecs)/1000;

			mf::LogInfo("PennReceiver") << "sbnddaq::PennReceiver::getNext_ : received " << millislices_received_ << " millislices, "
					<< float(total_bytes_received_)/(1024*1024) << " MB in " << elapsed_secs << " seconds";

			report_time_ = std::chrono::high_resolution_clock::now();
		}
	}


	// Determine return value, depending on whether run is stopping, if there are any filled buffers available
	// and if the receiver thread generated an exception

	bool is_active = true;
	if (should_stop())
	{
		if (data_receiver_->filled_buffers_available() > 0)
		{
			// Don't signal board reader can stop if there are still buffers available to release as fragments
			mf::LogDebug("PennReceiver")
				<< "sbnddaq::PennReceiver::getNext_ : should_stop() is true but buffers available";
		}
		else
		{
			// If all buffers released, set is_active false to signal board reader can stop
			mf::LogInfo("PennReceiver")
				<< "sbnddaq::PennReceiver::getNext_ : no unprocessed filled buffers available at end of run";
			is_active = false;
		}
	}
	if (data_receiver_->exception())
	{
	  set_exception(true);
	  mf::LogError("PennReceiver") << "sbnddaq::PennReceiver::getNext_ : found receiver thread in exception state";
	}

	// JCF, Dec-11-2015

	// Finally, if we haven't received a buffer after a set amount
	// of time, give up (i.e., throw an exception)

	if ( ! buffers_found_in_while_loop ) {

	  auto time_since_last_buffer = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - last_buffer_received_time_).count();

	  if (time_since_last_buffer > data_timeout_usecs_) {
	    mf::LogError("PennReceiver") << "sbnddaq::PennReceiver::getNext_ : timeout due to no data appearing after " << time_since_last_buffer / 1000000.0 << " seconds";
	  }
	}

	return is_active;
}
#else
bool sbnddaq::PennReceiver::getNext_(artdaq::FragmentPtrs & frags) {

  PennRawBufferPtr recvd_buffer;
  bool buffer_available = false;

  // Wait for a filled buffer to be available, with a timeout tick to allow the should_stop() state
  // to be checked
  do
  {
    buffer_available = data_receiver_->retrieve_filled_buffer(recvd_buffer, 500000);
    if (reporting_interval_time_ != 0) 
    {
      if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - report_time_).count() >
      (reporting_interval_time_ * 1000))
      {
        report_time_ = std::chrono::high_resolution_clock::now();
        mf::LogDebug("PennReceiver") << "Received " << millislices_received_ << " millislices so far";

      }
    }
  }
  while (!buffer_available && !should_stop());

// ***********************************************************************************************
// ** GDB: This #else (OLD_GETNEXT) #endif is not what we are testing at the moment.  If we want to
// ** use it, there is an update to be applied here.  (1) To the above do while() last line and (2)
// ** in the next if () block.  To update, look in the corresponding place in TpcRceReceiver_generator.cc
// ** and look for the calls to data_receiver_->exception() around this place and copy the code in
// ***********************************************************************************************

  // If stopping, Check if there are any filled buffers available (i.e. fragments received but not processed)
  // and print a warning, then return false to indicate no more fragments to process
  if (should_stop()) {
    // GBcopy:  We could add the [data_receiver_->exception()] here
    if( data_receiver_->filled_buffers_available() > 0)
    {
      mf::LogWarning("PennRecevier") << "getNext_ stopping while there were still filled buffers available";
    }
    else
    {
      mf::LogInfo("PennReceiver") << "No unprocessed filled buffers available at end of run";
    }

    return false;
  }


  // If there was no data received (or an error flagged), simply return
  // an empty list
  if (recvd_buffer->size() == 0)
  {
    mf::LogWarning("PennReceiver") << "getNext_ : no data received in raw buffer";
    return true;
  }

  uint8_t* data_ptr = recvd_buffer->dataPtr();

  std::unique_ptr<artdaq::Fragment> frag;
  uint32_t millislice_size = 0;

  if (use_fragments_as_raw_buffer_)
  {
    //mf::LogWarning("PennReceiver") << "getNext_ : Using fragments as raw buffers. This does not remove warning words from the Millislices.";

    // Map back onto the fragment from the raw buffer data pointer we have just received
    if (raw_to_frag_map_.count(data_ptr))
    {
      // Extract the fragment from the map
      frag = std::move(raw_to_frag_map_[data_ptr]);

      // Validate and finalize the fragment received
      millislice_size = validate_millislice_from_fragment_buffer(frag->dataBeginBytes(), recvd_buffer->size(),
#ifndef REBLOCK_PENN_USLICE
          recvd_buffer->count(),
#endif
          recvd_buffer->sequenceID(),
          recvd_buffer->countPayload(), recvd_buffer->countPayloadCounter(),
          recvd_buffer->countPayloadTrigger(), recvd_buffer->countPayloadTimestamp(),
          recvd_buffer->endTimestamp(), recvd_buffer->widthTicks(), recvd_buffer->overlapTicks());


      // Clean up entry in map to remove raw fragment buffer
      raw_to_frag_map_.erase(data_ptr);

      // Create a new raw buffer pointing at a new fragment and replace the received buffer
      // pointer with it - this will be recycled onto the empty queue later
      recvd_buffer = create_new_buffer_from_fragment();

    }
    else
    {
      mf::LogError("PennReceiver") << "Cannot map raw buffer with data address" << (void*)recvd_buffer->dataPtr() << " back onto fragment";
      return true;
    }
  }
  else
  {
    mf::LogWarning("PennReceiver") << "Raw buffer mode has not been tested.";
    // Create an artdaq::Fragment to format the raw data into. As a crude heuristic,
    // reserve 10 times the raw data space in the fragment for formatting overhead.
    size_t fragDataSize = recvd_buffer->size() * 10;
    frag = artdaq::Fragment::FragmentBytes(fragDataSize);

#ifndef REBLOCK_PENN_USLICE
    // Format the raw digitisations (nanoslices) in the received buffer into a millislice
    // within the data payload of the fragment
    millislice_size = format_millislice_from_raw_buffer((uint16_t*)data_ptr, recvd_buffer->size(),
        (uint8_t*)(frag->dataBeginBytes()), fragDataSize);
#endif

  }

  // Update statistics counters
  millislices_received_++;
  total_bytes_received_ += millislice_size;

  if ((millislices_received_ % reporting_interval_fragments_) == 0)
  {
    auto elapsed_msecs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time_).count();
    double elapsed_secs = ((double)elapsed_msecs)/1000;

    mf::LogDebug("PennReceiver") << "Received " << millislices_received_ << " millislices, "
				       << float(total_bytes_received_)/(1024*1024) << " MB in " << elapsed_secs << " seconds";
  }

// Update buffer high and low water marks
  size_t empty_buffers_available = data_receiver_->empty_buffers_available();
  if (empty_buffers_available < empty_buffer_low_mark_)
  {
          empty_buffer_low_mark_ = empty_buffers_available;
  }
  size_t filled_buffers_available = data_receiver_->filled_buffers_available();
  if (filled_buffers_available > filled_buffer_high_mark_)
  {
          filled_buffer_high_mark_ = filled_buffers_available;
  }


  // Recycle the raw buffer onto the commit queue for re-use by the receiver.
  // against a low water mark and create/inject ones to keep the receiver running if necessary
  data_receiver_->commit_empty_buffer(recvd_buffer);

  // Set fragment fields appropriately
  frag->setSequenceID(ev_counter());
  frag->setFragmentID(fragmentIDs()[0]);
  frag->setUserType(sbnddaq::detail::TRIGGER);

  // Resize fragment to final millislice size
  frag->resizeBytes(millislice_size);

  // add the fragment to the list
  frags.emplace_back(std::move (frag));

  // increment the event counter
  ev_counter_inc();

  return true;
}
#endif  // ifndef OLD_GETNEXT

sbnddaq::PennRawBufferPtr sbnddaq::PennReceiver::create_new_buffer_from_fragment(void)
{
  PennRawBufferPtr raw_buffer;

#ifdef __PTB_BOARD_READER_DEVEL_MODE__
  mf::LogDebug("PennReceiver") << "Producing a artdaq fragment with size: "
      << raw_buffer_size_ + sizeof(PennMilliSlice::Header);
#endif
  std::unique_ptr<artdaq::Fragment> frag = artdaq::Fragment::FragmentBytes(raw_buffer_size_ + sizeof(PennMilliSlice::Header));
  uint8_t* data_ptr = frag->dataBeginBytes() + sizeof(PennMilliSlice::Header);
  raw_buffer = sbnddaq::PennRawBufferPtr(new PennRawBuffer(data_ptr, raw_buffer_size_));

  raw_to_frag_map_.insert(std::pair<uint8_t*, std::unique_ptr<artdaq::Fragment> >(raw_buffer->dataPtr(), std::move(frag)));

  return raw_buffer;

}

#ifndef REBLOCK_PENN_USLICE
uint32_t sbnddaq::PennReceiver::format_millislice_from_raw_buffer(uint16_t* src_addr, size_t src_size,
    uint8_t* dest_addr, size_t dest_size)
{
  const uint32_t MICROSLICE_BUFFER_SIZE = 512;

  std::shared_ptr<sbnddaq::PennMicroSliceWriter> microslice_writer_ptr;

  uint16_t* sample_addr = src_addr;

  sbnddaq::PennMilliSliceWriter millislice_writer(dest_addr, dest_size);
  for (uint32_t udx = 0; udx < number_of_microslices_to_generate_; ++udx) {
    microslice_writer_ptr = millislice_writer.reserveMicroSlice(MICROSLICE_BUFFER_SIZE);
    if (microslice_writer_ptr.get() == 0) {
      mf::LogError("PennReceiver")
      << "Unable to create microslice number " << udx;
    }
    else {
      //get the size of the microslice from the header 
      // and divide by 2 (unit in header is bytes, but working with uint16_t)
      uint16_t microslice_size = sample_addr[1] / 2;
      //add the microslice to the microslice writer
      microslice_writer_ptr->addData(sample_addr, microslice_size);
      microslice_writer_ptr->finalize();
      //move to the next microslice
      sample_addr += microslice_size;
    }
  }

  // Check if we have overrun the end of the raw buffer
  if (sample_addr > (src_addr + src_size))
  {
    mf::LogError("PennReceiver")
    << "Raw buffer overrun during millislice formatting by " << ((src_addr + src_size) - sample_addr);
  }
  millislice_writer.finalize();
  return millislice_writer.size();

}
#endif

uint32_t sbnddaq::PennReceiver::validate_millislice_from_fragment_buffer(uint8_t* data_addr, size_t data_size,
#ifndef REBLOCK_PENN_USLICE
    uint32_t us_count,
#endif
    uint16_t millislice_id,
    uint16_t payload_count, uint16_t payload_count_counter,
    uint16_t payload_count_trigger, uint16_t payload_count_timestamp,
    uint64_t end_timestamp, uint32_t width_in_ticks, uint32_t overlap_in_ticks)
{
  sbnddaq::PennMilliSliceWriter millislice_writer(data_addr, data_size+sizeof(PennMilliSlice::Header));

  millislice_writer.finalize(true, data_size,
#ifndef REBLOCK_PENN_USLICE
      us_count,
#endif
      millislice_id,
      payload_count, payload_count_counter, payload_count_trigger, payload_count_timestamp,
      end_timestamp, width_in_ticks, overlap_in_ticks);
  //TODO add a check here to make sure the size agrees with the payload counts + header size

  return millislice_writer.size();
}

void sbnddaq::PennReceiver::generate_config_frag(std::ostringstream& config_frag) {

  // The config wrapper is added by the PennClient class just prior to send.
  config_frag << "<config>";
  config_frag << "<Hardware>"
      << "<PulseWidth>" << ptb_pulse_width_ << "</PulseWidth>"
      << "</Hardware>";

  std::string dry_run_bool = (penn_dry_run_)?"true":"false";
  // -- DataBuffer section. Controls the reader itself
  config_frag << "<DataBuffer>"
      << "<DryRun>" << dry_run_bool << "</DryRun>"
      << "<DaqHost>" << penn_data_dest_host_ << "</DaqHost>"
      << "<DaqPort>" << penn_data_dest_port_ << "</DaqPort>"
      // FIXME: Add missing variables
      // Should we have a rollover or just use a microslice size for time?
      << "<RollOver>" << penn_data_dest_rollover_ << "</RollOver>"
      << "<MicroSliceDuration>" << penn_data_microslice_size_ << "</MicroSliceDuration>"
      << "</DataBuffer>";

  // -- Channel masks section. Controls the reader itself
  config_frag << "<ChannelMask>"
      << "<BSU>0x" << std::hex << static_cast<uint64_t>(penn_channel_mask_bsu_) << std::dec << "</BSU>"
      << "<TSU>0x" << std::hex << static_cast<uint64_t>(penn_channel_mask_tsu_) << std::dec << "</TSU>"
      << "</ChannelMask>";

  // -- Muon trigger section


  config_frag << "<MuonTriggers num_triggers=\"" << penn_muon_num_triggers_ << "\">"
      << "<TriggerWindow>" << std::dec << penn_trig_in_window_  << std::dec << "</TriggerWindow>"
      << "<LockdownWindow>" << std::dec << penn_trig_lockdown_window_  << std::dec << "</LockdownWindow>";
  for (size_t i = 0; i < penn_muon_num_triggers_; ++i) {
    // I would rather put masks as hex strings to be easier to understand
    config_frag << "<TriggerMask id=\"" << muon_triggers_.at(i).id << "\" mask=\"" << muon_triggers_.at(i).id_mask <<  "\">"
        << "<ExtLogic>0x" << std::hex << static_cast<int>(muon_triggers_.at(i).logic) << std::dec << "</ExtLogic>"
        << "<Prescale>" << std::dec << static_cast<uint32_t>(muon_triggers_.at(i).prescale) << std::dec << "</Prescale>"
        << "<group1><Logic>0x" << std::hex << static_cast<int>(muon_triggers_.at(i).g1_logic) << std::dec << "</Logic>"
        << "<BSU>0x" << std::hex << static_cast<uint64_t>(muon_triggers_.at(i).g1_mask_bsu) << std::dec << "</BSU>"
        << "<TSU>0x" << std::hex << static_cast<uint64_t>(muon_triggers_.at(i).g1_mask_tsu) << std::dec << "</TSU></group1>"
        << "<group2><Logic>0x" << std::hex << static_cast<int>(muon_triggers_.at(i).g2_logic) << std::dec << "</Logic>"
        << "<BSU>0x" << std::hex << static_cast<uint64_t>(muon_triggers_.at(i).g2_mask_bsu) << std::dec << "</BSU>"
        << "<TSU>0x" << std::hex << static_cast<uint64_t>(muon_triggers_.at(i).g2_mask_tsu) << std::dec << "</TSU></group2>"
        << "</TriggerMask>";
  }
  config_frag << "</MuonTriggers>";

  // -- External triggers

  std::string status_bool = (penn_ext_triggers_echo_)?"true":"false";
  config_frag << "<ExtTriggers>"
      << "<mask>0x" << std::hex << static_cast<uint32_t>(penn_ext_triggers_mask_) << std::dec << "</mask>"
      << "<echo_enabled>" <<  status_bool << "</echo_enabled>"
      << "<gate>" << penn_ext_triggers_gate_ << "</gate>"
      << "<prescale>" << penn_ext_triggers_prescale_ << "</prescale>"
      << "</ExtTriggers>";

  // -- Calibration channels
  config_frag << "<Calibrations>";
  for (size_t i = 0; i < penn_num_calibration_channels_; ++i) {
    status_bool = (calib_channels_.at(i).enabled)?"true":"false";
    config_frag << "<CalibrationMask id=\"" << calib_channels_.at(i).id << "\""
        << " mask=\"" << calib_channels_.at(i).id_mask << "\">"
        << "<enabled>" << status_bool << "</enabled>"
        << "<period>" << std::dec << calib_channels_.at(i).period << std::dec << "</period>"
        << "</CalibrationMask>";
  } 
  config_frag << "</Calibrations>";


  // And finally close the tag
  config_frag << "</config>";

}

///////////////////////////////////////
///
/// Emulator methods. Most of these are outdated now.
/// TODO: Tentatively remove the methods below
///
///////////////////////////////////////


void sbnddaq::PennReceiver::generate_config_frag_emulator(std::ostringstream& config_frag) {

  config_frag << "<Emulator>" << " "
      << " <NumMillislices>" << penn_data_num_millislices_ << "</NumMillislices>" << " "
      << " <NumMicroslices>" << penn_data_num_microslices_ << "</NumMicroslices>" << " "
      << " <SendRate>"       << penn_data_frag_rate_       << "</SendRate>"       << " "
      << " <PayloadMode>"    << penn_data_payload_mode_    << "</PayloadMode>"    << " "
      << " <TriggerMode>"    << penn_data_trigger_mode_    << "</TriggerMode>"    << " "
      << " <FragmentUSlice>" << penn_data_fragment_microslice_at_ticks_ << "</FragmentUSlice>" << " "
      << " <SendRepeats>"    << penn_data_repeat_microslices_           << "</SendRepeats>" << " "
      << " <SendByByte>"     << penn_data_debug_partial_recv_           << "</SendByByte>" << " "
      << "</Emulator>" << " ";
}

// GBcopy - The next function is copied fropm the RCE boardreader, with comments from John
// JCF, Dec-12-2015

// startOfDatataking() will figure out whether we've started taking
// data either by seeing that the run number's incremented since the
// last time it was called (implying the start transition's occurred)
// or the subrun number's incremented (implying the resume
// transition's occurred). On the first call to the function, the
// "last checked" values of run and subrun are set to 0, guaranteeing
// that the first call will return true

bool sbnddaq::PennReceiver::startOfDatataking() {

  static int subrun_at_last_check = 0;
  static int run_at_last_check = 0;

  bool is_start = false;

  if ( (run_number() > run_at_last_check) ||
       (subrun_number() > subrun_at_last_check) ) {
    is_start = true;
  } 

  subrun_at_last_check=subrun_number();
  run_at_last_check=run_number();

  return is_start;
}


// The following macro is defined in artdaq's GeneratorMacros.hh header
DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(sbnddaq::PennReceiver) 
