#include "sbndaq-artdaq/Generators/SBND/BernCRTSerialReader.hh"
#define TRACE_NAME "BernCRTSerialReader"
#include "artdaq/DAQdata/Globals.hh"
#include "artdaq/Generators/GeneratorMacros.hh"
#include "fhiclcpp/ParameterSet.h"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "sbndaq-artdaq/Generators/Common/PoolBuffer.hh"
#include "sbndaq-artdaq/Generators/Common/workerThread.hh"

using artdaq::MetricMode;
using sbndaq::PoolBuffer;
using sbndaq::BernCRTSerialReader;



BernCRTSerialReader::BernCRTSerialReader(fhicl::ParameterSet const& ps)
  : CommandableFragmentGenerator{ps}
 {}

BernCRTSerialReader::~BernCRTSerialReader() {
  buffer_.reset();
}

void BernCRTSerialReader::start() {
}

void BernCRTSerialReader::stop() {
}

bool BernCRTSerialReader::getNext_(artdaq::FragmentPtrs& /*fragments*/) {
  return true;
}

bool BernCRTSerialReader::checkHWStatus_() {
  return true;
}

DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(BernCRTSerialReader)
