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
  , use_wr_time_(ps.get<bool>("use_wr_time"))
  , generated_fragments_per_event_(ps.get<int>("generated_fragments_per_event",0))
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
  //buffer = {'\0'};
  //peekBuffer = {0,0};
  /*
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
  */
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
  //std::cout << ts << std::endl;
  int size_bytes = poll_with_timeout(datasocket_,ip_data_, si_data_,500);
  //int size_bytes = poll_with_timeout(datasocket_,ip_data_,500);
  //int buffersize = 0;
  std::string data_input = "";
  //char buffer[size_bytes];
  //buffer[500] = {'\0'};
  buffer[0] = '\0';
  if(size_bytes>0){
    //char buffer[size_bytes];
    //buffer[size_bytes]='\0';
    //int x = read(datasocket_,ip_data_,si_data_,size_bytes,buffer);
    int x = read(datasocket_,ip_data_,si_data_,size_bytes,buffer);  
    TLOG(TLVL_DEBUG) << "x:: " << x << " errno:: " << errno << " data received:: " << buffer;
    //TLOG(TLVL_DEBUG) << "The error is:: " << strerror(errno);
    //TLOG(TLVL_DEBUG) << "data received:: " << buffer;
    //buffersize = sizeof(buffer)/sizeof(char);
    data_input = buffer;
  }

  size_t pos = 0;
  //size_t delim_pos = 0;
  TLOG(TLVL_DEBUG) << "string received:: " << data_input;
  //if shouldn't send fragments, then don't create fragment/send
  if(generated_fragments_per_event_==0){
    ++fEventCounter;
    return true;
  }


  std::string delimiter = ",";
  std::vector<std::string> sections;
  std::string token = "";
  while ((pos = data_input.find(delimiter)) != std::string::npos) {
    token = data_input.substr(0, pos);
    sections.push_back(token);
    data_input.erase(0, pos + delimiter.length());
  }
  sections.push_back(data_input);

  int trigger = -1;
  int wr_trig = -1;
  uint64_t event_no = fEventCounter;
  uint64_t event_no_wr = fEventCounter;
  uint64_t secs = -3;
  long wr_secs = -3;
  long nanosecs = -4;
  long wr_nsecs = -4;
  if(sections.size() >= 4)
  {
    std::string trig_name = sections[0];
    if(trig_name == "Local_TS1")
      trigger = 1;
    event_no = std::stoi(sections[1]);
    secs = std::stoi(sections[2]);
    nanosecs = std::stol(sections[3]);
    if(sections.size() > 5)
    {
      std::string wr_name = sections[4];
      if(wr_name == " WR_TS1")
	wr_trig = 2;
      event_no_wr = std::stoi(sections[5]);
      wr_secs = std::stol(sections[6]);
      wr_nsecs = std::stol(sections[7]);
      long long val = wr_secs*1e9+wr_nsecs;
      if(use_wr_time_)
	ts = val;
    }
    if(wr_trig == -1 || event_no_wr == -2 || wr_secs == -3 || wr_nsecs == -4)
      {
	TLOG(TLVL_WARNING) << "White Rabbit timestamp missing!";
      }
    //Add in fragment details and fragment filling function, want a fragment to contain all of the variables arriving with the trigger                                                                                    //Put user variables in metadata, maybe except trigger name, try all at first and might be doing not quite correctly    
    const auto metadata = icarus::ICARUSTriggerUDPFragmentMetadata(trigger, event_no, secs, nanosecs,wr_trig, event_no_wr, wr_secs, wr_nsecs);
    //const auto fragment_size = metadata.ExpectedDataSize();
    //Put data string in fragment -> make frag size size of data string, copy data string into fragment
    //Add timestamp, in number of nanoseconds, as extra argument to fragment after metadata. Add seconds and nanoseconds
    size_t fragment_size = max_fragment_size_bytes_;
    TLOG(TLVL_DEBUG) << "Created ICARUSTriggerUDP Fragment with size of 500 bytes";
    frags.emplace_back(artdaq::Fragment::FragmentBytes(fragment_size, event_no, fragment_id_, sbndaq::detail::FragmentType::ICARUSTriggerUDP, metadata, ts));
    //frags.emplace_back(artdaq::Fragment::FragmentBytes(fragment_size, fEventCounter, fragment_id_, sbndaq::detail::FragmentType::ICARUSTriggerUDP, metadata, ts));
    std::copy(&buffer[0], &buffer[sizeof(buffer)/sizeof(char)], (char*)(frags.back()->dataBeginBytes())); //attempt to copy data string into fragment
    icarus::ICARUSTriggerUDPFragment const &newfrag = *frags.back();
    //const char *name = metadata.getName();
    int name = metadata.getName();
    int number = metadata.getEventNo();
    int sec_frag = metadata.getSeconds();
    long nanosec_frag = metadata.getNanoSeconds();
    int wr_label = metadata.getWRName();
    int wr_num = metadata.getWREventNo();
    long wr_sec_frag = metadata.getWRSeconds();
    long wr_nsec_frag = metadata.getWRNanoSeconds();
    //std::cout << "Name: " << name << " Event Number: " << number << " Seconds: " << sec_frag << " Nanoseconds: " << nanosec_frag << " WR: " << wr_label << " WR Event: " << wr_num << " WR Seconds: " << wr_sec_frag << " WR Nanoseconds " << wr_nsec_frag << std::endl; 
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
      TRACE(TLVL_INFO,"data received:: %s", buffer_pmt);
      data_input = buffer_pmt;
      TRACE(TLVL_INFO,"string received:: %s", data_input.c_str());
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
      TRACE(TLVL_INFO, "Created ICARUSPMTGate Fragment with size of 500 bytes");
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
    TRACE(TLVL_INFO,"received:: %s",buffer);
  } 
  */   
//send_TRIG_ALLW();
  TLOG(TLVL_DEBUG) << "The artdaq timestamp value is: " << ts;
  std::chrono::duration<double> delta = std::chrono::steady_clock::now()-start;
  //std::cout << "getNext_ function takes: " << delta.count() << " seconds." << std::endl; 
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
  TLOG(TLVL_DEBUG) << "send:: COMMAND " << cmd << " to " << ip_config_.c_str() << ":" << configport_ << "\n";

  sendto(configsocket_,&cmd,sizeof(Command_t), 0, (struct sockaddr *) &si_config_, sizeof(si_config_));

  TLOG(TLVL_DEBUG) << "send:: COMMAND " << cmd << " to " << ip_config_.c_str() << ":" << configport_ << "\n";
}

//return size of available data
int sbndaq::ICARUSTriggerUDP::poll_with_timeout(int socket, std::string ip, struct sockaddr_in& si, int timeout_ms)
{

  TLOG(TLVL_DEBUG) << "poll:: DATA from " << ip.c_str() << " with " << timeout_ms << " ms timeout";
  struct pollfd ufds[1];
  ufds[0].fd = socket;
  ufds[0].events = POLLIN | POLLPRI;
  int rv = poll(ufds, 1, timeout_ms);

  //have something
  if (rv > 0){
    TLOG(TLVL_DEBUG) << "poll:: rv=" << rv << " with revents=" << ufds[0].revents;

    //have something good
    if (ufds[0].revents == POLLIN || ufds[0].revents == POLLPRI){
      //do something to get data size here?
      
     
      peekBuffer[1] = {0};
      //recvfrom(datasocket_, peekBuffer, sizeof(peekBuffer), MSG_PEEK,
      //(struct sockaddr *) &si_data_, (socklen_t*)sizeof(si_data_));
      socklen_t slen = sizeof(si);
      //int ret = recvfrom(socket, peekBuffer, sizeof(peekBuffer), MSG_PEEK,                               
      //(struct sockaddr *) &si, (socklen_t*)sizeof(si));
      int ret = recvfrom(socket, peekBuffer, sizeof(peekBuffer), MSG_PEEK,                                    
			 (struct sockaddr *) &si, &slen);    
      //std::cout << msg_size << std::endl;
      TLOG(TLVL_DEBUG) << "peek recvfrom:: " << ret << " " << errno;
      return (int)(peekBuffer[1]);
      //return sizeof(peekBuffer);
      //return sizeof(peekBuffer[1]);
      //return 1400;
    }
  }
  //timeout
  else if(rv==0){
    TLOG(TLVL_DEBUG) << "poll:: timed out after " << timeout_ms << " ms (rv=" << rv << ")";
    return 0;
  }
  //error
  else{
    TLOG(TLVL_ERROR) << "poll:: error in poll (rv=" << rv << ")";
    return -1;
  }

  return -1;

}

//read data size from socket
int sbndaq::ICARUSTriggerUDP::read(int socket, std::string ip, struct sockaddr_in& si, int size, char* buffer){
  TLOG(TLVL_DEBUG) << "read:: get " << size << " bytes from " << ip.c_str() << "\n";
  socklen_t slen = sizeof(si);
  //int size_rcv = recvfrom(socket, buffer, size, 0, (struct sockaddr *) &si, (socklen_t*)sizeof(si));
  int size_rcv = recvfrom(socket, buffer, size, 0, (struct sockaddr *) &si, &slen);
  
  if(size_rcv<0)
    TLOG(TLVL_ERROR) << "read:: error receiving data (" << size_rcv << " bytes from " << ip.c_str() << ")\n";
  else
    TLOG(TLVL_DEBUG) << "read:: received " << size_rcv << " bytes from " << ip.c_str() << "\n";

  return size_rcv;
}


int sbndaq::ICARUSTriggerUDP::send_TTLK_INIT(int retries, int sleep_time_ms)
{
  while(retries>-1){
  
  char cmd[16];
  sprintf(cmd,"%s","TTLK_CMD_INIT");

  TLOG(TLVL_DEBUG) << "to send:: COMMAND " << cmd << "to " << ip_config_.c_str() << ":" << configport_ << "\n"; 
  sendto(configsocket_,&cmd,16, 0, (struct sockaddr *) &si_config_, sizeof(si_config_));

  TLOG(TLVL_DEBUG) << "sent!:: COMMAND " << cmd << "to " << ip_config_.c_str() << ":" << configport_ << "\n";
  //send(TTLK_INIT);
  int size_bytes = poll_with_timeout(configsocket_,ip_config_, si_config_, sleep_time_ms);
  if(size_bytes>0){
    //uint16_t buffer[size_bytes/2+1];
    //char bufferinit[size_bytes];
    buffer[size_bytes+1] = {'\0'};
    read(configsocket_,ip_config_,si_config_,size_bytes,buffer);
    TLOG(TLVL_DEBUG) << "received:: " << buffer;
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
