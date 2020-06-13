#include "sbndaq-artdaq/Generators/Common/BernCRTZMQData.hh"
#include "artdaq/Generators/GeneratorMacros.hh"

#include <iomanip>
#include <iterator>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include "febdrv.hh"

#define id "BernCRT"
#define TRACE_NAME "BernCRTZMQData"

sbndaq::BernCRTZMQData::BernCRTZMQData(fhicl::ParameterSet const & ps)
  :
  BernCRTZMQ_GeneratorBase(ps) {
  TLOG(TLVL_INFO) << __func__ <<"() constructor called";

  febdrv.Init(ps_.get<std::string>("ethernet_port"));
  
  //compare detected list of FEBs with that declared in FHiCL file
  auto hardware_macs = febdrv.GetMACs();
  if(hardware_macs.size() != nFEBs()) {
    TLOG(TLVL_ERROR) <<  __func__ << "() Number of FEBs seen by febdrv (" << std::to_string(hardware_macs.size()) << ") differs from the ones defined in FCL file (" + std::to_string(nFEBs()) + ")!";
    throw cet::exception( std::string(TRACE_NAME) + __func__ + " Number of FEBs seen by febdrv (" + std::to_string(hardware_macs.size()) + ") differs from the ones defined in FCL file (" + std::to_string(nFEBs()) + ")!");
  }
  else {
    //here we assume both hardware_macs and MAC5s_ are sorted
    for(unsigned int i = 0; i < nFEBs(); i++) {
      if(hardware_macs[i] > MAC5s_[i]) {
        TLOG(TLVL_ERROR) <<  __func__ <<"() MAC address " << std::to_string(MAC5s_[i]) << " declared in FHiCL configuration not found! Check hardware MAC configuration!";
        throw cet::exception( std::string(TRACE_NAME) + __func__ + "() MAC address " +std::to_string(MAC5s_[i]) + " declared in FHiCL configuration not found. Check hardware MAC configuration!");
      }
      if(hardware_macs[i] < MAC5s_[i]) {
        TLOG(TLVL_ERROR) <<  __func__ <<"() found FEB with MAC address " << std::to_string(hardware_macs[i]) << " not declared in FHiCL configuration";
        throw cet::exception( std::string(TRACE_NAME) + __func__ + "() found FEB with MAC address " +std::to_string(hardware_macs[i]) + " not declared in FHiCL configuration!");
      }
    }
  }
  
  for(unsigned int iFEB = 0; iFEB < nFEBs(); iFEB++) {
    TLOG(TLVL_DEBUG) << __func__ << " Reading configuration for MAC5 #" << std::to_string(iFEB) << ": " << std::to_string(MAC5s_[iFEB]);
    feb_configuration[MAC5s_[iFEB]] = sbndaq::BernCRTFEBConfiguration(ps_, iFEB); //create configuration object
  }

  TLOG(TLVL_INFO) << __func__ << "() constructor completed";
} //constructor

sbndaq::BernCRTZMQData::~BernCRTZMQData() {
  TLOG(TLVL_INFO) << __func__ <<  "() called";

  Cleanup();
  TLOG(TLVL_INFO) << __func__ << "() completed";
} //destructor

void sbndaq::BernCRTZMQData::ConfigureStart() {
  TLOG(TLVL_INFO) << __func__ << "() called";
  
  //make sure the HV and DAQ are off before we start to send the configuration to the board
  febdrv.biasOFF();
  
  for(unsigned int iFEB = 0; iFEB < nFEBs(); iFEB++) {
    feb_send_bitstreams(MAC5s_[iFEB]); //send PROBE and SC configuration to FEB
    if(feb_configuration[MAC5s_[iFEB]].GetHVOnPermission()) febdrv.biasON(MAC5s_[iFEB]); //turn on SiPM HV (if FHiCL file allows it)
  }
  StartFebdrv(); //start data taking mode for all boards

  TLOG(TLVL_INFO) << __func__ << "() completed";
} //ConfigureStart

void sbndaq::BernCRTZMQData::ConfigureStop() {
  TLOG(TLVL_INFO) << __func__ << "() called";

  febdrv.biasOFF();

  TLOG(TLVL_INFO) << __func__ << "() completed";
} //ConfigureStop


void sbndaq::BernCRTZMQData::feb_send_bitstreams(uint8_t mac5) {
  /**
   * Sends configuration bitstream
   * Arguments:
   * mac5 of the board, note one cannot use 255 address to broadcast to all boards
   * Bitstreams should be read from (?)
   * - Probe_bitStream - monitoring configuration. It consists of 224 bits, typically all '0' for normal operation 
   * - SlowControl_bitStream - actual configuration to be sent to the board, 1144 bits
   */

  if(feb_configuration.find(mac5) == feb_configuration.end()) {
    TLOG(TLVL_ERROR) <<  __func__ << "() Could not find FEB " << mac5 << " in MAC5s!";
    throw cet::exception( std::string(TRACE_NAME) + __func__ + " Could not find FEB " + std::to_string(mac5) + " in MAC5s!");
  }

  if(mac5==255) {
    TLOG(TLVL_ERROR) <<  __func__ << "() Bitstreams cannot be sent to mac5 = 255!";
    return;
  }
  
  bool status = febdrv.sendconfig(
    mac5,
    feb_configuration[mac5].GetSlowControlBitStream(),
    feb_configuration[mac5].GetSlowControlBitStreamNBytes(),
    feb_configuration[mac5].GetProbeBitStream(),
    feb_configuration[mac5].GetProbeBitStreamNBytes()
  );
  
  if(!status)
    throw cet::exception( std::string(TRACE_NAME) + __func__ + "() Failed to send configuration bitstreams");

} //feb_send_bitstreams

void sbndaq::BernCRTZMQData::StartFebdrv() {
  //TODO remove this function and move restarting functionality to FEBDRV
  TLOG(TLVL_DEBUG)<< __func__<<": Starting febdrv";
  febdrv.startDAQ();
  last_restart_time = std::chrono::system_clock::now();
}

uint64_t sbndaq::BernCRTZMQData::GetTimeSinceLastRestart() {
  return (std::chrono::system_clock::now() -  last_restart_time).count();
}

/*---------------BERN CRT ZMQ DATA-------------------------------------*/
size_t sbndaq::BernCRTZMQData::GetZMQData() {
  /**
   * Reads data from FEB
   */
  
  TLOG(TLVL_DEBUG) << __func__ << "() called";

  size_t data_size=0;
  
  febdrv.polldata();
  
  size_t events = 0;
  
//  zmq_msg_t feb_data_msg;
//  zmq_msg_init(&feb_data_msg);

  //attempt to read data from febdrv
/*  while(zmq_msg_recv(&feb_data_msg, zmq_subscriber_, ZMQ_DONTWAIT) < 0) {
    if ( errno != EAGAIN ) { // No data awailable now
      TLOG(TLVL_ERROR) << __func__ << ": Instead of EAGAIN got the following errno: " <<  std::to_string(errno) << " " << zmq_strerror(errno);
      return 0;
    }
    usleep(1000);

    ++wait_count;
    
    if( (wait_count%500) == 0 ) {
      TLOG(TLVL_DEBUG) << __func__ << ": waiting for data : " << wait_count;
    }
  }*/
  
//   TLOG(TLVL_DEBUG) << __func__ << ": Completed wait loop after " << wait_count << " iterations";
  
/*  if(zmq_msg_size(&feb_data_msg)>0) {
    TLOG(TLVL_DEBUG) << __func__ << ": about to copy " <<zmq_msg_size(&feb_data_msg)<< " bytes";

    events = zmq_msg_size(&feb_data_msg)/sizeof(BernCRTZMQEvent);
    data_size = zmq_msg_size(&feb_data_msg);

    //can data fit in the buffer?
    if( events > ZMQBufferCapacity_ ) {
      TLOG(TLVL_ERROR) << __func__ << " Too many events for ZMQ buffer! " << std::to_string(events);
      throw cet::exception(std::string(TRACE_NAME) + " " + __func__ + " Too many events for ZMQ buffer!");
    }
    
    std::copy((uint8_t*)zmq_msg_data(&feb_data_msg),
	      (uint8_t*)zmq_msg_data(&feb_data_msg)+zmq_msg_size(&feb_data_msg), //last event contains time info
	      reinterpret_cast<uint8_t*>(&ZMQBufferUPtr[0]));
  }*/

//  zmq_msg_close(&feb_data_msg);

  TLOG(TLVL_DEBUG) << __func__ << "() copied " << std::to_string(events) << " events of size of " << std::to_string(data_size);

  return data_size;
} //GetZMQData

DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(sbndaq::BernCRTZMQData) 
