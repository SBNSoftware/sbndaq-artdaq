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
  TLOG_INFO(identification) << "MBBReader constructor" << TLOG_ENDL;
  auto configuration_tries = ps.get<unsigned>("MBB.configuration_tries");
  bool success = false;
  for (unsigned iTry=1; iTry <= configuration_tries; iTry++) 
  {
    try
    {
      setupMBB(ps);
      success = true;
      break;
    }
    catch (const WIBException::BAD_REPLY & exc)
    {
      TLOG_WARNING(identification) << "MBB communication error: "
				   << exc.what() << TLOG_ENDL;
    }
    catch (const WIBException::exBase & exc)
    {
      cet::exception excpt(identification);
      excpt << "Unhandled MBBException: "
	    << exc.what() << ": "
	    << exc.Description();
      throw excpt;
      }
    TLOG_INFO(identification) << "Configuraton try  " << iTry << " failed. Trying again..." << TLOG_ENDL;
  } // iTry

  if (!success)
  {
    cet::exception excpt(identification);
    excpt << "Failed to configure MBB after " << configuration_tries << " tries";
    throw excpt;
  }
}

void MBBReader::setupMBB(fhicl::ParameterSet const& ps) 
{

  const std::string identification = "MBBReader::setupMBB";
  TLOG_INFO(identification) << "Starting setupMBB " << TLOG_ENDL;

  auto mbb_address              = ps.get<std::string>("MBB.address");
  auto mbb_table                = ps.get<std::string>("MBB.mbb_table");
  auto expected_mbb_fw_version  = ps.get<unsigned>("MBB.expected_mbb_fw_version"); 

  const auto PULSE_SOURCE       = ps.get<uint32_t>("MBB.PULSE_SOURCE");
  const auto PULSE_PERIOD       = ps.get<uint64_t>("MBB.PULSE_PERIOD");

  //for D0 PTC
  const auto wib_pwr001         = ps.get<uint32_t>("MBB.wib_pwr001");//ptc 0 wib 01:001              
  const auto wib_pwr002         = ps.get<uint32_t>("MBB.wib_pwr002");//ptc 0 wib 02:002              
  const auto wib_pwr003         = ps.get<uint32_t>("MBB.wib_pwr003");
  const auto wib_pwr004         = ps.get<uint32_t>("MBB.wib_pwr004");
  const auto wib_pwr005         = ps.get<uint32_t>("MBB.wib_pwr005");
  const auto wib_pwr006         = ps.get<uint32_t>("MBB.wib_pwr006");
  
  const auto wib_pwr101         = ps.get<uint32_t>("MBB.wib_pwr101");//ptc 1 wib 01:101
  const auto wib_pwr102         = ps.get<uint32_t>("MBB.wib_pwr102");//ptc 1 wib 02:102
  const auto wib_pwr103         = ps.get<uint32_t>("MBB.wib_pwr103");
  const auto wib_pwr104         = ps.get<uint32_t>("MBB.wib_pwr104");
  const auto wib_pwr105         = ps.get<uint32_t>("MBB.wib_pwr105");
  const auto wib_pwr106         = ps.get<uint32_t>("MBB.wib_pwr106");

  const auto wib_pwr201         = ps.get<uint32_t>("MBB.wib_pwr201");//ptc 2 wib 01:201
  const auto wib_pwr202         = ps.get<uint32_t>("MBB.wib_pwr202");
  const auto wib_pwr203         = ps.get<uint32_t>("MBB.wib_pwr203");
  const auto wib_pwr204         = ps.get<uint32_t>("MBB.wib_pwr204");
  const auto wib_pwr205         = ps.get<uint32_t>("MBB.wib_pwr205");
  const auto wib_pwr206         = ps.get<uint32_t>("MBB.wib_pwr206");

  const auto wib_pwr301         = ps.get<uint32_t>("MBB.wib_pwr301");
  const auto wib_pwr302         = ps.get<uint32_t>("MBB.wib_pwr302");
  const auto wib_pwr303         = ps.get<uint32_t>("MBB.wib_pwr303");
  const auto wib_pwr304         = ps.get<uint32_t>("MBB.wib_pwr304");
  const auto wib_pwr305         = ps.get<uint32_t>("MBB.wib_pwr305");
  const auto wib_pwr306         = ps.get<uint32_t>("MBB.wib_pwr306");

  const auto wib_pwr401         = ps.get<uint32_t>("MBB.wib_pwr401");
  const auto wib_pwr402         = ps.get<uint32_t>("MBB.wib_pwr402");
  const auto wib_pwr403         = ps.get<uint32_t>("MBB.wib_pwr403");
  const auto wib_pwr404         = ps.get<uint32_t>("MBB.wib_pwr404");
  const auto wib_pwr405         = ps.get<uint32_t>("MBB.wib_pwr405");
  const auto wib_pwr406         = ps.get<uint32_t>("MBB.wib_pwr406");
  
  //Controlling which PTC to talk to. ptc_01=1 means talking to ptc_01 and so on.
  const auto ptc_00             = ps.get<uint32_t>("MBB.ptc_00");//the D0 PTC
  const auto ptc_01             = ps.get<uint32_t>("MBB.ptc_01");
  const auto ptc_02             = ps.get<uint32_t>("MBB.ptc_02");
  const auto ptc_03             = ps.get<uint32_t>("MBB.ptc_03");
  const auto ptc_04             = ps.get<uint32_t>("MBB.ptc_04");


  TLOG_INFO(identification) << "Connecting to MBB at " << mbb_address << TLOG_ENDL;
  mbb = std::make_unique<MBB>( mbb_address, mbb_table );
  TLOG_INFO(identification) << "Connected to MBB at " <<  mbb_address << TLOG_ENDL;


  // Check and print firmware version
  uint32_t mbb_fw_version = mbb->Read("FIRMWARE_VERSION");
  
  TLOG_INFO(identification) << "MBB Firmware Version: 0x" 
        << std::hex << std::setw(8) << std::setfill('0')
        <<  mbb_fw_version
        << " Synthesized: " 
        << std::hex << std::setw(2) << std::setfill('0')
        << mbb->Read("SYSTEM.SYNTH_DATE.CENTURY")
        << std::hex << std::setw(2) << std::setfill('0')
        << mbb->Read("SYSTEM.SYNTH_DATE.YEAR") << "-"
        << std::hex << std::setw(2) << std::setfill('0')
        << mbb->Read("SYSTEM.SYNTH_DATE.MONTH") << "-"
        << std::hex << std::setw(2) << std::setfill('0')
        << mbb->Read("SYSTEM.SYNTH_DATE.DAY") << " "
        << std::hex << std::setw(2) << std::setfill('0')
        << mbb->Read("SYSTEM.SYNTH_TIME.HOUR") << ":"
        << std::hex << std::setw(2) << std::setfill('0')
        << mbb->Read("SYSTEM.SYNTH_TIME.MINUTE") << ":"
        << std::hex << std::setw(2) << std::setfill('0')
        << mbb->Read("SYSTEM.SYNTH_TIME.SECOND")  << TLOG_ENDL; 
  
  if (expected_mbb_fw_version != mbb_fw_version)
  {
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

  // Throwing errors if the argument is not logical.
  if (PULSE_SOURCE > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
        << " PULSE_SOURCE should be 0 or 1, but is: "
        << PULSE_SOURCE;
    throw excpt;
  }
  if (PULSE_PERIOD > 4294967295){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
        << " PULSE_PERIOD shouldn't be larger than 4294967295, but is: "
        << PULSE_PERIOD;
    throw excpt;
  }

  //for PTC 00: the D0 PTC                                                                                                                                  
    if (wib_pwr001 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
          << " wib_pwr001 should be 0 or 1, but is: "
          << wib_pwr001;
    throw excpt;
  }
  if (wib_pwr002 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
	  << " wib_pwr002 should be 0 or 1, but is: "
	  << wib_pwr002;
    throw excpt;
  }
  if (wib_pwr003 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
	  << " wib_pwr003 should be 0 or 1, but is: "
	  << wib_pwr003;
    throw excpt;
  }
  if (wib_pwr004 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
	  << " wib_pwr004 should be 0 or 1, but is: "
	  << wib_pwr004;
    throw excpt;
  }
  if (wib_pwr005 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
	  << " wib_pwr005 should be 0 or 1, but is: "
	  << wib_pwr005;
    throw excpt;
  }

  if (wib_pwr006 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
	  << " wib_pwr006 should be 0 or 1, but is: "
	  << wib_pwr006;
    throw excpt;
  }

  //for PTC 01
  if (wib_pwr101 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
          << " wib_pwr101 should be 0 or 1, but is: "
          << wib_pwr101;
    throw excpt;
  }
  if (wib_pwr102 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
        << " wib_pwr102 should be 0 or 1, but is: "
        << wib_pwr102;
    throw excpt;
  }
  if (wib_pwr103 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
        << " wib_pwr103 should be 0 or 1, but is: "
        << wib_pwr103;
    throw excpt;
  }
  if (wib_pwr104 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
        << " wib_pwr104 should be 0 or 1, but is: "
        << wib_pwr104;
    throw excpt;
  }
  if (wib_pwr105 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
        << " wib_pwr105 should be 0 or 1, but is: "
        << wib_pwr105;
    throw excpt;
  }

  if (wib_pwr106 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
        << " wib_pwr106 should be 0 or 1, but is: "
        << wib_pwr106;
    throw excpt;
  }

  //for PTC 02
  if (wib_pwr201 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
          << " wib_pwr201 should be 0 or 1, but is: "
          << wib_pwr201;
    throw excpt;
  }
  if (wib_pwr202 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
	  << " wib_pwr202 should be 0 or 1, but is: "
	  << wib_pwr202;
    throw excpt;
  }
  if (wib_pwr203 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
	  << " wib_pwr203 should be 0 or 1, but is: "
	  << wib_pwr203;
    throw excpt;
  }
  if (wib_pwr204 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
	  << " wib_pwr204 should be 0 or 1, but is: "
	  << wib_pwr204;
    throw excpt;
  }
  if (wib_pwr205 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
	  << " wib_pwr205 should be 0 or 1, but is: "
	  << wib_pwr205;
    throw excpt;
  }

  if (wib_pwr206 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
	  << " wib_pwr206 should be 0 or 1, but is: "
	  << wib_pwr206;
    throw excpt;
  }

  //for PTC 03
  if (wib_pwr301 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
          << " wib_pwr301 should be 0 or 1, but is: "
          << wib_pwr301;
    throw excpt;
  }
  if (wib_pwr302 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
	  << " wib_pwr302 should be 0 or 1, but is: "
	  << wib_pwr302;
    throw excpt;
  }
  if (wib_pwr303 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
	  << " wib_pwr303 should be 0 or 1, but is: "
	  << wib_pwr303;
    throw excpt;
  }
  if (wib_pwr304 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
	  << " wib_pwr304 should be 0 or 1, but is: "
	  << wib_pwr304;
    throw excpt;
  }
  if (wib_pwr305 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
	  << " wib_pwr305 should be 0 or 1, but is: "
	  << wib_pwr305;
    throw excpt;
  }

  if (wib_pwr306 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
	  << " wib_pwr306 should be 0 or 1, but is: "
	  << wib_pwr306;
    throw excpt;
  }

  //for PTC 04
  if (wib_pwr401 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
          << " wib_pwr401 should be 0 or 1, but is: "
          << wib_pwr401;
    throw excpt;
  }
  if (wib_pwr402 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
	  << " wib_pwr402 should be 0 or 1, but is: "
	  << wib_pwr402;
    throw excpt;
  }
  if (wib_pwr403 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
	  << " wib_pwr403 should be 0 or 1, but is: "
	  << wib_pwr403;
    throw excpt;
  }
  if (wib_pwr404 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
	  << " wib_pwr404 should be 0 or 1, but is: "
	  << wib_pwr404;
    throw excpt;
  }
  if (wib_pwr405 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
	  << " wib_pwr405 should be 0 or 1, but is: "
	  << wib_pwr405;
    throw excpt;
  }

  if (wib_pwr406 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
	  << " wib_pwr406 should be 0 or 1, but is: "
	  << wib_pwr406;
    throw excpt;
  }

  sleep(5);

  if (ptc_00 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
	  << " ptc_00 should be 0 or 1, but is: "
	  << ptc_00;
    throw excpt;
  }

  if (ptc_01 > 1){
      cet::exception excpt(identification);
      excpt << "setupMBB:"
	    << " ptc_01 should be 0 or 1, but is: "
	    << ptc_01;
      throw excpt;
    }
  if (ptc_02 > 1){
      cet::exception excpt(identification);
      excpt << "setupMBB:"
	    << " ptc_02 should be 0 or 1, but is: "
	    << ptc_02;
      throw excpt;
    }
  if (ptc_03 > 1){
      cet::exception excpt(identification);
      excpt << "ptc_03:"
	    << " ptc_03 should be 0 or 1, but is: "
	    << ptc_03;
      throw excpt;
    }
  if (ptc_04 > 1){
      cet::exception excpt(identification);
      excpt << "setupMBB:"
	    << " ptc_04 should be 0 or 1, but is: "
	    << ptc_04;
      throw excpt;
    }

  sleep(5);

  //configuring PTCs.
  if(ptc_00==1) {
    setupPTC(0);
    mbb->ConfigMBB(PULSE_SOURCE, PULSE_PERIOD, wib_pwr001, wib_pwr002, wib_pwr003, wib_pwr004, wib_pwr005, wib_pwr006);
  }

  if(ptc_01==1) {
     setupPTC(1);
     mbb->ConfigMBB(PULSE_SOURCE, PULSE_PERIOD, wib_pwr101, wib_pwr102, wib_pwr103, wib_pwr104, wib_pwr105, wib_pwr106);//we are writing into PTC_DATA twice!! Remove the one in configPTC?     
  }

  if(ptc_02==1) {
    setupPTC(2);
    mbb->ConfigMBB(PULSE_SOURCE, PULSE_PERIOD, wib_pwr201, wib_pwr202, wib_pwr203, wib_pwr204, wib_pwr205, wib_pwr206);
  }

  if(ptc_03==1) {
    setupPTC(3);
    mbb->ConfigMBB(PULSE_SOURCE, PULSE_PERIOD, wib_pwr301, wib_pwr302, wib_pwr303, wib_pwr304, wib_pwr305, wib_pwr306);
  }

  if(ptc_04==1) {
    setupPTC(4);
    mbb->ConfigMBB(PULSE_SOURCE, PULSE_PERIOD, wib_pwr401, wib_pwr402, wib_pwr403, wib_pwr404, wib_pwr405, wib_pwr406);
  }

}

void MBBReader::setupPTC(size_t icrate )
{
  const std::string identification = "MBBReader::setupPTC";
  if ( /*icrate<0 ||*/ icrate > 4)
    {
      cet::exception excpt(identification);
      excpt << "setupPTC:"
	    << " icrate should be between 0 and 4, but is: "
	    << icrate;
      throw excpt;
    }
  mbb->ConfigPTC(icrate);
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
  mbb->TimeStampReset();
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
