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

#include "BNL_UDP.hh"
#include "ItemConversion.hh"
#include "AddressTable.hh"
#include "WIBBase.hh"
#include "WIB.hh"
//#include "WIBStatus.hh"

#include <sstream>
#include <vector>
#include <memory>
#include <iomanip>
#include <chrono>
#include <thread>
#include <ctime>
#include <time.h>

namespace sbndaq 
{

 WIBReader::WIBReader(fhicl::ParameterSet const& ps): CommandableFragmentGenerator(ps),
     semaphore_acquire_timeout_ms{ps.get<decltype(calibration_mode)>("semaphore_acquire_timeout_ms", 10000)},
     calibration_mode{ps.get<decltype(calibration_mode)>("calibration_mode", false)},
     semaphores_acquired{acquireSemaphores_ThrowOnFailure()}
 {
   const std::string identification = "WIBReader";
   TLOG_INFO(identification) << "WIBReader constructor" << TLOG_ENDL;
   
   //setupWIB(ps);
   
   bool success = false;
   unsigned configuration_tries=5;
   unsigned int success_index=0;
   
   for(unsigned iTry=1; iTry <= configuration_tries; iTry++){
       try{
          setupWIB(ps);
          success=true;
	  success_index=iTry;
          break;
       }
       catch(const WIBException::BAD_REPLY & exc){
             TLOG_WARNING(identification) << "WIB communication error: " << exc.what() << TLOG_ENDL;
       }
       catch(const WIBException::WIB_DTS_ERROR & exc){
             TLOG_WARNING(identification) << "WIB timing config error: " << exc.what() << TLOG_ENDL;
       }
       catch(const WIBException::exBase & exc){
             cet::exception excpt(identification);
             excpt << "Unhandled WIBException: "
	           << exc.what() << ": "
	            << exc.Description();
             throw excpt;
       }
       TLOG_INFO(identification) << "Configuraton try  " << iTry << " failed. Trying again..." << TLOG_ENDL;
       sleep(10);
    } // for iRetry

    if(!success){
       cet::exception excpt(identification);
       excpt << "Failed to configure WIB after " << configuration_tries << " tries";
       throw excpt;
    }
    
    if(!calibration_mode) releaseSemaphores();

    if(success){
       TLOG_INFO(identification) << "******** Configuration is successful in the " << success_index << " th try ***************" << TLOG_ENDL;
    }
 }

 void WIBReader::setupWIB(fhicl::ParameterSet const& WIB_config) 
 {
   auto wib_address                = WIB_config.get<std::string>("WIB.address");
   auto wib_table                  = WIB_config.get<std::string>("WIB.wib_table");
   auto femb_table                 = WIB_config.get<std::string>("WIB.femb_table");
   auto DTS_source                 = WIB_config.get<uint8_t>("WIB.DTS_source");
   auto enable_FEMBs               = WIB_config.get<std::vector<bool> >("WIB.enable_FEMBs");
   auto FEMB_configs               = WIB_config.get<std::vector<fhicl::ParameterSet> >("WIB.FEMBs");
      
   const std::string identification = "SBNDWIBReader::setupWIB";
   TLOG_INFO(identification) << "Starting setupWIB " << TLOG_ENDL;
   
      
   //TLOG_INFO(identification) << "Connecting to WIB at " <<  wib_address << TLOG_ENDL;
   
   //TLOG_INFO(identification) << "wib table " <<  wib_table << TLOG_ENDL;
   
   //TLOG_INFO(identification) << "femb table " << femb_table << TLOG_ENDL;
   
   wib = std::make_unique<WIB>("192.168.230.50","WIB_SBND_REGS.adt","SBND_FEMB.adt",true);
   //wib = std::make_unique<WIB>( wib_address, wib_table, femb_table );
   
   //TLOG_INFO(identification) << "Connected to WIB at " <<  wib_address << TLOG_ENDL;
   
   //wib=std::make_unique<WIB>(wib_address,wib_table,femb_table,true);
   
   TLOG_INFO(identification) << "Trying to execute configWIB function " << TLOG_ENDL;
   
   wib->configWIB(DTS_source);
   
   TLOG_INFO(identification) << "config WIB completed " << TLOG_ENDL;
   
   TLOG_INFO(identification) << "Trying to talk to FEMBs " << TLOG_ENDL;
      
   for(size_t iFEMB=1; iFEMB <= 4; iFEMB++){
       TLOG_INFO(identification) << "FEMB No. " << iFEMB << TLOG_ENDL; 
       if(enable_FEMBs.at(iFEMB-1)){
          TLOG_INFO(identification) << "FEMB is enabled" << TLOG_ENDL; 
	  fhicl::ParameterSet const& FEMB_config = FEMB_configs.at(iFEMB-1);
	  TLOG_INFO(identification) << "FEMB parameter is assigned" << TLOG_ENDL;
	  setupFEMB(iFEMB,FEMB_config);
	  TLOG_INFO(identification) << "setup FEMB " << iFEMB << TLOG_ENDL;
       }
   }
   
   TLOG_INFO(identification) << "WIB setup completed " << TLOG_ENDL;
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
  const std::string identification = "WIBReader::setupFEMB";
  TLOG_INFO(identification) << "inside setupFEMB " << TLOG_ENDL;
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
  
  TLOG_INFO(identification) << " Just before FEMBPower function" << TLOG_ENDL;
      
  wib->FEMBPower(iFEMB,1); 
  
  TLOG_INFO(identification) << " After powering up the FEMBs" << TLOG_ENDL;
  
  sleep(5);
      
  std::vector<uint32_t> fe_config = {gain,shape,baselineHigh,leakHigh,leak10X,acCouple,buffer,extClk};
  wib->ConfigFEMB(iFEMB, fe_config, clk_phases, pls_mode, pls_dac_val, start_frame_mode_sel, start_frame_swap);
  
  TLOG_INFO(identification) << " After configureing FEMBs" << TLOG_ENDL;
}

// "shutdown" transition
WIBReader::~WIBReader() 
{
 releaseSemaphores();
}

// "start" transition
void WIBReader::start() 
{
  const std::string identification = "WIBReader::start";
  if (!wib && calibration_mode) 
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
  if (!wib && calibration_mode ) 
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

bool WIBReader::acquireSemaphores(){
  //Create semaphores
  TLOG(TLVL_INFO) << "Acquiring semaphores.";
  sem_t *sem_wib_lck = sem_open(WIB::SEMNAME_WIBLCK, O_CREAT, 0666, 1);
  sem_t *sem_wib_yld = sem_open(WIB::SEMNAME_WIBYLD, O_CREAT, 0666, 1);
  if (sem_wib_lck == SEM_FAILED || sem_wib_yld == SEM_FAILED) {
    TLOG(TLVL_ERROR) << "Failed to create either " << WIB::SEMNAME_WIBLCK << " or " << WIB::SEMNAME_WIBYLD <<".";
    releaseSemaphores();
    return false;
  }

  struct timespec timeout;
  clock_gettime(CLOCK_REALTIME, &timeout);
  timeout.tv_nsec += 500000;
  if (sem_timedwait(sem_wib_yld, &timeout) != 0){
    TLOG(TLVL_ERROR) << "Failed to acquire " << WIB::SEMNAME_WIBYLD<< " semaphore.";
    releaseSemaphores();
    return false;
  }

  clock_gettime(CLOCK_REALTIME, &timeout);
  timeout.tv_sec += semaphore_acquire_timeout_ms / 1000;
  timeout.tv_nsec += (semaphore_acquire_timeout_ms % 1000) * 1000000;
  if (sem_timedwait(sem_wib_lck, &timeout) != 0){
    TLOG(TLVL_ERROR) << "Failed to acquire " << WIB::SEMNAME_WIBLCK << " semaphore.";
    releaseSemaphores();
    return false;
  }
  TLOG(TLVL_INFO) << "Acquired semaphores.";
  return true;
}

bool WIBReader::acquireSemaphores_ThrowOnFailure(){
  if (acquireSemaphores())
    return true;

  cet::exception excpt("WIBReader::acquireSemaphores_ThrowOnFailure");
  excpt << "The operation was unsuccessful. Please try the following steps to resolve the issue. Terminate any running instances of the FEMBreceiver (femb), WIBTool.exe, or WIB Boardreader processes. Then, delete the semaphores /dev/shm/sem.WIB_LCK and /dev/shm/sem.WIB_YLD. If you intend to run both FEMBreceiver and WIBTool.exe, start with FEMBreceiver first.";
  throw excpt;
}

void WIBReader::releaseSemaphores(){
  if (nullptr!=sem_wib_yld){
    sem_post(sem_wib_yld);
    sem_close(sem_wib_yld);
    sem_wib_yld=nullptr;
  }

  if(nullptr!=sem_wib_lck){
    sem_post(sem_wib_lck);
    sem_close(sem_wib_lck);
    sem_wib_lck=nullptr;
  }

  semaphores_acquired=false;

  TLOG(TLVL_INFO) << "Released semaphores";
}
} // namespace

DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(sbndaq::WIBReader)
