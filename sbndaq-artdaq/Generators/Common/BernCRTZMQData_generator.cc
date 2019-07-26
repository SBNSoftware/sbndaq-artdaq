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

  TLOG_INFO(id) << "BernCRTZMQData constructor : Calling zmq subscriber with port " << zmq_data_pub_port_.c_str() << TLOG_ENDL;
  
  zmq_context_    = zmq_ctx_new();

  TLOG_INFO(id) << "BernCRTZMQData constructor completed" << TLOG_ENDL;  
}

sbndaq::BernCRTZMQData::~BernCRTZMQData()
{
  TLOG_INFO(id) << "BernCRTZMQData destructor called" << TLOG_ENDL;  
  zmq_ctx_destroy(zmq_context_);
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
