#ifndef artdaq_demo_Overlays_ToyFragment_hh
#define artdaq_demo_Overlays_ToyFragment_hh

#include "artdaq-core/Data/Fragment.hh"

#include <ostream>

namespace demo
{
	class ToyFragment;

	/// Let the "<<" operator dump the ToyFragment's data to stdout
	std::ostream& operator <<(std::ostream&, ToyFragment const&);
}

/**
 * \brief An artdaq::Fragment overlay class used for pedagogical purposes.
 *
 * The intention of this class is to provide an example of how an Overlay
 * can be constructed for a hypothetical 16-bit ADC module.  In this
 * example, the ADC data can have either 12 or 14 bits of meaningful
 * information per 16-bit word.  (This is just a characteristic of this
 * example.  Separate classes could certainly be used to overlay the two
 * types of ADC data.)
 */
class demo::ToyFragment
{
public:

	/**
	 * \brief The ToyFragment::Metadata struct holds data about the readout that
	 * may not be present in the primary data stream.
	 *
	 * The "Metadata" struct declared here is used to define the set of
	 * information that is stored in the "metadata" section of
	 * artdaq::Fragments that contain ToyFragments in their payload.
	 * This struct is not used by the ToyFragment Overlay class itself;
	 * it is defined here simply as a convenience.  The types of
	 * information that are typically stored in artdaq::Fragment
	 * metadata blocks are things like hardware serial numbers or values
	 * of an interesting configuration parameter. These things are
	 * generally *not* part of the data read out from the electronics
	 * hardware in each event, hence the need to store them outside the
	 * block of data that the hardware provides.
	 */
	struct Metadata
	{
		// "data_t" is a typedef of the fundamental unit of data the
		// metadata structure thinks of itself as consisting of; it can give
		// its size via the static "size_words" variable (
		// ToyFragment::Metadata::size_words )

		typedef uint64_t data_t; ///< Fundamental unit of data representation within the ToyFragment::Metadata

		data_t board_serial_number : 16; ///< Serial number of the simulated board
		data_t num_adc_bits : 8; ///< Number of ADC bits this simulated board uses
		data_t unused : 40; ///< Unused

		static size_t const size_words = 1ul; ///< Size of the ToyFragment::Metadata struct, in units of Metadata::data_t
	};

	static_assert (sizeof(Metadata) == Metadata::size_words * sizeof(Metadata::data_t), "ToyFragment::Metadata size changed");

	/**
	 * \brief ToyFragment is intended to represent/interpret data that has an
	 * inherent size of 16 bits (unsigned). This is represented by the
	 * adc_t type that is declared here.
	 */
	typedef uint16_t adc_t;

	/**
	 * \brief The "Header" struct is used to interpret the header information that is
	 * created by the hardware and *is* part of the data blob that is read from
	 * the hardware for each event. Not to be confused with the Fragment::Header or
	 * the ToyFragment::Metadata objects.
	 */
	struct Header
	{
		// Header::data_t -- not to be confused with Metadata::data_t ! --
		// describes the standard size of a data type in the header.
		// In this example, it is ALSO used to describe the size of the physics data
		// beyond the header. This is not a general requirement for Overlay classes;
		// it is simply the choice that was made for this example.
		// The size of the header in units of Header::data_t is given by "size_words",
		// and the size of the fragment beyond the header in units of
		// Header::data_t is given by "event_size". Again, this is simply an
		// artifact of this example. A real-life hardware module may pack its data
		// differently, and any "size" fields in that real-life data could include
		// the size of any header information provided by the hardware.

		typedef uint32_t data_t; ///< Fundamental unit of data represenation within the ToyFragment::Header

		typedef uint32_t event_size_t; ///< Type used for the event size
		typedef uint32_t trigger_number_t; ///< Type used for the trigger number

		event_size_t event_size : 28; ///< The size of the event, in data_t words
		event_size_t distribution_type : 4; ///< Which distribution_type was used to generate the Fragment data

		trigger_number_t trigger_number : 32; ///< The "trigger number" from the simulated hardware

		static size_t const size_words = 2ul; ///< Size of the ToyFragment::Header class, in units of Header::data_t
	};

	static_assert (sizeof(Header) == Header::size_words * sizeof(Header::data_t), "ToyFragment::Header size changed");

	/**
	* \brief The ToyFragment constructor
	* \param f The raw artdaq::Fragment object to overlay
	*
	* The constructor simply sets its const private member "artdaq_Fragment_" to refer to the artdaq::Fragment object
	*/
	explicit ToyFragment(artdaq::Fragment const& f) : artdaq_Fragment_(f) {}

	/**
	* \brief Gets the event_size from the ToyFragment::Header
	* \return The number of Header::data_t words in the ToyFragment
	*/
	Header::event_size_t hdr_event_size() const { return header_()->event_size; }
	/**
	* \brief Gets the trigger_number from the ToyFragment::Header
	* \return The "trigger number" of the ToyFragment::Header
	*/
	Header::trigger_number_t hdr_trigger_number() const { return header_()->trigger_number; }

	/**
	 * \brief Returns the distribution_type field from the ToyFragment::Header
	 * \return The distribution_type field from the ToyFragment::Header
	 */
	uint8_t hdr_distribution_type() const { return static_cast<uint8_t>(header_()->distribution_type); }

	/**
	* \brief Gets the size_words variable from the artdaq::Header
	* \return The size of the Fragment payload
	*/
	static constexpr size_t hdr_size_words() { return Header::size_words; }

	/**
	 * \brief Get the number of ADC values describing data beyond the header
	 * \return The number of ADC values describing data beyond the header
	 */
	size_t total_adc_values() const
	{
		return (hdr_event_size() - hdr_size_words()) * adcs_per_word_();
	}

	/**
	 * \brief Retrieve the given ADC value from the Fragment
	 * \param index Index of the ADC value
	 * \return ADC value at given index, or 0xFFFF if out-of-bounds
	 */
	adc_t adc_value(uint32_t index)
	{
		// Simple way to handle index out of bounds - better ways are surely possible
		if (index >= total_adc_values()) { return 0xffff; }
		return dataBeginADCs()[index];
	}

	/**
	 * \brief Start of the ADC values, returned as a const pointer to the ADC type
	 * \return const adc_t pointer to the start of the payload
	 */
	adc_t const* dataBeginADCs() const
	{
		return reinterpret_cast<adc_t const *>(header_() + 1);
	}

	/**
	 * \brief End of the ADC values, returned as a const pointer to the ADC type
	 * \return const adc_t pointer to the end of the payload
	 */
	adc_t const* dataEndADCs() const
	{
		return dataBeginADCs() + total_adc_values();
	}

	// Functions to check if any ADC values are corrupt
	/**
	 * \brief Look through stored ADC values and determine if any are out-of-range
	 * \param daq_adc_bits Number of bits for ADC values
	 * \return const adc_t pointer to first bad ADC value (if any), or dataEndADCs otherwise
	 *
	 * findBadADC() checks to make sure that the ADC type (adc_t) variable
	 * holding the ADC value doesn't contain bits beyond the expected
	 * range, i.e., can't be evaluated to a larger value than the max
	 * permitted ADC value
	 */
	adc_t const* findBadADC(int daq_adc_bits) const
	{
		return std::find_if(dataBeginADCs(), dataEndADCs(),
							[&](adc_t const adc) -> bool
		{
			return (adc >> daq_adc_bits);
		});
	}

	/**
	 * \brief Do any ADC values in the ToyFragment appear to be out-of-range?
	 * \param daq_adc_bits Number of bits for ADC values
	 * \return Whether any of the ADC values in the ToyFragment are out-of-range
	 */
	bool fastVerify(int daq_adc_bits) const
	{
		return (findBadADC(daq_adc_bits) == dataEndADCs());
	};

	/**
	 * \brief Throw if any ADC value appears corrupt
	 * \param daq_adc_bits Number of bits for ADC values
	 * \throws cet::exception if any ADC value is out-of-range
	 */
	void checkADCData(int daq_adc_bits) const;

	/**
	 * \brief Get the largest ADC value possible for a given number of ADC bits
	 * \param daq_adc_bits Number of bits in the ADC field
	 * \return Largest value possible with the given number of ADC bits
	 */
	static size_t adc_range(int daq_adc_bits)
	{
		return (1ul << daq_adc_bits) - 1;
	}

protected:

	/**
	 * \brief Translates between the size of an ADC and the size in Header::data_t words
	 * \return Number of adc_t counts per Header::data_t word
	 */
	static constexpr size_t adcs_per_word_()
	{
		return sizeof(Header::data_t) / sizeof(adc_t);
	}

	/**
	 * \brief Get a pointer to the ToyFragment::Header object
	 * \return Const pointer to the ToyFragment::Header object
	 *
	 * header_() simply takes the address of the start of this overlay's
	 * data (i.e., where the ToyFragment::Header object begins) and
	 * casts it as a const pointer to ToyFragment::Header
	 */
	Header const* header_() const
	{
		return reinterpret_cast<ToyFragment::Header const *>(artdaq_Fragment_.dataBeginBytes());
	}

private:

	artdaq::Fragment const& artdaq_Fragment_;
};

#endif /* artdaq_demo_Overlays_ToyFragment_hh */
