#define TRACE_NAME "CAENV1730SerialReader"

#include "sbndaq-artdaq/Generators/SBND/CAENV1730SerialReader.hh"

#include "artdaq/Generators/GeneratorMacros.hh"
#include <boost/archive/binary_iarchive.hpp>

using sbndaq::CAENV1730SerialReader;

CAENV1730SerialReader::CAENV1730SerialReader(fhicl::ParameterSet const& ps)
  : CommandableFragmentGenerator{ps},
  binary_file_path_(ps.get<std::string>("BinaryFilePath"))
  {
    binary_file_.open(binary_file_path_);
  }

CAENV1730SerialReader::~CAENV1730SerialReader() {
  binary_file_.close();
}

void CAENV1730SerialReader::start() {}

void CAENV1730SerialReader::stop() {}

bool CAENV1730SerialReader::getNext_(artdaq::FragmentPtrs& fragments) {

  boost::archive::binary_iarchive ia(binary_file_);
  artdaq::Fragment::type_t frag_type;

  try {
    ia >> frag_type;
  }
  catch(const std::exception &ex) {
    TLOG(TLVL_ERROR) << "\nCatching exception in getNext_(artdaq::FragmentPtrs& fragments)\n"
		    << ex.what() << '\n'
		    << "Failed to access fragment type\n"
		    << "Exiting...\n" << std::endl;
  }

  if(frag_type == sbndaq::detail::FragmentType::CAENV1730)
    {
      TLOG(TLVL_INFO) << "\nReading fragment serial of type: CAENV1730\n";
      sbndaq::CAENV1730FragmentSerial serial;

      try {
	ia >> serial;
      }
      catch(const std::exception &ex) {
	TLOG(TLVL_ERROR) << "\nCatching exception in getNext_(artdaq::FragmentPtrs& fragments)\n"
			<< ex.what() << '\n'
			<< "Failed to access fragment serial\n"
			<< "Exiting...\n" << std::endl;
	return false;
      }

      fragments.emplace_back(artdaq::Fragment::FragmentBytes(
			     sizeof(sbndaq::CAENV1730Event),
			     serial.sequence_id,
			     serial.fragment_id,
			     serial.fragment_type,
			     serial.metadata,
			     serial.timestamp));

      memcpy(fragments.back()->dataBeginBytes(),
	     &serial.event,
	     sizeof(sbndaq::CAENV1730Event));

      size_t counter = 0;
      for(auto const &channel_wvfm : serial.wvfmvec)
	{
	  for(auto const &value : channel_wvfm)
	    {
	      memcpy(fragments.back()->dataBeginBytes() + sizeof(sbndaq::CAENV1730Event) + counter * sizeof(uint16_t),
		     &value,
		     sizeof(uint16_t));
	      ++counter;
	    }
	}
      TLOG(TLVL_NOTICE) << '\n' << *fragments.back();

      return true;
    }
  else if(frag_type == sbndaq::detail::FragmentType::INVALID) 
    {
      TLOG(TLVL_WARNING) << "\nFragment type INVALID, exiting!" << "\nShould only be seen at end of file\n";
      return false;
    }
  else
    {
      sbndaq::FragmentSerialBase serial;

      try {
	ia >> serial;
	TLOG(TLVL_WARNING) << "\nUnexpected fragment type in CAEN file: " << fragmentTypeToString(sbndaq::detail::FragmentType(frag_type)) << '\n'
			   << "Exiting...\n" << std::endl;
      }
      catch(const std::exception &ex) {
	TLOG(TLVL_ERROR)  << "\nCatching exception in getNext_(artdaq::FragmentPtrs& fragments)\n"
			 << ex.what() << '\n'
			 << "Failed to access fragment serial\n"
			 << "Exiting...\n" << std::endl;
	return false;
      }
      return false;
    }
}

DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(CAENV1730SerialReader)
