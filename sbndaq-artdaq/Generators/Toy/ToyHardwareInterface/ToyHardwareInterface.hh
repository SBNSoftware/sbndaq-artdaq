#ifndef artdaq_demo_Generators_ToyHardwareInterface_ToyHardwareInterface_hh
#define artdaq_demo_Generators_ToyHardwareInterface_ToyHardwareInterface_hh

#include "../Overlay/FragmentType.hh"

#include "fhiclcpp/fwd.h"

#include <random>
#include <chrono>

/**
 * \brief JCF, Mar-17-2016: ToyHardwareInterface is meant to mimic a vendor-provided hardware
 * API, usable within the the ToySimulator fragment generator. For
 * purposes of realism, it's a C++03-style API, as opposed to, say,
 * one based in C++11 capable of taking advantage of smart pointers,
 * etc. An important point to make is that it has ownership of the
 * buffer into which it dumps its data - so rather than use
 * new/delete, use its functions
 * AllocateReadoutBuffer/FreeReadoutBuffer
 */
class ToyHardwareInterface
{
public:

	typedef uint16_t data_t; ///< The type used to represent ADC counts (which are 12 or 14 bits, for TOY1 or TOY2)

	/**
	 * \brief Construct and configure ToyHardwareInterface
	 * \param ps fhicl::ParameterSet with configuration options for ToyHardwareInterface
	 */
	explicit ToyHardwareInterface(fhicl::ParameterSet const& ps);

	/**
	 * \brief  "StartDatataking" is meant to mimic actions one would take when
	 * telling the hardware to start sending data - the uploading of
	 * values to registers, etc.
	 */
	void StartDatataking();

	/**
	 * \brief Performs shutdown actions
	 */
	void StopDatataking();

	/**
	 * \brief Use configured generator to fill a buffer with data
	 * \param buffer Buffer to fill
	 * \param bytes_read Number of bytes to fill
	 */
	void FillBuffer(char* buffer, size_t* bytes_read);

	/**
	 * \brief Request a buffer from the hardware
	 * \param buffer (output) Pointer to buffer
	 */
	void AllocateReadoutBuffer(char** buffer);

	/**
	 * \brief Release the given buffer to the hardware
	 * \param buffer Buffer to release
	 */
	void FreeReadoutBuffer(char* buffer);

	/**
	 * \brief Gets the serial number of the simulated hardware
	 * \return Serial number of the simulated hardware
	 */
	int SerialNumber() const;

	/**
	 * \brief Get the number of ADC bits used in generating data
	 * \return The number of ADC bits used
	 */
	int NumADCBits() const;

	/**
	 * \brief Return the "board type" of the simulated hardware
	 * \return A vendor-provided integer identifying the board type
	 */
	int BoardType() const;

	/**
	 * \brief Allow for the selection of output distribution
	 */
	enum class DistributionType
	{
		uniform, ///< A uniform distribution
		gaussian, ///< A Gaussian distribution
		monotonic, ///< A monotonically-increasing distribution
		uninitialized, ///< A use-after-free expliot distribution
		uninit2		   // like uninitialized, but do memcpy
	};

private:

	bool taking_data_;

	std::size_t nADCcounts_;
	std::size_t maxADCcounts_;
	std::size_t change_after_N_seconds_;
	int nADCcounts_after_N_seconds_;
	bool exception_after_N_seconds_;
	bool exit_after_N_seconds_;
	bool abort_after_N_seconds_;
	demo::FragmentType fragment_type_;
	std::size_t maxADCvalue_;
	std::size_t throttle_usecs_;
	std::size_t usecs_between_sends_;
	DistributionType distribution_type_;

	using time_type = decltype(std::chrono::steady_clock::now());

	const time_type fake_time_ = std::numeric_limits<time_type>::max();

	// Members needed to generate the simulated data

	std::mt19937 engine_;
	std::unique_ptr<std::uniform_int_distribution<data_t>> uniform_distn_;
	std::unique_ptr<std::normal_distribution<double>> gaussian_distn_;

	time_type start_time_;
	int send_calls_;
	int serial_number_;
};


#endif
