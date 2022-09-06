#define TRACE_NAME "BernCRTSerialReader"

#include "sbndaq-artdaq/Generators/SBND/BernCRTV2SerialReader.hh"

#include "artdaq/Generators/GeneratorMacros.hh"
#include <boost/archive/binary_iarchive.hpp>

using sbndaq::BernCRTV2SerialReader;

BernCRTV2SerialReader::BernCRTV2SerialReader(fhicl::ParameterSet const& ps)
  : CommandableFragmentGenerator{ps},
  binary_file_path_(ps.get<std::string>("BinaryFilePath"))
  {
    binary_file_.open(binary_file_path_);
  }

BernCRTV2SerialReader::~BernCRTV2SerialReader() {
  binary_file_.close();
}

void BernCRTV2SerialReader::start() {}

void BernCRTV2SerialReader::stop() {}

bool BernCRTV2SerialReader::getNext_(artdaq::FragmentPtrs& fragments) {

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

  if(frag_type == sbndaq::detail::FragmentType::BERNCRTV2)
    {
      TLOG(TLVL_INFO) << "\nReading fragment serial of type: BERNCRTV2\n";
      sbndaq::BernCRTFragmentV2Serial serial;

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
			     sizeof(sbndaq::BernCRTHitV2) * serial.n_hits,
			     serial.sequence_id,
			     serial.fragment_id,
			     serial.fragment_type,
			     serial.metadata,
			     serial.timestamp));

      memcpy(fragments.back()->dataBeginBytes(),
	     serial.bern_crt_hits.data(),
	     sizeof(sbndaq::BernCRTHitV2) * serial.n_hits);

      TLOG(TLVL_DEBUG) << '\n' << *fragments.back();

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
	TLOG(TLVL_ERROR) << "\nCatching exception in getNext_(artdaq::FragmentPtrs& fragments)\n"
			 << ex.what() << '\n'
			 << "Failed to access fragment serial\n"
			 << "Exiting...\n" << std::endl;
	return false;
      }
      return false;
    }
}

DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(BernCRTV2SerialReader)
