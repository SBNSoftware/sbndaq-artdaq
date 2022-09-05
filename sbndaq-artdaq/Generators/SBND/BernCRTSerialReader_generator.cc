#define TRACE_NAME "BernCRTSerialReader"

#include "sbndaq-artdaq/Generators/SBND/BernCRTSerialReader.hh"

#include "artdaq/Generators/GeneratorMacros.hh"
#include <boost/archive/binary_iarchive.hpp>

using sbndaq::BernCRTSerialReader;

BernCRTSerialReader::BernCRTSerialReader(fhicl::ParameterSet const& ps)
  : CommandableFragmentGenerator{ps},
  binary_file_path_(ps.get<std::string>("BinaryFilePath"))
  {
    binary_file_.open(binary_file_path_);
  }

BernCRTSerialReader::~BernCRTSerialReader() {
  binary_file_.close();
}

void BernCRTSerialReader::start() {}

void BernCRTSerialReader::stop() {}

bool BernCRTSerialReader::getNext_(artdaq::FragmentPtrs& fragments) {

  boost::archive::binary_iarchive ia(binary_file_);
  artdaq::Fragment::type_t frag_type;

  try {
    ia >> frag_type;
  }
  catch(const std::exception &ex) {
    std::cout << "\nCatching exception in getNext_(artdaq::FragmentPtrs& fragments)\n"
	      << ex.what() << '\n'
	      << "Failed to access fragment type\n"
	      << "Exiting...\n" << std::endl;
  }

  if(frag_type == sbndaq::detail::FragmentType::BERNCRTV2)
    {
      sbndaq::BernCRTFragmentSerial serial;

      try {
	ia >> serial;
      }
      catch(const std::exception &ex) {
	std::cout << "\nCatching exception in getNext_(artdaq::FragmentPtrs& fragments)\n"
		  << ex.what() << '\n'
		  << "Failed to access fragment serial\n"
		  << "Exiting...\n" << std::endl;
	return false;
      }

      fragments.emplace_back(artdaq::Fragment::FragmentBytes(
			     sizeof(sbndaq::BernCRTHitV2) * serial.n_hits,
			     serial.sequence_id,
			     serial.fragment_id,
			     serial.fragment_type,
			     serial.metadata,
			     serial.timestamp));

      memcpy(fragments.back()->dataBeginBytes(),
	     serial.bern_crt_hits.data(),
	     sizeof(sbndaq::BernCRTHitV2) * serial.n_hits);

      return true;
    }
  else if(frag_type == sbndaq::detail::FragmentType::INVALID)
    return false;
  else
    {
      sbndaq::FragmentSerialBase serial;

      try {
	ia >> serial;
	std::cout << "\nUnexpected fragment type in CAEN file: " << fragmentTypeToString(sbndaq::detail::FragmentType(frag_type)) << '\n'
                  << "Exiting...\n" << std::endl;
      }
      catch(const std::exception &ex) {
	std::cout << "\nCatching exception in getNext_(artdaq::FragmentPtrs& fragments)\n"
		  << ex.what() << '\n'
		  << "Failed to access fragment serial\n"
		  << "Exiting...\n" << std::endl;
	return false;
      }
      return false;
    }
}

DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(BernCRTSerialReader)
