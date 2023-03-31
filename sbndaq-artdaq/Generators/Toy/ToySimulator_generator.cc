// For an explanation of this class, look at its header,
// ToySimulator.hh, as well as
// https://cdcvs.fnal.gov/redmine/projects/artdaq-demo/wiki/Fragments_and_FragmentGenerators_w_Toy_Fragments_as_Examples

#include "ToySimulator.hh"

#include "canvas/Utilities/Exception.h"

#include "artdaq/Generators/GeneratorMacros.hh"
#include "artdaq-core/Utilities/SimpleLookupPolicy.hh"

#include "Overlay/ToyFragment.hh"
#include "Overlay/FragmentType.hh"

#include "fhiclcpp/ParameterSet.h"

#include <fstream>
#include <iomanip>
#include <iterator>
#include <iostream>

#include <unistd.h>
#define TRACE_NAME "ToySimulator"
#include "tracemf.h"		// TRACE, TLOG*
#include "cetlib_except/exception.h"

demo::ToySimulator::ToySimulator(fhicl::ParameterSet const& ps)
	:
	CommandableFragmentGenerator(ps)
	, hardware_interface_(new ToyHardwareInterface(ps))
	, timestamp_(0)
	, timestampScale_(ps.get<int>("timestamp_scale_factor", 1))
	, rollover_subrun_interval_(ps.get<int>("rollover_subrun_interval", 0))
	, metadata_({ 0,0,0 })
	, readout_buffer_(nullptr)
	, fragment_type_(static_cast<decltype(fragment_type_)>(artdaq::Fragment::InvalidFragmentType))
	, distribution_type_(static_cast<ToyHardwareInterface::DistributionType>(ps.get<int>("distribution_type")))
        , generated_fragments_per_event_(ps.get<int>("generated_fragments_per_event", 1))
        , exception_on_config_(ps.get<bool>("exception_on_config", false))                                             , dies_on_config_(ps.get<bool>("dies_on_config", false))

{
	hardware_interface_->AllocateReadoutBuffer(&readout_buffer_);

	if (exception_on_config_) {
	  throw cet::exception("ToySimulator") << "This is an engineered exception designed for testing purposes, set by the exception_on_config FHiCL variable";
	} else if (dies_on_config_) {
	  TLOG(TLVL_ERROR) << "This is an engineered process death, set by the dies_on_config FHiCL variable";
	  std::exit(1);
	}

	metadata_.board_serial_number = hardware_interface_->SerialNumber() & 0xFFFF;
	metadata_.num_adc_bits = hardware_interface_->NumADCBits();
	TLOG(TLVL_INFO) << "Constructor: metadata_.unused = 0x" << std::hex << metadata_.unused << " sizeof(metadata_) = " << std::dec << sizeof(metadata_);

	switch (hardware_interface_->BoardType())
	{
	case 1002:
		fragment_type_ = toFragmentType("TOY1");
		break;
	case 1003:
		fragment_type_ = toFragmentType("TOY2");
		break;
	default:
		throw cet::exception("ToySimulator") << "Unable to determine board type supplied by hardware";
	}
}

demo::ToySimulator::~ToySimulator()
{
	hardware_interface_->FreeReadoutBuffer(readout_buffer_);
}

bool demo::ToySimulator::getNext_(artdaq::FragmentPtrs& frags)
{
	if (should_stop())
	{
		return false;
	}

	// ToyHardwareInterface (an instance to which "hardware_interface_"
	// is a unique_ptr object) is just one example of the sort of
	// interface a hardware library might offer. For example, other
	// interfaces might require you to allocate and free the memory used
	// to store hardware data in your generator using standard C++ tools
	// (rather than via the "AllocateReadoutBuffer" and
	// "FreeReadoutBuffer" functions provided here), or could have a
	// function which directly returns a pointer to the data buffer
	// rather than sticking the data in the location pointed to by your
	// pointer (which is what happens here with readout_buffer_)

	std::size_t bytes_read = 0;
	hardware_interface_->FillBuffer(readout_buffer_, &bytes_read);

	// We'll use the static factory function 

	// artdaq::Fragment::FragmentBytes(std::size_t payload_size_in_bytes, sequence_id_t sequence_id,
	//  fragment_id_t fragment_id, type_t type, const T & metadata)

	// which will then return a unique_ptr to an artdaq::Fragment
	// object. 

#if 1
	for (auto i_f = 0; i_f < generated_fragments_per_event_; ++i_f) {

	  // The offset logic below is designed to both ensure
	  // backwards compatibility and to (help) avoid collisions
	  // with fragment_ids from other boardreaders if more than
	  // one fragment is generated per event

	  auto offset = i_f == 0 ? 0 : i_f + 10000;
	  std::unique_ptr<artdaq::Fragment> fragptr(
						    artdaq::Fragment::FragmentBytes(bytes_read,
										    ev_counter(), 
										    fragment_id() + offset,
										    fragment_type_,
										    metadata_, timestamp_));
	  frags.emplace_back(std::move(fragptr));
	}
#else
	std::unique_ptr<artdaq::Fragment> fragptr(
		artdaq::Fragment::FragmentBytes(/*bytes_read*/ 1024 - 40,
										ev_counter(), fragment_id(),
										fragment_type_,
										metadata_, timestamp_));
	frags.emplace_back(std::move(fragptr));
	artdaq::detail::RawFragmentHeader *hdr = (artdaq::detail::RawFragmentHeader*)(frags.back()->headerBeginBytes());
	// Need a way to fake frag->sizeBytes() (which calls frag->size() which calls fragmentHeader()->word_count
	hdr->word_count = ceil((bytes_read + 32) / static_cast<double>(sizeof(artdaq::RawDataType)));
#endif

	if ( !frags.empty() ) {

	  if (distribution_type_ != ToyHardwareInterface::DistributionType::uninitialized)
	    memcpy(frags.back()->dataBeginBytes(), readout_buffer_, bytes_read);
	  else
	    {
	      // Must preserve the Header!
	      memcpy(frags.back()->dataBeginBytes(), readout_buffer_, sizeof(ToyFragment::Header));
	    }

	  TLOG(50) << "getNext_ after memcpy " << bytes_read
		   << " bytes and std::move dataSizeBytes()=" << frags.back()->sizeBytes() << " metabytes=" << sizeof(metadata_);
	}

	// if (metricMan != nullptr)
	// {
	//	metricMan->sendMetric("Fragments Sent", ev_counter(), "Events", 3, artdaq::MetricMode::LastPoint);
	//}

	if (rollover_subrun_interval_ > 0 && ev_counter() % rollover_subrun_interval_ == 0 && fragment_id() == 0)
	{
                unsigned my_rank = 0;
		artdaq::FragmentPtr endOfSubrunFrag(new artdaq::Fragment(static_cast<size_t>(ceil(sizeof(my_rank) / static_cast<double>(sizeof(artdaq::Fragment::value_type))))));
		endOfSubrunFrag->setSystemType(artdaq::Fragment::EndOfSubrunFragmentType);

		endOfSubrunFrag->setSequenceID(ev_counter() + 1);
		endOfSubrunFrag->setTimestamp(1 + (ev_counter() / rollover_subrun_interval_));

		*endOfSubrunFrag->dataBegin() = my_rank;
		frags.emplace_back(std::move(endOfSubrunFrag));
	}

	ev_counter_inc();
	timestamp_ += timestampScale_;

	return true;
}

void demo::ToySimulator::start()
{
	hardware_interface_->StartDatataking();
	timestamp_ = 0;
}

void demo::ToySimulator::stop()
{
	hardware_interface_->StopDatataking();
}

// The following macro is defined in artdaq's GeneratorMacros.hh header
DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(demo::ToySimulator)
