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

#include "BernCRT_TRACE_defines.h"

#include <zmq.h>

#define id "BernCRT"

sbndaq::BernCRTZMQData::BernCRTZMQData(fhicl::ParameterSet const & ps)
  :
  BernCRTZMQ_GeneratorBase(ps)  
{
  TRACE(TR_LOG, "BernCRTZMQData constructor called"); 
 
  zmq_listening_port_          = std::string("tcp://localhost:") + std::to_string(ps_.get<int>("zmq_listening_port"));
  zmq_data_pub_port_           = std::string("tcp://localhost:") + std::to_string(ps_.get<int>("zmq_listening_port")+1);
  zmq_data_receive_timeout_ms_ = ps_.get<int>("zmq_data_receive_timeout_ms",500);
  
  zmq_context_    = zmq_ctx_new();

  TRACE(TR_LOG, std::string("BernCRTZMQData constructor : Calling zmq requester with port ")+ zmq_listening_port_);
  zmq_requester_ = zmq_socket(zmq_context_, ZMQ_REQ);
  const int linger = 0;
  zmq_setsockopt(zmq_requester_, ZMQ_LINGER, &linger, sizeof(linger));

  febctl(GETINFO);

  for(unsigned int iFEB = 0; iFEB < nFEBs(); iFEB++) {
    TRACE(TR_LOG, std::string("BernCRTZMQData constructor : Last 8 bits of FEBID ") + std::to_string(iFEB)+ ": " + std::to_string(FEBIDs_[iFEB]&0xff));
    feb_configuration.push_back(sbndaq::BernCRTFEBConfiguration(ps_, iFEB)); //create configuration object
//    std::ofstream file("/tmp/aaduszki_tmp_" + std::to_string(iFEB));
//    file << feb_configuration[iFEB].GetString();
  }

  TRACE(TR_LOG, "BernCRTZMQData constructor completed");  
}

sbndaq::BernCRTZMQData::~BernCRTZMQData()
{
  TRACE(TR_LOG, "BernCRTZMQData destructor called");  

  zmq_close (zmq_requester_);

  zmq_ctx_destroy(zmq_context_);

  Cleanup();
  TRACE(TR_LOG, "BernCRTZMQData destructor completed");  
}



void sbndaq::BernCRTZMQData::ConfigureStart()
{
  TRACE(TR_LOG, "BernCRTZMQData::ConfigureStart() called");  
  
  //make sure the HV and DAQ are off before we start to send the configuration to the board
  febctl(BIAS_OF);
  febctl(DAQ_END);

  for(unsigned int iFEB = 0; iFEB < nFEBs(); iFEB++) feb_send_bitstreams(iFEB);
  febctl(BIAS_ON);
  febctl(DAQ_BEG); //start data taking mode for all boards 

  zmq_subscriber_ = zmq_socket(zmq_context_,ZMQ_SUB);

  int res=0;

  TRACE(TR_LOG, std::string("BernCRTZMQData::ConfigureStart() : Calling zmq subscriber with port ")+ zmq_data_pub_port_);
  res = zmq_connect(zmq_subscriber_,zmq_data_pub_port_.c_str());
  if(res!=0)
    TRACE(TR_ERROR, "BernCRTZMQDataZMQ::ConfigureStart() failed to connect.");

  res = zmq_setsockopt(zmq_subscriber_,ZMQ_SUBSCRIBE,NULL,0);
  //res = zmq_setsockopt(zmq_subscriber_,ZMQ_RCVTIMEO,&zmq_data_receive_timeout_ms_,2);

  if(res!=0)
    TRACE(TR_ERROR, "BernCRTZMQDataZMQ::ConfigureStart() socket options failed.");

  TRACE(TR_LOG, "BernCRTZMQData::ConfigureStart() completed");  
}

void sbndaq::BernCRTZMQData::ConfigureStop()
{
  TRACE(TR_LOG, "BernCRTZMQData::ConfigureStop() called");
  
  febctl(DAQ_END);
  febctl(BIAS_OF);

  zmq_close(zmq_subscriber_);

  TRACE(TR_LOG, "BernCRTZMQData::ConfigureStop() completed");  
}

int sbndaq::BernCRTZMQData::GetDataSetup()
{
  return 1;
}

int sbndaq::BernCRTZMQData::GetDataComplete()
{
  return 1;
}

void sbndaq::BernCRTZMQData::febctl(feb_command command, int iFEB) {
/**
 * Copied functionality of febctl tool from febdriver
 * Sends a command to febdrv.
 * Arguments:
 * - command: BIAS_OF BIAS_ON DAQ_BEG DAQ_END GETINFO
 * - iFEB: number of the FEB in the chain. If iFEB is -1, mac5 is set to 255 and command is sent to all boards.
 * 
 * If the command is GETINFO this function will verify if the
 * list of MAC5s reported by febdrv matches the list in FHiCL
 */

  char trace_message[256];
  
  char command_string[8];
  switch (command) {
    case DAQ_BEG : strcpy ( command_string, "DAQ_BEG");   break;
    case DAQ_END : strcpy ( command_string, "DAQ_END");   break;
    case BIAS_ON : strcpy ( command_string, "BIAS_ON");   break;
    case BIAS_OF : strcpy ( command_string, "BIAS_OF");   break;
    case GETINFO : strcpy ( command_string, "GETINFO");   break;
    default:
      TRACE(TR_ERROR, std::string("BernCRTZMQData::") + __func__ + " Unrecognized command! [software error, this should never happen]");
      throw cet::exception(std::string("BernCRTZMQData::") + __func__ + " Unrecognized command! [software error, this should never happen]");

  }
  
  TRACE(TR_DEBUG, std::string("BernCRTZMQData::") + __func__ + " Connecting to febdrv...");
  if(zmq_connect (zmq_requester_, zmq_listening_port_.c_str())) {
    TRACE(TR_ERROR, std::string("BernCRTZMQData::") + __func__ + " Connection to " + zmq_listening_port_ + " failed!");
    throw cet::exception(std::string("BernCRTZMQData::") + __func__ + " Connection to " + zmq_listening_port_ + " failed!");
  }

  uint8_t mac5;
  if((int)nFEBs() <= iFEB) {
    TRACE(TR_ERROR, std::string("BernCRTZMQData::") + __func__ + " Could not find FEB " + std::to_string(iFEB) + " in the FEBIDs!");
    throw cet::exception(std::string("BernCRTZMQData::") + __func__ + " Could not find FEB " + std::to_string(iFEB) + " in the FEBIDs!");
  }
  else {
    if(iFEB == -1) mac5 = 255;
    else           mac5 = FEBIDs_[iFEB] & 0xff;
  }

  zmq_msg_t request;
  zmq_msg_init_size (&request, 9);
  memcpy(zmq_msg_data (&request), command_string,7);
  ((uint8_t*)zmq_msg_data (&request))[7]=0;
  ((uint8_t*)zmq_msg_data (&request))[8]=mac5;
  TRACE(TR_DEBUG, std::string("BernCRTZMQData::") + __func__ + " Sending command \"" + command_string + "\" to mac5 = " + std::to_string(mac5));
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
    TRACE(TR_ERROR, std::string("BernCRTZMQData::") + __func__ + " Received no reply from febdrv after waiting for " + std::to_string(timeout) + " seconds");
    zmq_msg_close (&reply);
    throw cet::exception(std::string("BernCRTZMQData::") + __func__ + " Received no reply from febdrv after waiting for " + std::to_string(timeout) + " seconds");
  }

  std::string reply_string = (char*)zmq_msg_data(&reply);
  zmq_msg_close (&reply);

  if(command != GETINFO) {
    if(reply_string.compare("OK")) {
      TRACE(TR_ERROR, std::string("BernCRTZMQData::") + __func__ + " Received unexpected reply from febdrv: " + reply_string);
    }
    else {
      TRACE(TR_DEBUG, std::string("BernCRTZMQData::") + __func__ + " Received reply: " + reply_string);
    }
  }
  else { //check if the boards we see correspond to the FHICL file configuration
    TRACE(TR_DEBUG, std::string("BernCRTZMQData::") + __func__ + " Received reply: " + reply_string);
    std::istringstream ireply_string(reply_string);
    std::string line;
    if(!std::getline(ireply_string, line)) {
      TRACE(TR_ERROR, std::string("BernCRTZMQData::") + __func__ + " Received empty reply febdrv!");
    }
    else { //parse febdrv reply
      //parse number of reported FEBs
      const unsigned int read_nFEBs = stoi(line);
      if(read_nFEBs != nFEBs()) {
        TRACE(TR_ERROR, std::string("BernCRTZMQData::") + __func__ + " Number of FEBs seen by febdrv (" + std::to_string(read_nFEBs) + ") differs from the ones defined in FCL file (" + std::to_string(nFEBs()) + ")!");
        throw cet::exception(std::string("BernCRTZMQData::") + __func__ + " Number of FEBs seen by febdrv (" + std::to_string(read_nFEBs) + ") differs from the ones defined in FCL file (" + std::to_string(nFEBs()) + ")!");
      }
      else {
        //read firmwares and mac addresses MAC addresses
        std::vector <std::string> firmwares;
        std::vector <uint64_t> mac5s;
        while (std::getline(ireply_string, line)) {
          firmwares.push_back(line.substr(18, 100));
          unsigned int mac[6];
          if (sscanf(line.substr(0,17).c_str(), "%x:%x:%x:%x:%x:%x", &mac[5], &mac[4], &mac[3], &mac[2], &mac[1], &mac[0]) != 6) {
            TRACE(TR_ERROR, std::string("BernCRTZMQData::") + __func__ + " Error during parsing MAC address returned by febdrv (" + line +  ")!");
          }
          else {
            uint64_t full_mac = 0;
            for(int i = 0; i < 6; i++) {
              full_mac += (uint64_t)mac[i] << i * 8;
            }
            mac5s.push_back(full_mac);
          }
        }
        //check if all MAC addresses are unique, present in the FHiCL configuration, and all MACs expected by FHiCL are found
        std::vector<bool> fcl_mac_found; for(unsigned int i=0;i<nFEBs();i++) fcl_mac_found.push_back(false);
        bool any_mac_missing = false;
        for(unsigned int iFEB = 0; iFEB < mac5s.size(); iFEB++) {
          bool mac_found = false;
          for(unsigned int jFEB = 0; jFEB < nFEBs(); jFEB++) {
            if(mac5s[iFEB] == FEBIDs_[jFEB]) {
              if(fcl_mac_found[jFEB]) {
                TRACE(TR_ERROR, std::string("BernCRTZMQData::") + __func__ + " MAC address ending with " +std::to_string(mac5&0xff) + " (dec) found twice by febdrv. Check hardware MAC configuration!");
                throw cet::exception(std::string("BernCRTZMQData::") + __func__ + " MAC address ending with " +std::to_string(mac5&0xff) + " (dec) found twice by febdrv. Check hardware MAC configuration!");
                
              }
              else {
                fcl_mac_found[jFEB] = true;
                mac_found = true;
              }
            }
          }
          if(!mac_found) {
            any_mac_missing = true;
            TRACE(TR_ERROR, std::string("BernCRTZMQData::") + __func__ + " MAC address seen by febdrv " + std::to_string(mac5s[iFEB] &0xff) +  " (dec) missing in the FCL file!");
            throw cet::exception(std::string("BernCRTZMQData::") + __func__ + " MAC address seen by febdrv " + std::to_string(mac5s[iFEB] &0xff) +  " (dec) missing in the FCL file!");
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
            TRACE(TR_ERROR, std::string("BernCRTZMQData::") + __func__ + " Some of the MAC addresses defined in the FCL file do not match any FEBs seen by febdrv.");
            throw cet::exception(std::string("BernCRTZMQData::") + __func__ + " Some of the MAC addresses defined in the FCL file do not match any FEBs seen by febdrv.");
        }
        else if(!any_mac_missing) {
            TRACE(TR_LOG, std::string("BernCRTZMQData::") + __func__ + " All "+std::to_string(nFEBs())+" FEBs found in the FCL.");
        }
      }
    }
  }
}

void sbndaq::BernCRTZMQData::feb_send_bitstreams(unsigned int iFEB) {
  /**
   * Sends configuration bitstream to febdriver
   * Based on febconf main()
   * Arguments:
   * mac5 of the board, note one cannot use 255 address to broadcast to all boards
   * Bitstreams should be read from (?)
   * - Probe_bitStream - monitoring configuration. It consists of 224 bits, typically all '0' for normal operation 
   * - SlowControl_bitStream - actual configuration to be sent to the board, 1144 bits
   */

  uint8_t mac5;
  if(FEBIDs_.size() <= iFEB) {
    TRACE(TR_ERROR, std::string("BernCRTZMQData::") + __func__ + " Could not find FEB " + std::to_string(iFEB) + " in the FEBIDs!");
    throw cet::exception(std::string("BernCRTZMQData::") + __func__ + " Could not find FEB " + std::to_string(iFEB) + " in the FEBIDs!");
  }
  else {
    mac5 = FEBIDs_[iFEB] & 0xff;
  }

  if(mac5==255) {
    TRACE(TR_ERROR, std::string("BernCRTZMQData::") + __func__ + " Bitstreams cannot be sent to mac5 = 255!");
    return;
  }

  TRACE(TR_DEBUG, std::string("BernCRTZMQData::") + __func__ + " Connecting to febdrv...");
  if(zmq_connect(zmq_requester_, zmq_listening_port_.c_str())) {
    TRACE(TR_ERROR, std::string("BernCRTZMQData::") + __func__ + " Connection to " + zmq_listening_port_ + " failed!");
    throw cet::exception(std::string("BernCRTZMQData::") + __func__ + " Connection to " + zmq_listening_port_ + " failed!");
  }

  char cmd[32];

  sprintf(cmd,"SETCONF");
  cmd[8]=mac5;

  TRACE(TR_DEBUG, std::string("BernCRTZMQData::") + __func__ + " Sending command " + cmd + " to mac address " + std::to_string(mac5) + "...");

  const int MAXPACKLEN = 1500;
  uint8_t buffer[MAXPACKLEN];

  memcpy(buffer,cmd,9);
  memcpy(
      buffer+9,
      feb_configuration[iFEB].GetSlowControlBitStream(),
      feb_configuration[iFEB].GetSlowControlBitStreamNBytes());
  memcpy(
      buffer+9 + feb_configuration[iFEB].GetSlowControlBitStreamNBytes(),
      feb_configuration[iFEB].GetProbeBitStream(),
      feb_configuration[iFEB].GetProbeBitStreamNBytes());
  zmq_send(
      zmq_requester_,
      buffer,
      feb_configuration[iFEB].GetSlowControlBitStreamNBytes()+feb_configuration[iFEB].GetProbeBitStreamNBytes()+9, 0);

  TRACE(TR_DEBUG, std::string("BernCRTZMQData::") + __func__ + " Waiting for reply...");

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
    TRACE(TR_ERROR, std::string("BernCRTZMQData::") + __func__ + " Received no reply from febdrv after waiting for " + std::to_string(timeout) + " seconds");
    zmq_msg_close (&reply);
    throw cet::exception(std::string("BernCRTZMQData::") + __func__ + " Received no reply from febdrv after waiting for " + std::to_string(timeout) + " seconds");
  }

  std::string reply_string = (char*)zmq_msg_data(&reply);
  if(reply_string.compare("OK")) {
    TRACE(TR_ERROR, std::string("BernCRTZMQData::") + __func__ + " Received unexpected reply from febdrv: " + reply_string);
    throw cet::exception(std::string("BernCRTZMQData::") + __func__ + " Received unexpected reply from febdrv: " + reply_string);
  }
  else {
    TRACE(TR_DEBUG, std::string("BernCRTZMQData::") + __func__ + " Received reply: " + reply_string);
  }
  zmq_msg_close (&reply);
}

/*---------------BERN CRT ZMQ DATA-------------------------------------*/
size_t sbndaq::BernCRTZMQData::GetZMQData()
{
  std::cout << "Calling GetZMQData" << std::endl;
  
  TRACE(TR_LOG, "BernCRTZMQData::GetZMQData called");
  
  size_t data_size=0;
  size_t events=0;
  
  size_t wait_count=0;
  
  zmq_msg_t feb_data_msg;
  zmq_msg_init(&feb_data_msg);

  while(zmq_msg_recv(&feb_data_msg,zmq_subscriber_,ZMQ_DONTWAIT) < 0)
  {
    if ( errno != EAGAIN ) // No data awailable now
    {
      TRACE(TR_ERROR, std::string("BernCRTZMQData::GetFEBData() called ") +  std::to_string(errno) + " "  + zmq_strerror(errno));
    return 0;
    }
    usleep(1000);
    //return 0;
    ++wait_count;
    
    if( (wait_count%500) == 0 )
    {
      TRACE(TR_LOG, std::string("BernCRTZMQData::GetZMQData() wait count: ") + std::to_string(wait_count));
    }
  }
  
  TRACE(TR_LOG, std::string("BernCRTZMQData::GetZMQData() outside wait loop: ") + std::to_string(wait_count));
  
  if(zmq_msg_size(&feb_data_msg)>0)
  {
    TRACE(TR_LOG, "BernCRTZMQData::GetZMQData() about to copy");
    
    std::copy((uint8_t*)zmq_msg_data(&feb_data_msg),
	      (uint8_t*)zmq_msg_data(&feb_data_msg)+zmq_msg_size(&feb_data_msg), //last event contains time info
	      reinterpret_cast<uint8_t*>(&ZMQBufferUPtr[events]));
    
    TRACE(TR_LOG, "BernCRTZMQData::GetZMQData() copied!");
    
    events += zmq_msg_size(&feb_data_msg)/sizeof(BernCRTZMQEvent);
    data_size += zmq_msg_size(&feb_data_msg);

    TRACE(TR_LOG, std::string("BernCRTZMQData::GetZMQData() copied ") + std::to_string(events) + " events (" + std::to_string(data_size) + " size)");

    //check : is this too much data for the buffer?
    if( events>ZMQBufferCapacity_ )
    {
      TRACE(TR_ERROR, std::string("BernCRTZMQData::GetZMQData : Too many events in ZMQ buffer! ") + std::to_string(events));
      throw cet::exception("In BernCRTZMQData::GetZMQData, Too many events in ZMQ buffer!");
    }
  }

  zmq_msg_close(&feb_data_msg);

  TRACE(TR_LOG, std::string("BernCRTZMQData::GetZMQData() size returned was ") + std::to_string(data_size));

  return data_size;

}

DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(sbndaq::BernCRTZMQData) 
