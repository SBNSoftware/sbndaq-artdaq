#define TRACE_NAME "ICARUSTriggerUDP"
#include "sbndaq-artdaq-core/Trace/trace_defines.h"

#include "artdaq/DAQdata/Globals.hh"

#include "sbndaq-artdaq/Generators/ICARUS/ICARUSTriggerUDP.hh"
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
  , wr_time_offset_ns_(ps.get<long>("wr_time_offset_ns",2e9))
  , generated_fragments_per_event_(ps.get<int>("generated_fragments_per_event",0))
{

  configsocket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (configsocket_ < 0)
    {
      throw cet::exception("ICARUSTriggerUDP") << "ICARUSTriggerUDP: Error creating socket!" << std::endl;
      exit(1);
    }

  struct sockaddr_in si_me_config;
  si_me_config.sin_family = AF_INET;
  si_me_config.sin_port = htons(configport_);
  si_me_config.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(configsocket_, (struct sockaddr *)&si_me_config, sizeof(si_me_config)) == -1)
    {
      throw cet::exception("ICARUSTriggerUDP")  <<
	"ICARUSTriggerUDP: Cannot bind config socket to port " << configport_ << std::endl;
      exit(1);
    }
  si_config_.sin_family = AF_INET;
  si_config_.sin_port = htons(configport_);
  if (inet_aton(ip_config_.c_str(), &si_config_.sin_addr) == 0)
    {
      throw cet::exception("ICARUSTriggerUDP")  <<
	"ICARUSTriggerUDP: Could not translate provided IP Address: " << ip_config_ << "\n";
      exit(1);
    }

  datasocket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (datasocket_ < 0)
    {
       throw cet::exception("ICARUSTriggerUDP") << "ICARUSTriggerUDP: Error creating socket!" << std::endl;
      exit(1);
    }
  struct sockaddr_in si_me_data;
  si_me_data.sin_family = AF_INET;
  si_me_data.sin_port = htons(dataport_);
  si_me_data.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(datasocket_, (struct sockaddr *)&si_me_data, sizeof(si_me_data)) == -1)
    {
       throw cet::exception("ICARUSTriggerUDP") <<
	"ICARUSTriggerUDP: Cannot bind data socket to port " << dataport_ << std::endl;
      exit(1);
    }
  si_data_.sin_family = AF_INET;
  si_data_.sin_port = htons(dataport_);
  if (inet_aton(ip_data_.c_str(), &si_data_.sin_addr) == 0)
    {
      throw cet::exception("ICARUSTriggerUDP")  <<
	"ICARUSTriggerUDP: Could not translate provided IP Address: " << ip_data_ << "\n";
      exit(1);
    }

  fEventCounter = 1;
  fLastEvent = 0;
  fLastTimestamp = 0;
  fLastTimestampBNB = 0;
  fLastTimestampNuMI = 0;
  fLastTimestampOther = 0;
  fLastTimestampBNBOff = 0;
  fLastTimestampNuMIOff = 0;
  fLastGatesNum = 0;
  fLastGatesNumBNB = 0;
  fLastGatesNumNuMI = 0;
  fLastGatesNumOther = 0;
  fLastGatesNumBNBOff = 0;
  fLastGatesNumNuMIOff = 0;
  fDeltaGates = 0;
  fDeltaGatesBNB = 0;
  fDeltaGatesNuMI = 0;
  fDeltaGatesOther = 0;
  fDeltaGatesBNBOff = 0;
  fDeltaGatesNuMIOff = 0;
  fStartOfRun = 0;
  fInitialStep = 0;
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

  int size_bytes = poll_with_timeout(datasocket_,ip_data_, si_data_,500);
  std::string data_input = "";
  buffer[0] = '\0';
  if(size_bytes>0){
    //Not currently using peek buffer result, just using buffer
    //int x = read(datasocket_,ip_data_,si_data_,size_bytes,buffer);
    int x = read(datasocket_,ip_data_,si_data_,sizeof(buffer)-1,buffer);
    TLOG(TLVL_DEBUG) << "x:: " << x << " errno:: " << errno << " data received:: " << buffer;
    data_input = buffer;
  }
  TLOG(TLVL_DEBUG) << "string received:: " << data_input;

  artdaq::Fragment::timestamp_t ts(0);
  {
    using namespace boost::gregorian;
    using namespace boost::posix_time;

    ptime t_now(microsec_clock::universal_time());
    ptime time_t_epoch(date(1970,1,1));
    time_duration diff = t_now - time_t_epoch;

    ts = diff.total_nanoseconds();
    fNTP_time = ts;
  }

  if(fInitialStep == 0)
  {
    fStartOfRun = ts;
    fInitialStep = 1;
  }

  if(data_input==""){
    TLOG(TLVL_DEBUG) << "No data after poll with timeout? " << data_input;
    return true;
  }

  //if shouldn't send fragments, then don't create fragment/send
  if(generated_fragments_per_event_==0){
    fLastEvent = fEventCounter;
    ++fEventCounter;
    return true;
  }

  icarus::ICARUSTriggerInfo datastream_info = icarus::parse_ICARUSTriggerString(buffer);

  uint64_t event_no = fEventCounter;
  uint64_t wr_ts = datastream_info.getNanoseconds_since_UTC_epoch() + wr_time_offset_ns_;
  if(use_wr_time_ && wr_ts > 0)
    ts = wr_ts;
  if(use_wr_time_ && wr_ts == 0)
  {
    TLOG(TLVL_WARNING) << "WR time = 0";
  }
  if(datastream_info.event_no > -1)
    event_no = datastream_info.event_no;

  if(datastream_info.wr_name != " WR_TS1 " || datastream_info.wr_seconds == -3 || datastream_info.wr_nanoseconds == -4)
  {
    TLOG(TLVL_WARNING) << "White Rabbit timestamp missing!";
  }
    //Add in fragment details and fragment filling function, want a fragment to contain all of the variables arriving with the trigger
  //Put user variables in metadata, maybe except trigger name, try all at first and might be doing not quite correctly
    //Only create and send fragment if the trigger number has increased, noticed can get multiple of the same trigger from the board
  if(fLastEvent < event_no)
  {
    if(fLastEvent == 0)
    {
      fLastTimestamp = fStartOfRun;
      fLastTimestampBNB = fStartOfRun;
      fLastTimestampNuMI = fStartOfRun;
      fLastTimestampOther = fStartOfRun;
      fLastTimestampBNBOff = fStartOfRun;
      fLastTimestampNuMIOff = fStartOfRun;
    }

    fDeltaGates = datastream_info.gate_id - fLastGatesNum;
    metricMan->sendMetric("EventRate",1, "Hz", 1,artdaq::MetricMode::Rate);

    if(fDeltaGates <= 0)
      TLOG(TLVL_WARNING) << "Change in total number of beam gates for ALL <= 0!";

    if(datastream_info.gate_type == 1)
    {
      fDeltaGatesBNB = datastream_info.gate_id_BNB - fLastGatesNumBNB;
      metricMan->sendMetric("BNBEventRate",1, "Hz", 1,artdaq::MetricMode::Rate);
      if(fDeltaGatesBNB <= 0)
	TLOG(TLVL_WARNING) << "Change in total number of beam gates for BNB <= 0!";
    }
    else if(datastream_info.gate_type == 2)
    {
      fDeltaGatesNuMI = datastream_info.gate_id_NuMI - fLastGatesNumNuMI;
      metricMan->sendMetric("NuMIEventRate",1, "Hz", 1,artdaq::MetricMode::Rate);
      if(fDeltaGatesNuMI <= 0)
        TLOG(TLVL_WARNING) << "Change in total number of beam gates for NuMI <= 0!";
    }
    else if(datastream_info.gate_type == 3)
    {
      fDeltaGatesBNBOff = datastream_info.gate_id_BNBOff - fLastGatesNumBNBOff;
      metricMan->sendMetric("BNBOffbeamEventRate",1, "Hz", 1, artdaq::MetricMode::Rate);
      if(fDeltaGatesBNBOff <= 0)
	TLOG(TLVL_WARNING) << "Change in total number of beam gates for BNB Offbeam <= 0!";
    }
    else if(datastream_info.gate_type == 4)
    {
      fDeltaGatesNuMIOff = datastream_info.gate_id_NuMIOff - fLastGatesNumNuMIOff;
      metricMan->sendMetric("NuMIOffbeamEventRate",1, "Hz", 1, artdaq::MetricMode::Rate);
      if(fDeltaGatesNuMIOff <= 0)
	TLOG(TLVL_WARNING) << "Change in total number of beam gates for NuMI Offbeam <= 0!";
    }
    else {
      fDeltaGatesOther = datastream_info.gate_id - fLastGatesNumOther;
      metricMan->sendMetric("OtherEventRate",1, "Hz", 1,artdaq::MetricMode::Rate);
      if(fDeltaGatesOther <= 0)
        TLOG(TLVL_WARNING) << "Change in total number of beam gates for Other <= 0!";
    }

    auto metadata = icarus::ICARUSTriggerUDPFragmentMetadata(fNTP_time,
							     fLastTimestamp,
							     fLastTimestampBNB,fLastTimestampNuMI,fLastTimestampBNBOff, 
							     fLastTimestampNuMIOff,fLastTimestampOther,
							     fDeltaGates,
							     fDeltaGatesBNB,fDeltaGatesNuMI,fDeltaGatesBNBOff,
							     fDeltaGatesNuMIOff, fDeltaGatesOther);

    //Put data string in fragment -> make frag size size of data string, copy data string into fragment
    //Add timestamp, in number of nanoseconds, as extra argument to fragment after metadata. Add seconds and nanoseconds
    size_t fragment_size = max_fragment_size_bytes_;
    TLOG(TLVL_DEBUG + 10) << "Created ICARUSTriggerUDP Fragment with size of 500 bytes";
    frags.emplace_back(artdaq::Fragment::FragmentBytes(fragment_size, event_no, fragment_id_, sbndaq::detail::FragmentType::ICARUSTriggerUDP, metadata, ts));
    std::copy(&buffer[0], &buffer[sizeof(buffer)/sizeof(char)], (char*)(frags.back()->dataBeginBytes())); //attempt to copy data string into fragment
    icarus::ICARUSTriggerUDPFragment const &newfrag = *frags.back();
    fLastEvent = event_no;

    TLOG(TLVL_DEBUG) << "The artdaq timestamp value is: " << ts << ". Diff from last timestamp is " << ts-fLastTimestamp;

    long tdiff = (long)wr_ts - (long)fNTP_time;

    TLOG(TLVL_DEBUG) << "(WR TIME - NTP TIME) is (" << wr_ts << " - " << fNTP_time << ") = " << tdiff << " nanoseconds."
		     << " (" << tdiff/1e6 << " ms)";

    if(wr_ts>0 && std::abs(tdiff) > 20e6)
      TLOG(TLVL_WARNING) << "abs(WR TIME - NTP TIME) is " << tdiff << " nanoseconds, which is greater than 20e6 threshold!!";


    fLastTimestamp = ts;
    fLastGatesNum = datastream_info.gate_id;

    if(datastream_info.gate_type == 1)
    {
      fLastTimestampBNB = ts;
      fLastGatesNumBNB = datastream_info.gate_id_BNB;
    }
    else if(datastream_info.gate_type == 2)
    {
      fLastTimestampNuMI = ts;
      fLastGatesNumNuMI = datastream_info.gate_id_NuMI;
    }
    else if(datastream_info.gate_type == 3)
    {
      fLastTimestampBNBOff = ts;
      fLastGatesNumBNBOff = datastream_info.gate_id_BNBOff;
    }
    else if(datastream_info.gate_type == 4)
    {
      fLastTimestampNuMIOff = ts;
      fLastGatesNumNuMIOff = datastream_info.gate_id_NuMIOff;
    }
    else{
      fLastTimestampOther = ts;
      fLastGatesNumOther = datastream_info.gate_id;
    }
    ++fEventCounter;
  }

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
  TLOG(TLVL_DEBUG + 10) << "send:: COMMAND " << cmd << " to " << ip_config_.c_str() << ":" << configport_ << "\n";

  sendto(configsocket_,&cmd,sizeof(Command_t), 0, (struct sockaddr *) &si_config_, sizeof(si_config_));

  TLOG(TLVL_DEBUG + 10) << "send:: COMMAND " << cmd << " to " << ip_config_.c_str() << ":" << configport_ << "\n";
}

//return size of available data
int sbndaq::ICARUSTriggerUDP::poll_with_timeout(int socket, std::string ip, struct sockaddr_in& si, int timeout_ms)
{

  TLOG(TLVL_DEBUG + 10) << "poll:: DATA from " << ip.c_str() << " with " << timeout_ms << " ms timeout";
  struct pollfd ufds[1];
  ufds[0].fd = socket;
  ufds[0].events = POLLIN | POLLPRI;
  int rv = poll(ufds, 1, timeout_ms);

  //have something
  if (rv > 0){
    TLOG(TLVL_DEBUG + 10) << "poll:: rv=" << rv << " with revents=" << ufds[0].revents;

    //have something good
    if (ufds[0].revents == POLLIN || ufds[0].revents == POLLPRI){
      //do something to get data size here?


      peekBuffer[1] = {0};
      socklen_t slen = sizeof(si);
      int ret = recvfrom(socket, peekBuffer, sizeof(peekBuffer), MSG_PEEK,
			 (struct sockaddr *) &si, &slen);
      //std::cout << msg_size << std::endl;
      TLOG(TLVL_DEBUG) << "peek recvfrom:: " << ret << " " << errno << " size: " << (int)(peekBuffer[1]);
      return 1;
    }
  }
  //timeout
  else if(rv==0){
    TLOG(TLVL_DEBUG + 10) << "poll:: timed out after " << timeout_ms << " ms (rv=" << rv << ")";
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
