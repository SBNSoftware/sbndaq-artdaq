#include "sbndaq-artdaq/Generators/SBND/BernCRTSerialReader.hh"
#define TRACE_NAME "BernCRTSerialReader"
#include "artdaq/DAQdata/Globals.hh"
#include "artdaq/Generators/GeneratorMacros.hh"
#include "fhiclcpp/ParameterSet.h"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "sbndaq-artdaq/Generators/Common/PoolBuffer.hh"
#include "sbndaq-artdaq/Generators/Common/workerThread.hh"

#include <boost/archive/binary_iarchive.hpp>

using artdaq::MetricMode;
using sbndaq::PoolBuffer;
using sbndaq::BernCRTSerialReader;



BernCRTSerialReader::BernCRTSerialReader(fhicl::ParameterSet const& ps)
  : CommandableFragmentGenerator{ps},
  binary_file_path_(ps.get<std::string>("BinaryFilePath"))
  {
    binary_file_.open(binary_file_path_);
  }

BernCRTSerialReader::~BernCRTSerialReader() {
  buffer_.reset();
  binary_file_.close();
}

void BernCRTSerialReader::start() {
}

void BernCRTSerialReader::stop() {
}

bool BernCRTSerialReader::getNext_(artdaq::FragmentPtrs& fragments) {

  sbndaq::BernCRTFragmentSerial serial;
  boost::archive::binary_iarchive ia(binary_file_);

  try {
    ia >> serial;
  }
  catch(const std::exception &ex) {
    std::cout << "Catching exception in getNext_(artdaq::FragmentPtrs& fragments)\n"
	      << ex.what() << std::endl;
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

bool BernCRTSerialReader::checkHWStatus_() {
  return true;
}

DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(BernCRTSerialReader)
