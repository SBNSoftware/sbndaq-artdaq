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

  TRACE(TR_LOG, std::string("BernCRTZMQData constructor : Calling zmq subscriber with port ")+ zmq_data_pub_port_);
  
  zmq_context_    = zmq_ctx_new();

//  FEBIDs_ = ps_.get< std::vector<uint64_t> >("FEBIDs");
  TRACE(TR_DEBUG, std::string("There are ") + std::to_string(FEBIDs_.size()) + " FEBID!");
  for(unsigned int iFEB = 0; iFEB < nFEBs(); iFEB++) {
    TRACE(TR_DEBUG, std::string("FEBID ") + std::to_string(iFEB)+ ": " + std::to_string(FEBIDs_[iFEB]));
  }
  //for(unsigned int iFEB = 0; iFEB < FEBIDs_.size(); iFEB++) {
  for(unsigned int iFEB = 0; iFEB < nFEBs(); iFEB++) {
    febctl(GETINFO, iFEB);
    feb_send_bitstreams(iFEB);
    febctl(DAQ_BEG, iFEB);
  }

  TRACE(TR_LOG, "BernCRTZMQData constructor completed");  
}

sbndaq::BernCRTZMQData::~BernCRTZMQData()
{
  TRACE(TR_LOG, "BernCRTZMQData destructor called");  
  zmq_ctx_destroy(zmq_context_);

  for(unsigned int iFEB = 0; iFEB < FEBIDs_.size(); iFEB++) {
    febctl(DAQ_END, iFEB);
  }

  Cleanup();
  TRACE(TR_LOG, "BernCRTZMQData destructor completed");  
}



void sbndaq::BernCRTZMQData::ConfigureStart()
{
  TRACE(TR_LOG, "BernCRTZMQData::ConfigureStart() called");  

  zmq_subscriber_ = zmq_socket(zmq_context_,ZMQ_SUB);

  int res=0;

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

void sbndaq::BernCRTZMQData::febctl(feb_command command, unsigned int iFEB) {
/**
 * Copied functionality of febctl tool from febdriver
 * Sends a command to febdrv.
 * Arguments:
 * - command: BIAS_OF BIAS_ON DAQ_BEG DAQ_END GETINFO
 * - mac5 is the last 8 bits of the mac address of the FEB
 * 
 * TODO:
 *  - Do something with the firmware versions returned with GETINFO)
 *  - Do something during unexpected events (throw exceptions?)
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
      TRACE(TR_ERROR, "BernCRTZMQData::febctl Unrecognized command!");
      return;

  }
  
  void * context = zmq_ctx_new ();

  TRACE(TR_DEBUG, "BernCRTZMQData::febctl Connecting to febdrv...");
  void *requester = zmq_socket (context, ZMQ_REQ);
  const int linger = 0;
  zmq_setsockopt(requester, ZMQ_LINGER, &linger, sizeof(linger));
  if(zmq_connect (requester, zmq_listening_port_.c_str()) < 0) {
    TRACE(TR_ERROR, std::string("BernCRTZMQData::febctl Connection to ") + zmq_listening_port_ + " failed!");
    return;
    //throw exception? return special code?
  }

  uint8_t mac5;
  if(FEBIDs_.size() <= iFEB) {
    TRACE(TR_ERROR, std::string("BernCRTZMQData::febctl Could not find FEB ") + std::to_string(iFEB) + " in the FEBIDs!");
    return;
    //throw exception? return special code?
  }
  else {
    mac5 = FEBIDs_[iFEB] & 0xff;
  }

  zmq_msg_t request;
  zmq_msg_init_size (&request, 9);
  memcpy(zmq_msg_data (&request), command_string,7);
  ((uint8_t*)zmq_msg_data (&request))[7]=0;
  ((uint8_t*)zmq_msg_data (&request))[8]=mac5;
  TRACE(TR_DEBUG, std::string("BernCRTZMQData::febctl Sending command \"") + command_string + "\" to mac5 = " + std::to_string(mac5));
  zmq_msg_send (&request, requester, 0);
  zmq_msg_close (&request);
  zmq_msg_t reply;
  zmq_msg_init (&reply);
  const int timeout = 5; //s hardcoded value, move to config file?
  const int attempts = 10;
  int rv = zmq_msg_recv (&reply, requester, ZMQ_DONTWAIT);
  for(int i = 0; i < attempts && rv < 0; i++) {
    usleep(timeout*1000000/attempts);
    rv = zmq_msg_recv (&reply, requester, ZMQ_DONTWAIT);
  }
  if(rv<0) {
    TRACE(TR_ERROR, std::string("BernCRTZMQData::febctl_bitstreams Received no reply after waiting for ") + std::to_string(timeout) + " seconds");
    zmq_msg_close (&reply);
    zmq_close (requester);
    zmq_ctx_destroy (context);
    return;
  }

  std::string reply_string = (char*)zmq_msg_data(&reply);
  zmq_msg_close (&reply);
  zmq_close (requester);
  zmq_ctx_destroy (context);

  if(command != GETINFO) {
    if(reply_string.compare("OK")) {
      TRACE(TR_ERROR, std::string("BernCRTZMQData::febctl_bitstreams Received unexpected reply from febdrv: ") + reply_string);
    }
    else {
      TRACE(TR_DEBUG, std::string("BernCRTZMQData::febctl_bitstreams Received reply: ") + reply_string);
    }
  }
  else { //check if the boards we see correspond to the FHICL file configuration
    TRACE(TR_DEBUG, std::string("BernCRTZMQData::febctl_bitstreams Received reply: ") + reply_string);
    std::istringstream ireply_string(reply_string);
    std::string line;
    if(!std::getline(ireply_string, line)) {
      TRACE(TR_ERROR, "BernCRTZMQData::febctl_bitstreams Received empty reply febdrv!");
    }
    else { //parse febdrv reply
      //parse number of reported FEBs
      const unsigned int read_nFEBs = stoi(line);
      if(read_nFEBs != nFEBs()) {
        TRACE(TR_ERROR, std::string("BernCRTZMQData::febctl_bitstreams Number of FEBs seen by febdrv (") + std::to_string(read_nFEBs) + ") differs from the ones defined in FCL file (" + std::to_string(nFEBs()) + ")!");
      }
      else {
        //read firmwares and mac addresses MAC addresses
        std::vector <std::string> firmwares;
        std::vector <uint64_t> mac5s;
        while (std::getline(ireply_string, line)) {
          firmwares.push_back(line.substr(18, 100));
          unsigned int mac[6];
          if (sscanf(line.substr(0,17).c_str(), "%x:%x:%x:%x:%x:%x", &mac[5], &mac[4], &mac[3], &mac[2], &mac[1], &mac[0]) != 6) {
            TRACE(TR_ERROR, std::string("BernCRTZMQData::febctl_bitstreams Error during parsing MAC address returned by febdrv (" + line +  ")!"));
          }
          else {
            TRACE(TR_DEBUG, std::string("BernCRTZMQData::febctl_bitstreams Raw mac address is: ") + std::to_string(mac[5]) +":"+ std::to_string(mac[4]) +":"+ std::to_string(mac[3]) +":"+ std::to_string(mac[2]) +":"+ std::to_string(mac[1]) +":"+ std::to_string(mac[0]));
            uint64_t full_mac = 0;
            for(int i = 0; i < 6; i++) {
              full_mac += (uint64_t)mac[i] << i * 8;
            }
            mac5s.push_back(full_mac);
          }
        }
        //check if all MAC addresses are present in the FHiCL configuration
        for(unsigned int iFEB = 0; iFEB < mac5s.size(); iFEB++) {
          bool config_found = false;
          for(unsigned int jFEB = 0; jFEB < nFEBs(); jFEB++) {
            if(mac5s[iFEB] == FEBIDs_[jFEB]) {
              config_found = true;
            }
          }
          if(!config_found) {
            TRACE(TR_ERROR, std::string("BernCRTZMQData::febctl_bitstreams Configuration for MAC address seen by febdrv (" + std::to_string(mac5s[iFEB]) +  ") missing in the FCL file!"));
          }
        }
      }
    }
  }
}

int sbndaq::BernCRTZMQData::ConvertASCIIToBitstream(std::string ASCII_bitstream, uint8_t *buffer) {
/**
 * Converts bitstream saved in ASCII format to an actuall bitstream
 * Bitstream is returned via argument buffer
 * Returned value is the length of the bitstream (number of bits)
 * The format of the ASCII stream is the following:
 * Read '0' and '1' characters, ignoring spaces until you encounter character different than '0', '1' or ' ', then skip to the next line.
 */

  const int MAXPACKLEN = 1500; //TODO: this should be a global variable, or something...
  memset(buffer,0,MAXPACKLEN); //reset buffer

  int length = 0;
  std::istringstream iASCII_bitstream(ASCII_bitstream);
  std::string line;    
  while (std::getline(iASCII_bitstream, line)) { //loop over lines
    for(char& c : line) { //loop over characters
      if(c == ' ') continue; //ignore blank characters
      if(c == '0' || c == '1') { //encode the bit into the bitstream
        const int byte = (++length) / 8;
        const int bit = length % 8;
        if(length % 8) buffer[byte] = 0; //clear new byte
        if(c == '1')  buffer[byte] |= 1 << bit;
      }
      else break; //if the character is not a space, '0' or '1', go to next line
    }
  }

  return length;
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
   * 
   * TODO:
   *  - Handle return string from the function
   *  - Do something during unexpected events (throw exceptions?)
   *  - think of a more clever way of passing the bitstreams to this function. Probably they should have their own class/object
   */

  const int MAXPACKLEN = 1500; //TODO this should be a global variable or something
  uint8_t Probe_bitStream[MAXPACKLEN], SlowControl_bitStream[MAXPACKLEN];
  const int probe_length = ConvertASCIIToBitstream(ps_.get<std::string>("CITIROC_Probe_bitStream"), Probe_bitStream);
  const int sc_length    = ConvertASCIIToBitstream(ps_.get<std::string>("CITIROC_SlowControl_bitStream0"), SlowControl_bitStream);

  uint8_t mac5;
  if(FEBIDs_.size() <= iFEB) {
    TRACE(TR_ERROR, std::string("BernCRTZMQData::febctl Could not find FEB ") + std::to_string(iFEB) + " in the FEBIDs!");
    return;
    //throw exception? return special code?
  }
  else {
    mac5 = FEBIDs_[iFEB] & 0xff;
  }

  if(mac5==255) {
    TRACE(TR_ERROR, "BernCRTZMQData::feb_send_bitstreams Bitstreams cannot be sent to mac5 = 255!");
    return;
  }

  if(probe_length != PROBE_BITSTREAM_LENGTH) {
    TRACE(TR_ERROR, std::string("BernCRTZMQData::feb_send_bitstreams Probe bitstream length incorrect: ") + std::to_string(probe_length) + " (expected " + std::to_string(PROBE_BITSTREAM_LENGTH) + ")");
    return;
  }
  if(sc_length != SLOW_CONTROL_BITSTREAM_LENGTH) {
    TRACE(TR_ERROR, std::string("BernCRTZMQData::feb_send_bitstreams Slow Control bitstream length incorrect: ") + std::to_string(sc_length) + " (expected " + std::to_string(SLOW_CONTROL_BITSTREAM_LENGTH)+")");
    return;
  }

  void * context = zmq_ctx_new ();

  TRACE(TR_DEBUG, "BernCRTZMQData::feb_send_bitstreams Connecting to febdrv...");
  void *requester = zmq_socket (context, ZMQ_REQ);
  const int linger = 0;
  zmq_setsockopt(requester, ZMQ_LINGER, &linger, sizeof(linger));
  zmq_connect (requester, zmq_listening_port_.c_str());

  char cmd[32];

  sprintf(cmd,"SETCONF");
  cmd[8]=mac5;

  uint8_t buffer[MAXPACKLEN];

  TRACE(TR_DEBUG, std::string("BernCRTZMQData::feb_send_bitstreams Sending command ") + cmd + "...");

  memcpy(buffer,cmd,9);
  memcpy(buffer+9,SlowControl_bitStream,SLOW_CONTROL_BITSTREAM_LENGTH/8);
  memcpy(buffer+9+SLOW_CONTROL_BITSTREAM_LENGTH/8,Probe_bitStream,PROBE_BITSTREAM_LENGTH/8);
  zmq_send ( requester, buffer, (SLOW_CONTROL_BITSTREAM_LENGTH+PROBE_BITSTREAM_LENGTH)/8+9, 0);

  TRACE(TR_DEBUG, "BernCRTZMQData::feb_send_bitstreams Waiting for reply...");

  zmq_msg_t reply;
  zmq_msg_init (&reply);

  const int timeout = 5; //s hardcoded value, move to config file?
  const int attempts = 10;
  int rv = zmq_msg_recv (&reply, requester, ZMQ_DONTWAIT);
  for(int i = 0; i < attempts && rv < 0; i++) {
    usleep(timeout*1000000/attempts);
    rv = zmq_msg_recv (&reply, requester, ZMQ_DONTWAIT);
  }
  if(rv<0) {
    TRACE(TR_ERROR, std::string("BernCRTZMQData::")+__func__+" Received no reply after waiting for " + std::to_string(timeout) + " seconds");
    zmq_msg_close (&reply);
    zmq_close (requester);
    zmq_ctx_destroy (context);
    return;
  }


  std::string reply_string = (char*)zmq_msg_data(&reply);
  if(reply_string.compare("OK")) {
    TRACE(TR_ERROR, std::string("BernCRTZMQData::febctl_send_bitstreams Received unexpected reply from febdrv: ") + reply_string);
  }
  else {
    TRACE(TR_DEBUG, std::string("BernCRTZMQData::febctl_send_bitstreams Received reply: ") + reply_string);
  }
  TRACE(TR_DEBUG, std::string("BernCRTZMQData::feb_send_bitstreams Received reply: ") + reply_string);
  zmq_msg_close (&reply);
  zmq_close (requester);
  zmq_ctx_destroy (context);


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
      TRACE(TR_ERROR, std::string("BernCRTZMQData::GetFEBData() called ") +  std::to_string(errno) + " "  + strerror(errno));
    }
    usleep(1000);
    //return 0;
    ++wait_count;
    
    if( (wait_count%500) == 0 )
    {
      TRACE(TR_LOG, std::string("\twait count: ") + std::to_string(wait_count));
    }
  }
  
  TRACE(TR_LOG, std::string("\toutside wait loop: ") + std::to_string(wait_count));
  
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
