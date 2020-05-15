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

 WIBReader::WIBReader(fhicl::ParameterSet const& ps): CommandableFragmentGenerator(ps) 
 {
   const std::string identification = "WIBReader";
   TLOG_INFO(identification) << "WIBReader constructor" << TLOG_ENDL;
   setupWIB(ps);
 }

 void WIBReader::setupWIB(fhicl::ParameterSet const& WIB_config) 
 {
   auto wib_address                = WIB_config.get<std::string>("wib000_standard.fragment_receiver.config.WIB.address");
   auto wib_table                  = WIB_config.get<std::string>("wib000_standard.fragment_receiver.config.wib_table");
   auto femb_table                 = WIB_config.get<std::string>("wib000_standard.fragment_receiver.config.femb_table");
   auto DTS_source                 = WIB_config.get<uint8_t>("wib000_standard.fragment_receiver.config.WIB.DTS_source");
   auto enable_FEMBs               = WIB_config.get<std::vector<bool> >("wib000_standard.fragment_receiver.config.enable_FEMBs");
   auto FEMB_configs               = WIB_config.get<std::vector<fhicl::ParameterSet> >("wib000_standard.fragment_receiver.config.WIB.FEMBs");
      
   const std::string identification = "SBNDWIBReader::setupWIB";
   TLOG_INFO(identification) << "Starting setupWIB " << TLOG_ENDL;
      
   wib=std::make_unique<WIB>(wib_address,wib_table,femb_table,true);
   wib->configWIB(DTS_source);
      
   for(size_t iFEMB=1; iFEMB <= 4; iFEMB++){
       if(enable_FEMBs.at(iFEMB-1)){
	  fhicl::ParameterSet const& FEMB_config = FEMB_configs.at(iFEMB-1);
	  setupFEMB(iFEMB,FEMB_config);
       }
   }
 }

void WIBReader::setupFEMBFakeData(size_t iFEMB, fhicl::ParameterSet const& FEMB_config, bool continueOnFEMBRegReadError) 
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

void WIBReader::setupFEMB(size_t iFEMB, fhicl::ParameterSet const& FEMB_configure)
{
  const std::string identification = "wibdaq::SBNDWIBReader::setupFEMB";
  const auto gain         = FEMB_configure.get<uint32_t>("gain");
  const auto shape        = FEMB_configure.get<uint32_t>("shape");
  const auto baselineHigh = FEMB_configure.get<uint32_t>("baselineHigh");
  const auto leakHigh     = FEMB_configure.get<uint32_t>("leakHigh");
  const auto leak10X      = FEMB_configure.get<uint32_t>("leak10X");
  const auto acCouple     = FEMB_configure.get<uint32_t>("acCouple");
  const auto buffer       = FEMB_configure.get<uint32_t>("buffer");
  const auto extClk       = FEMB_configure.get<uint32_t>("extClk");
  const auto clk_phases   = FEMB_configure.get<std::vector<uint16_t> >("clk_phases");
  const auto pls_mode     = FEMB_configure.get<uint32_t>("pls_mode");
  const auto pls_dac_val  = FEMB_configure.get<uint32_t>("pls_dac_val");
  const auto start_frame_mode_sel = FEMB_configure.get<uint32_t>("start_frame_mode_sel");
  const auto start_frame_swap     = FEMB_configure.get<uint32_t>("start_frame_swap");
  
  if(signed(gain)>3 || signed(gain)<0){
     cet::exception excpt(identification);
     excpt << "FEMB gain is not acceptable";
     throw excpt;
  }
  
  if(signed(shape)>3 || signed(shape)<0){
     cet::exception excpt(identification);
     excpt << "FEMB shape time is not acceptable";
     throw excpt;
  }
  
  if(signed(baselineHigh)>2 || signed(baselineHigh)<0){
     cet::exception excpt(identification);
     excpt << "FEMB baseline height is not acceptable";
     throw excpt;
  }
  
  if(signed(leakHigh)>1 || signed(leakHigh)<0){
     cet::exception excpt(identification);
     excpt << "FEMB leakage height is not acceptable";
     throw excpt;
  }
  
  if(signed(leakHigh)>1 || signed(leakHigh)<0){
     cet::exception excpt(identification);
     excpt << "FEMB leakage height is not acceptable";
     throw excpt;
  }
  
  if(signed(leak10X)>1 || signed(leak10X)<0){
     cet::exception excpt(identification);
     excpt << "FEMB leak10X is not acceptable";
     throw excpt;
  }
  
  if(signed(acCouple)>1 || signed(acCouple)<0){
     cet::exception excpt(identification);
     excpt << "FEMB AC/DC coupling is not acceptable";
     throw excpt;
  }
  
  if(signed(buffer)>1 || signed(buffer)<0){
     cet::exception excpt(identification);
     excpt << "FEMB buffer is not acceptable";
     throw excpt;
  }
  
  if(signed(extClk)!=1){
     cet::exception excpt(identification);
     excpt << "FEMB external clock is not acceptable";
     throw excpt;
  }
  
  if(signed(pls_mode)>2 || signed(pls_mode)<0){
     cet::exception excpt(identification);
     excpt << "FEMB pulse mode is not acceptable";
     throw excpt;
  }
  
  if(signed(pls_dac_val)!=0){
     cet::exception excpt(identification);
     excpt << "FEMB calibration pulser amplitude setting is not acceptable";
     throw excpt;
  }
  
  if(signed(start_frame_mode_sel)!=1){
     cet::exception excpt(identification);
     excpt << "FEMB start frame mode setting is not acceptable";
     throw excpt;
  }
  
  if(signed(start_frame_swap)!=1){
     cet::exception excpt(identification);
     excpt << "FEMB start frame mode swapping is not acceptable";
     throw excpt;
  }
      
  wib->FEMBPower(iFEMB,1); 
  sleep(5);
      
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
