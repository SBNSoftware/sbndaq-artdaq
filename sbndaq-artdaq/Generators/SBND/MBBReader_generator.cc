//
//  sbndaq-artdaq/Generators/SBND/MBBReader_generator.cc
//
//  Configuring board reader for Magic Blue Box (MBB)
//

#include "MBBReader.hh"
#include "artdaq/Generators/GeneratorMacros.hh"
#include "cetlib_except/exception.h"

#include "WIBException/ExceptionBase.hh"
#include "WIBException.hh"
#include "BNL_UDP_Exception.hh"

#include <sstream>
#include <vector>
#include <memory>
#include <iomanip>
#include <chrono>
#include <thread>

namespace sbndaq 
{

// "initialize" transition
MBBReader::MBBReader(fhicl::ParameterSet const& ps): CommandableFragmentGenerator(ps) 
{
  const std::string identification = "MBBReader";

  int success = setupMBB(ps);
  if (!success)
  {
    cet::exception excpt(identification);
    throw excpt;
  }
}

int MBBReader::setupMBB(fhicl::ParameterSet const& ps) 
{
  int success = 1;
  const std::string identification = "MBBReader::setupMBB";
  auto mbb_address              = ps.get<std::string>("MBB.address");
  auto mbb_table                = ps.get<std::string>("MBB.mbb_table");
  auto expected_mbb_fw_version  = ps.get<unsigned>("MBB.expected_mbb_fw_version");

  TLOG_INFO(identification) << "Connecting to MBB at " <<  
    mbb_address << TLOG_ENDL;
  mbb = std::make_unique<MBB>( mbb_address, mbb_table );

  // Need to check firmware version here

  // Check and print firmware version
  uint32_t mbb_fw_version = mbb->Read("FW_VERSION");
  
  //  TLOG_INFO(identification) << "MBB Firmware Version: 0x" 
  
  if (expected_mbb_fw_version != mbb_fw_version)
  {
    success = 0;
    cet::exception excpt(identification);
    excpt << "MBB Firmware version is "
        << std::hex << std::setw(8) << std::setfill('0')
        << mbb_fw_version
        <<" but expect "
        << std::hex << std::setw(8) << std::setfill('0')
        << expected_mbb_fw_version
        <<" version in fcl";
    throw excpt;
  }

  TLOG_INFO(identification) << "Configured MBB" << TLOG_ENDL;
  return success;
}

// "shutdown" transition
MBBReader::~MBBReader() 
{

}

// "start" transition
void MBBReader::start() 
{
  const std::string identification = "MBBReader::start";
  if (!mbb) 
  {
    cet::exception excpt(identification);
    excpt << "MBB object pointer NULL";
    throw excpt;
  }
}

// "stop" transition
void MBBReader::stop() 
{
  const std::string identification = "MBBReader::stop";
  if (!mbb) 
  {
    cet::exception excpt(identification);
    excpt << "MBB object pointer NULL";
    throw excpt;
  }
}

// Called by BoardReaderMain in a loop between "start" and "stop"
bool MBBReader::getNext_(artdaq::FragmentPtrs& /*frags*/) 
{
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  return (! should_stop()); // returning false before should_stop makes all other BRs stop
}

} // namespace

DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(sbndaq::MBBReader)
