#define TRACE_NAME "CAENV1730SerialReader"

#include "sbndaq-artdaq/Generators/SBND/CAENV1730SerialReader.hh"

#include "artdaq/Generators/GeneratorMacros.hh"
#include <boost/archive/binary_iarchive.hpp>

using sbndaq::CAENV1730SerialReader;

CAENV1730SerialReader::CAENV1730SerialReader(fhicl::ParameterSet const& ps)
  : CommandableFragmentGenerator{ps}
  , binary_file_path_(ps.get<std::string>("binary_file_path"))
  , post_event_delay_ms(ps.get<unsigned>("post_event_delay_ms"))
  {
    binary_file_.open(binary_file_path_);
    fragment_counter = 0;
    event_counter = 1;
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

      if(serial.sequence_id == event_counter+1)
        {
          TLOG(TLVL_NOTICE) << "\nSerial from new event, sequence ID: " << serial.sequence_id 
                            << "\nIncrementing event counter and delaying " << post_event_delay_ms << "ms"
                            << std::endl;
          std::this_thread::sleep_for(std::chrono::milliseconds(post_event_delay_ms));
          ++event_counter;
        }


      ++fragment_counter;

      auto timenow = std::chrono::system_clock::now().time_since_epoch().count();
      auto fractimenow = timenow % 1000000000;

      fragments.emplace_back(artdaq::Fragment::FragmentBytes(
                                                             serial.metadata.ExpectedDataSize(),
                                                             fragment_counter,
                                                             serial.fragment_id,
                                                             serial.fragment_type,
                                                             serial.metadata,
                                                             serial.sequence_id * 1e9 + fractimenow));

      memcpy(fragments.back()->dataBeginBytes(),
             &serial.event.Header,
             sizeof(sbndaq::CAENV1730EventHeader));

      size_t counter = 0;
      for(auto const &channel_wvfm : serial.wvfmvec)
        {
          for(auto const &value : channel_wvfm)
            {
              memcpy(fragments.back()->dataBeginBytes() + sizeof(sbndaq::CAENV1730EventHeader) + counter * sizeof(uint16_t),
                     &value,
                     sizeof(uint16_t));
              ++counter;
            }
        }
      TLOG(TLVL_NOTICE) << '\n' << *fragments.back();
      TLOG(TLVL_NOTICE) << "\nSerial SeqID:  " << serial.sequence_id 
                        << "\nSerial FragID: " << serial.fragment_id
                        << "\nSerial TS:     " << artdaq::Fragment::print_timestamp(serial.timestamp)
                        << "\nTimenow:       " << artdaq::Fragment::print_timestamp(timenow)
                        << "\nFractimenow:   " << artdaq::Fragment::print_timestamp(fractimenow)
                        << std::endl;

      return true;
    }
  else if(frag_type == sbndaq::detail::FragmentType::INVALID) 
    {
      TLOG(TLVL_WARNING) << "\nFragment type INVALID, exiting after " << 4 * post_event_delay_ms << "ms delay!" 
                         << "\nShould only be seen at end of file\n";
      std::this_thread::sleep_for(std::chrono::milliseconds(4 * post_event_delay_ms));
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
