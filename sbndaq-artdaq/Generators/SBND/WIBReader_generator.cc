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
     use_semaphores{ps.get<bool>("use_semaphores",false)}, 
     semaphore_acquire_timeout_ms{ps.get<decltype(semaphore_acquire_timeout_ms)>("semaphore_acquire_timeout_ms")}, // 10000
     calibration_mode{ps.get<decltype(calibration_mode)>("calibration_mode")},
     sem_wib_yld{nullptr},sem_wib_lck{nullptr},
     semaphores_acquired{acquireSemaphores_ThrowOnFailure()},
     wib{nullptr}
 {
   time_t start, end;
   time(&start);
   const std::string identification = "WIBReader";
   TLOG_INFO(identification) << "WIBReader constructor";
   
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

    if(success){
       TLOG_INFO(identification) << "******** Configuration is successful in the " << success_index << " th try for WIB with IP address " << wib->GetAddress() << "  ***************" << TLOG_ENDL;
    }
    time(&end);
    double time_taken = double(end - start);
    TLOG_INFO(identification) << "Time took to run DAQ : " << time_taken << std::setprecision(5) << "  seconds." << TLOG_ENDL;
    TLOG_INFO(identification) << "================== WIB SUMMARY AFTER CONFIGURATION ======================" << TLOG_ENDL;
    TLOG_INFO(identification) << "WIB FW VERSION : " << std::hex << int(wib->Read("FW_VERSION")) << TLOG_ENDL;
    TLOG_INFO(identification) << "SLOT ADDRESS : " << std::hex << int(wib->Read("SLOT_ADDR")) << TLOG_ENDL;
    TLOG_INFO(identification) << "CRATE ADDRESS : " << std::hex << int(wib->Read("CRATE_ADDR")) << TLOG_ENDL;
    TLOG_INFO(identification) << "FIRMWARE TRACKER : " << std::hex << int(wib->Read(0x100)) << TLOG_ENDL;

    if(!calibration_mode) disconnectWIB_releaseSemaphores();
    TLOG_INFO(identification) << "WIBReader constructor completed";
 }

 void WIBReader::setupWIB(fhicl::ParameterSet const& WIB_config) 
 {
   auto expected_wib_fw_version    = WIB_config.get<uint32_t>("WIB.expected_wib_fw_version");
   auto wib_address                = WIB_config.get<std::string>("WIB.address");
   auto wib_table                  = WIB_config.get<std::string>("WIB.wib_table");
   auto femb_table                 = WIB_config.get<std::string>("WIB.femb_table");
   auto DTS_source                 = WIB_config.get<uint8_t>("WIB.DTS_source");
   auto enable_FEMBs               = WIB_config.get<std::vector<bool> >("WIB.enable_FEMBs");
   auto FEMB_configs               = WIB_config.get<std::vector<fhicl::ParameterSet> >("WIB.FEMBs");
   auto wib_fake_data              = WIB_config.get<bool>("WIB.run_wib_fake_data_mode");
   auto wib_fake_data_id           = WIB_config.get<uint8_t>("WIB.wib_fake_data_mode");
   auto do_femb_scan               = WIB_config.get<bool>("WIB.run_femb_scan");
   auto use_old_wib_config         = WIB_config.get<bool>("WIB.use_old_wib_config");
   
   const std::string identification = "SBNDWIBReader::setupWIB";
   
   TLOG_INFO(identification) << "************* Now Starting setupWIB  ****************" << TLOG_ENDL;
   
   wib = std::make_unique<WIB>(wib_address,wib_table,femb_table,true);
   
   if (use_old_wib_config){
       uint32_t wib_fw_version = wib->Read("FW_VERSION");
       sleep(1);
       wib_fw_version = wib->Read("FW_VERSION");
       
       if (wib_fw_version != expected_wib_fw_version){
         cet::exception excpt(identification);
         excpt << " WIB Firmware version is "
         << std::hex << std::setw(8) << std::setfill('0')
         << wib_fw_version
         <<" but expect "
         << std::hex << std::setw(8) << std::setfill('0')
         << expected_wib_fw_version
         <<" version in fcl";
         throw excpt;
       }
   } 
   
   if (use_old_wib_config){ 
       TLOG_INFO(identification) << "Now Trying to execute configWIB function " << TLOG_ENDL;
       wib->configWIB(DTS_source);
   }
   
   if (!use_old_wib_config){ 
      TLOG_INFO(identification) << "Now Trying to execute WIBs_SCAN function " << TLOG_ENDL; 
      wib->WIBs_SCAN(expected_wib_fw_version, DTS_source);
   }
   
   if (!use_old_wib_config){
      if (do_femb_scan){ 
         TLOG_INFO(identification) << "Now Trying to execute FEMB_SCAN function " << TLOG_ENDL;
         FEMB_SCAN(enable_FEMBs, (FEMB_configs.at(0)).get<uint32_t>("expected_femb_fw_version"));
      }
   }
   
   if(!use_old_wib_config){ 
      TLOG_INFO(identification) << "Now Trying to execute WIBs_CFG_INIT function " << TLOG_ENDL;
      wib->WIBs_CFG_INIT();
   }
   
   if(use_old_wib_config){ 
     TLOG_INFO(identification) << "Now Trying to execute disable_dat_stream_and_sync_to_NEVIS function " << TLOG_ENDL;
     disable_dat_stream_and_sync_to_NEVIS();
   }
   
   if (wib_fake_data){
      TLOG_INFO(identification) << "Now Trying to execute setupWIBFakeData function " << TLOG_ENDL;
      setupWIBFakeData(wib_fake_data_id);
   } 
   
   TLOG_INFO(identification) << "Now Connecting to FEMBs " << TLOG_ENDL;
     
   for(size_t iFEMB=1; iFEMB <= 4; iFEMB++){
       TLOG_INFO(identification) << "FEMB No. " << iFEMB << TLOG_ENDL; 
       
       fhicl::ParameterSet const& FEMB_config = FEMB_configs.at(iFEMB-1);
       
       if(enable_FEMBs.at(iFEMB-1)){
          TLOG_INFO(identification) << "FEMB is enabled" << TLOG_ENDL; 
	  fhicl::ParameterSet const& FEMB_config = FEMB_configs.at(iFEMB-1);
	  TLOG_INFO(identification) << "FEMB parameter is assigned" << TLOG_ENDL;
	  if (!wib_fake_data) setupFEMB(iFEMB,FEMB_config);
	  TLOG_INFO(identification) << "setup FEMB " << iFEMB << TLOG_ENDL;
       }
   }
   
   TLOG_INFO(identification) << "FEMBs are configured." << TLOG_ENDL;
   TLOG_INFO(identification) << "About to issue WIB sync command once. " << TLOG_ENDL;
   IssueWIBSYNC();
   
   TLOG_INFO(identification) << "***************** setupWIB completed **************************" << TLOG_ENDL;
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

void WIBReader::setupWIBFakeData(int datamode)
{
    const std::string identification = "WIBReader::setupWIBFakeData";
    
    TLOG_INFO(identification) << "***************** Now starting setupWIBFakeData **************************" << TLOG_ENDL;
    
    int ver_value = wib->Read(0xFF);
    if (datamode == 1){
        std::vector<unsigned int> wfs = { 0x2FF, 0x2FE, 0x2FB, 0x2FF, 0x300, 0x303, 0x301, 0x301,
        0x2FF, 0x2FE, 0x2FF, 0x2FF, 0x2FF, 0x2FF, 0x2FF, 0x2FF,
        0x301, 0x301, 0x301, 0x301, 0x301, 0x301, 0x300, 0x301,
        0x301, 0x301, 0x2FF, 0x2FF, 0x2FF, 0x2FF, 0x2FF, 0x2FF,
        0x2FF, 0x2FF, 0x300, 0x2FF, 0x2FF, 0x2FF, 0x2FF, 0x300,
        0x2FF, 0x300, 0x301, 0x301, 0x301, 0x2FF, 0x2FF, 0x2FF,
        0x2FE, 0x2FE, 0x301, 0x302, 0x303, 0x301, 0x2FF, 0x2FB,
        0x2FE, 0x2FF, 0x301, 0x306, 0x304, 0x321, 0x39D, 0x455,
        0x4DF, 0x501, 0x4C1, 0x454, 0x3DF, 0x37F, 0x340, 0x31A,
        0x308, 0x300, 0x2FF, 0x2FF, 0x2FF, 0x2FF, 0x300, 0x300,
        0x2FF, 0x300, 0x301, 0x300, 0x2FF, 0x2FF, 0x2FF, 0x2FF,
        0x300, 0x2FF, 0x303, 0x301, 0x302, 0x301, 0x300, 0x2FF,
        0x2FF, 0x300, 0x2FF, 0x2FF, 0x2FF, 0x2DF, 0x264, 0x1B9,
        0x13F, 0x122, 0x150, 0x1B8, 0x228, 0x281, 0x2C3, 0x2E8,
        0x2FB, 0x2FF, 0x2FF, 0x300, 0x300, 0x301, 0x301, 0x2FF,
        0x2FF, 0x2FF, 0x300, 0x300, 0x300, 0x2FF, 0x2FF, 0x2FF,
        0x2FE, 0x2FF, 0x2FF, 0x300, 0x2FF, 0x302, 0x302, 0x301,
        0x301, 0x2FF, 0x2FE, 0x2FE, 0x2FE, 0x2FE, 0x2FD, 0x2FF,
        0x2FF, 0x2FF, 0x2FF, 0x2FE, 0x2FE, 0x2FF, 0x301, 0x304,
        0x304, 0x302, 0x2FF, 0x2FB, 0x2FA, 0x2FF, 0x300, 0x301,
        0x302, 0x301, 0x301, 0x301, 0x301, 0x301, 0x301, 0x301,
        0x301, 0x301, 0x301, 0x302, 0x301, 0x301, 0x300, 0x301,
        0x301, 0x300, 0x301, 0x301, 0x300, 0x2FF, 0x2FF, 0x2FF,
        0x301, 0x301, 0x301, 0x301, 0x301, 0x303, 0x302, 0x300,
        0x301, 0x2FF, 0x2FF, 0x2FF, 0x2FF, 0x300, 0x301, 0x301,
        0x2FF, 0x2FE, 0x2FE, 0x2FF, 0x300, 0x303, 0x306, 0x304,
        0x302, 0x2FF, 0x2FF, 0x2FF, 0x2FF, 0x2FF, 0x2FF, 0x2FF,
        0x2FF, 0x2FF, 0x301, 0x300, 0x2FF, 0x2FF, 0x301, 0x301,
        0x300, 0x2FF, 0x2FF, 0x2FF, 0x2FF, 0x2FF, 0x2FF, 0x301,
        0x303, 0x306, 0x303, 0x302, 0x2FF, 0x2FF, 0x2FF, 0x300,
        0x2FF, 0x2FF, 0x2FF, 0x301, 0x301, 0x303, 0x301, 0x2FF,
        0x2FF, 0x2FF, 0x301, 0x302, 0x304, 0x2FF, 0x2FF, 0x2FB };
	
	std::vector<unsigned int> datas;
	for (int i = 0; i < 256; i++){
	     datas.push_back( (wfs[i]<<12) + ((i&0xfff)<<4));
	}
	
	for (int i = 0; i < 256; i++){
	     int adr1 = i + 0x300;
	     wib->Write(adr1, datas[i]);
	     int adr2 = i + 0x200;
	     wib->Write(adr2, datas[i]);
	}
	
	wib->Write(9, 0x10);
	if (wib->Read(9) != 0x10){
	    cet::exception excpt(identification);
            excpt << "WIB register is not properly written. Written value : " << std::hex << 0x10 << " Read value : " << std::hex << wib->Read(9);
            throw excpt;
	}
    }
    
    else if(datamode == 2){
        wib->Write(9, 0x20);
	if (wib->Read(9) != 0x20){
	    cet::exception excpt(identification);
	    excpt << "WIB register 9 is not properly written. Written value : " << std::hex << 0x20 << " Read value : " << std::hex << wib->Read(9);
	    throw excpt;
	}
    }
    
    else{
      wib->Write(9, 0x0);
      if (wib->Read(9) != 0x0){
          cet::exception excpt(identification);
	  excpt << "WIB register 9 is not properly written. Written value : " << std::hex << 0x0 << " Read value : " << std::hex << wib->Read(9);
	  throw excpt;
      }
      return;
    }
    
    wib->Write(8, 0x0);
    if (wib->Read(8) != 0x0){
        cet::exception excpt(identification);
	excpt << "WIB register 8 is not properly written. Written value : " << std::hex << 0x0 << " Read value : " << std::hex << wib->Read(8);
	throw excpt;
    }
    sleep(0.1);
    TLOG_INFO(identification) << "***************** setupWIBFakeData completed **************************" << TLOG_ENDL;
}

void WIBReader::setupNoiseMinConfig(int FEMB_NO, int tries)
{
     const std::string identification = "WIBReader::setupNoiseMinConfig";
     if (wib->Read(0x04) != 0x03){
         wib->Write(0x04, 0x03);
	 sleep(0.1);
	 int iTries = 1;
	 while (true){
	    if (wib->Read(0x04) != 0x03){
		wib->Write(0x04, 0x03);
		iTries++;
		if (iTries > tries){
		    cet::exception excpt(identification);
		    excpt << "WIB register 4 is not properly written. Written value : " << std::hex << 0x3 << " Read value : " << std::hex <<     wib->Read(0x04);
		    throw excpt;
		    break;
		}
		sleep(0.1);
	    }
	    else{
	      break;
	    }   
	 } // while loop
     } // checking wib register address 0x04
     
     TLOG_INFO(identification) << "WIB register 0x04 value for FEMB " << FEMB_NO << " : " << std::hex << wib->Read(0x04) << TLOG_ENDL;
     
     if (wib->ReadFEMB(FEMB_NO, 0x09) != 0x09){
         wib->WriteFEMB(FEMB_NO, 0x09, 0x09);
	 sleep(0.1);
	 int iTries = 1;
	 while (true){
	   if (wib->ReadFEMB(FEMB_NO, 0x09) != 0x09){
	       wib->WriteFEMB(FEMB_NO, 0x09, 0x09);
	       iTries++;
	       if (iTries > tries){
	           cet::exception excpt(identification);
		   excpt << "FEMB register 9 is not properly written. Written value : " << std::hex << 0x09 << " Read value : " << std::hex << wib->ReadFEMB(FEMB_NO, 0x09);
		   throw excpt;
		   break;
	       }
	       sleep(0.1);
	   }
	   
	   else{
	      break;
	   }
	 } // while loop 
     } // checking FEMB register address 0x09
     
     TLOG_INFO(identification) << "FEMB register 0x09 value for FEMB " << FEMB_NO << " : " << std::hex << wib->ReadFEMB(FEMB_NO, 0x09) << TLOG_ENDL;
     //sleep(5);
}

void WIBReader::IssueWIBSYNC()
{
  const std::string identification = "WIBReader::IssueWIBSYNC";
  wib->Write(0x14, 0x0);
  sleep(0.1);
  if (wib->Read(0x14) != 0x0){
      cet::exception excpt(identification);
      excpt << "WIB register 0x14 is not properly written. Written value : " << std::hex << 0x0 << " Read value : " << std::hex << wib->Read(0x14);
      throw excpt;
  }
  wib->Write(0x14, 0x2);
  sleep(0.1);
  if (wib->Read(0x14) != 0x2){
     cet::exception excpt(identification);
     excpt << "WIB register 0x14 is not properly written. Written value : " << std::hex << 0x2 << " Read value : " << std::hex << wib->Read(0x14);
     throw excpt; 
  }
 
  wib->Write(0x14, 0x0);
  sleep(0.1);
  if (wib->Read(0x14) != 0x0){
      cet::exception excpt(identification);
      excpt << "WIB register 0x14 is not properly written. Written value : " << std::hex << 0x0 << " Read value : " << std::hex << wib->Read(0x14);
      throw excpt;
  }
}

void WIBReader::FEMBHsLinkCheck(int FEMB_NO, int tries)
{
  const std::string identification = "WIBReader::FEMBHsLinkCheck";
  
  TLOG_INFO(identification) << "************* Now Starting FEMBHsLinkCheck  ****************" << TLOG_ENDL;
  
  uint32_t linkStatBits = wib->ReadWithRetry("LINK_SYNC_STATUS_BRD"+std::to_string(FEMB_NO));
  auto link_1_stat = 0x3  & linkStatBits;
  auto link_2_stat = 0xC  & linkStatBits; 
  auto link_3_stat = 0x30 & linkStatBits;
  auto link_4_stat = 0xC0 & linkStatBits;
   
  if (link_1_stat == 0x0 || link_2_stat == 0x0 || link_3_stat == 0x0 || link_4_stat == 0x0){
      sleep(0.1);
      int iTries = 1;
       
      while (true){
       linkStatBits = wib->ReadWithRetry("LINK_SYNC_STATUS_BRD"+std::to_string(FEMB_NO));
       link_1_stat = 0x3  & linkStatBits;
       link_2_stat = 0xC  & linkStatBits; 
       link_3_stat = 0x30 & linkStatBits;
       link_4_stat = 0xC0 & linkStatBits;
       if (link_1_stat == 0x0 || link_2_stat == 0x0 || link_3_stat == 0x0 || link_4_stat == 0x0){
	   iTries++;
	   if (iTries > tries){
	       cet::exception excpt(identification);
	       excpt << "FEMB " << FEMB_NO << " Link status is not OK, " << " Link Stat. 1 : " << link_1_stat << " Link Stat. 2 : " << link_2_stat << "Link Stat. 3" << link_3_stat << " Link Stat. 4 : " << link_4_stat;
	       throw excpt;
	       break;
	   } // link stat check
	   sleep(0.1);
       } // link stat check
       else{
	 break;
       }
     } // while loop
   } // link stat check
   
   uint32_t eqStatBits = wib->ReadWithRetry("EQ_LOS_BRD"+std::to_string(FEMB_NO)+"_RX");
   auto link_1_eqlz = 0x1  & eqStatBits;
   auto link_2_eqlz = 0x2  & eqStatBits; 
   auto link_3_eqlz = 0x4 & eqStatBits;
   auto link_4_eqlz = 0x8 & eqStatBits;
   
   if (link_1_eqlz == 0x0 || link_2_eqlz == 0x0 || link_3_eqlz == 0x0 || link_4_eqlz == 0x0){
       sleep(0.1);
       int iTries = 1;
       
       while (true){
         eqStatBits = wib->ReadWithRetry("EQ_LOS_BRD"+std::to_string(FEMB_NO)+"_RX");
	 link_1_eqlz = 0x1  & eqStatBits;
         link_2_eqlz = 0x2  & eqStatBits; 
         link_3_eqlz = 0x4 & eqStatBits;
         link_4_eqlz = 0x8 & eqStatBits;
	 if (link_1_eqlz == 0x0 || link_2_eqlz == 0x0 || link_3_eqlz == 0x0 || link_4_eqlz == 0x0){
	     iTries++;
	     if (iTries > tries){
	         cet::exception excpt(identification);
		 excpt << "FEMB " << FEMB_NO << " Equlizer status is not OK, " << " Equalizer Stat. 1 : " << link_1_eqlz << " Equalizer Stat. 2 : " << link_2_eqlz << "Equalizer Stat. 3" << link_3_eqlz << " Equalizer Stat. 4 : " << link_4_eqlz;
		 throw excpt;
		 break;
	     }
	     sleep(0.1);
	 } // equalizer stat check
	 else{
	   break;
	 }
       } // while loop
   } // equlizer stat check
   
   TLOG_INFO(identification) << "************* FEMBHsLinkCheck completed ****************" << TLOG_ENDL;
}

void WIBReader::InitFEMBRegCheck(uint32_t expected_val, std::string reg_addrs, int FEMB_NO, int tries)
{
  const std::string identification = "WIBReader::InitFEMBRegCheck";
  
  TLOG_INFO(identification) << "************* Now Starting InitFEMBRegCheck  ****************" << TLOG_ENDL;
   
  if (wib->ReadFEMB(FEMB_NO, reg_addrs) != expected_val){
      sleep(0.1);
      int iTries = 1; 
      TLOG_INFO(identification) << "Try : " << iTries << " FEMB NO : " << FEMB_NO << " Expected Val : " << std::hex << expected_val << " Read Val : " << std::hex << wib->ReadFEMB(FEMB_NO, reg_addrs) << TLOG_ENDL;
      while (true){
        if (wib->ReadFEMB(FEMB_NO, reg_addrs) != expected_val){
	    iTries++;
	    TLOG_INFO(identification) << "Try : " << iTries << " FEMB NO : " << FEMB_NO << " Expected Val : " << std::hex << expected_val << " Read Val : " << std::hex << wib->ReadFEMB(FEMB_NO, reg_addrs) << TLOG_ENDL;
	    if (iTries > tries){
	        cet::exception excpt(identification);
		excpt << "FEMB " << FEMB_NO << " register " << reg_addrs << " value is " << std::hex << wib->ReadFEMB(FEMB_NO, reg_addrs) << " and expected value is " << std::hex << expected_val;
		throw excpt;
		break;
	    }
	    sleep(0.1);
	}
	else{
	  TLOG_INFO(identification) << "Try : " << iTries << " FEMB NO : " << FEMB_NO << " Expected Val : " << std::hex << expected_val << " Read Val : " << std::hex << wib->ReadFEMB(FEMB_NO, reg_addrs) << TLOG_ENDL;
	  break;
	}
     }
  }
  TLOG_INFO(identification) << "Expected value : " << std::hex << expected_val << " Read value : " << std::hex << wib->ReadFEMB(FEMB_NO, reg_addrs) << TLOG_ENDL;
  TLOG_INFO(identification) << "************* InitFEMBRegCheck completed ****************" << TLOG_ENDL;
}

void WIBReader::disable_dat_stream_and_sync_to_NEVIS()
{
  const std::string identification = "WIBReader::disable_dat_stream_and_sync_to_NEVIS";
  
  TLOG_INFO(identification) << "************* Now Starting disable_dat_stream_and_sync_to_NEVIS ****************" << TLOG_ENDL;
  
  wib->Write(20, 0x00);
  sleep(0.1);
  if (wib->Read(20) != 0x00){
      cet::exception excpt(identification);
      excpt << "Register 20 is not equal to 0";
      throw excpt;
  }
  wib->Write(20, 0x03);
  sleep(0.1);
  if (wib->Read(20) != 0x03){
      cet::exception excpt(identification);
      excpt << "Register 20 is not equal to 0x03";
      throw excpt;
  }
  wib->Write(20, 0x00);
  sleep(0.1);
  if (wib->Read(20) != 0x00){
      cet::exception excpt(identification);
      excpt << "Register 20 is not equal to 0";
      throw excpt;
  }
  
  TLOG_INFO(identification) << "************* disable_dat_stream_and_sync_to_NEVIS completed ****************" << TLOG_ENDL;
}

void WIBReader::FEMB_DECTECT(int FEMB_NO, uint32_t FEMB_V)
{
    // This function is a modified version of one of the funcitons available in Shanshan's python script
    // to configure WIB/FEMB.
    // The original function is in cls_config.py module inside the repository CE_LD with same name (git branch name is, Installation_Support)
    const std::string identification = "WIBReader::FEMB_DECTECT";
    std::map<std::string,double> map = wib->WIB_STATUS();
    
    if (map.find("FEMB"+std::to_string(FEMB_NO)+"_LINK")->second != 0xFF ){
	if(Retry_Read(map, "FEMB"+std::to_string(FEMB_NO)+"_LINK", 0xFF, 'e', 9)){ 
	   cet::exception excpt(identification);
	   excpt << "FEMB " << FEMB_NO << " LINK is broken";
	   throw excpt;
	}
    }
    
    if (map.find("FEMB"+std::to_string(FEMB_NO)+"_EQ")->second != 0xF ){
        if(Retry_Read(map, "FEMB"+std::to_string(FEMB_NO)+"_EQ", 0xF, 'e', 9)){
	  cet::exception excpt(identification);
	  excpt << "FEMB " << FEMB_NO << " EQ is broken";
	  throw excpt;
	}
    }
    
    if (map.find("FEMB"+std::to_string(FEMB_NO)+"_BIAS_I")->second < 0.001 ){
        if(Retry_Read(map, "FEMB"+std::to_string(FEMB_NO)+"_BIAS_I", 0.001, 'g', 9)){
	   cet::exception excpt(identification);
	   excpt << "FEMB " << FEMB_NO << " BIAS current " << map.find("FEMB"+std::to_string(FEMB_NO)+"_BIAS_I")->second << " A" << " is lower than expected";
	   throw excpt;
	}
    }
    
    if (map.find("FEMB"+std::to_string(FEMB_NO)+"_BIAS_I")->second > 0.1 ){
        if(Retry_Read(map, "FEMB"+std::to_string(FEMB_NO)+"_BIAS_I", 0.1, 'l', 9)){
	  cet::exception excpt(identification);
	  excpt << "FEMB " << FEMB_NO << " BIAS current " << map.find("FEMB"+std::to_string(FEMB_NO)+"_BIAS_I")->second << " A" << " is higher than expected";
	  throw excpt;
	}
    }
    
    if (map.find("FEMB"+std::to_string(FEMB_NO)+"_FMV39_I")->second < 0.010 ){
        if(Retry_Read(map, "FEMB"+std::to_string(FEMB_NO)+"_FMV39_I", 0.010, 'g', 9)){
	   cet::exception excpt(identification);
	   excpt << "FEMB " << FEMB_NO << " FM_V39 current " << map.find("FEMB"+std::to_string(FEMB_NO)+"_FMV39_I")->second << " A" << " lower than expected";
	   throw excpt;
	}
    }
    
    if (map.find("FEMB"+std::to_string(FEMB_NO)+"_FMV39_I")->second > 0.2 ){
        if(Retry_Read(map, "FEMB"+std::to_string(FEMB_NO)+"_FMV39_I", 0.2, 'l', 9)){
	   cet::exception excpt(identification);
	   excpt << "FEMB " << FEMB_NO << " FM_V39 current " << map.find("FEMB"+std::to_string(FEMB_NO)+"_FMV39_I")->second << " A" << " higher than expected";
	   throw excpt;
	}
    }
    
    if (map.find("FEMB"+std::to_string(FEMB_NO)+"_FMV30_I")->second < 0.050 ){
        if(Retry_Read(map, "FEMB"+std::to_string(FEMB_NO)+"_FMV30_I", 0.050, 'g', 9)){
	   cet::exception excpt(identification);
	   excpt << "FEMB " << FEMB_NO << " FM_V30 current " << map.find("FEMB"+std::to_string(FEMB_NO)+"_FMV30_I")->second << " A" << " lower than expected";
	   throw excpt;
	}
    }
    
    if (map.find("FEMB"+std::to_string(FEMB_NO)+"_FMV30_I")->second > 0.5 ){
        if(Retry_Read(map, "FEMB"+std::to_string(FEMB_NO)+"_FMV30_I", 0.5, 'l', 9)){
	   cet::exception excpt(identification);
	   excpt << "FEMB " << FEMB_NO << " FM_V30 current " << map.find("FEMB"+std::to_string(FEMB_NO)+"_FMV30_I")->second << " A" << " higher than expected";
	   throw excpt;
	}
    }
    
    if (map.find("FEMB"+std::to_string(FEMB_NO)+"_FMV18_I")->second < 0.200 ){
        if(Retry_Read(map, "FEMB"+std::to_string(FEMB_NO)+"_FMV18_I", 0.200, 'g', 9)){
	  cet::exception excpt(identification);
	  excpt << "FEMB " << FEMB_NO << " FM_V18 current " << map.find("FEMB"+std::to_string(FEMB_NO)+"_FMV18_I")->second << " A" << " lower than expected";
	  throw excpt;
	}
    }
    
    if (map.find("FEMB"+std::to_string(FEMB_NO)+"_FMV18_I")->second > 1.0 ){
        if(Retry_Read(map, "FEMB"+std::to_string(FEMB_NO)+"_FMV18_I", 1.0, 'l', 9)){
	   cet::exception excpt(identification);
	   excpt << "FEMB " << FEMB_NO << " FM_V18 current " << map.find("FEMB"+std::to_string(FEMB_NO)+"_FMV18_I")->second << " A" << " higher than expected";
	   throw excpt;
	}
    }
    
    if (map.find("FEMB"+std::to_string(FEMB_NO)+"_AMV33_I")->second < 0.100 ){
        if(Retry_Read(map, "FEMB"+std::to_string(FEMB_NO)+"_AMV33_I", 0.100, 'g', 9)){
	   cet::exception excpt(identification);
	   excpt << "FEMB " << FEMB_NO << " AM_V33 current " << map.find("FEMB"+std::to_string(FEMB_NO)+"_AMV33_I")->second << " A" << " lower than expected";
	   throw excpt;
	}
    }
    
    if (map.find("FEMB"+std::to_string(FEMB_NO)+"_AMV33_I")->second > 1.0 ){
        if(Retry_Read(map, "FEMB"+std::to_string(FEMB_NO)+"_AMV33_I", 1.0, 'l', 9)){
	   cet::exception excpt(identification);
	   excpt << "FEMB " << FEMB_NO << " AM_V33 current " << map.find("FEMB"+std::to_string(FEMB_NO)+"_AMV33_I")->second << " A" << " higher than expected";
	   throw excpt;
	}
    }
    
    if (map.find("FEMB"+std::to_string(FEMB_NO)+"_AMV28_I")->second < 0.100 ){
        if(Retry_Read(map, "FEMB"+std::to_string(FEMB_NO)+"_AMV28_I", 0.100, 'g', 9)){
	  cet::exception excpt(identification);
	  excpt << "FEMB " << FEMB_NO << " AM_V28 current " << map.find("FEMB"+std::to_string(FEMB_NO)+"_AMV28_I")->second << " A" << " lower than expected";
	  throw excpt;
	}
    }
    
    if (map.find("FEMB"+std::to_string(FEMB_NO)+"_AMV28_I")->second > 1.5 ){
        if(Retry_Read(map, "FEMB"+std::to_string(FEMB_NO)+"_AMV28_I", 1.5, 'l', 9)){
	   cet::exception excpt(identification);
	   excpt << "FEMB " << FEMB_NO << " AM_V28 current " << map.find("FEMB"+std::to_string(FEMB_NO)+"_AMV28_I")->second << " A" << " higher than expected";
	   throw excpt;
	}
    }
    
    wib->WriteFEMB(FEMB_NO, 0x0, 0x0);
    wib->ReadFEMB(FEMB_NO, 0x102);
    auto ver_value = wib->ReadFEMB(FEMB_NO, 0x101); // int earlier ver_value
    
    if (ver_value > 0){
       if ((ver_value&0xFFFF) != FEMB_V){
           cet::exception excpt(identification);
	   excpt << "FEMB " << FEMB_NO << " FE version is " << ver_value << " which is different from default " << FEMB_V;
	   throw excpt;
       }
    }
    
    else if (ver_value <= 0){
       cet::exception excpt(identification);
       excpt << "FEMB " << FEMB_NO << " 12C is broken";
       throw excpt;
    }
}

bool WIBReader::Retry_Read(std::map<std::string,double> &map, std::string var_name, double val, char check_type, int Tries){
     const std::string identification = "WIBReader::Retry_Read";
     
     bool throw_excpt = true;
    
     switch(check_type){
      case 'e':
         for (int i=0; i<Tries; i++){
            sleep(0.1);
	    map = wib->WIB_STATUS();
            if (map.find(var_name)->second == val){
	       throw_excpt = false;
	       break;
	    }
         }
	 break;
      case 'g':
         for (int i=0; i<Tries; i++){
	   sleep(0.1);
	   map = wib->WIB_STATUS();
	   if (map.find(var_name)->second >= val){
	      throw_excpt = false;
	      break;
	   }
	 }
	 break;
      case 'l':
         for (int i=0; i<Tries; i++){
	    sleep(0.1);
	    map = wib->WIB_STATUS();
	    if (map.find(var_name)->second <= val){
	       throw_excpt = false;
	       break;
	    }
	 }
	 break;
      default:
	cet::exception excpt(identification);
	excpt << "Unacceptable argument for check_type variable  is provided. Argument should be either e, g or l. ";
	throw excpt;
	break;
     } // switch statement 
     
     return throw_excpt;
}

void WIBReader::FEMB_DECTECT_V2(int FEMB_NO, uint32_t FEMB_V, int Tries){
  // This function is a modified version of one of the funcitons available in Shanshan's python script
  // to configure WIB/FEMB.
  // The original function is in cls_config.py module inside the repository CE_LD with same name (git branch name is, Installation_Support)
  const std::string identification = "WIBReader::FEMB_DECTECT_V2";
  
  TLOG_INFO(identification) << "************* Now Starting FEMB_DECTECT_V2  ****************" << TLOG_ENDL;
   
  std::map<std::string,double> map = wib->WIB_STATUS();
  for (int i=0; i<Tries; i++){
    
    if (i != 0){ 
       map = wib->WIB_STATUS();
       sleep(3);
    }
    
    
    if (map.find("FEMB"+std::to_string(FEMB_NO)+"_LINK")->second != 0xFF ){
	if (i < (Tries-1)){
	    TLOG_WARNING(identification) << "Try " << i+1 << " FEMB " << FEMB_NO << " LINK is broken. Trying again." << TLOG_ENDL;
	    continue;
	}
	else{ 
	   cet::exception excpt(identification);
	   excpt << "FEMB " << FEMB_NO << " LINK is broken";
	   throw excpt;
	}
    }
    
    if (map.find("FEMB"+std::to_string(FEMB_NO)+"_EQ")->second != 0xF ){
        if (i < (Tries-1)){ 
	    TLOG_WARNING(identification) << "Try " << i+1 << " FEMB " << FEMB_NO << " EQ is broken. Trying again." << TLOG_ENDL;
	    continue;
	}
	else{
	  cet::exception excpt(identification);
	  excpt << "FEMB " << FEMB_NO << " EQ is broken";
	  throw excpt;
	}
    }
    
    if (map.find("FEMB"+std::to_string(FEMB_NO)+"_BIAS_I")->second < 0.001 ){
        if (i < (Tries-1)){
	    TLOG_WARNING(identification) << "Try " << i+1 << " FEMB " << FEMB_NO << " BIAS current " << map.find("FEMB"+std::to_string(FEMB_NO)+"_BIAS_I")->second << " A" << " is lower than expected. Trying again." << TLOG_ENDL;
	    continue;
	}
	else{
	 cet::exception excpt(identification);
	 excpt << "FEMB " << FEMB_NO << " BIAS current " << map.find("FEMB"+std::to_string(FEMB_NO)+"_BIAS_I")->second << " A" << " is lower than expected";
	 throw excpt;
	}
    }
    
    if (map.find("FEMB"+std::to_string(FEMB_NO)+"_BIAS_I")->second > 0.1 ){
        if (i < (Tries-1)){ 
	    TLOG_WARNING(identification) << "Try " << i+1 << " FEMB " << FEMB_NO << " BIAS current " << map.find("FEMB"+std::to_string(FEMB_NO)+"_BIAS_I")->second << " A" << " is higher than expected. Trying again." << TLOG_ENDL;
	    continue;
	}
	else{
	  cet::exception excpt(identification);
	  excpt << "FEMB " << FEMB_NO << " BIAS current " << map.find("FEMB"+std::to_string(FEMB_NO)+"_BIAS_I")->second << " A" << " is higher than expected";
	  throw excpt;
	}
    }
    
    if (map.find("FEMB"+std::to_string(FEMB_NO)+"_FMV39_I")->second < 0.010 ){
        if (i < (Tries-1)){ 
	   TLOG_WARNING(identification) << "Try " << i+1 << " FEMB " << FEMB_NO << " FM_V39 current " << map.find("FEMB"+std::to_string(FEMB_NO)+"_FMV39_I")->second << " A" << " lower than expected. Trying again." << TLOG_ENDL;
	   continue;
	}
	else{
	   cet::exception excpt(identification);
	   excpt << "FEMB " << FEMB_NO << " FM_V39 current " << map.find("FEMB"+std::to_string(FEMB_NO)+"_FMV39_I")->second << " A" << " lower than expected";
	   throw excpt;
	}
    }
    
    if (map.find("FEMB"+std::to_string(FEMB_NO)+"_FMV39_I")->second > 0.2 ){
        if (i < (Tries-1)){ 
	    TLOG_WARNING(identification) << "Try " << i+1 << " FEMB " << FEMB_NO << " FM_V39 current " << map.find("FEMB"+std::to_string(FEMB_NO)+"_FMV39_I")->second << " A" << " higher than expected. Trying again." << TLOG_ENDL;
	    continue;
	}
	else{
	   cet::exception excpt(identification);
	   excpt << "FEMB " << FEMB_NO << " FM_V39 current " << map.find("FEMB"+std::to_string(FEMB_NO)+"_FMV39_I")->second << " A" << " higher than expected";
	   throw excpt;
	}
    }
    
    if (map.find("FEMB"+std::to_string(FEMB_NO)+"_FMV30_I")->second < 0.050 ){
        if (i < (Tries-1)){ 
	    TLOG_WARNING(identification) << "Try " << i+1 << " FEMB " << FEMB_NO << " FM_V30 current " << map.find("FEMB"+std::to_string(FEMB_NO)+"_FMV30_I")->second << " A" << " lower than expected. Trying again." << TLOG_ENDL;
	    continue;
	}
	else{
	   cet::exception excpt(identification);
	   excpt << "FEMB " << FEMB_NO << " FM_V30 current " << map.find("FEMB"+std::to_string(FEMB_NO)+"_FMV30_I")->second << " A" << " lower than expected";
	   throw excpt;
	}
    }
    
    if (map.find("FEMB"+std::to_string(FEMB_NO)+"_FMV30_I")->second > 0.5 ){
        if (i < (Tries-1)){ 
	    TLOG_WARNING(identification) << "Try " << i+1 << " FEMB " << FEMB_NO << " FM_V30 current " << map.find("FEMB"+std::to_string(FEMB_NO)+"_FMV30_I")->second << " A" << " higher than expected. Trying again." << TLOG_ENDL;
	    continue;
	}
	else{
	   cet::exception excpt(identification);
	   excpt << "FEMB " << FEMB_NO << " FM_V30 current " << map.find("FEMB"+std::to_string(FEMB_NO)+"_FMV30_I")->second << " A" << " higher than expected";
	   throw excpt;
	}
    }
    
    if (map.find("FEMB"+std::to_string(FEMB_NO)+"_FMV18_I")->second < 0.200 ){
        if (i < (Tries-1)){ 
	    TLOG_WARNING(identification) << "Try " << i+1 << " FEMB " << FEMB_NO << " FM_V18 current " << map.find("FEMB"+std::to_string(FEMB_NO)+"_FMV18_I")->second << " A" << " lower than expected. Trying again." << TLOG_ENDL;
	    continue;
	}
	else{
	  cet::exception excpt(identification);
	  excpt << "FEMB " << FEMB_NO << " FM_V18 current " << map.find("FEMB"+std::to_string(FEMB_NO)+"_FMV18_I")->second << " A" << " lower than expected";
	  throw excpt;
	}
    }
    
    if (map.find("FEMB"+std::to_string(FEMB_NO)+"_FMV18_I")->second > 1.0 ){
        if (i < (Tries-1)){ 
	   TLOG_WARNING(identification) << "Try " << i+1 << " FEMB " << FEMB_NO << " FM_V18 current " << map.find("FEMB"+std::to_string(FEMB_NO)+"_FMV18_I")->second << " A" << " higher than expected. Trying again." << TLOG_ENDL;
	   continue;
	}
	else{
	   cet::exception excpt(identification);
	   excpt << "FEMB " << FEMB_NO << " FM_V18 current " << map.find("FEMB"+std::to_string(FEMB_NO)+"_FMV18_I")->second << " A" << " higher than expected";
	   throw excpt;
	}
    }
    
    if (map.find("FEMB"+std::to_string(FEMB_NO)+"_AMV33_I")->second < 0.100 ){
        if (i < (Tries-1)){ 
	   TLOG_WARNING(identification) << "Try " << i+1 << " FEMB " << FEMB_NO << " AM_V33 current " << map.find("FEMB"+std::to_string(FEMB_NO)+"_AMV33_I")->second << " A" << " lower than expected. Trying again." << TLOG_ENDL;
	   continue;
	}
	else{
	   cet::exception excpt(identification);
	   excpt << "FEMB " << FEMB_NO << " AM_V33 current " << map.find("FEMB"+std::to_string(FEMB_NO)+"_AMV33_I")->second << " A" << " lower than expected";
	   throw excpt;
	}
    }
    
    if (map.find("FEMB"+std::to_string(FEMB_NO)+"_AMV33_I")->second > 1.0 ){
        if (i < (Tries-1)){ 
	    TLOG_WARNING(identification) << "Try " << i+1 << " FEMB " << FEMB_NO << " AM_V33 current " << map.find("FEMB"+std::to_string(FEMB_NO)+"_AMV33_I")->second << " A" << " higher than expected. Trying again." << TLOG_ENDL;
	    continue;
	}
	else{
	   cet::exception excpt(identification);
	   excpt << "FEMB " << FEMB_NO << " AM_V33 current " << map.find("FEMB"+std::to_string(FEMB_NO)+"_AMV33_I")->second << " A" << " higher than expected";
	   throw excpt;
	}
    }
    
    if (map.find("FEMB"+std::to_string(FEMB_NO)+"_AMV28_I")->second < 0.100 ){
        if (i < (Tries-1)){ 
	   TLOG_WARNING(identification) << "Try " << i+1 << " FEMB " << FEMB_NO << " AM_V28 current " << map.find("FEMB"+std::to_string(FEMB_NO)+"_AMV28_I")->second << " A" << " lower than expected. Trying again." << TLOG_ENDL;
	   continue;
	}
	else{
	  cet::exception excpt(identification);
	  excpt << "FEMB " << FEMB_NO << " AM_V28 current " << map.find("FEMB"+std::to_string(FEMB_NO)+"_AMV28_I")->second << " A" << " lower than expected";
	  throw excpt;
	}
    }
    
    if (map.find("FEMB"+std::to_string(FEMB_NO)+"_AMV28_I")->second > 1.5 ){
        if (i < (Tries-1)){ 
	   TLOG_WARNING(identification) << "Try " << i+1 << " FEMB " << FEMB_NO << " AM_V28 current " << map.find("FEMB"+std::to_string(FEMB_NO)+"_AMV28_I")->second << " A" << " higher than expected. Trying again." << TLOG_ENDL;
	   continue;
	}
	else{
	   cet::exception excpt(identification);
	   excpt << "FEMB " << FEMB_NO << " AM_V28 current " << map.find("FEMB"+std::to_string(FEMB_NO)+"_AMV28_I")->second << " A" << " higher than expected";
	   throw excpt;
	}
    }
    
    wib->WriteFEMB(FEMB_NO, 0x0, 0x0);
    wib->ReadFEMB(FEMB_NO, 0x102);
    auto ver_value = wib->ReadFEMB(FEMB_NO, 0x101);
    
    if (ver_value > 0){
        if ((ver_value&0xFFFF) != FEMB_V){
	   if (i < (Tries-1)){
	     TLOG_WARNING(identification) << "Try " << i+1 << " FEMB " << FEMB_NO << " FE version is " << ver_value << " which is different from default " << FEMB_V << TLOG_ENDL;
	     continue;
	   }
	   
	   else{
	     cet::exception excpt(identification);
	     excpt << "FEMB " << FEMB_NO << " FE version is " << ver_value << " which is different from default " << FEMB_V;
	     throw excpt;
	   }
	}  
    }
    
    else if (ver_value <= 0){
      if (i < (Tries-1)){
         TLOG_WARNING(identification) << "Try " << i+1 << " FEMB " << FEMB_NO << " 12C is broken" << TLOG_ENDL;
	 continue;
      }
      else{
        cet::exception excpt(identification);
	excpt << "FEMB " << FEMB_NO << " 12C is broken";
	throw excpt;
      }
    }
    
    break;
  } // for loop
  
  TLOG_INFO(identification) << "************* FEMB_DECTECT_V2 completed ****************" << TLOG_ENDL;
}

void WIBReader::FEMB_DETECT_ALL(std::vector<bool> &FEMB_NOs, uint32_t FEMB_V, int Tries){
  // This function is a modified version of one of the funcitons available in Shanshan's python script
  // to configure WIB/FEMB.
  // The original function is in cls_config.py module inside the repository CE_LD with the name FEMB_DETECT (git branch name is, Installation_Support)
  const std::string identification = "WIBReader::FEMB_DETECT_ALL"; 
  std::map<std::string,double> map = wib->WIB_STATUS();
  
  bool restart_main_loop = false;
  
  for (int i=0; i<Tries; i++){
      
      if (i != 0){ 
        restart_main_loop = false;
	map = wib->WIB_STATUS();
        sleep(3);
      }
  
     for (unsigned int j=0; j<FEMB_NOs.size(); j++){
         if (FEMB_NOs[j]){
	    if (map.find("FEMB"+std::to_string(j+1)+"_LINK")->second != 0xFF ){
	       if (i < (Tries-1)){
	          TLOG_WARNING(identification) << "Try " << i+1 << " FEMB " << j+1 << " LINK is broken. Trying again." << TLOG_ENDL;
		  restart_main_loop = true;
	          break;
	       }
	       else{ 
	         cet::exception excpt(identification);
	         excpt << "FEMB " << j+1 << " LINK is broken";
	         throw excpt;
	       }
            }
	 
	    if (map.find("FEMB"+std::to_string(j+1)+"_EQ")->second != 0xF ){
                if (i < (Tries-1)){ 
	            TLOG_WARNING(identification) << "Try " << i+1 << " FEMB " << j+1 << " EQ is broken. Trying again." << TLOG_ENDL;
	            restart_main_loop = true;
		    break;
	        }
	        else{
	          cet::exception excpt(identification);
	          excpt << "FEMB " << j+1 << " EQ is broken";
	          throw excpt;
	        }
            }
    
            if (map.find("FEMB"+std::to_string(j+1)+"_BIAS_I")->second < 0.001 ){
                if (i < (Tries-1)){
	            TLOG_WARNING(identification) << "Try " << i+1 << " FEMB " << j+1 << " BIAS current " << map.find("FEMB"+std::to_string(j+1)+"_BIAS_I")->second << " A" << " is lower than expected. Trying again." << TLOG_ENDL;
	            restart_main_loop = true;
		    break;
	        }
	        else{
	          cet::exception excpt(identification);
	          excpt << "FEMB " << j+1 << " BIAS current " << map.find("FEMB"+std::to_string(j+1)+"_BIAS_I")->second << " A" << " is lower than expected";
	          throw excpt;
	        }
             }
    
             if (map.find("FEMB"+std::to_string(j+1)+"_BIAS_I")->second > 0.1 ){
                 if (i < (Tries-1)){ 
	             TLOG_WARNING(identification) << "Try " << i+1 << " FEMB " << j+1 << " BIAS current " << map.find("FEMB"+std::to_string(j+1)+"_BIAS_I")->second << " A" << " is higher than expected. Trying again." << TLOG_ENDL;
	             restart_main_loop = true;
		     break;
	         }
	         else{
	          cet::exception excpt(identification);
	          excpt << "FEMB " << j+1 << " BIAS current " << map.find("FEMB"+std::to_string(j+1)+"_BIAS_I")->second << " A" << " is higher than expected";
	          throw excpt;
	        }
             }
    
             if (map.find("FEMB"+std::to_string(j+1)+"_FMV39_I")->second < 0.010 ){
                 if (i < (Tries-1)){ 
	             TLOG_WARNING(identification) << "Try " << i+1 << " FEMB " << j+1 << " FM_V39 current " << map.find("FEMB"+std::to_string(j+1)+"_FMV39_I")->second << " A" << " lower than expected. Trying again." << TLOG_ENDL;
	             restart_main_loop = true;
		     break;
	         }
	         else{
	           cet::exception excpt(identification);
	           excpt << "FEMB " << j+1 << " FM_V39 current " << map.find("FEMB"+std::to_string(j+1)+"_FMV39_I")->second << " A" << " lower than expected";
	           throw excpt;
	         }
             }
    
             if (map.find("FEMB"+std::to_string(j+1)+"_FMV39_I")->second > 0.2 ){
                 if (i < (Tries-1)){ 
	             TLOG_WARNING(identification) << "Try " << i+1 << " FEMB " << j+1 << " FM_V39 current " << map.find("FEMB"+std::to_string(j+1)+"_FMV39_I")->second << " A" << " higher than expected. Trying again." << TLOG_ENDL;
	             restart_main_loop = true;
	             break;
	         }
	         else{
	            cet::exception excpt(identification);
	            excpt << "FEMB " << j+1 << " FM_V39 current " << map.find("FEMB"+std::to_string(j+1)+"_FMV39_I")->second << " A" << " higher than expected";
	            throw excpt;
	         }
              }
    
              if (map.find("FEMB"+std::to_string(j+1)+"_FMV30_I")->second < 0.050 ){
                  if (i < (Tries-1)){ 
	              TLOG_WARNING(identification) << "Try " << i+1 << " FEMB " << j+1 << " FM_V30 current " << map.find("FEMB"+std::to_string(j+1)+"_FMV30_I")->second << " A" << " lower than expected. Trying again." << TLOG_ENDL;
	              restart_main_loop = true;
	              break;
	          }
	          else{
	            cet::exception excpt(identification);
	            excpt << "FEMB " << j+1 << " FM_V30 current " << map.find("FEMB"+std::to_string(j+1)+"_FMV30_I")->second << " A" << " lower than expected";
	            throw excpt;
	          }
               }
    
               if (map.find("FEMB"+std::to_string(j+1)+"_FMV30_I")->second > 0.5 ){
                   if (i < (Tries-1)){ 
	               TLOG_WARNING(identification) << "Try " << i+1 << " FEMB " << j+1 << " FM_V30 current " << map.find("FEMB"+std::to_string(j+1)+"_FMV30_I")->second << " A" << " higher than expected. Trying again." << TLOG_ENDL;
	               restart_main_loop = true;
	               break;
	           }
	           else{
	             cet::exception excpt(identification);
	             excpt << "FEMB " << j+1 << " FM_V30 current " << map.find("FEMB"+std::to_string(j+1)+"_FMV30_I")->second << " A" << " higher than expected";
	             throw excpt;
	           }
                }
    
                if (map.find("FEMB"+std::to_string(j+1)+"_FMV18_I")->second < 0.200 ){
                    if (i < (Tries-1)){ 
	                TLOG_WARNING(identification) << "Try " << i+1 << " FEMB " << j+1 << " FM_V18 current " << map.find("FEMB"+std::to_string(j+1)+"_FMV18_I")->second << " A" << " lower than expected. Trying again." << TLOG_ENDL;
	                restart_main_loop = true;
	                break;
	            }
	            else{
	              cet::exception excpt(identification);
	              excpt << "FEMB " << j+1 << " FM_V18 current " << map.find("FEMB"+std::to_string(j+1)+"_FMV18_I")->second << " A" << " lower than expected";
	              throw excpt;
	            }
                 }
    
                 if (map.find("FEMB"+std::to_string(j+1)+"_FMV18_I")->second > 1.0 ){
                     if (i < (Tries-1)){ 
	                 TLOG_WARNING(identification) << "Try " << i+1 << " FEMB " << j+1 << " FM_V18 current " << map.find("FEMB"+std::to_string(j+1)+"_FMV18_I")->second << " A" << " higher than expected. Trying again." << TLOG_ENDL;
	                 restart_main_loop = true;
	                 break;
	              }
	              else{
	                 cet::exception excpt(identification);
	                 excpt << "FEMB " << j+1 << " FM_V18 current " << map.find("FEMB"+std::to_string(j+1)+"_FMV18_I")->second << " A" << " higher than expected";
	                 throw excpt;
	              }
                  }
    
                  if (map.find("FEMB"+std::to_string(j+1)+"_AMV33_I")->second < 0.100 ){
                      if (i < (Tries-1)){ 
	                  TLOG_WARNING(identification) << "Try " << i+1 << " FEMB " << j+1 << " AM_V33 current " << map.find("FEMB"+std::to_string(j+1)+"_AMV33_I")->second << " A" << " lower than expected. Trying again." << TLOG_ENDL;
	                  restart_main_loop = true;
	                  break;
	               }
	               else{
	                 cet::exception excpt(identification);
	                 excpt << "FEMB " << j+1 << " AM_V33 current " << map.find("FEMB"+std::to_string(j+1)+"_AMV33_I")->second << " A" << " lower than expected";
	                 throw excpt;
	               }
                    }
    
                    if (map.find("FEMB"+std::to_string(j+1)+"_AMV33_I")->second > 1.0 ){
                        if (i < (Tries-1)){ 
	                    TLOG_WARNING(identification) << "Try " << i+1 << " FEMB " << j+1 << " AM_V33 current " << map.find("FEMB"+std::to_string(j+1)+"_AMV33_I")->second << " A" << " higher than expected. Trying again." << TLOG_ENDL;
	                    restart_main_loop = true;
	                    break;
	                }
	                else{
	                  cet::exception excpt(identification);
	                  excpt << "FEMB " << j+1 << " AM_V33 current " << map.find("FEMB"+std::to_string(j+1)+"_AMV33_I")->second << " A" << " higher than expected";
	                  throw excpt;
	                }
                     }
    
                     if (map.find("FEMB"+std::to_string(j+1)+"_AMV28_I")->second < 0.100 ){
                         if (i < (Tries-1)){ 
	                     TLOG_WARNING(identification) << "Try " << i+1 << " FEMB " << j+1 << " AM_V28 current " << map.find("FEMB"+std::to_string(j+1)+"_AMV28_I")->second << " A" << " lower than expected. Trying again." << TLOG_ENDL;
	                     restart_main_loop = true;
	                     break;
	                 }
	                 else{
	                    cet::exception excpt(identification);
	                    excpt << "FEMB " << j+1 << " AM_V28 current " << map.find("FEMB"+std::to_string(j+1)+"_AMV28_I")->second << " A" << " lower than expected";
	                    throw excpt;
	                 }
                      }
    
                      if (map.find("FEMB"+std::to_string(j+1)+"_AMV28_I")->second > 1.5 ){
                          if (i < (Tries-1)){ 
	                      TLOG_WARNING(identification) << "Try " << i+1 << " FEMB " << j+1 << " AM_V28 current " << map.find("FEMB"+std::to_string(j+1)+"_AMV28_I")->second << " A" << " higher than expected. Trying again." << TLOG_ENDL;
	                      restart_main_loop = true;
	                      break;
	                  }
	                  else{
	                    cet::exception excpt(identification);
	                    excpt << "FEMB " << j+1 << " AM_V28 current " << map.find("FEMB"+std::to_string(j+1)+"_AMV28_I")->second << " A" << " higher than expected";
	                    throw excpt;
	                 }
                      }
		      
		      wib->WriteFEMB(j+1, 0x0, 0x0);
                      wib->ReadFEMB(j+1, 0x102);
                      auto ver_value = wib->ReadFEMB(j+1, 0x101);
		      
		      if (ver_value > 0){
                          if ((ver_value&0xFFFF) != FEMB_V){
	                       if (i < (Tries-1)){
	                           TLOG_WARNING(identification) << "Try " << i+1 << " FEMB " << j+1 << " FE version is " << ver_value << " which is  different from default " << FEMB_V << TLOG_ENDL;
	                           restart_main_loop = true;
				   break;
	                       }
	                       else{
	                          cet::exception excpt(identification);
	                          excpt << "FEMB " << j+1 << " FE version is " << ver_value << " which is different from default " << FEMB_V;
	                          throw excpt;
	                      }
	                   }  
                       }
		       
		       else if (ver_value <= 0){
                                if (i < (Tries-1)){
                                    TLOG_WARNING(identification) << "Try " << i+1 << " FEMB " << j+1 << " 12C is broken" << TLOG_ENDL;
	                            restart_main_loop = true;
				    break;
                                }
                                else{
                                  cet::exception excpt(identification);
	                          excpt << "FEMB " << j+1 << " 12C is broken";
	                          throw excpt;
                                }
                        }
		    } // if femb [j] == true
               } // loop over femb vector
	       
	       if (restart_main_loop)  continue;
               break;
         } // loop over number of tries
}

void WIBReader::FEMB_SCAN(std::vector<bool> &FEMB_NOs, uint32_t FEMB_V){
   // This function is a modified version of one of the funcitons available in Shanshan's python script
  // to configure WIB/FEMB.
  // The original function is in cls_config.py module inside the repository CE_LD with the name FEMB_SCAN (git branch name is, Installation_Support)
  const std::string identification = "WIBReader::FEMB_SCAN"; 
  TLOG_INFO(identification) << "************* Now Starting FEMB_SCAN  ****************" << TLOG_ENDL;
  wib->WIB_PWR_FEMB(FEMB_NOs);
  FEMB_DETECT_ALL(FEMB_NOs,FEMB_V,10);
  wib->WIB_PWR_FEMB(FEMB_NOs,false,{0,0,0,0});
  TLOG_INFO(identification) << "************* FEMB_SCAN completed  ****************" << TLOG_ENDL;
}

void WIBReader::setupFEMB(size_t iFEMB, fhicl::ParameterSet const& FEMB_configure)
{
  const std::string identification = "WIBReader::setupFEMB";
  
  TLOG_INFO(identification) << "************* Now Starting setupFEMB  ****************" << TLOG_ENDL;
  
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
  const auto power_off_femb       = FEMB_configure.get<bool>("turn_off_femb");
  const auto BNL_enable_test_cap   = FEMB_configure.get<uint32_t>("BNL_enable_test_cap");
  const auto BNLbaselineHigh       = FEMB_configure.get<uint32_t>("BNLbaselineHigh");
  const auto BNLgain               = FEMB_configure.get<std::vector<uint32_t> >("BNLgain");
  const auto BNLshape              = FEMB_configure.get<std::vector<uint32_t> >("BNLshape");
  const auto BNL_enable_output_mon = FEMB_configure.get<uint32_t>("BNL_enable_output_mon");
  const auto BNL_buffter_ctrl      = FEMB_configure.get<uint32_t>("BNL_buffter_ctrl");
  const auto BNL_output_coupl      = FEMB_configure.get<uint32_t>("BNL_output_coupl");
  const auto BNL_enable_high_filt  = FEMB_configure.get<uint32_t>("BNL_enable_high_filt");
  const auto BNL_mon_STB   = FEMB_configure.get<uint32_t>("BNL_mon_STB");
  const auto BNL_mon_STB1  = FEMB_configure.get<uint32_t>("BNL_mon_STB1");
  const auto BNL_sdacsw1   = FEMB_configure.get<uint32_t>("BNL_sdacsw1");
  const auto BNL_sdacsw2   = FEMB_configure.get<uint32_t>("BNL_sdacsw2");
  const auto FEMB_channel_map = FEMB_configure.get<uint8_t>("FEMB_channel_map");
  const auto use_old_femb_config = FEMB_configure.get<bool>("use_old_femb_config");
  const auto chnl_mapping_mode = FEMB_configure.get<bool>("chnl_mapping_mode");
  const auto test_chnl_number  = FEMB_configure.get<uint32_t>("test_chnl_number");
  
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
  
  if(signed(pls_dac_val)<0 || signed(pls_dac_val)>63){
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
  
  if (power_off_femb){
      wib->WIB_PWR_FEMB(iFEMB,false,0);
      sleep(2);
  }
  
  wib->WIB_PWR_FEMB(iFEMB);
  
  TLOG_INFO(identification) << " After powering up the FEMBs" << TLOG_ENDL;
  
  auto expected_femb_fw_version = FEMB_configure.get<uint32_t>("expected_femb_fw_version");
  
  if(use_old_femb_config){
     InitFEMBRegCheck(expected_femb_fw_version, "VERSION_ID", iFEMB, 30);
  
     FEMBHsLinkCheck(iFEMB, 30);
  }
  
  if(!use_old_femb_config) FEMB_DECTECT_V2(iFEMB,expected_femb_fw_version,3);
  
  std::vector<uint32_t> fe_config = {gain,shape,baselineHigh,leakHigh,leak10X,acCouple,buffer,extClk};
  
  //setupNoiseMinConfig(iFEMB, 30); // Uncomment this line to run Shanshan's minimum configuration to see FEMB noise.
  
  //wib->ConfigFEMB(iFEMB, fe_config, clk_phases, pls_mode, pls_dac_val, start_frame_mode_sel, start_frame_swap); // This is the old femb configuration which was found to be very problematic
  
  if(use_old_femb_config) wib->New_ConfigFEMB(iFEMB, FEMB_channel_map, fe_config, clk_phases, pls_mode, pls_dac_val, start_frame_mode_sel, start_frame_swap); // This is     the fix to the old femb configuration which sloved some of the issues.
  
  // Following is the latest FEMB configuration as done in BNL python scripts 
   
  
  //******************************************************************************************************************************  
  //*                                      Definiton of arguments to CE_CHK_CFG function                                         *
  //****************************************************************************************************************************** 
  //* Argument No.  *    Argument Name    *        Description                                                                   *
  //******************************************************************************************************************************
  //*    1          *       iFEMB         *      FEMB Number (1-4)                                                               *
  //*    2          *     configuration   *      0 - config. 1, 1 - config. 2, any other number = config. 3                      *
  //*    3          *   use_chnl_map_test *      going test channel mapping by setting a pulse in a given channel                *
  //*    4          *       chnl_no       *      Channel number for channel mapping test (0-127)                                 *        
  //*    5          *       pls_cs        *      set to 0 always                                                                 *
  //*    6          *       dac_sel       *      set to 1 always                                                                 *
  //*    7          *       fpgadac_en    *      set to 1 when using FPGA pulsar                                                 *
  //*    8          *       asicdac_en    *      set to 1 when using internal ASIC pulsar                                        *
  //*    9          *       fpgadac_v     *      DAC setting used when using FPGA pulsar                                         * 
  //*    10         *       pls_gap       *      Shanshan keeps it always 500 in his scirpts                                     *
  //*    11         *       pls_dly       *      Shanshan keeps it 10 in his scirpts                                             * 
  //*    12         *       mon_cs        *      Shanshan keeps it 0 in his scirpts                                              *
  //*    13         *       data_cs       *      Shanshan keeps it 0 in his scirpts                                              *
  //*    14         *       sts           *      Enable test capacitor (this should be 1 when using either ASIC or FPGA pulsar)  * 
  //*    15         *       snc           *      Sets the baseline value                                                         *
  //*    16         *       sg0           *      First argument in setting the gain                                              *
  //*    17         *       sg1           *      Second argument in setting the gain                                             *
  //*    18         *       st0           *      First argument in setting the shaping time                                      *
  //*    19         *       st1           *      second argument in setting the shaping time                                     *
  //*    20         *       smn           *      Enable output monitor                                                           *
  //*    21         *       sdf           *      Buffer control                                                                  *
  //*    22         *       slk0          *      Set to 0 in shanshan scripts                                                    *
  //*    23         *       stb1          *      Set monitoring (always set to 0)                                                *
  //*    24         *       stb           *      Set monitoring (always set to 0)                                                *
  //*    25         *       s16           *      Enable high filter                                                              *
  //*    26         *       slk1          *      Set to 0 always in shanshan scripts                                             *
  //*    27         *       sdc           *      Output coupling                                                                 *
  //*    28         *       swdac1        *      Set to 1 when FPGA pulsar is being used (otherwise 0)                           *
  //*    29         *       swdac2        *      Set to 1 when ASIC pulsar is being used (otherwise 0)                           *
  //*    30         *       dac           *      DAC setting when using ASIC pulsar                                              *
  //*    31         *       fecfg_loadflg *      Always set to 0 in shanshan scripts                                             * 
  //******************************************************************************************************************************   
  
  if(!use_old_femb_config){
  
     if (chnl_mapping_mode){
         if (signed(test_chnl_number)<0 || signed(test_chnl_number)>127){
	     cet::exception excpt(identification);
	     excpt << "In FEMB " << iFEMB << " Channel number " << test_chnl_number << " Provided for channel map test is not acceptable";
	     throw excpt;
	 }
	 
	 else if(pls_mode == 0){
	     cet::exception excpt(identification);
	     excpt << "In FEMB " << iFEMB << " Pulse mode " << pls_mode << " is unacceptable for doing channel map tests";
	     throw excpt;
	 }
     }
  
     if (pls_mode == 0){
         wib->CE_CHK_CFG(iFEMB, FEMB_channel_map, false, 0, 1, 1, 0, 0, 0, 500, 10, 0, 0, 0, BNLbaselineHigh, BNLgain[0], BNLgain[1], BNLshape[0],
                         BNLshape[1], BNL_enable_output_mon, BNL_buffter_ctrl, 0, BNL_mon_STB1, BNL_mon_STB, BNL_enable_high_filt,
		         0, BNL_output_coupl, 0, 0, 0, false);
     }
  
     else if (pls_mode == 1){
         if (chnl_mapping_mode){
             wib->CE_CHK_CFG(iFEMB, FEMB_channel_map, chnl_mapping_mode, test_chnl_number, 1, 1, 0, 1, 0, 500, 10, 0, 0, 1, BNLbaselineHigh, BNLgain[0],                              BNLgain[1], BNLshape[0], BNLshape[1], BNL_enable_output_mon, BNL_buffter_ctrl, 0, BNL_mon_STB1, BNL_mon_STB,                                             BNL_enable_high_filt,0, BNL_output_coupl, 0, 1, pls_dac_val, false);
	 }
	 
	 else{
	    wib->CE_CHK_CFG(iFEMB, FEMB_channel_map, false, 0, 1, 1, 0, 1, 0, 500, 10, 0, 0, 1, BNLbaselineHigh, BNLgain[0], BNLgain[1], BNLshape[0],
                             BNLshape[1], BNL_enable_output_mon, BNL_buffter_ctrl, 0, BNL_mon_STB1, BNL_mon_STB, BNL_enable_high_filt,
		             0, BNL_output_coupl, 0, 1, pls_dac_val, false);
	 }
     }
  
     else if (pls_mode == 2){
         if (chnl_mapping_mode){
	     wib->CE_CHK_CFG(iFEMB, FEMB_channel_map, chnl_mapping_mode, test_chnl_number, 1, 1, 1, 0, pls_dac_val, 500, 10, 0, 0, 1, BNLbaselineHigh,                                BNLgain[0], BNLgain[1],BNLshape[0],BNLshape[1], BNL_enable_output_mon, BNL_buffter_ctrl, 0, BNL_mon_STB1, BNL_mon_STB,                                   BNL_enable_high_filt,0, BNL_output_coupl, 1, 0, 0, false);
	 }
	 
	 else{
	    wib->CE_CHK_CFG(iFEMB, FEMB_channel_map, false, 0, 1, 1, 1, 0, pls_dac_val, 500, 10, 0, 0, 1, BNLbaselineHigh,                                                           BNLgain[0], BNLgain[1],BNLshape[0],BNLshape[1], BNL_enable_output_mon, BNL_buffter_ctrl, 0, BNL_mon_STB1, BNL_mon_STB,                                   BNL_enable_high_filt,0, BNL_output_coupl, 1, 0, 0, false);
	 }
     }
  }
  
  TLOG_INFO(identification) << "************* setupFEMB completed ****************" << TLOG_ENDL;
}

// "shutdown" transition
WIBReader::~WIBReader() 
{
  disconnectWIB_releaseSemaphores();
}

// "start" transition
void WIBReader::start() 
{
  const std::string identification = "WIBReader::start";
  TLOG_INFO(identification) << "=================== Start function started ==========================" << TLOG_ENDL;
  time_t start, end;
  time(&start);
  if (!wib && calibration_mode) 
  {
    cet::exception excpt(identification);
    excpt << "WIB object pointer NULL";
    throw excpt;
  }
  time(&end);
  double time_taken = double(end - start);
  TLOG_INFO(identification) << "============== Time took to run START function : " << time_taken << std::setprecision(5) << TLOG_ENDL;  
  //wib->GetAddress() << " =================" << TLOG_ENDL;
  TLOG_INFO(identification) << "=================== Start function completed ==========================" << TLOG_ENDL;
}

// "stop" transition
void WIBReader::stop() 
{
  const std::string identification = "WIBReader::stop";
  TLOG_INFO(identification) << "=================== Stop function started ==========================" << TLOG_ENDL;
  time_t start, end;
  time(&start);
  if (!wib && calibration_mode ) 
  {
    cet::exception excpt(identification);
    excpt << "WIB object pointer NULL";
    throw excpt;
  }
  time(&end);
  double time_taken = double(end - start);
  TLOG_INFO(identification) << "=============== Time took to run STOP function : " << time_taken << std::setprecision(5) << TLOG_ENDL;
  //wib->GetAddress() << " ==================" << TLOG_ENDL;
  TLOG_INFO(identification) << "=================== Stop function completed ==========================" << TLOG_ENDL;
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
  sem_wib_lck = sem_open(WIB::SEMNAME_WIBLCK, O_CREAT, 0666, 1);
  sem_wib_yld = sem_open(WIB::SEMNAME_WIBYLD, O_CREAT, 0666, 1);
  if (sem_wib_lck == SEM_FAILED || sem_wib_yld == SEM_FAILED) {
    TLOG(TLVL_ERROR) << "Failed to create either " << WIB::SEMNAME_WIBLCK << " or " << WIB::SEMNAME_WIBYLD <<".";
    goto release_semaphores;
  }

  struct timespec timeout;
  if ( -1 == clock_gettime(CLOCK_REALTIME, &timeout) ){
    TLOG(TLVL_ERROR) << "The call to the clock_gettime function has failed.";
    goto release_semaphores;
  }
  timeout.tv_nsec += 500000;
  if (sem_timedwait(sem_wib_yld, &timeout) != 0){
    TLOG(TLVL_ERROR) << "Failed to acquire " << WIB::SEMNAME_WIBYLD<< " semaphore.";
    goto release_semaphores;
  }

  if ( -1 == clock_gettime(CLOCK_REALTIME, &timeout) ){
    TLOG(TLVL_ERROR) << "The call to the clock_gettime function has failed.";
    goto release_semaphores;
  }
  timeout.tv_sec += semaphore_acquire_timeout_ms / 1000;
  timeout.tv_nsec += (semaphore_acquire_timeout_ms % 1000) * 1000000;
  if (sem_timedwait(sem_wib_lck, &timeout) != 0){
    TLOG(TLVL_ERROR) << "Failed to acquire " << WIB::SEMNAME_WIBLCK << " semaphore.";
    if (errno == ETIMEDOUT) {
      TLOG(TLVL_ERROR) << "The semaphore timed out. Consider increasing the semaphore_acquire_timeout_ms setting to resolve the issue.";
    } else if (errno == EINTR) {
      TLOG(TLVL_ERROR) << "The call was interrupted by a signal.";
    } else {
      TLOG(TLVL_ERROR) << "An unknown error occurred.";
    }
    goto release_semaphores;
  }
  TLOG(TLVL_INFO) << "Acquired semaphores.";
  return true;

release_semaphores:
    releaseSemaphores();
    return false;
}

bool WIBReader::acquireSemaphores_ThrowOnFailure(){
  if(!use_semaphores) return false;
  if (acquireSemaphores())
    return true;

  cet::exception excpt("WIBReader::acquireSemaphores_ThrowOnFailure");
  excpt << "The operation was unsuccessful. Please try the following steps to resolve the issue. Terminate any running instances of the FEMBreceiver (femb), WIBTool.exe, or WIB Boardreader processes. Then, delete the semaphores /dev/shm/sem.WIB_LCK and /dev/shm/sem.WIB_YLD. If you intend to run both FEMBreceiver and WIBTool.exe, start with FEMBreceiver first.";
  throw excpt;
}

void WIBReader::releaseSemaphores(){
  unsigned int sem_release_count=0;
  if (nullptr!=sem_wib_yld){
    sem_post(sem_wib_yld);
    sem_close(sem_wib_yld);
    sem_release_count++;
    sem_wib_yld=nullptr;
  }

  if(nullptr!=sem_wib_lck){
    sem_post(sem_wib_lck);
    sem_close(sem_wib_lck);
    sem_release_count++;
    sem_wib_lck=nullptr;
  }

  semaphores_acquired=false;

  if (sem_release_count > 0 ){
    TLOG(TLVL_INFO) << "Released " << sem_release_count << " semaphore(s).";
  }
}

void WIBReader::disconnectWIB_releaseSemaphores(){
  if(!use_semaphores) return;
  wib.reset();
  sleep(2);
  releaseSemaphores();
}

} // namespace

DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(sbndaq::WIBReader)
