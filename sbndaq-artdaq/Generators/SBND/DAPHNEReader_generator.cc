//
//  sbndaq-artdaq/Generators/SBND/DAPHNEReader_generator.cc
//
//  Configuring board reader for the DAPHNE SiPM Readout
//

#include "DAPHNEReader.hh"
#include "artdaq/Generators/GeneratorMacros.hh"
#include "cetlib_except/exception.h"

#include <sstream>
#include <vector>
#include <memory>
#include <iomanip>
#include <chrono>
#include <thread>

namespace sbndaq 
{

// "initialize" transition
DAPHNEReader::DAPHNEReader(fhicl::ParameterSet const& ps): CommandableFragmentGenerator(ps) 
{
  const std::string identification = "DAPHNEReader";
  TLOG_INFO(identification) << "DAPHNEReader constructor" << TLOG_ENDL;
  setupDAPHNE(ps);
}

void DAPHNEReader::setupDAPHNE(fhicl::ParameterSet const& ps) 
{

  const std::string identification = "DAPHNEReader::setupDAPHNE";
  TLOG_INFO(identification) << "Starting setupDAPHNE " << TLOG_ENDL;
  pedestal = ps.get<uint16_t>("DAPHNE.pedestal");
  TLOG_INFO(identification) << "Using DAPHNE pedestal " << pedestal 
			    << TLOG_ENDL;}
  

// "shutdown" transition
DAPHNEReader::~DAPHNEReader() 
{

}

// "start" transition
void DAPHNEReader::start() 
{

}

// "stop" transition
void DAPHNEReader::stop() 
{

}

// Called by BoardReaderMain in a loop between "start" and "stop"
bool DAPHNEReader::getNext_(artdaq::FragmentPtrs& /*frags*/) 
{
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  return (! should_stop()); // returning false before should_stop makes all other BRs stop
}

} // namespace

DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(sbndaq::DAPHNEReader)
