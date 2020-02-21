#define TRACE_NAME "ICARUSTriggerUDP"
#include "sbndaq-artdaq-core/Trace/trace_defines.h"

#include "artdaq/DAQdata/Globals.hh"

#include "sbndaq-artdaq/Generators/ICARUS/ICARUSTriggerUDP.hh"

#include "canvas/Utilities/Exception.h"
#include "artdaq/Generators/GeneratorMacros.hh"
#include "cetlib_except/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "artdaq-core/Utilities/SimpleLookupPolicy.hh"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <fstream>
#include <iomanip>
#include <iterator>
#include <iostream>
#include <sys/poll.h>


sbndaq::ICARUSTriggerUDP::ICARUSTriggerUDP(fhicl::ParameterSet const& ps)
  : CommandableFragmentGenerator(ps)
  , dataport_(ps.get<int>("port", 6343))
  , ip_(ps.get<std::string>("ip", "127.0.0.1"))
  , n_init_retries_(ps.get<int>("n_init_retries",10))
  , n_init_timeout_ms_(ps.get<size_t>("n_init_timeout_ms",1000))
{
  datasocket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (datasocket_ < 0)
    {
      throw art::Exception(art::errors::Configuration) << "ICARUSTriggerUDP: Error creating socket!" << std::endl;
      exit(1);
    }
  struct sockaddr_in si_me_data;
  si_me_data.sin_family = AF_INET;
  si_me_data.sin_port = htons(dataport_);
  si_me_data.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(datasocket_, (struct sockaddr *)&si_me_data, sizeof(si_me_data)) == -1)
    {
      throw art::Exception(art::errors::Configuration) <<
	"ICARUSTriggerUDP: Cannot bind data socket to port " << dataport_ << std::endl;
      exit(1);
    }
  si_data_.sin_family = AF_INET;
  si_data_.sin_port = htons(dataport_);
  if (inet_aton(ip_.c_str(), &si_data_.sin_addr) == 0)
    {
      throw art::Exception(art::errors::Configuration) <<
	"ICARUSTriggerUDP: Could not translate provided IP Address: " << ip_ << "\n";
      exit(1);
    }
}
bool sbndaq::ICARUSTriggerUDP::getNext_(artdaq::FragmentPtrs& )//frags)
{
  if (should_stop()){
    return false;
  }

  /*
    //do something in here to get data...
   */

  poll_with_timeout(100);
  /*
  int size_bytes = poll_with_timeout(sleep_time_ms);
  if(size_bytes>0){
    //uint16_t buffer[size_bytes/2+1];
    char buffer[size_bytes];
    read(size_bytes,buffer);
    TRACE(TR_LOG,"received:: %s",buffer);
  } 
  */   
//send_TRIG_ALLW();
  
  return true;

}
void sbndaq::ICARUSTriggerUDP::start()
{
  send_TTLK_INIT(n_init_retries_,n_init_timeout_ms_);
  //send_TRIG_ALLW();
}
void sbndaq::ICARUSTriggerUDP::stop()
{
  //send_TRIG_VETO();
}
void sbndaq::ICARUSTriggerUDP::pause()
{
  //send_TRIG_VETO();
}
void sbndaq::ICARUSTriggerUDP::resume()
{
  //send_TRIG_ALLW();
}

//send a command
void sbndaq::ICARUSTriggerUDP::send(const Command_t cmd)
{
  TRACE(TR_LOG,"send:: COMMAND %s to %s:%d\n",cmd,ip_.c_str(),dataport_);  

  sendto(datasocket_,&cmd,sizeof(Command_t), 0, (struct sockaddr *) &si_data_, sizeof(si_data_));

  TRACE(TR_LOG,"send:: COMMAND %s to %s:%d\n",cmd,ip_.c_str(),dataport_);  
}

//return size of available data
int sbndaq::ICARUSTriggerUDP::poll_with_timeout(int timeout_ms)
{

  TRACE(TR_LOG,"poll:: DATA from %s with %d ms timeout",ip_.c_str(),timeout_ms);

  struct pollfd ufds[1];
  ufds[0].fd = datasocket_;
  ufds[0].events = POLLIN | POLLPRI;
  int rv = poll(ufds, 1, timeout_ms);

  //have something
  if (rv > 0){
    TRACE(TR_LOG,"poll:: rv=%d with revents=%d",rv,ufds[0].revents);
    
    //have something good
    if (ufds[0].revents == POLLIN || ufds[0].revents == POLLPRI){
      //do something to get data size here?
      
      /*
	uint8_t peekBuffer[2];
	recvfrom(datasocket_, peekBuffer, sizeof(peekBuffer), MSG_PEEK,
	(struct sockaddr *) &si_data_, (socklen_t*)sizeof(si_data_));
	return (int)(peekBuffer[1]);
      */
      return 8;
    }
  }
  //timeout
  else if(rv==0){
    TRACE(TR_LOG,"poll:: timed out after %d ms (rv=%d)",timeout_ms,rv);
    return 0;
  }
  //error
  else{
    TRACE(TR_ERROR,"poll:: error in poll (rv=%d)",rv);
    return -1;
  }

  return -1;

}

//read data size from socket
int sbndaq::ICARUSTriggerUDP::read(int size, char* buffer){
  TRACE(TR_LOG,"read:: get %d bytes from %s\n",size,ip_.c_str());
  int size_rcv = recvfrom(datasocket_, buffer, size, 0, (struct sockaddr *) &si_data_, (socklen_t*)sizeof(si_data_));
  
  if(size_rcv<0) 
    TRACE(TR_ERROR,"read:: error receiving data (%d bytes from %s)\n",size_rcv,ip_.c_str());
  else
    TRACE(TR_LOG,"read:: received %d bytes from %s\n",size_rcv,ip_.c_str());

  return size_rcv;
}


int sbndaq::ICARUSTriggerUDP::send_TTLK_INIT(int retries, int sleep_time_ms)
{
  while(retries>-1){
  
  char cmd[16];
  sprintf(cmd,"%s","TTLK_CMD_INIT");

  TRACE(TR_LOG,"send:: COMMAND %s to %s:%d\n",cmd,ip_.c_str(),dataport_);  
  sendto(datasocket_,&cmd,16, 0, (struct sockaddr *) &si_data_, sizeof(si_data_));

  TRACE(TR_LOG,"send:: COMMAND %s to %s:%d\n",cmd,ip_.c_str(),dataport_);  

  //send(TTLK_INIT);
    int size_bytes = poll_with_timeout(sleep_time_ms);
    if(size_bytes>0){
      //uint16_t buffer[size_bytes/2+1];
      char buffer[size_bytes];
      read(size_bytes,buffer);
      TRACE(TR_LOG,"received:: %s",buffer);
      
      return retries;
    }

    retries--;
  }

  return retries;

}

//no need for confirmation on these...
void sbndaq::ICARUSTriggerUDP::send_TRIG_VETO()
{
  send(TRIG_VETO);
}

void sbndaq::ICARUSTriggerUDP::send_TRIG_ALLW()
{
  send(TRIG_ALLW);
}

// The following macro is defined in artdaq's GeneratorMacros.hh header
DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(sbndaq::ICARUSTriggerUDP)
