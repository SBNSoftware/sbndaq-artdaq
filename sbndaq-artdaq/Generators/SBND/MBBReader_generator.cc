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
  
  // Reading in the fhicl parameters.
  auto mbb_address              = ps.get<std::string>("MBB.address");
  auto mbb_table                = ps.get<std::string>("MBB.mbb_table");
  auto expected_mbb_fw_version  = ps.get<unsigned>("MBB.expected_mbb_fw_version"); 

  const auto PULSE_SOURCE       = ps.get<uint32_t>("MBB.PULSE_SOURCE");
  const auto PULSE_PERIOD       = ps.get<uint64_t>("MBB.PULSE_PERIOD");

  //for D0 PTC
  const auto wib_pwr1400         = ps.get<uint32_t>("MBB.wib_pwr1400");              
  const auto wib_pwr1401         = ps.get<uint32_t>("MBB.wib_pwr1401");              
  const auto wib_pwr1402         = ps.get<uint32_t>("MBB.wib_pwr1402");
  const auto wib_pwr1403         = ps.get<uint32_t>("MBB.wib_pwr1403");
  const auto wib_pwr1404         = ps.get<uint32_t>("MBB.wib_pwr1404");
  const auto wib_pwr1405         = ps.get<uint32_t>("MBB.wib_pwr1405");
  
  //for WOW PTC
  const auto wib_pwr000         = ps.get<uint32_t>("MBB.wib_pwr000");//ptc 0 wib 00:000
  const auto wib_pwr001         = ps.get<uint32_t>("MBB.wib_pwr001");//ptc 0 wib 01:001
  const auto wib_pwr002         = ps.get<uint32_t>("MBB.wib_pwr002");
  const auto wib_pwr003         = ps.get<uint32_t>("MBB.wib_pwr003");
  const auto wib_pwr004         = ps.get<uint32_t>("MBB.wib_pwr004");
  const auto wib_pwr005         = ps.get<uint32_t>("MBB.wib_pwr005");
 
  const auto wib_pwr100         = ps.get<uint32_t>("MBB.wib_pwr100");//ptc 1 wib 00:100                                                                     
  const auto wib_pwr101         = ps.get<uint32_t>("MBB.wib_pwr101");                                                                                       
  const auto wib_pwr102         = ps.get<uint32_t>("MBB.wib_pwr102");
  const auto wib_pwr103         = ps.get<uint32_t>("MBB.wib_pwr103");
  const auto wib_pwr104         = ps.get<uint32_t>("MBB.wib_pwr104");
  const auto wib_pwr105         = ps.get<uint32_t>("MBB.wib_pwr105");

  const auto wib_pwr200         = ps.get<uint32_t>("MBB.wib_pwr200");
  const auto wib_pwr201         = ps.get<uint32_t>("MBB.wib_pwr201");
  const auto wib_pwr202         = ps.get<uint32_t>("MBB.wib_pwr202");
  const auto wib_pwr203         = ps.get<uint32_t>("MBB.wib_pwr203");
  const auto wib_pwr204         = ps.get<uint32_t>("MBB.wib_pwr204");
  const auto wib_pwr205         = ps.get<uint32_t>("MBB.wib_pwr205");

  const auto wib_pwr300         = ps.get<uint32_t>("MBB.wib_pwr300");
  const auto wib_pwr301         = ps.get<uint32_t>("MBB.wib_pwr301");
  const auto wib_pwr302         = ps.get<uint32_t>("MBB.wib_pwr302");
  const auto wib_pwr303         = ps.get<uint32_t>("MBB.wib_pwr303");
  const auto wib_pwr304         = ps.get<uint32_t>("MBB.wib_pwr304");
  const auto wib_pwr305         = ps.get<uint32_t>("MBB.wib_pwr305");

  const auto wib_pwr400         = ps.get<uint32_t>("MBB.wib_pwr400");
  const auto wib_pwr401         = ps.get<uint32_t>("MBB.wib_pwr401");
  const auto wib_pwr402         = ps.get<uint32_t>("MBB.wib_pwr402");
  const auto wib_pwr403         = ps.get<uint32_t>("MBB.wib_pwr403");
  const auto wib_pwr404         = ps.get<uint32_t>("MBB.wib_pwr404");
  const auto wib_pwr405         = ps.get<uint32_t>("MBB.wib_pwr405");
  
  //Controlling which PTC to talk to. ptc_01=1 means talking to ptc_01 and so on.
  const auto ptc_14             = ps.get<uint32_t>("MBB.ptc_14");//the D0 PTC
  const auto ptc_00             = ps.get<uint32_t>("MBB.ptc_00");//the WOW PTC
  const auto ptc_01             = ps.get<uint32_t>("MBB.ptc_01");
  const auto ptc_02             = ps.get<uint32_t>("MBB.ptc_02");
  const auto ptc_03             = ps.get<uint32_t>("MBB.ptc_03");
  const auto ptc_04             = ps.get<uint32_t>("MBB.ptc_04");

  // connecting to mbb.
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

  // Throwing errors if the fhicl parameters are not logical.
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

  //for PTC 14: the D0 PTC                                                                                                                                  
    if (wib_pwr1400 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
          << " wib_pwr1400 should be 0 or 1, but is: "
          << wib_pwr1400;
    throw excpt;
  }
  if (wib_pwr1401 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
	  << " wib_pwr1401 should be 0 or 1, but is: "
	  << wib_pwr1401;
    throw excpt;
  }
  if (wib_pwr1402 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
	  << " wib_pwr1402 should be 0 or 1, but is: "
	  << wib_pwr1402;
    throw excpt;
  }
  if (wib_pwr1403 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
	  << " wib_pwr1403 should be 0 or 1, but is: "
	  << wib_pwr1403;
    throw excpt;
  }
  if (wib_pwr1404 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
	  << " wib_pwr1404 should be 0 or 1, but is: "
	  << wib_pwr1404;
    throw excpt;
  }

  if (wib_pwr1405 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
	  << " wib_pwr1405 should be 0 or 1, but is: "
	  << wib_pwr1405;
    throw excpt;
  }

  
  //for PTC 00: the WOW PTC                                                                                                                                 
  if (wib_pwr000 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
          << " wib_pwr000 should be 0 or 1, but is: "
          << wib_pwr000;
    throw excpt;
  }
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

  //for PTC 01
  if (wib_pwr100 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
          << " wib_pwr100 should be 0 or 1, but is: "
          << wib_pwr100;
    throw excpt;
  }
  if (wib_pwr101 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
        << " wib_pwr101 should be 0 or 1, but is: "
        << wib_pwr102;
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

  //for PTC 02
  if (wib_pwr200 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
          << " wib_pwr200 should be 0 or 1, but is: "
          << wib_pwr200;
    throw excpt;
  }
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

  //for PTC 03
  if (wib_pwr300 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
          << " wib_pwr300 should be 0 or 1, but is: "
          << wib_pwr300;
    throw excpt;
  }
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

  //for PTC 04
  if (wib_pwr400 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
          << " wib_pwr400 should be 0 or 1, but is: "
          << wib_pwr400;
    throw excpt;
  }
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

  sleep(5);

  if (ptc_14 > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
          << " ptc_14 should be 0 or 1, but is: "
          << ptc_14;
    throw excpt;
  }

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

  // configuring the MBB with pulse source and pulse period: will be the same for all PTCs.
  const auto config_mbb = ps.get<uint32_t>("MBB.config_mbb");
  if(config_mbb > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
	  << " config_mbb should be 0 or 1, but is: "
	  << config_mbb;
    throw excpt;
  }
  if(config_mbb == 1){
    mbb->ConfigMBB(PULSE_SOURCE, PULSE_PERIOD);
  }

  //configuring the PTCs.
  const auto config_ptc = ps.get<uint32_t>("MBB.config_ptc");
  if(config_ptc > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
          << " config_ptc should be 0 or 1, but is: "
          << config_ptc;
    throw excpt;
  }
  if(config_ptc == 1){
    if(ptc_14==1) {
      mbb->ConfigPTC(14, wib_pwr1400, wib_pwr1401, wib_pwr1402, wib_pwr1403, wib_pwr1404, wib_pwr1405);
    }
 
    if(ptc_00==1) {
      mbb->ConfigPTC(0, wib_pwr000, wib_pwr001, wib_pwr002, wib_pwr003, wib_pwr004, wib_pwr005);
    }

    if(ptc_01==1) {
      mbb->ConfigPTC(1, wib_pwr100, wib_pwr101, wib_pwr102, wib_pwr103, wib_pwr104, wib_pwr105);     
    }

    if(ptc_02==1) {
      mbb->ConfigPTC(2, wib_pwr200, wib_pwr201, wib_pwr202, wib_pwr203, wib_pwr204, wib_pwr205);
    }

    if(ptc_03==1) {
      mbb->ConfigPTC(3, wib_pwr300, wib_pwr301, wib_pwr302, wib_pwr303, wib_pwr304, wib_pwr305);
    }

    if(ptc_04==1) {
      mbb->ConfigPTC(4, wib_pwr400, wib_pwr401, wib_pwr402, wib_pwr403, wib_pwr404, wib_pwr405);
    }
  }


  //after turning everything on/off sleep for some time for changes to take effect.
  const auto sleep_time = ps.get<uint32_t>("MBB.sleep_time");
  if(sleep_time > 100){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
          << " sleep_time should be between 0 and 100 seconds, but is: "
          << sleep_time;
    throw excpt;
  }
  sleep(sleep_time);


  // synchronous FEMB start.
  const auto start_femb_daq   = ps.get<uint32_t>("MBB.start_femb_daq");
  if(start_femb_daq > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
          << " start_femb_daq should be 0 or 1, but is: "
          << start_femb_daq;
    throw excpt;
  }
  if(start_femb_daq == 1){
    TLOG_INFO(identification) << "Resetting the timestamp and starting FEMB Daq." << TLOG_ENDL;
    mbb->TimeStampReset();
    mbb->StartFEMBDaq();
    TLOG_INFO(identification) << "FEMB Daq started." << TLOG_ENDL;
    sleep(sleep_time);
  }
  else{
    TLOG_INFO(identification) << "Not running start FEMB Daq right now." << TLOG_ENDL;
  }

  // synchronous femb stop.
  const auto stop_femb_daq   = ps.get<uint32_t>("MBB.stop_femb_daq");
  if(stop_femb_daq > 1){
    cet::exception excpt(identification);
    excpt << "setupMBB:"
          << " stop_femb_daq should be 0 or 1, but is: "
          << stop_femb_daq;
    throw excpt;
  }
  if(stop_femb_daq == 1){
    TLOG_INFO(identification) << "Stopping FEMB Daq." << TLOG_ENDL;
    mbb->StopFEMBDaq();
    TLOG_INFO(identification) << "FEMB Daq stopped." << TLOG_ENDL;
    sleep(sleep_time);
  }
  else{
    TLOG_INFO(identification) << "Not running stop FEMB Daq right now." << TLOG_ENDL;
  }

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
