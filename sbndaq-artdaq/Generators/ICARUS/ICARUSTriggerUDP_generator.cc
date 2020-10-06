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

#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"

#include <fstream>
#include <iomanip>
#include <iterator>
#include <iostream>
#include <string>
#include <sys/poll.h>
#include <time.h>

sbndaq::ICARUSTriggerUDP::ICARUSTriggerUDP(fhicl::ParameterSet const& ps)
  : CommandableFragmentGenerator(ps)
  , fragment_id_(ps.get<uint32_t>("fragment_id", 0x4001))
  , fragment_id_pmt_(ps.get<uint32_t>("fragment_id_pmt", 0x4001))
  , max_fragment_size_bytes_(ps.get<size_t>("max_fragment_size_bytes", 100))
  , max_fragment_size_bytes_pmt_(ps.get<size_t>("max_fragment_size_bytes_pmt", 100))
  , configport_(ps.get<int>("config_port", 63001))
  , ip_config_(ps.get<std::string>("config_ip", "127.0.0.1"))
  , dataport_(ps.get<int>("data_port", 63000))
  , ip_data_(ps.get<std::string>("data_ip", "127.0.0.1"))
  , pmtdataport_(ps.get<int>("data_port_pmt", 63002))
  , ip_data_pmt_(ps.get<std::string>("data_pmt_ip", "127.0.0.1"))
  , n_init_retries_(ps.get<int>("n_init_retries",10))
  , n_init_timeout_ms_(ps.get<size_t>("n_init_timeout_ms",1000))
{
  
  configsocket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (configsocket_ < 0)
    {
      throw art::Exception(art::errors::Configuration) << "ICARUSTriggerUDP: Error creating socket!" << std::endl;
      exit(1);
    }
  
  struct sockaddr_in si_me_config;
  si_me_config.sin_family = AF_INET;
  si_me_config.sin_port = htons(configport_);
  si_me_config.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(configsocket_, (struct sockaddr *)&si_me_config, sizeof(si_me_config)) == -1)
    {
      throw art::Exception(art::errors::Configuration) <<
	"ICARUSTriggerUDP: Cannot bind config socket to port " << configport_ << std::endl;
      exit(1);
    }
  si_config_.sin_family = AF_INET;
  si_config_.sin_port = htons(configport_);
  if (inet_aton(ip_config_.c_str(), &si_config_.sin_addr) == 0)
    {
      throw art::Exception(art::errors::Configuration) <<
	"ICARUSTriggerUDP: Could not translate provided IP Address: " << ip_config_ << "\n";
      exit(1);
    }
  
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
  if (inet_aton(ip_data_.c_str(), &si_data_.sin_addr) == 0)
    {
      throw art::Exception(art::errors::Configuration) <<
	"ICARUSTriggerUDP: Could not translate provided IP Address: " << ip_data_ << "\n";
      exit(1);
    }
  
  pmtsocket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); 
  if(pmtsocket_ < 0)
    {
      throw art::Exception(art::errors::Configuration) << "ICARUSPMTGate: Error creating socket!" << std::endl;
      exit(1);
    }
  struct sockaddr_in si_me_pmtdata;
  si_me_pmtdata.sin_family = AF_INET;
  si_me_pmtdata.sin_port = htons(pmtdataport_);
  si_me_pmtdata.sin_addr.s_addr = htonl(INADDR_ANY);
  if(bind(pmtsocket_, (struct sockaddr *)&si_me_pmtdata, sizeof(si_me_pmtdata)) == -1)
    {
      throw art::Exception(art::errors::Configuration) << "ICARUSPMTGate: Cannot bind config socket to port " << pmtdataport_ << std::endl;
      exit(1);
    }
  si_pmtdata_.sin_family = AF_INET;
  si_pmtdata_.sin_port = htons(pmtdataport_);
  if(inet_aton(ip_data_pmt_.c_str(), &si_pmtdata_.sin_addr) == 0)
    {
      throw art::Exception(art::errors::Configuration) << "ICARUSPMTGate: Error creating socket!" << std::endl;
      exit(1);
    }
  
  fEventCounter = 1;
}
bool sbndaq::ICARUSTriggerUDP::getNext_(artdaq::FragmentPtrs& frags)
{
  static auto start= std::chrono::steady_clock::now();

  if (should_stop()){
    return false;
  }

  /*
    //do something in here to get data...
   */

  artdaq::Fragment::timestamp_t ts(0);
  {
    using namespace boost::gregorian;
    using namespace boost::posix_time;

    ptime t_now(microsec_clock::universal_time());
    ptime time_t_epoch(date(1970,1,1));
    time_duration diff = t_now - time_t_epoch;

    ts = diff.total_nanoseconds();
  }

  int size_bytes = poll_with_timeout(datasocket_,ip_data_,500);
  //int buffersize = 0;
  std::string data_input = "";
  char buffer[size_bytes];
  if(size_bytes>0){
    //char buffer[size_bytes];
    read(datasocket_,ip_data_,si_data_,size_bytes,buffer);
    TRACE(TR_LOG,"data received:: %s",buffer);
    //buffersize = sizeof(buffer)/sizeof(char);
    data_input = buffer;
  }

  size_t pos = 0;
  //size_t delim_pos = 0;
  TRACE(TR_LOG, "string received:: %s", data_input.c_str());
  std::string delimiter = ",";
  std::vector<std::string> sections;
  std::string token = "";
  while ((pos = data_input.find(delimiter)) != std::string::npos) {
    token = data_input.substr(0, pos);
    sections.push_back(token);
    data_input.erase(0, pos + delimiter.length());
    //delim_pos = pos;
  }
  sections.push_back(data_input);
  
  if(sections.size() >= 4)
  {
    std::string trig_name = sections[0];
    int trigger = -1;
    if(trig_name == "TS1")
      trigger = 1;
    int event_no = std::stoi(sections[1]);
    int secs = std::stoi(sections[2]);
    long nanosecs = std::stol(sections[3]);
    //std::cout << "trig_name:: " << trig_name << " event_number:: " << event_no << " seconds since start:: " << secs << " nanoseconds since start:: " << nanosecs << std::endl;
    
    //TRACE(TR_LOG, "trig_name:: %s", trig_name);
    //TRACE(TR_LOG, "event_number:: %i", event_no);
    //TRACE(TR_LOG, "seconds since start:: %i", secs);
    //TRACE(TR_LOG, "nanoseconds since start:: %h", nanosecs);    }

    //Add in fragment details and fragment filling function, want a fragment to contain all of the variables arriving with the trigger                                                                                    //Put user variables in metadata, maybe except trigger name, try all at first and might be doing not quite correctly    
    const auto metadata = icarus::ICARUSTriggerUDPFragmentMetadata(trigger, event_no, secs, nanosecs);
    //const auto fragment_size = metadata.ExpectedDataSize();
    //Put data string in fragment -> make frag size size of data string, copy data string into fragment
    //Add timestamp, in number of nanoseconds, as extra argument to fragment after metadata. Add seconds and nanoseconds
    size_t fragment_size = max_fragment_size_bytes_;
    TRACE(TR_LOG, "Created ICARUSTriggerUDP Fragment with size of 100 bytes");
    frags.emplace_back(artdaq::Fragment::FragmentBytes(fragment_size, fEventCounter, fragment_id_, sbndaq::detail::FragmentType::ICARUSTriggerUDP, metadata, ts));
    std::copy(&buffer[0], &buffer[sizeof(buffer)/sizeof(char)], (char*)(frags.back()->dataBeginBytes())); //attempt to copy data string into fragment
    //auto frag = artdaq::Fragment::FragmentBytes(fragment_size, fEventCounter,fragment_id_,sbndaq::detail::FragmentType::ICARUSTriggerUDP, metadata);
    icarus::ICARUSTriggerUDPFragment const &newfrag = *frags.back();
    //const char *name = metadata.getName();
    int name = metadata.getName();
    int number = metadata.getEventNo();
    int sec_frag = metadata.getSeconds();
    long nanosec_frag = metadata.getNanoSeconds();
    std::cout << "Name: " << name << " Event Number: " << number << " Seconds: " << sec_frag << " Nanoseconds: " << nanosec_frag << std::endl; 
    //Old method, try new method which inserts data string from hardware into the fragment as well 
    //frags.emplace_back(nullptr);
    //std::swap(frags.back(), frag);
    /*
    size_bytes = poll_with_timeout(pmtsocket_, ip_data_pmt_, 500);
    data_input = "";
    char buffer_pmt[size_bytes];
    if(size_bytes>0)
      {
	read(pmtsocket_, ip_data_pmt_, si_pmtdata_, size_bytes, buffer_pmt);
	TRACE(TR_LOG,"data received:: %s", buffer_pmt);
	data_input = buffer_pmt;
	TRACE(TR_LOG,"string received:: %s", data_input.c_str());
      }
    size_t pos = 0;

    delimiter = ",";
    std::vector<std::string> sections_pmt;
    token = "";
    while ((pos = data_input.find(delimiter)) != std::string::npos) {
      token = data_input.substr(0, pos);
      sections_pmt.push_back(token);
      data_input.erase(0, pos + delimiter.length());
    }
    if(sections_pmt.size() > 0)
    {
      std::vector<int> mult;
      for(unsigned int i = 0; i < sections_pmt.size(); ++i)
      {
	int pmt_mult = std::stoi(sections_pmt[i]);
	mult.push_back(pmt_mult);
      }
      const auto metadata_pmt = icarus::ICARUSPMTGateFragmentMetadata(mult);
      fragment_size = max_fragment_size_bytes_pmt_;
      TRACE(TR_LOG, "Created ICARUSPMTGate Fragment with size of 500 bytes");
      frags.emplace_back(artdaq::Fragment::FragmentBytes(fragment_size, fEventCounter, fragment_id_pmt_, sbndaq::detail::FragmentType::ICARUSPMTGate, metadata_pmt, ts));
      std::copy(&buffer_pmt[0], &buffer_pmt[sizeof(buffer_pmt)/sizeof(char)], (char*)(frags.back()->dataBeginBytes()));
    }
    */
    ++fEventCounter;
  }

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
  std::chrono::duration<double> delta = std::chrono::steady_clock::now()-start;
  std::cout << "getNext_ function takes: " << delta.count() << " seconds." << std::endl; 
  return true;

}


void sbndaq::ICARUSTriggerUDP::start()
{
  send_TTLK_INIT(n_init_retries_,n_init_timeout_ms_); //comment out for fake trigger tests
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
  TRACE(TR_LOG,"send:: COMMAND %s to %s:%d\n",cmd,ip_config_.c_str(),configport_);  

  sendto(configsocket_,&cmd,sizeof(Command_t), 0, (struct sockaddr *) &si_config_, sizeof(si_config_));

  TRACE(TR_LOG,"send:: COMMAND %s to %s:%d\n",cmd,ip_config_.c_str(),configport_);  
}

//return size of available data
int sbndaq::ICARUSTriggerUDP::poll_with_timeout(int socket, std::string ip, int timeout_ms)
{

  TRACE(TR_LOG,"poll:: DATA from %s with %d ms timeout",ip.c_str(),timeout_ms);

  struct pollfd ufds[1];
  ufds[0].fd = socket;
  ufds[0].events = POLLIN | POLLPRI;
  int rv = poll(ufds, 1, timeout_ms);

  //have something
  if (rv > 0){
    TRACE(TR_LOG,"poll:: rv=%d with revents=%d",rv,ufds[0].revents);
    
    //have something good
    if (ufds[0].revents == POLLIN || ufds[0].revents == POLLPRI){
      //do something to get data size here?
      
     
      uint8_t peekBuffer[2];
      recvfrom(datasocket_, peekBuffer, sizeof(peekBuffer), MSG_PEEK,
	       (struct sockaddr *) &si_data_, (socklen_t*)sizeof(si_data_));
      //std::cout << msg_size << std::endl;
      return (int)(peekBuffer[1]);
      //return sizeof(peekBuffer);
      //return sizeof(peekBuffer[1]);
      //return 1400;
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
int sbndaq::ICARUSTriggerUDP::read(int socket, std::string ip, struct sockaddr_in& si, int size, char* buffer){
  TRACE(TR_LOG,"read:: get %d bytes from %s\n",size,ip.c_str());
  int size_rcv = recvfrom(socket, buffer, size, 0, (struct sockaddr *) &si, (socklen_t*)sizeof(si));
  
  if(size_rcv<0) 
    TRACE(TR_ERROR,"read:: error receiving data (%d bytes from %s)\n",size_rcv,ip.c_str());
  else
    TRACE(TR_LOG,"read:: received %d bytes from %s\n",size_rcv,ip.c_str());

  return size_rcv;
}


int sbndaq::ICARUSTriggerUDP::send_TTLK_INIT(int retries, int sleep_time_ms)
{
  while(retries>-1){
  
  char cmd[16];
  sprintf(cmd,"%s","TTLK_CMD_INIT");

  TRACE(TR_LOG,"to send:: COMMAND %s to %s:%d\n",cmd,ip_config_.c_str(),configport_);  
  sendto(configsocket_,&cmd,16, 0, (struct sockaddr *) &si_config_, sizeof(si_config_));

  TRACE(TR_LOG,"sent! COMMAND %s to %s:%d\n",cmd,ip_config_.c_str(),configport_);  

  //send(TTLK_INIT);
  int size_bytes = poll_with_timeout(configsocket_,ip_config_,sleep_time_ms);
  if(size_bytes>0){
    //uint16_t buffer[size_bytes/2+1];
    char buffer[size_bytes];
    read(configsocket_,ip_config_,si_config_,size_bytes,buffer);
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
