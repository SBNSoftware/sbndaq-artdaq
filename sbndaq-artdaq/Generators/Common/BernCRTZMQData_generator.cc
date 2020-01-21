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

#include <zmq.h>

#define id "BernCRT"
#define TRACE_NAME "BernCRTZMQData"

sbndaq::BernCRTZMQData::BernCRTZMQData(fhicl::ParameterSet const & ps)
  :
  BernCRTZMQ_GeneratorBase(ps) {
  TLOG(TLVL_INFO) << "BernCRTZMQData constructor called"; 
 
  zmq_listening_port_          = std::string("tcp://localhost:") + std::to_string(ps_.get<int>("zmq_listening_port"));
  zmq_data_pub_port_           = std::string("tcp://localhost:") + std::to_string(ps_.get<int>("zmq_listening_port")+1);
  
  zmq_context_    = zmq_ctx_new();

  TLOG(TLVL_DEBUG) << __func__ << " : Calling zmq requester with port " << zmq_listening_port_;
  zmq_requester_ = zmq_socket(zmq_context_, ZMQ_REQ);
  const int linger = 0;
  zmq_setsockopt(zmq_requester_, ZMQ_LINGER, &linger, sizeof(linger));

  febctl(GETINFO);
  
  for(unsigned int iFEB = 0; iFEB < nFEBs(); iFEB++) {
    TLOG(TLVL_DEBUG) << __func__ << " Reading bitstream configuration for MAC5 " << std::to_string(iFEB) << ": " << std::to_string(MAC5s_[iFEB]);
    feb_configuration[MAC5s_[iFEB]] = sbndaq::BernCRTFEBConfiguration(ps_, iFEB); //create configuration object
  }

  TLOG(TLVL_INFO) << __func__ << " constructor completed";  
} //constructor

sbndaq::BernCRTZMQData::~BernCRTZMQData()
{
  TLOG(TLVL_INFO) << __func__ <<  "() called";  

  zmq_close (zmq_requester_);

  zmq_ctx_destroy(zmq_context_);

  Cleanup();
  TLOG(TLVL_INFO) << __func__ << "() completed";  
} //destructor

void sbndaq::BernCRTZMQData::ConfigureStart() {
  TLOG(TLVL_INFO) << __func__ << "() called";  
  
  //make sure the HV and DAQ are off before we start to send the configuration to the board
  febctl(DAQ_END);
  febctl(BIAS_OF);

  for(unsigned int iFEB = 0; iFEB < nFEBs(); iFEB++) {
    feb_send_bitstreams(MAC5s_[iFEB]); //send PROBE and SC configuration to FEB
    if(feb_configuration[MAC5s_[iFEB]].GetHVOnPermission()) febctl(BIAS_ON, MAC5s_[iFEB]); //turn on SiPM HV (if FHiCL file allows it)
  }
  febctl(DAQ_BEG); //start data taking mode for all boards 

  zmq_subscriber_ = zmq_socket(zmq_context_,ZMQ_SUB);

  int res=0;

  TLOG(TLVL_INFO) << __func__ << " Calling zmq subscriber with port " << zmq_data_pub_port_;
  res = zmq_connect(zmq_subscriber_,zmq_data_pub_port_.c_str());
  if(res!=0)
    TLOG(TLVL_ERROR) << __func__  << " failed to connect.";

  res = zmq_setsockopt(zmq_subscriber_,ZMQ_SUBSCRIBE,NULL,0);

  if(res!=0)
    TLOG(TLVL_ERROR) << __func__ << " socket options failed.";
  TLOG(TLVL_INFO) << __func__ << " completed";
} //ConfigureStart

void sbndaq::BernCRTZMQData::ConfigureStop() {
  TLOG(TLVL_INFO) << __func__ << "() called";

  febctl(DAQ_END);
  febctl(BIAS_OF);

  zmq_close(zmq_subscriber_);

  TLOG(TLVL_INFO) << __func__ << "() completed";
} //ConfigureStop

int sbndaq::BernCRTZMQData::GetDataSetup() {
  return 1;
}

int sbndaq::BernCRTZMQData::GetDataComplete() {
  return 1;
}

void sbndaq::BernCRTZMQData::febctl(feb_command command, uint8_t mac5) {
/**
 * Copied functionality of febctl tool from febdriver
 * Sends a command to febdrv.
 * Arguments:
 * - command: BIAS_OF BIAS_ON DAQ_BEG DAQ_END GETINFO
 * - mac5: last 8 bits of FEB MAC5 address. If set to 255 and command is sent to all boards.
 * 
 * If the command is GETINFO this function will verify if the
 * list of MAC5s reported by febdrv matches the list in FHiCL
 */

  char command_string[8];
  switch (command) {
    case DAQ_BEG : strcpy ( command_string, "DAQ_BEG");   break;
    case DAQ_END : strcpy ( command_string, "DAQ_END");   break;
    case BIAS_ON : strcpy ( command_string, "BIAS_ON");   break;
    case BIAS_OF : strcpy ( command_string, "BIAS_OF");   break;
    case GETINFO : strcpy ( command_string, "GETINFO");   break;
    default:
      TLOG(TLVL_ERROR) <<   __func__ << " Unrecognized command! [software error, this should never happen]";
      throw cet::exception( std::string(TRACE_NAME) +  __func__ + " Unrecognized command! [software error, this should never happen]");
  }

  TLOG(TLVL_DEBUG) <<  __func__ << " Connecting to febdrv...";
  if(zmq_connect (zmq_requester_, zmq_listening_port_.c_str())) {
    TLOG(TLVL_ERROR) << __func__ << " Connection to " << zmq_listening_port_ << " failed!";
    throw cet::exception( std::string(TRACE_NAME) + __func__ + " Connection to " + zmq_listening_port_ + " failed!");
  }

  if(mac5 != 255 && feb_configuration.find(mac5) == feb_configuration.end()) {
    TLOG(TLVL_ERROR) << __func__ << " Could not find FEB " << std::to_string(mac5) << " in the MAC5s!";
    throw cet::exception( std::string(TRACE_NAME) + __func__ + " Could not find FEB " + std::to_string(mac5) + " in the MAC5s!");
  }

  zmq_msg_t request;
  zmq_msg_init_size (&request, 9);
  memcpy(zmq_msg_data (&request), command_string,7);
  ((uint8_t*)zmq_msg_data (&request))[7]=0;
  ((uint8_t*)zmq_msg_data (&request))[8]=mac5;
  TLOG(TLVL_DEBUG) <<  __func__ << " Sending command \"" << command_string << "\" to mac5 = " << std::to_string(mac5);
  zmq_msg_send (&request, zmq_requester_, 0);
  zmq_msg_close (&request);
  zmq_msg_t reply;
  zmq_msg_init (&reply);
  const int timeout = 5; //total waiting time in seconds, hardcoded value, move to config file?
  const int attempts = 10;
  int rv = zmq_msg_recv (&reply, zmq_requester_, ZMQ_DONTWAIT);
  for(int i = 0; i < attempts && rv < 0; i++) {
    usleep(timeout*1000000/attempts);
    rv = zmq_msg_recv (&reply, zmq_requester_, ZMQ_DONTWAIT);
  }
  if(rv<0) {
    TLOG(TLVL_ERROR) <<  __func__ << " Received no reply from febdrv after waiting for " << std::to_string(timeout) << " seconds";
    zmq_msg_close (&reply);
    throw cet::exception( std::string(TRACE_NAME) + __func__ + " Received no reply from febdrv after waiting for " + std::to_string(timeout) + " seconds");
  }

  std::string reply_string = (char*)zmq_msg_data(&reply);
  zmq_msg_close (&reply);

  //Process reply message
  if(command != GETINFO) {
    if(reply_string.compare("OK")) {
      TLOG(TLVL_ERROR) <<  __func__ << " Received unexpected reply from febdrv: " << reply_string;
    }
    else {
      TLOG(TLVL_DEBUG) <<  __func__ << " Received reply: " << reply_string;
    }
  }
  else {
    //GETINFO returns the number of FEBs connected, their macs and firmware versions.
    //Check if the boards we see correspond to the FHICL file configuration
    TLOG(TLVL_DEBUG) <<  __func__ << " Received reply: " << reply_string;
    std::istringstream ireply_string(reply_string);
    std::string line;
    if(!std::getline(ireply_string, line)) {
      TLOG(TLVL_ERROR) <<  __func__ << " Received empty reply febdrv!";
    }
    else { //parse febdrv reply
      //parse number of reported FEBs
      const unsigned int read_nFEBs = stoi(line);
      if(read_nFEBs != nFEBs()) {
        TLOG(TLVL_ERROR) <<  __func__ << " Number of FEBs seen by febdrv (" << std::to_string(read_nFEBs) << ") differs from the ones defined in FCL file (" + std::to_string(nFEBs()) + ")!";
        throw cet::exception( std::string(TRACE_NAME) + __func__ + " Number of FEBs seen by febdrv (" + std::to_string(read_nFEBs) + ") differs from the ones defined in FCL file (" + std::to_string(nFEBs()) + ")!");
      }
      else {
        //read firmwares and mac addresses MAC addresses
        std::vector <std::string> firmwares;
        std::vector <uint8_t> mac5s;
        while (std::getline(ireply_string, line)) {
          firmwares.push_back(line.substr(18, 100));
          unsigned int mac[6];
          if (sscanf(line.substr(0,17).c_str(), "%x:%x:%x:%x:%x:%x", &mac[5], &mac[4], &mac[3], &mac[2], &mac[1], &mac[0]) != 6) {
            TLOG(TLVL_ERROR) <<  __func__ << " Error during parsing MAC address returned by febdrv (" << line <<  ")!";
          }
          else {
            mac5s.push_back(mac[0]); //we care only about last 8 bits of the address
          }
        }
        //check if all MAC addresses are unique, present in the FHiCL configuration, and all MACs expected by FHiCL are found
        std::vector<bool> fcl_mac_found; for(unsigned int i=0;i<nFEBs();i++) fcl_mac_found.push_back(false);
        bool any_mac_missing = false;
        for(unsigned int iFEB = 0; iFEB < mac5s.size(); iFEB++) {
          bool mac_found = false;
          for(unsigned int jFEB = 0; jFEB < nFEBs(); jFEB++) {
            if(mac5s[iFEB] == MAC5s_[jFEB]) {
              if(fcl_mac_found[jFEB]) {
                TLOG(TLVL_ERROR) <<  __func__ << " MAC address ending with " << std::to_string(mac5) << " (dec) found twice by febdrv. Check hardware MAC configuration!";
                throw cet::exception( std::string(TRACE_NAME) + __func__ + " MAC address ending with " +std::to_string(mac5) + " (dec) found twice by febdrv. Check hardware MAC configuration!");
                
              }
              else {
                fcl_mac_found[jFEB] = true;
                mac_found = true;
              }
            }
          }
          if(!mac_found) {
            any_mac_missing = true;
            TLOG(TLVL_ERROR) <<  __func__ << " MAC address seen by febdrv " << std::to_string(mac5s[iFEB]) <<  " (dec) missing in the FCL file!";
            throw cet::exception( std::string(TRACE_NAME) + __func__ + " MAC address seen by febdrv " + std::to_string(mac5s[iFEB]) +  " (dec) missing in the FCL file!");
          }
        }
        bool some_fcl_mac_missing = 0;
        for(unsigned int jFEB = 0; jFEB < nFEBs(); jFEB++) {
          if(!fcl_mac_found[jFEB]) {
            some_fcl_mac_missing = true;
            break;
          }
        }
        if(some_fcl_mac_missing) {
            TLOG(TLVL_ERROR) <<  __func__ << " Some of the MAC addresses defined in the FCL file do not match any FEBs seen by febdrv.";
            throw cet::exception( std::string(TRACE_NAME) + __func__ + " Some of the MAC addresses defined in the FCL file do not match any FEBs seen by febdrv.");
        }
        else if(!any_mac_missing) {
            TLOG(TLVL_INFO)<< __func__ << " All " << std::to_string(nFEBs()) << " FEBs found in the FCL.";
        }
      }
    }
  }
} //febctl

void sbndaq::BernCRTZMQData::feb_send_bitstreams(uint8_t mac5) {
  /**
   * Sends configuration bitstream to febdriver
   * Based on febconf main()
   * Arguments:
   * mac5 of the board, note one cannot use 255 address to broadcast to all boards
   * Bitstreams should be read from (?)
   * - Probe_bitStream - monitoring configuration. It consists of 224 bits, typically all '0' for normal operation 
   * - SlowControl_bitStream - actual configuration to be sent to the board, 1144 bits
   */

  if(feb_configuration.find(mac5) == feb_configuration.end()) {
    TLOG(TLVL_ERROR) <<  __func__ << " Could not find FEB " << mac5 << " in MAC5s!";
    throw cet::exception( std::string(TRACE_NAME) + __func__ + " Could not find FEB " + std::to_string(mac5) + " in MAC5s!");
  }

  if(mac5==255) {
    TLOG(TLVL_ERROR) <<  __func__ << " Bitstreams cannot be sent to mac5 = 255!";
    return;
  }

  TLOG(TLVL_DEBUG)<< __func__ << " Connecting to febdrv...";
  if(zmq_connect(zmq_requester_, zmq_listening_port_.c_str())) {
    TLOG(TLVL_ERROR) <<  __func__ << " Connection to " << zmq_listening_port_ << " failed!";
    throw cet::exception( std::string(TRACE_NAME) + __func__ + " Connection to " + zmq_listening_port_ + " failed!");
  }

  char cmd[32];

  sprintf(cmd,"SETCONF");
  cmd[8]=mac5;

  TLOG(TLVL_DEBUG)<< __func__ << " Sending command " << cmd << " to mac address " << std::to_string(mac5) << "...";

  const int MAXPACKLEN = 1500;
  uint8_t buffer[MAXPACKLEN];

  memcpy(buffer,cmd,9);
  memcpy(
      buffer+9,
      feb_configuration[mac5].GetSlowControlBitStream(),
      feb_configuration[mac5].GetSlowControlBitStreamNBytes());
  memcpy(
      buffer+9 + feb_configuration[mac5].GetSlowControlBitStreamNBytes(),
      feb_configuration[mac5].GetProbeBitStream(),
      feb_configuration[mac5].GetProbeBitStreamNBytes());
  zmq_send(
      zmq_requester_,
      buffer,
      feb_configuration[mac5].GetSlowControlBitStreamNBytes()+feb_configuration[mac5].GetProbeBitStreamNBytes()+9, 0);

  TLOG(TLVL_DEBUG)<< __func__ << " Waiting for reply...";

  zmq_msg_t reply;
  zmq_msg_init (&reply);

  const int timeout = 5; //s hardcoded value, move to config file?
  const int attempts = 10;
  int rv = zmq_msg_recv (&reply, zmq_requester_, ZMQ_DONTWAIT);
  for(int i = 0; i < attempts && rv < 0; i++) {
    usleep(timeout*1000000/attempts);
    rv = zmq_msg_recv (&reply, zmq_requester_, ZMQ_DONTWAIT);
  }
  if(rv<0) {
    TLOG(TLVL_ERROR) <<  __func__ << " Received no reply from febdrv after waiting for " << std::to_string(timeout) << " seconds";
    zmq_msg_close (&reply);
    throw cet::exception(std::string(TRACE_NAME) + __func__ + " Received no reply from febdrv after waiting for " + std::to_string(timeout) + " seconds");
  }

  std::string reply_string = (char*)zmq_msg_data(&reply);
  if(reply_string.compare("OK")) {
    TLOG(TLVL_ERROR) <<  __func__ << " Received unexpected reply from febdrv: " << reply_string;
    throw cet::exception(std::string(TRACE_NAME) + __func__ + " Received unexpected reply from febdrv: " + reply_string);
  }
  else {
    TLOG(TLVL_DEBUG)<< __func__ << " Received reply: " << reply_string;
  }
  zmq_msg_close (&reply);
} //feb_send_bitstreams

/*---------------BERN CRT ZMQ DATA-------------------------------------*/
size_t sbndaq::BernCRTZMQData::GetZMQData() {
  /**
   * Reads data from febdrv via zeromq and copies it into ZMQBufferUPtr
   */
  
  TLOG(TLVL_DEBUG) << __func__ << "() called";
  
  size_t data_size=0;
  size_t wait_count=0;
  size_t events = 0;
  
  zmq_msg_t feb_data_msg;
  zmq_msg_init(&feb_data_msg);

  //attempt to read data from febdrv
  while(zmq_msg_recv(&feb_data_msg, zmq_subscriber_, ZMQ_DONTWAIT) < 0) {
    if ( errno != EAGAIN ) { // No data awailable now
      TLOG(TLVL_ERROR) << __func__ << ": Instead of EAGAIN got the following errno: " <<  std::to_string(errno) << " " << zmq_strerror(errno);
      return 0;
    }
    usleep(1000);

    ++wait_count;
    
    if( (wait_count%500) == 0 ) {
      TLOG(TLVL_DEBUG) << __func__ << ": waiting for data : " << wait_count;
    }
  }
  
  TLOG(TLVL_DEBUG) << __func__ << ": Completed wait loop after " << wait_count << " iterations";
  
  if(zmq_msg_size(&feb_data_msg)>0) {
    TLOG(TLVL_DEBUG) << __func__ << ": about to copy " <<zmq_msg_size(&feb_data_msg)<< " bytes";

    events = zmq_msg_size(&feb_data_msg)/sizeof(BernCRTZMQEvent);
    data_size = zmq_msg_size(&feb_data_msg);

    //check : is this too much data for the buffer? //TODO: shouldn't we check it before copying?
    if( events > ZMQBufferCapacity_ ) {
      TLOG(TLVL_ERROR) << __func__ << " Too many events for ZMQ buffer! " << std::to_string(events);
      throw cet::exception(std::string(TRACE_NAME) + " " + __func__ + " Too many events for ZMQ buffer!");
    }
    
    std::copy((uint8_t*)zmq_msg_data(&feb_data_msg),
	      (uint8_t*)zmq_msg_data(&feb_data_msg)+zmq_msg_size(&feb_data_msg), //last event contains time info
	      reinterpret_cast<uint8_t*>(&ZMQBufferUPtr[0]));
  }

  zmq_msg_close(&feb_data_msg);

  TLOG(TLVL_DEBUG) << __func__ << " copied " << std::to_string(events) << " events of size of " << std::to_string(data_size);

  return data_size;
} //GetZMQData

DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(sbndaq::BernCRTZMQData) 
