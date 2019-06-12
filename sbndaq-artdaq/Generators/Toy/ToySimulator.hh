#ifndef artdaq_demo_Generators_ToySimulator_hh
#define artdaq_demo_Generators_ToySimulator_hh


// Some C++ conventions used:

// -Append a "_" to every private member function and variable

#include "fhiclcpp/fwd.h"
#include "artdaq-core/Data/Fragment.hh"
#include "artdaq/Generators/CommandableFragmentGenerator.hh"
#include "Overlay/ToyFragment.hh"
#include "Overlay/FragmentType.hh"

#include "ToyHardwareInterface/ToyHardwareInterface.hh"

#include <random>
#include <vector>
#include <atomic>

namespace demo
{
	/**
	 * \brief ToySimulator is a simple type of fragment generator intended to be
	 * studied by new users of artdaq as an example of how to create such
	 * a generator in the "best practices" manner. Derived from artdaq's
	 * CommandableFragmentGenerator class, it can be used in a full DAQ
	 * simulation, obtaining data from the ToyHardwareInterface class
	 *
	 * ToySimulator is designed to simulate values coming in from one of
	 * two types of digitizer boards, one called "TOY1" and the other
	 * called "TOY2"; the only difference between the two boards is the #
	 * of bits in the ADC values they send. These values are declared as
	 * FragmentType enum's in artdaq-demo's
	 * artdaq-core-demo/Overlays/FragmentType.hh header.
	 */
	class ToySimulator : public artdaq::CommandableFragmentGenerator
	{
	public:
		/**
		 * \brief ToySimulator Constructor
		 * \param ps ParameterSet used to configure ToySimulator
		 * 
		 * The ToySimulator FragmentGenerator accepts the following configuration paramters:
		 * "timestamp_scale_factor" (Default: 1): How much to increment the timestamp Fragment Header field for each event
		 * "distribution_type" (REQUIRED): Which type of distribution to use when generating data. See ToyHardwareInterface for more information
		 * "rollover_subrun_interval" (Default: 0): If this ToySimulator has fragment_id 0, will cause the system to rollover subruns every N events. 0 (default) disables.
		 */
		explicit ToySimulator(fhicl::ParameterSet const& ps);

		/**
		 * \brief Shutdown the ToySimulator
		 */
		virtual ~ToySimulator();

	private:
		
		/**
		 * \brief The "getNext_" function is used to implement user-specific
		 * functionality; it's a mandatory override of the pure virtual
		 * getNext_ function declared in CommandableFragmentGenerator
		 * \param output New FragmentPtrs will be added to this container
		 * \return True if data-taking should continue
		 */
		bool getNext_(artdaq::FragmentPtrs& output) override;

		// The start, stop and stopNoMutex methods are declared pure
		// virtual in CommandableFragmentGenerator and therefore MUST be
		// overridden; note that stopNoMutex() doesn't do anything here

		/**
		 * \brief Perform start actions
		 * 
		 * Override of pure virtual function in CommandableFragmentGenerator.
		 */
		void start() override;

		/**
		* \brief Perform stop actions
		* 
		* Override of pure virtual function in CommandableFragmentGenerator.
		*/
		void stop() override;

		/**
		* \brief Override of pure virtual function in CommandableFragmentGenerator.
		* stopNoMutex does not do anything in ToySimulator
		*/
		void stopNoMutex() override {}

		std::unique_ptr<ToyHardwareInterface> hardware_interface_;
		artdaq::Fragment::timestamp_t timestamp_;
		int timestampScale_;
		int rollover_subrun_interval_;

		ToyFragment::Metadata metadata_;

		// buffer_ points to the buffer which the hardware interface will
		// fill. Notice that it's a raw pointer rather than a smart
		// pointer as the API to ToyHardwareInterface was chosen to be a
		// C++03-style API for greater realism

		char* readout_buffer_;

		FragmentType fragment_type_;
		ToyHardwareInterface::DistributionType distribution_type_;
	        int generated_fragments_per_event_;
	        bool exception_on_config_;
	        bool dies_on_config_;
	};
}

#endif /* artdaq_demo_Generators_ToySimulator_hh */
