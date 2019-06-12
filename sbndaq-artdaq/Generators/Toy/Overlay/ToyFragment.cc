#include "ToyFragment.hh"

#include "cetlib_except/exception.h"

#if 0
namespace {
  unsigned int pop_count (unsigned int n) {
    unsigned int c; 
    for (c = 0; n; c++) n &= n - 1; 
    return c;
  }
}
#endif

void demo::ToyFragment::checkADCData(int daq_adc_bits) const
{
	demo::ToyFragment::adc_t const* adcPtr(findBadADC(daq_adc_bits));
	if (adcPtr != dataEndADCs())
	{
		throw cet::exception("IllegalADCVal")
		      << "Illegal value of ADC word #"
		      << (adcPtr - dataBeginADCs())
		      << ": 0x"
		      << std::hex
		      << *adcPtr
		      << ".";
	}
}

std::ostream& demo::operator <<(std::ostream& os, ToyFragment const& f)
{
	os << "ToyFragment event size: "
		<< f.hdr_event_size()
		<< ", trigger number: "
		<< f.hdr_trigger_number()
		<< "\n";

	return os;
}
