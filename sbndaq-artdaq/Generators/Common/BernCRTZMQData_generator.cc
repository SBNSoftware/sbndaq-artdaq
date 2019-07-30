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
  TLOG_INFO(id) << "BernCRTZMQData constructor called" << TLOG_ENDL; 
 
  zmq_data_pub_port_           = ps_.get<std::string>("zmq_data_pub_port");
  zmq_data_receive_timeout_ms_ = ps_.get<int>("zmq_data_receive_timeout_ms",500);

  febctl(DAQ_BEG, 82); //note, the mac address is presently hardcoded for tests at DAB, this is must be changed later of course

  TLOG_INFO(id) << "BernCRTZMQData constructor : Calling zmq subscriber with port " << zmq_data_pub_port_.c_str() << TLOG_ENDL;
  
  zmq_context_    = zmq_ctx_new();

  TLOG_INFO(id) << "BernCRTZMQData constructor completed" << TLOG_ENDL;  
}

sbndaq::BernCRTZMQData::~BernCRTZMQData()
{
  TLOG_INFO(id) << "BernCRTZMQData destructor called" << TLOG_ENDL;  
  zmq_ctx_destroy(zmq_context_);


  febctl(DAQ_END, 82); //warning, fixed mac address!!!

  Cleanup();
  TLOG_INFO(id) << "BernCRTZMQData destructor completed" << TLOG_ENDL;  
}



void sbndaq::BernCRTZMQData::ConfigureStart()
{
  TLOG_INFO(id) << "BernCRTZMQData::ConfigureStart() called" << TLOG_ENDL;  

  zmq_subscriber_ = zmq_socket(zmq_context_,ZMQ_SUB);

  int res=0;

  res = zmq_connect(zmq_subscriber_,zmq_data_pub_port_.c_str());
  if(res!=0)
    TLOG_ERROR(id) << "BernCRTZMQDataZMQ::ConfigureStart() failed to connect." << TLOG_ENDL;

  res = zmq_setsockopt(zmq_subscriber_,ZMQ_SUBSCRIBE,NULL,0);
  //res = zmq_setsockopt(zmq_subscriber_,ZMQ_RCVTIMEO,&zmq_data_receive_timeout_ms_,2);

  if(res!=0)
    TLOG_ERROR(id) << "BernCRTZMQDataZMQ::ConfigureStart() socket options failed." << TLOG_ENDL;

  TLOG_INFO(id) << "BernCRTZMQData::ConfigureStart() completed" << TLOG_ENDL;  
}

void sbndaq::BernCRTZMQData::ConfigureStop()
{
  TLOG_INFO(id) << "BernCRTZMQData::ConfigureStop() called" << TLOG_ENDL;

  zmq_close(zmq_subscriber_);

  TLOG_INFO(id) << "BernCRTZMQData::ConfigureStop() completed" << TLOG_ENDL;  
}

int sbndaq::BernCRTZMQData::GetDataSetup()
{
  return 1;
}

int sbndaq::BernCRTZMQData::GetDataComplete()
{
  return 1;
}

void sbndaq::BernCRTZMQData::febctl(feb_command command, unsigned char mac5) {
/**
 * Copied functionality of febctl tool from febdriver
 * Sends a command to febdrv.
 * Arguments:
 * - command: BIAS_OF BIAS_ON DAQ_BEG DAQ_END GETINFO
 * - mac5 is the last 8 bits of the mac address of the FEB
 * 
 * TODO:
 *  - Somehow pass the socket string, e.g. by an argument, presently it's hardcoded
 *  - Handle return string from the function (we need if with GETINFO)
 *  - Do something during unexpected events (throw exceptions?)
 */
  
  char command_string[8];
  switch (command) {
    case DAQ_BEG : strcpy ( command_string, "DAQ_BEG");   break;
    case DAQ_END : strcpy ( command_string, "DAQ_END");   break;
    case BIAS_ON : strcpy ( command_string, "BIAS_ON");   break;
    case BIAS_OF : strcpy ( command_string, "BIAS_OF");   break;
    case GETINFO : strcpy ( command_string, "GETINFO");   break;
    default:
      TLOG_ERROR(id)   << "BernCRTZMQData::febctl Command "<<command<<" unrecognized!" << TLOG_ENDL;
      return;

  }
  
  void * context = zmq_ctx_new ();

  char socket[] = "tcp://localhost:5555"; //warning: hardcoded value, to be updated!

  //  Socket to talk to server
  TLOG_DEBUG(id)   << "BernCRTZMQData::febctl Connecting to febdrv..." << TLOG_ENDL;
  void *requester = zmq_socket (context, ZMQ_REQ);
  if(zmq_connect (requester, socket) < 0) {
    TLOG_ERROR(id)   << "BernCRTZMQData::febctl Connection to "<<socket<<" failed!" << TLOG_ENDL;
    return;
    //throw exception? return special code?
  }

  //zmq_connect (requester, "ipc://command");
  zmq_msg_t request;
  zmq_msg_init_size (&request, 9);
  memcpy(zmq_msg_data (&request), command_string,7);
  ((uint8_t*)zmq_msg_data (&request))[7]=0;
  ((uint8_t*)zmq_msg_data (&request))[8]=mac5;
  TLOG_DEBUG(id) << "BernCRTZMQData::febctl Sending command \"" << command_string << "\" to mac5 = " << mac5 << TLOG_ENDL;
  zmq_msg_send (&request, requester, 0);
  zmq_msg_close (&request);
  zmq_msg_t reply;
  zmq_msg_init (&reply);
  zmq_msg_recv (&reply, requester, 0);
  TLOG_DEBUG(id) << "BernCRTZMQData::febctl Received reply: "<< (char*)zmq_msg_data (&reply) << TLOG_ENDL;
  zmq_msg_close (&reply);
  zmq_close (requester);
  zmq_ctx_destroy (context);
}

void sbndaq::BernCRTZMQData::feb_send_bitstreams(char * Probe_bitStream, char * SlowControl_bitStream, unsigned char mac5) {
/**
 * Sends configuration bitstream to febdriver
 * Based on febconf main()
 * Arguments:
 * - Probe_bitStream - monitoring configuration. It consists of 224 bits, typically all '0' for normal operation 
 * - SlowControl_bitStream - actual configuration to be sent to the board, 1144 bits
 * the bitstreams are expected to be actual strings of binary numbers. There is no real way to check their validity at this point
 * 
 * TODO:
 *  - Somehow pass the socket string, e.g. by an argument, presently it's hardcoded
 *  - Handle return string from the function (we need if with GETINFO)
 *  - Do something during unexpected events (throw exceptions?)
 *  - think of a more clever way of passing the bitstreams to this function. Probably they should have their own class/object
 */

  if(mac5==255)
  {
    TLOG_ERROR(id)   << "BernCRTZMQData::feb_send_bitstreams Bitstreams cannot be sent to mac5 = 255!" << TLOG_ENDL;
    return;
  }

  char socket[] = "tcp://localhost:5555"; //warning: hardcoded value, to be updated!

  void * context = zmq_ctx_new ();

  //  Socket to talk to server
  TLOG_DEBUG(id)   << "BernCRTZMQData::feb_send_bitstreams Connecting to febdrv..."<< TLOG_ENDL;
  void *requester = zmq_socket (context, ZMQ_REQ);
  zmq_connect (requester, socket);

  char cmd[32];

  sprintf(cmd,"SETCONF");
  cmd[8]=mac5;

  const int MAXPACKLEN = 1500;
  uint8_t buffer[MAXPACKLEN];

  TLOG_DEBUG(id)   << "BernCRTZMQData::feb_send_bitstreams Sending command "<<cmd<<"..."<< TLOG_ENDL;

  memcpy(buffer,cmd,9);
  memcpy(buffer+9,SlowControl_bitStream,SLOW_CONTROL_BITSTREAM_LENGTH/8);
  memcpy(buffer+9+SLOW_CONTROL_BITSTREAM_LENGTH/8,Probe_bitStream,PROBE_BITSTREAM_LENGTH/8);
  zmq_send ( requester, buffer, (SLOW_CONTROL_BITSTREAM_LENGTH+PROBE_BITSTREAM_LENGTH)/8+9, 0);

  TLOG_DEBUG(id)   << "BernCRTZMQData::feb_send_bitstreams Waiting for reply..."<< TLOG_ENDL;

  zmq_msg_t reply;
  zmq_msg_init (&reply);
  zmq_msg_recv (&reply, requester, 0);
  TLOG_DEBUG(id)   << "BernCRTZMQData::feb_send_bitstreams Received reply: "<<(char*)zmq_msg_data (&reply)<< TLOG_ENDL;
  zmq_msg_close (&reply);
  zmq_close (requester);
  zmq_ctx_destroy (context);


}

/*---------------BERN CRT ZMQ DATA-------------------------------------*/
size_t sbndaq::BernCRTZMQData::GetZMQData()
{
  std::cout << "Calling GetZMQData" << std::endl;
  
  TLOG_INFO(id) << "BernCRTZMQData::GetZMQData called" << TLOG_ENDL;
  
  size_t data_size=0;
  size_t events=0;
  
  size_t wait_count=0;
  
  zmq_msg_t feb_data_msg;
  zmq_msg_init(&feb_data_msg);

  while(zmq_msg_recv(&feb_data_msg,zmq_subscriber_,ZMQ_DONTWAIT) < 0)
  {
    if ( errno != EAGAIN ) // No data awailable now
    {
      TLOG_ERROR(id) << "BernCRTZMQData::GetFEBData() called " << errno << " " << strerror(errno) << TLOG_ENDL;
    }
    usleep(1000);
    //return 0;
    ++wait_count;
    
    if( (wait_count%500) == 0 )
    {
      TLOG_INFO(id) << "\twait count: " << wait_count << TLOG_ENDL;
    }
  }
  
  TLOG_INFO(id) << "\toutside wait loop: " << wait_count << TLOG_ENDL;
  
  if(zmq_msg_size(&feb_data_msg)>0)
  {
    TLOG_INFO(id) << "BernCRTZMQData::GetZMQData() about to copy" << TLOG_ENDL;
    
    std::copy((uint8_t*)zmq_msg_data(&feb_data_msg),
	      (uint8_t*)zmq_msg_data(&feb_data_msg)+zmq_msg_size(&feb_data_msg), //last event contains time info
	      reinterpret_cast<uint8_t*>(&ZMQBufferUPtr[events]));
    
    TLOG_INFO(id) << "BernCRTZMQData::GetZMQData() copied!" << TLOG_ENDL;
    
    events += zmq_msg_size(&feb_data_msg)/sizeof(BernCRTZMQEvent);
    data_size += zmq_msg_size(&feb_data_msg);

    TLOG_INFO(id) << "BernCRTZMQData::GetZMQData() copied " << events << " events (" << data_size << " size)" << TLOG_ENDL;

    //check : is this too much data for the buffer?
    if( events>ZMQBufferCapacity_ )
    {
      TLOG_ERROR(id) << "BernCRTZMQData::GetZMQData : Too many events in ZMQ buffer! " << events << TLOG_ENDL;
      throw cet::exception("In BernCRTZMQData::GetZMQData, Too many events in ZMQ buffer!");
    }
  }

  zmq_msg_close(&feb_data_msg);

  TLOG_INFO(id) << "BernCRTZMQData::GetZMQData() size returned was " << data_size << TLOG_ENDL;

  return data_size;

}

DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(sbndaq::BernCRTZMQData) 
