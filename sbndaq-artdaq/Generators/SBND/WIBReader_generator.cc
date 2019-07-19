//
//  sbndaq-artdaq/Generators/SBND/WIBReader_generator.cc
//
//  Ported from protoDUNE and extensively modifed for SBND use
//

#include "WIBReader.hh"
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
WIBReader::WIBReader(fhicl::ParameterSet const& ps): CommandableFragmentGenerator(ps) 
{
  const std::string identification = "WIBReader";
  TLOG_INFO(identification) << "WIBReader constructor" << TLOG_ENDL;
  auto configuration_tries = ps.get<unsigned>("WIB.configuration_tries");
  bool success = false;
  for (unsigned iTry=1; iTry <= configuration_tries; iTry++) 
  {
    try
    {
      setupWIB(ps);
      success = true;
      break;
    }
    catch (const WIBException::BAD_REPLY & exc)
    {
      TLOG_WARNING(identification) << "WIB communication error: "
				   << exc.what() << TLOG_ENDL;
    }
    catch (const WIBException::WIB_DTS_ERROR & exc)
    {
      TLOG_WARNING(identification) << "WIB timing config error: "
				   << exc.what() << TLOG_ENDL;
    }
    catch (const WIBException::exBase & exc)
    {
      cet::exception excpt(identification);
      excpt << "Unhandled WIBException: "
	    << exc.what() << ": "
	    << exc.Description();
      throw excpt;
    }
    TLOG_INFO(identification) << "Configuraton try  " << iTry << " failed. Trying again..." << TLOG_ENDL;
  } // for iRetry

  if (!success)
  {
    cet::exception excpt(identification);
    excpt << "Failed to configure WIB after " << configuration_tries << " tries";
    throw excpt;
  }
}

void WIBReader::setupWIB(fhicl::ParameterSet const& ps) 
{

  const std::string identification = "WIBReader::setupWIB";
  auto wib_address                = ps.get<std::string>("WIB.address");
  auto wib_table                  = ps.get<std::string>("WIB.wib_table");
  auto femb_table                 = ps.get<std::string>("WIB.femb_table");
  auto expected_wib_fw_version    = ps.get<unsigned>("WIB.expected_wib_fw_version");

  auto use_WIB_fake_data          = ps.get<std::vector<bool> >("WIB.use_WIB_fake_data");
  // false SAMPLES, true COUNTER
  auto use_WIB_fake_data_counter  = ps.get<bool>("WIB.use_WIB_fake_data_counter");

  auto local_clock                = ps.get<bool>("WIB.local_clock"); // use local clock if true, else DTS
  auto DTS_source                 = ps.get<unsigned>("WIB.DTS_source"); // 0 back plane, 1 front panel
  auto partition_number           = ps.get<unsigned>("WIB.partition_number"); // partition or timing group number

  // If these are true, will continue on error, if false, will raise an exception
  auto continueOnFEMBRegReadError = ps.get<bool>("WIB.continueOnError.FEMBRegReadError");
  auto continueOnFEMBSPIError     = ps.get<bool>("WIB.continueOnError.FEMBSPIError");
  auto continueOnFEMBSyncError    = ps.get<bool>("WIB.continueOnError.FEMBSyncError");
  auto continueIfListOfFEMBClockPhasesDontSync = ps.get<bool>("WIB.continueOnError.ListOfFEMBClockPhasesDontSync");

  auto enable_FEMBs               = ps.get<std::vector<bool> >("WIB.enable_FEMBs");
  auto FEMB_configs               = ps.get<std::vector<fhicl::ParameterSet> >("WIB.FEMBs");

  auto force_full_reset           = ps.get<bool>("WIB.force_full_reset");
  auto dnd_wait_time              = ps.get<unsigned>("WIB.dnd_wait_time");

  if (use_WIB_fake_data.size() != 4)
  {
    cet::exception excpt(identification);
    excpt << "Length of WIB.use_WIB_fake_data must be 4, not: " << use_WIB_fake_data.size();
    throw excpt;
  }

  if (FEMB_configs.size() != 4)
  {
    cet::exception excpt(identification);
    excpt << "Length of WIB.FEMBs must be 4, not: " << FEMB_configs.size();
    throw excpt;
  }

  if(partition_number > 15)
  {
    cet::exception excpt(identification);
    excpt << "partition_number must be 0-15, not: " << partition_number;
    throw excpt;
  }

  TLOG_INFO(identification) << "Connecting to WIB at " <<  wib_address << TLOG_ENDL;
  wib = std::make_unique<WIB>( wib_address, wib_table, femb_table );


  // Set whether to continue on errors
  wib->SetContinueOnFEMBRegReadError(continueOnFEMBRegReadError);
  wib->SetContinueOnFEMBSPIError(continueOnFEMBSPIError);
  wib->SetContinueOnFEMBSyncError(continueOnFEMBSyncError);
  wib->SetContinueIfListOfFEMBClockPhasesDontSync(continueIfListOfFEMBClockPhasesDontSync);
  
  // For SBND, number of FEMBs is fixed
  TLOG_DEBUG(identification) << "N DAQ Links:  "  << FEMB_COUNT << TLOG_ENDL;
  TLOG_DEBUG(identification) << "N FEMB Ports: "  << FEMB_COUNT << TLOG_ENDL;
  WIB::WIB_DAQ_t daqMode = wib->GetDAQMode();
  
  // Need to check firmware version here

  // Check and print firmware version
  uint32_t wib_fw_version = wib->Read("SYSTEM.FW_VERSION");
  
  TLOG_INFO(identification) << "WIB Firmware Version: 0x" 
        << std::hex << std::setw(8) << std::setfill('0')
        <<  wib_fw_version
        << " Synthesized: " 
        << std::hex << std::setw(2) << std::setfill('0')
        << wib->Read("SYSTEM.SYNTH_DATE.CENTURY")
        << std::hex << std::setw(2) << std::setfill('0')
        << wib->Read("SYSTEM.SYNTH_DATE.YEAR") << "-"
        << std::hex << std::setw(2) << std::setfill('0')
        << wib->Read("SYSTEM.SYNTH_DATE.MONTH") << "-"
        << std::hex << std::setw(2) << std::setfill('0')
        << wib->Read("SYSTEM.SYNTH_DATE.DAY") << " "
        << std::hex << std::setw(2) << std::setfill('0')
        << wib->Read("SYSTEM.SYNTH_TIME.HOUR") << ":"
        << std::hex << std::setw(2) << std::setfill('0')
        << wib->Read("SYSTEM.SYNTH_TIME.MINUTE") << ":"
        << std::hex << std::setw(2) << std::setfill('0')
        << wib->Read("SYSTEM.SYNTH_TIME.SECOND")  << TLOG_ENDL;
  
  if (expected_wib_fw_version != wib_fw_version)
  {
    cet::exception excpt(identification);
    excpt << "WIB Firmware version is "
        << std::hex << std::setw(8) << std::setfill('0')
        << wib_fw_version
        <<" but expect "
        << std::hex << std::setw(8) << std::setfill('0')
        << expected_wib_fw_version
        <<" version in fcl";
    throw excpt;
  }

  // Reset and setup clock
  if(force_full_reset)
  {
    TLOG_INFO(identification) << "Running Full Reset on the WIB" << TLOG_ENDL;
    wib->ResetWIBAndCfgDTS(local_clock,partition_number,DTS_source);
  }
  else
  {
    TLOG_INFO(identification) << "Running Checked Reset on the WIB" << TLOG_ENDL;
    wib->CheckedResetWIBAndCfgDTS(local_clock,partition_number,DTS_source);    
    TLOG_INFO(identification) << "Finished Checked Reset on the WIB" << TLOG_ENDL;
    if (daqMode == WIB::FELIX)
    {                                                  
      TLOG_INFO(identification) << "SI5342 Status: " << wib->Read("DAQ.SI5342.ENABLE") << TLOG_ENDL;
    }                                               
  }
  std::this_thread::sleep_for(std::chrono::seconds(1));

  // Check DAQ link copy mode
  if (wib->Read("FEMB_REPLACE_FEMB_3_N_5_WITH_1_N_2"))
  {
    cet::exception excpt(identification);
    excpt << "WIB is set to duplicate data from links 1 and 2 to 3 and 4. This shouldn't happen!";
    throw excpt;
  }
  
  // Configure WIB fake data enable and mode
  TLOG_INFO(identification) << "Configuring WIB Fake Data" << TLOG_ENDL;
  TLOG_INFO(identification) << "Is Fake:"
                                        << " FEMB1: " << use_WIB_fake_data.at(0)
                                        << " FEMB2: " << use_WIB_fake_data.at(1)
                                        << " FEMB3: " << use_WIB_fake_data.at(2)
                                        << " FEMB4: " << use_WIB_fake_data.at(3) << TLOG_ENDL;
  wib->ConfigWIBFakeData(use_WIB_fake_data.at(0),
                         use_WIB_fake_data.at(1),
                         use_WIB_fake_data.at(2),
                         use_WIB_fake_data.at(3),
                         use_WIB_fake_data_counter);
  
  // Configure FEMBs
  for(size_t iFEMB=1; iFEMB <= 4; iFEMB++)
  {
    if(enable_FEMBs.at(iFEMB-1))
    {
      fhicl::ParameterSet const& FEMB_config = FEMB_configs.at(iFEMB-1);
      auto enable_FEMB_fake_data = FEMB_config.get<bool>("enable_fake_data");

      if(enable_FEMB_fake_data)
      {
        TLOG_INFO(identification) << "Setting up FEMB "<<iFEMB<<" for fake data" << TLOG_ENDL;
        setupFEMBFakeData(iFEMB,FEMB_config,continueOnFEMBRegReadError);
      }
      else
      {
        TLOG_INFO(identification) << "Setting up FEMB"<<iFEMB << TLOG_ENDL;
        setupFEMB(iFEMB,FEMB_config,continueOnFEMBRegReadError);
      }
    }
    else
    {
      TLOG_INFO(identification) << "FEMB"<<iFEMB<<" not enabled" << TLOG_ENDL;
    }
  }

  TLOG_INFO(identification) << "Enabling DAQ links" << TLOG_ENDL;
  wib->StartStreamToDAQ();
  
  TLOG_INFO(identification) << "Configured WIB" << TLOG_ENDL;
}

void WIBReader::setupFEMBFakeData(size_t iFEMB, fhicl::ParameterSet const& FEMB_config, 
				  bool continueOnFEMBRegReadError) 
{
  // Don't forget to disable WIB fake data

  const std::string identification = "WIBReader::setupFEMBFakeData";
  
  wib->FEMBPower(iFEMB,1);
  sleep(5);

  if(wib->ReadFEMB(iFEMB,"VERSION_ID") == wib->ReadFEMB(iFEMB,"SYS_RESET")) // can't read register if equal
  { 
    if(continueOnFEMBRegReadError)
    {
      TLOG_WARNING(identification) << "Warning: Can't read registers from FEMB " 
				   << int(iFEMB) 
				   << ". Powering it down and continuing on to others" << TLOG_ENDL;
      wib->FEMBPower(iFEMB,0);
      return;
    }
    else
    {
      wib->FEMBPower(iFEMB,0);
      cet::exception excpt(identification);
      excpt << "Can't read registers from FEMB " << int(iFEMB);
      throw excpt;
    }
  }

  auto expected_femb_fw_version = FEMB_config.get<uint32_t>("expected_femb_fw_version");
  uint32_t femb_fw_version = wib->ReadFEMB(iFEMB,"VERSION_ID");
  if (expected_femb_fw_version != femb_fw_version)
  {
    cet::exception excpt(identification);
    excpt << "FEMB" << iFEMB << " Firmware version is "
        << std::hex << std::setw(8) << std::setfill('0')
        << femb_fw_version
        <<" but expect "
        << std::hex << std::setw(8) << std::setfill('0')
        << expected_femb_fw_version
        <<" version in fcl";
    throw excpt;
  }

  uint8_t fake_mode = 0;
  uint16_t fake_word = 0;
  uint8_t femb_number = iFEMB;
  std::vector<uint32_t> fake_waveform;

  auto fakeDataSelect = FEMB_config.get<std::string>("fake_data_select");
  if (fakeDataSelect == "fake_word")
  {
    fake_mode = 1;
    fake_word = FEMB_config.get<uint32_t>("fake_word");
  }
  else if (fakeDataSelect == "fake_waveform")
  {
    fake_mode = 2;
    fake_waveform = FEMB_config.get<std::vector<uint32_t> >("fake_waveform");
    if (fake_waveform.size() != 255)
    {
      cet::exception excpt(identification);
      excpt << "setupFEMBFakeData: FEMB "
          << iFEMB
          << " fake_waveform must be 255 long, but is "
          << fake_waveform.size()
          << " long";
      throw excpt;
    }
  }
  else if (fakeDataSelect == "femb_channel_id")
  {
    fake_mode = 3;
  }
  else if (fakeDataSelect == "counter_channel_id")
  {
    fake_mode = 4;
  }
  else
  {
    cet::exception excpt(identification);
    excpt << "FEMB" << iFEMB << " fake_data_select is \""
        << fakeDataSelect
        <<"\" but expect "
        <<" fake_word, fake_waveform, femb_channel_id, or counter_channel_id";
    throw excpt;
  }

  wib->ConfigFEMBFakeData(iFEMB,fake_mode,fake_word,femb_number,fake_waveform);
}

void WIBReader::setupFEMB(size_t iFEMB, fhicl::ParameterSet const& FEMB_config, bool continueOnFEMBRegReadError)
{
  // Don't forget to disable WIB fake data

  const std::string identification = "wibdaq::WIBReader::setupFEMB";
  
  const auto gain         = FEMB_config.get<uint32_t>("gain");
  const auto shape        = FEMB_config.get<uint32_t>("shape");
  const auto baselineHigh = FEMB_config.get<uint32_t>("baselineHigh");
  const auto leakHigh     = FEMB_config.get<uint32_t>("leakHigh");
  const auto leak10X      = FEMB_config.get<uint32_t>("leak10X");
  const auto acCouple     = FEMB_config.get<uint32_t>("acCouple");
  const auto buffer       = FEMB_config.get<uint32_t>("buffer");
  const auto extClk       = FEMB_config.get<uint32_t>("extClk");
  const auto clk_phases   = FEMB_config.get<std::vector<uint16_t> >("clk_phases");
  const auto pls_mode     = FEMB_config.get<uint32_t>("pls_mode");
  const auto pls_dac_val  = FEMB_config.get<uint32_t>("pls_dac_val");

  const auto start_frame_mode_sel = FEMB_config.get<uint32_t>("start_frame_mode_sel");
  const auto start_frame_swap     = FEMB_config.get<uint32_t>("start_frame_swap");

  const auto expected_femb_fw_version = FEMB_config.get<uint32_t>("expected_femb_fw_version");

  if (gain > 3)
  {
    cet::exception excpt(identification);
    excpt << "setupFEMB: FEMB "
        << iFEMB
        << " gain shouldn't be larger than 3 is: "
        << gain;
    throw excpt;
  }

  if (shape > 3)
  {
    cet::exception excpt(identification);
    excpt << "setupFEMB: FEMB "
        << iFEMB
        << " shape shouldn't be larger than 3 is: "
        << shape;
    throw excpt;
  }

  if (baselineHigh > 2)
  {
    cet::exception excpt(identification);
    excpt << "setupFEMB: FEMB "
        << iFEMB
        << " baselineHigh should be 0 or 1 or 2 is: "
        << baselineHigh;
    throw excpt;
  }

  if (leakHigh > 1)
  {
    cet::exception excpt(identification);
    excpt << "setupFEMB: FEMB "
        << iFEMB
        << " leakHigh should be 0 or 1 is: "
        << leakHigh;
    throw excpt;
  }

  if (leak10X > 1)
  {
    cet::exception excpt(identification);
    excpt << "setupFEMB: FEMB "
        << iFEMB
        << " leak10X should be 0 or 1 is: "
        << leak10X;
    throw excpt;
  }

  if (acCouple > 1)
  {
    cet::exception excpt(identification);
    excpt << "setupFEMB: FEMB "
        << iFEMB
        << " acCouple should be 0 or 1 is: "
        << acCouple;
    throw excpt;
  }

  if (buffer > 1)
  {
    cet::exception excpt(identification);
    excpt << "setupFEMB: FEMB "
        << iFEMB
        << " buffer should be 0 or 1 is: "
        << buffer;
    throw excpt;
  }

  if (extClk > 1)
  {
    cet::exception excpt(identification);
    excpt << "setupFEMB: FEMB "
        << iFEMB
        << " extClk should be 0 or 1 is: "
        << extClk;
    throw excpt;
  }

  if (clk_phases.size() == 0)
  {
    cet::exception excpt(identification);
    excpt << "setupFEMB: FEMB "
        << iFEMB
        << " clk_phases size should be > 0 ";
    throw excpt;
  }

  if (pls_mode > 2)
  {
    cet::exception excpt(identification);
    excpt << "setupFEMB: FEMB "
        << iFEMB
        << " pls_mode should be 0 (off) 1 (FE ASIC internal) or 2 (FPGA external) is: "
        << pls_mode;
    throw excpt;
  }

  if ((pls_mode == 1 && pls_dac_val > 63) || (pls_mode == 2 && pls_dac_val > 31))
  {
    cet::exception excpt(identification);
    excpt << "setupFEMB: FEMB "
        << iFEMB
        << " pls_dac_val should be 0-31 in pls_mode 1 or 0-63 in pls_mode 2."
        << " pls_mode is " << pls_mode
        << " pls_dac_val is " << pls_dac_val;
    throw excpt;
  }

  if (start_frame_mode_sel > 1)
  {
    cet::exception excpt(identification);
    excpt << "setupFEMB: FEMB "
        << iFEMB
        << " start_frame_mode_sel should be 0 or 1 is: "
        << start_frame_mode_sel;
    throw excpt;
  }
  if (start_frame_swap > 1)

  {
    cet::exception excpt(identification);
    excpt << "setupFEMB: FEMB "
        << iFEMB
        << " start_frame_swap should be 0 or 1 is: "
        << start_frame_swap;
    throw excpt;
  }


  wib->FEMBPower(iFEMB,1);
  sleep(5);

  if(wib->ReadFEMB(iFEMB,"VERSION_ID") == wib->ReadFEMB(iFEMB,"SYS_RESET")) // can't read register if equal
  { 
    if(continueOnFEMBRegReadError)
    {
      TLOG_WARNING(identification) << "Warning: Can't read registers from FEMB " << int(iFEMB) 
				   << ". Powering it down and continuing on to others" << TLOG_ENDL;
      wib->FEMBPower(iFEMB,0);
      return;
    }
    else
    {
      wib->FEMBPower(iFEMB,0);
      cet::exception excpt(identification);
      excpt << "Can't read registers from FEMB " << int(iFEMB);
      throw excpt;
    }
  }

  uint32_t femb_fw_version = wib->ReadFEMB(iFEMB,"VERSION_ID");
  if (expected_femb_fw_version != femb_fw_version)
  {
    cet::exception excpt(identification);
    excpt << "FEMB" << iFEMB << " Firmware version is "
        << std::hex << std::setw(8) << std::setfill('0')
        << femb_fw_version
        <<" but expect "
        << std::hex << std::setw(8) << std::setfill('0')
        << expected_femb_fw_version
        <<" version in fcl";
    throw excpt;
  }

  std::vector<uint32_t> fe_config = {gain,shape,baselineHigh,leakHigh,leak10X,acCouple,buffer,extClk};
  wib->ConfigFEMB(iFEMB, fe_config, clk_phases, pls_mode, pls_dac_val, start_frame_mode_sel, start_frame_swap);
}

// "shutdown" transition
WIBReader::~WIBReader() 
{

}

// "start" transition
void WIBReader::start() 
{
  const std::string identification = "WIBReader::start";
  if (!wib) 
  {
    cet::exception excpt(identification);
    excpt << "WIB object pointer NULL";
    throw excpt;
  }
}

// "stop" transition
void WIBReader::stop() 
{
  const std::string identification = "WIBReader::stop";
  if (!wib) 
  {
    cet::exception excpt(identification);
    excpt << "WIB object pointer NULL";
    throw excpt;
  }
}

// Called by BoardReaderMain in a loop between "start" and "stop"
bool WIBReader::getNext_(artdaq::FragmentPtrs& /*frags*/) 
{
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  return (! should_stop()); // returning false before should_stop makes all other BRs stop
}

} // namespace

DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(sbndaq::WIBReader)
