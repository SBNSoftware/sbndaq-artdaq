#define TRACE_NAME "ICARUSTriggerV3"
#include "sbndaq-artdaq-core/Trace/trace_defines.h"

#include "artdaq/DAQdata/Globals.hh"

#include "sbndaq-artdaq/Generators/ICARUS/ICARUSTriggerV3.hh"
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

sbndaq::ICARUSTriggerV3::ICARUSTriggerV3(fhicl::ParameterSet const& ps)
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
  , initialization_data_fpga_(ps.get<fhicl::ParameterSet>("fpga_init_params"))
  , initialization_data_spexi_(ps.get<fhicl::ParameterSet>("spexi_init_params"))
{
  
  configsocket_ = socket(AF_INET, SOCK_STREAM, 0);
  if (configsocket_ < 0)
    {
      throw art::Exception(art::errors::Configuration) << "ICARUSTriggerV3: Error creating socket!" << std::endl;
      exit(1);
    }
  
  struct sockaddr_in si_me_config;
  si_me_config.sin_family = AF_INET;
  si_me_config.sin_port = htons(configport_);
  si_me_config.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(configsocket_, (struct sockaddr *)&si_me_config, sizeof(si_me_config)) == -1)
    {
      throw art::Exception(art::errors::Configuration) <<
	"ICARUSTriggerV3: Cannot bind config socket to port " << configport_ << std::endl;
      exit(1);
    }
  si_config_.sin_family = AF_INET;
  si_config_.sin_port = htons(configport_);
  if (inet_aton(ip_config_.c_str(), &si_config_.sin_addr) == 0)
    {
      throw art::Exception(art::errors::Configuration) <<
	"ICARUSTriggerV3: Could not translate provided IP Address: " << ip_config_ << "\n";
      exit(1);
    }
  
  datasocket_ = socket(AF_INET, SOCK_STREAM, 0);
  if (datasocket_ < 0)
    {
      throw art::Exception(art::errors::Configuration) << "ICARUSTriggerV3: Error creating socket!" << std::endl;
      exit(1);
    }
  struct sockaddr_in si_me_data;
  si_me_data.sin_family = AF_INET;
  si_me_data.sin_port = htons(dataport_);
  si_me_data.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(datasocket_, (struct sockaddr *)&si_me_data, sizeof(si_me_data)) == -1)
    {
      throw art::Exception(art::errors::Configuration) <<
	"ICARUSTriggerV3: Cannot bind data socket to port " << dataport_ << std::endl;
      exit(1);
    }
  si_data_.sin_family = AF_INET;
  si_data_.sin_port = htons(dataport_);
  if (inet_aton(ip_data_.c_str(), &si_data_.sin_addr) == 0)
    {
      throw art::Exception(art::errors::Configuration) <<
	"ICARUSTriggerV3: Could not translate provided IP Address: " << ip_data_ << "\n";
      exit(1);
    }

  socklen_t configlen = sizeof((struct sockaddr_in&) si_config_);
  bind(configsocket_, (struct sockaddr *) &si_config_, configlen);
  if(listen(configsocket_, 1) >= 0)
    {
      TLOG(TLVL_INFO) << "Moving to accept function" << "\n";
      datafd_ = accept(configsocket_, (struct sockaddr *) &si_config_, &configlen);
    }
  else
    {
      throw art::Exception(art::errors::Configuration) << "Unable to accept request to connect to SPEXI" << "\n";
      exit(1);
    }
  TLOG(TLVL_INFO) << "Sending Initialization Parameters for FPGA";
  std::vector<std::string> fpga_init_keys = initialization_data_fpga_.get_pset_names();
  if(fpga_init_keys.size() > 0)
  {
    int setup_fpga = send_init_params(fpga_init_keys, initialization_data_fpga_);
    if(setup_fpga == 0)
    {
      throw art::Exception(art::errors::Configuration) <<
	"ICARUSTriggerV3: Did not successfully communicate FPGA parameters to SPEXI ";
      exit(1);
    }
  TLOG(TLVL_INFO) << "Initialization Parameters for FPGA successfully communicated to SPEXI";
  }
  else
    TLOG(TLVL_WARNING) << "No keys detected for FPGA parameter initialization, continuing";
  TLOG(TLVL_INFO) << "Sending Initialization Parameters for SPEXI";
  std::vector<std::string> spexi_init_keys = initialization_data_spexi_.get_pset_names();
  if(spexi_init_keys.size() > 0)
  {
    int setup_spexi = send_init_params(spexi_init_keys, initialization_data_spexi_);
    if(setup_spexi == 0)
    {
      throw art::Exception(art::errors::Configuration) <<
	"ICARUSTriggerV3: Did not successfully communicate SPEXI Initialize parameters to SPEXI ";
      exit(1);
    }
    TLOG(TLVL_INFO) << "Initialization Parameters for SPEXI successfully communicated to SPEXI";
  }
  else
    TLOG(TLVL_WARNING) << "No keys detected for SPEXI parameter initialization, continuing";

  TLOG(TLVL_INFO) << "Waiting for response from board to finish initialization";

  usleep(30000000); /*30s*/  TLOG(TLVL_DEBUG+1) << "Continuing after init (debugging) inserted 30s sleep";

  int tries = n_init_retries_;
  while(tries>-1){
    int size_bytes = poll_with_timeout(datafd_,ip_config_, si_config_, n_init_timeout_ms_*2);
      if(size_bytes>0){
        buffer[size_bytes+1] = {'\0'};
        readTCP(datafd_,ip_config_,si_config_,size_bytes,buffer);
        TLOG(TLVL_DEBUG) << "Initialization final step - received:: " << buffer;
	break;
      }
      if(tries == 0)
      {
	throw art::Exception(art::errors::Configuration) <<
	  "ICARUSTriggerV3: Did not successfully communicate ability to go to start of run communication to SPEXI";
	exit(1);
      }
      --tries;
  }

  TLOG(TLVL_INFO) << "Proceeding to start of run";

  fEventCounter = 1;
  fLastEvent = 0;
  fLastTimestamp = 0;
  fLastTimestampBNBMaj = 0;
  fLastTimestampBNBMinbias = 0;
  fLastTimestampNuMIMaj = 0;
  fLastTimestampNuMIMinbias = 0;
  fLastTimestampOther = 0;
  fLastTimestampBNBOffMaj = 0;
  fLastTimestampBNBOffMinbias = 0;
  fLastTimestampNuMIOffMaj = 0;
  fLastTimestampNuMIOffMinbias = 0;
  fLastTimestampCalibMaj = 0;
  fLastTimestampCalibMinbias = 0;
  fLastGatesNum = 0;
  fLastGatesNumBNBMaj = 0;
  fLastGatesNumBNBMinbias = 0;
  fLastGatesNumNuMIMaj = 0;
  fLastGatesNumNuMIMinbias = 0;
  fLastGatesNumOther = 0;
  fLastGatesNumBNBOffMaj = 0;
  fLastGatesNumBNBOffMinbias = 0;
  fLastGatesNumNuMIOffMaj = 0;
  fLastGatesNumNuMIOffMinbias = 0;
  fLastGatesNumCalibMaj = 0;
  fLastGatesNumCalibMinbias = 0;
  fDeltaGates = 0;
  fDeltaGatesBNBMaj = 0;
  fDeltaGatesBNBMinbias = 0;
  fDeltaGatesNuMIMaj = 0;
  fDeltaGatesNuMIMinbias = 0;
  fDeltaGatesOther = 0;
  fDeltaGatesBNBOffMaj = 0;
  fDeltaGatesBNBOffMinbias = 0;
  fDeltaGatesNuMIOffMaj = 0;
  fDeltaGatesNuMIOffMinbias = 0;
  fDeltaGatesCalibMaj = 0;
  fDeltaGatesCalibMinbias = 0;
  fLastTriggerType = 0;
  fLastTriggerBNBMaj = 0;
  fLastTriggerBNBMinbias = 0;
  fLastTriggerNuMIMaj = 0;
  fLastTriggerNuMIMinbias = 0;
  fLastTriggerBNBOffMaj = 0;
  fLastTriggerBNBOffMinbias = 0;
  fLastTriggerNuMIOffMaj = 0;
  fLastTriggerNuMIOffMinbias = 0;
  fLastTriggerCalibMaj = 0;
  fLastTriggerCalibMinbias = 0;
  fTotalTriggerBNBMaj = 0;
  fTotalTriggerBNBMinbias = 0;
  fTotalTriggerNuMIMaj = 0;
  fTotalTriggerNuMIMinbias = 0;
  fTotalTriggerBNBOffMaj = 0;
  fTotalTriggerBNBOffMinbias = 0;
  fTotalTriggerNuMIOffMaj = 0;
  fTotalTriggerNuMIOffMinbias = 0;
  fTotalTriggerCalib = 0;
  fTotalTriggerCalibMaj = 0;
  fTotalTriggerCalibMinbias = 0;
  fStartOfRun = 0;
  fInitialStep = 0;
}
bool sbndaq::ICARUSTriggerV3::getNext_(artdaq::FragmentPtrs& frags)
{
  static auto start= std::chrono::steady_clock::now();
  if (should_stop()){
    return false;
  }
  /*
    //do something in here to get data...
   */
  //int size_bytes = poll_with_timeout(datasocket_,ip_data_, si_data_,500);
  int size_bytes = poll_with_timeout(dataconnfd_,ip_data_, si_data_,500);  
  std::string data_input = "";
  std::fill( buffer, buffer + sizeof(buffer), '\0' ); 
  if(size_bytes>0){
    //TLOG(TLVL_DEBUG) << "size_bytes from poll_with_timeout " << size_bytes ;

    //Not currently using peek buffer result, just using buffer
    //int x = read(datasocket_,ip_data_,si_data_,size_bytes,buffer);
    //int x = read(datasocket_,ip_data_,si_data_,sizeof(buffer)-1,buffer);

    //here it seems we request to read 999 bytes (sizeof(buffer)-1)from the socket
    //original code (JZ)
    //int x = readTCP(dataconnfd_,ip_data_,si_data_,sizeof(buffer)-1,buffer)
    //TLOG(TLVL_DEBUG) << "in getNext x:: " << x << " errno:: " << errno << " data received:: " << buffer;
    //

    int bytes_desired = 4;
    int bytes_so_far = 0;

    int bytes_this_read =0;

    while (bytes_desired - bytes_so_far > 0 ) {
    bytes_this_read = readTCP(dataconnfd_ , ip_data_, si_data_, (bytes_desired - bytes_so_far), &buffer[bytes_so_far] );
    if (bytes_this_read > 0 ) {
        bytes_so_far += bytes_this_read;}
    }

    TLOG(TLVL_DEBUG) << " buffer 0-3 bytes "  << buffer[0] <<" *  " << buffer[1] << "  *"<< buffer[2] <<"* "<< buffer[3]; 

    //TLOG(TLVL_DEBUG) << "after readTCP first 4 bytes: bytes_this_read " << bytes_this_read << " bytes_so_far  " << bytes_so_far ; 

    //2nd loop on rest of buffer to get remaining bytes 
    //bytes_desired = ntohl(*(long*)buffer);  NO!!
    //example:  0 5 3 9
    //bytes_desired = atoi(buffer[1])*100 + atoi(buffer[2])*10 + atoi(buffer[3]);
    std::string strtmp(&buffer[0], 4);
    //TLOG(TLVL_DEBUG) << "printout of strtmp &buffer " << strtmp ;;

    bytes_desired = std::stoi(strtmp);
   
    TLOG(TLVL_DEBUG) << " bytes_desired " << bytes_desired ;
    bytes_so_far = 0;

    while (bytes_desired - bytes_so_far > 0 ) {
    bytes_this_read = readTCP(dataconnfd_ , ip_data_, si_data_, (bytes_desired - bytes_so_far), &buffer[bytes_so_far] ) ;
    if (bytes_this_read > 0 ) {
        bytes_so_far += bytes_this_read;}
    }

    // fill the data array (GT string data) here
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
    metricMan->sendMetric("EventRate",1, "Hz", 1,artdaq::MetricMode::Rate);
    metricMan->sendMetric("EventCounter",uint64_t{fEventCounter}, "Triggers", 1,artdaq::MetricMode::LastPoint);
    return true;
  }
  
  icarus::ICARUSTriggerInfo datastream_info = icarus::parse_ICARUSTriggerV3String(buffer);

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
  if(fLastEvent >= event_no) TLOG(TLVL_WARNING)<< "fLastEvent="<<fLastEvent << ", event_no="<<event_no;

  if(fLastEvent < event_no)
  {
    if(fLastEvent == 0)
    {
      fLastTimestamp = fStartOfRun;
      fLastTimestampBNBMaj = fStartOfRun;
      fLastTimestampBNBMinbias = fStartOfRun;
      fLastTimestampNuMIMaj = fStartOfRun;
      fLastTimestampNuMIMinbias = fStartOfRun;
      fLastTimestampOther = fStartOfRun;
      fLastTimestampBNBOffMaj = fStartOfRun;
      fLastTimestampBNBOffMinbias = fStartOfRun;
      fLastTimestampNuMIOffMaj = fStartOfRun;
      fLastTimestampNuMIOffMinbias = fStartOfRun;
      fLastTimestampCalibMaj = fStartOfRun;
      fLastTimestampCalibMinbias = fStartOfRun;
    }

    fDeltaGates = datastream_info.gate_id - fLastGatesNum;
    metricMan->sendMetric("EventRate",1, "Hz", 1,artdaq::MetricMode::Rate);
    metricMan->sendMetric("EventCounter", uint64_t(fEventCounter), "Triggers", 1,artdaq::MetricMode::LastPoint);
    
    if(fDeltaGates <= 0)
      TLOG(TLVL_WARNING) << "Change in total number of beam gates for ALL <= 0!";

    if(datastream_info.gate_type == 1)
    {
      if(datastream_info.trigger_type == 0)
      {
	fDeltaGatesBNBMaj = datastream_info.gate_id_BNB - fLastGatesNumBNBMaj;
	++fTotalTriggerBNBMaj;
	++fTotalTriggerBNB;
	fTotalGatesBNBMaj += fDeltaGatesBNBMaj;
	if(fDeltaGatesBNBMaj <= 0)
	  TLOG(TLVL_WARNING) << "Change in total number of beam gates for BNB Majority <= 0!";
      }
      else if(datastream_info.trigger_type == 1)
      {
	fDeltaGatesBNBMinbias = datastream_info.gate_id_BNB - fLastGatesNumBNBMinbias;
        ++fTotalTriggerBNBMinbias;
        ++fTotalTriggerBNB;
	++fTotalGatesBNBMinbias;
        if(fDeltaGatesBNBMinbias <= 0)
          TLOG(TLVL_WARNING) << "Change in total number of beam gates for BNB Minbias <= 0!";
      }
      else
	TLOG(TLVL_ERROR) << "Unsupported light-based trigger type found for BNB trigger!";
      metricMan->sendMetric("BNBEventRate",1, "Hz", 1,artdaq::MetricMode::Rate);
      metricMan->sendMetric("BNBEventTotal",uint64_t(fTotalTriggerBNB), "Triggers", 1,artdaq::MetricMode::LastPoint);
    }
    else if(datastream_info.gate_type == 2)
    {
      if(datastream_info.trigger_type == 0)
      {
	fDeltaGatesNuMIMaj = datastream_info.gate_id_NuMI - fLastGatesNumNuMIMaj;
	++fTotalTriggerNuMIMaj;
	++fTotalTriggerNuMI;
	fTotalGatesNuMIMaj += fDeltaGatesNuMIMaj;
	if(fDeltaGatesNuMIMaj <= 0)
	  TLOG(TLVL_WARNING) << "Change in total number of beam gates for NuMI Majority <= 0!";	
      }
      else if(datastream_info.trigger_type == 1)
      {
	fDeltaGatesNuMIMinbias = datastream_info.gate_id_NuMI - fLastGatesNumNuMIMinbias;
        ++fTotalTriggerNuMIMinbias;
	++fTotalTriggerNuMI;
	++fTotalGatesNuMIMinbias;
	if(fDeltaGatesNuMIMaj <= 0)
          TLOG(TLVL_WARNING) << "Change in total number of beam gates for NuMI Minbias <= 0!";
      }
      else
	TLOG(TLVL_ERROR) << "Unsupported light-based trigger type found for NuMI trigger!";
      metricMan->sendMetric("NuMIEventRate",1, "Hz", 1,artdaq::MetricMode::Rate);
      metricMan->sendMetric("NuMIEventTotal",uint64_t(fTotalTriggerNuMI), "Triggers", 1,artdaq::MetricMode::LastPoint);
    }
    else if(datastream_info.gate_type == 3)
    {
      if(datastream_info.trigger_type == 0)
      {
	fDeltaGatesBNBOffMaj = datastream_info.gate_id_BNBOff - fLastGatesNumBNBOffMaj;
	++fTotalTriggerBNBOffMaj;
	++fTotalTriggerBNBOff;
	fTotalGatesBNBOffMaj += fDeltaGatesBNBOffMaj;
	if(fDeltaGatesBNBOffMaj <= 0)
          TLOG(TLVL_WARNING) << "Change in total number of beam gates for BNB Offbeam Majority <= 0!";
      }
      else if(datastream_info.trigger_type == 1)
      {
	fDeltaGatesBNBOffMinbias = datastream_info.gate_id_BNBOff - fLastGatesNumBNBOffMinbias;
	++fTotalTriggerBNBOffMinbias;
	++fTotalTriggerBNBOff;
	++fTotalGatesBNBOff;
	if(fDeltaGatesBNBOffMinbias <= 0)
	  TLOG(TLVL_WARNING) << "Change in total number of beam gates for BNB Offbeam Minbias <= 0!";
      } 
      else
        TLOG(TLVL_ERROR) << "Unsupported light-based trigger type found for BNBOffBeam trigger!";
      metricMan->sendMetric("BNBOffbeamEventRate",1, "Hz", 1, artdaq::MetricMode::Rate);
      metricMan->sendMetric("BNBOffbeamEventTotal",uint64_t(fTotalTriggerBNBOff), "Triggers", 1,artdaq::MetricMode::LastPoint);
    }
    else if(datastream_info.gate_type == 4)
    {
      if(datastream_info.trigger_type == 0)
      {
	fDeltaGatesNuMIOffMaj = datastream_info.gate_id_NuMIOff - fLastGatesNumNuMIOffMaj;
	++fTotalTriggerNuMIOffMaj;
	++fTotalTriggerNuMIOff;
	fTotalGatesNuMIOffMaj += fDeltaGatesNuMIOffMaj;
	if(fDeltaGatesNuMIOffMaj <= 0)
          TLOG(TLVL_WARNING) << "Change in total number of beam gates for NuMI Offbeam Majority <= 0!";
      }
      else if(datastream_info.trigger_type == 1)
      {
	fDeltaGatesNuMIOffMinbias = datastream_info.gate_id_NuMIOff - fLastGatesNumNuMIOffMinbias;
        ++fTotalTriggerNuMIOffMinbias;
	++fTotalTriggerNuMIOff;
	++fTotalGatesNuMIOffMinbias;
	if(fDeltaGatesNuMIOffMinbias <= 0)
          TLOG(TLVL_WARNING) << "Change in total number of beam gates for NuMI Offbeam Minbias <= 0!";
      }
      else
	TLOG(TLVL_ERROR) << "Unsupported light-based trigger type found for NuMIOffBeam trigger!";
      metricMan->sendMetric("NuMIOffbeamEventRate",1, "Hz", 1, artdaq::MetricMode::Rate);
      metricMan->sendMetric("NuMIOffbeamEventTotal",uint64_t(fTotalTriggerNuMIOff), "Triggers", 1,artdaq::MetricMode::LastPoint);
    }
    else if(datastream_info.gate_type == 5)
    {
      if(datastream_info.trigger_type == 0)
      {
	fDeltaGatesCalibMaj = datastream_info.gate_id - fLastGatesNumCalibMaj;
	++fTotalTriggerCalibMaj; 
	++fTotalTriggerCalib;
	fTotalGatesCalibMaj += fDeltaGatesCalibMaj;
	if(fDeltaGatesCalibMaj <= 0)
          TLOG(TLVL_WARNING) << "Change in total number of beam gates for Calibration Majority <= 0!";
      }
      if(datastream_info.trigger_type == 1)
      {
	fDeltaGatesCalibMinbias = datastream_info.gate_id - fLastGatesNumCalibMinbias;
	++fTotalTriggerCalibMinbias;
	++fTotalGatesCalibMinbias;
	++fTotalTriggerCalib;
	if(fDeltaGatesCalibMinbias <= 0)
	  TLOG(TLVL_WARNING) << "Change in total number of beam gates for Calibration Minbias <= 0!";
      } 
      metricMan->sendMetric("CalibrationRate",1, "Hz", 1, artdaq::MetricMode::Rate);
      metricMan->sendMetric("CalibrationTotal",uint64_t(fTotalTriggerCalib), "Triggers", 1,artdaq::MetricMode::LastPoint);
    }
    else {
      fDeltaGatesOther = datastream_info.gate_id - fLastGatesNumOther;
      metricMan->sendMetric("OtherEventRate",1, "Hz", 1,artdaq::MetricMode::Rate);
      if(fDeltaGatesOther <= 0)
        TLOG(TLVL_WARNING) << "Change in total number of beam gates for Other <= 0!";
    }

    auto metadata = icarus::ICARUSTriggerV3FragmentMetadata(fNTP_time,
							     fLastTimestamp,
							     fLastTimestampBNBMaj,fLastTimestampBNBMinbias,
							     fLastTimestampNuMIMaj,fLastTimestampNuMIMinbias,
							     fLastTimestampBNBOffMaj, fLastTimestampBNBOffMinbias,  
							     fLastTimestampNuMIOffMaj, fLastTimestampNuMIOffMinbias,
							     fLastTimestampCalibMaj, fLastTimestampCalibMinbias,
							     fLastTimestampOther,
							     fLastTriggerType, fLastSourceType, 
							     fLastTriggerBNBMaj, fLastTriggerBNBMinbias,
							     fLastTriggerNuMIMaj, fLastTriggerNuMIMinbias, 
							     fLastTriggerBNBOffMaj, fLastTriggerBNBOffMinbias,
							     fLastTriggerNuMIOffMaj, fLastTriggerNuMIOffMinbias, 
							     fLastTriggerCalibMaj, fLastTriggerCalibMinbias,
							     fTotalTriggerBNB, fTotalTriggerBNBMaj,
							     fTotalTriggerBNBMinbias, fTotalTriggerNuMI,
							     fTotalTriggerNuMIMaj, fTotalTriggerNuMIMinbias,
							     fTotalTriggerBNBOff, fTotalTriggerBNBOffMaj,
							     fTotalTriggerBNBOffMinbias, fTotalTriggerNuMIOff,
							     fTotalTriggerNuMIOffMaj, fTotalTriggerNuMIOffMinbias,
							     fTotalTriggerCalibMaj, fTotalTriggerCalibMinbias,
							     fDeltaGates,
							     fDeltaGatesBNBMaj, fDeltaGatesBNBMinbias, 
							     fDeltaGatesNuMIMaj,fDeltaGatesNuMIMinbias,
							     fDeltaGatesBNBOffMaj, fDeltaGatesBNBOffMinbias,
							     fDeltaGatesNuMIOffMaj,fDeltaGatesNuMIOffMinbias, 
							     fDeltaGatesCalibMaj, fDeltaGatesCalibMinbias,fDeltaGatesOther,
							     fTotalGatesBNBMaj, fTotalGatesBNBMinbias,
							     fTotalGatesNuMIMaj, fTotalGatesNuMIMinbias,
							     fTotalGatesBNBOffMaj, fTotalGatesBNBOffMinbias,
							     fTotalGatesNuMIOffMaj, fTotalGatesNuMIOffMinbias,
							     fTotalGatesCalibMaj, fTotalGatesCalibMinbias);

    //Put data string in fragment -> make frag size size of data string, copy data string into fragment
    //Add timestamp, in number of nanoseconds, as extra argument to fragment after metadata. Add seconds and nanoseconds
    size_t fragment_size = max_fragment_size_bytes_;
    TLOG(TLVL_DEBUG + 10) << "Created ICARUSTriggerV3 Fragment with size of " << max_fragment_size_bytes_ << " bytes";
    frags.emplace_back(artdaq::Fragment::FragmentBytes(fragment_size, event_no, fragment_id_, sbndaq::detail::FragmentType::ICARUSTriggerV3, metadata, ts));
    std::copy(&buffer[0], &buffer[sizeof(buffer)/sizeof(char)], (char*)(frags.back()->dataBeginBytes())); //attempt to copy data string into fragment
    icarus::ICARUSTriggerV3Fragment const &newfrag = *frags.back();
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
      if(datastream_info.trigger_type == 0)
      {
	fLastTimestampBNBMaj = ts;
	fLastGatesNumBNBMaj = datastream_info.gate_id_BNB;
	fLastTriggerBNBMaj = datastream_info.wr_event_no;
      }
      else if(datastream_info.trigger_type == 1)
      {
	fLastTimestampBNBMinbias = ts;
        fLastGatesNumBNBMinbias = datastream_info.gate_id_BNB;
        fLastTriggerBNBMinbias = datastream_info.wr_event_no;
      }
    }
    else if(datastream_info.gate_type == 2)
    {
      if(datastream_info.trigger_type == 0)
      {
	fLastTimestampNuMIMaj = ts;
	fLastGatesNumNuMIMaj = datastream_info.gate_id_NuMI;
	fLastTriggerNuMIMaj = datastream_info.wr_event_no;
      }
      else if(datastream_info.trigger_type == 1)
      {
	fLastTimestampNuMIMinbias = ts;
        fLastGatesNumNuMIMinbias = datastream_info.gate_id_NuMI;
        fLastTriggerNuMIMinbias = datastream_info.wr_event_no;
      }
    }
    else if(datastream_info.gate_type == 3)
    {
      if(datastream_info.trigger_type == 0)
      {
	fLastTimestampBNBOffMaj = ts;
	fLastGatesNumBNBOffMaj = datastream_info.gate_id_BNBOff;
	fLastTriggerBNBOffMaj = datastream_info.wr_event_no;
      }
      else if(datastream_info.trigger_type == 1)
      {
	fLastTimestampBNBOffMinbias = ts;
        fLastGatesNumBNBOffMinbias = datastream_info.gate_id_BNBOff;
        fLastTriggerBNBOffMinbias = datastream_info.wr_event_no;
      }
    }
    else if(datastream_info.gate_type == 4)
    {
      if(datastream_info.trigger_type == 0)
      {
	fLastTimestampNuMIOffMaj = ts;
	fLastGatesNumNuMIOffMaj = datastream_info.gate_id_NuMIOff;
	fLastTriggerNuMIOffMaj = datastream_info.wr_event_no;
      }
      else if(datastream_info.trigger_type == 1)
      {
	fLastTimestampNuMIOffMinbias = ts;
        fLastGatesNumNuMIOffMinbias = datastream_info.gate_id_NuMIOff;
        fLastTriggerNuMIOffMinbias = datastream_info.wr_event_no;
      }
    }
    else if(datastream_info.gate_type == 5)
    {
      if(datastream_info.trigger_type == 0)
      {
	fLastTimestampCalibMaj = ts;
	fLastGatesNumCalibMaj = datastream_info.gate_id;
	fLastTriggerCalibMaj = datastream_info.wr_event_no;
      }
      if(datastream_info.trigger_type == 1)
      {
	fLastTimestampCalibMinbias = ts;
	fLastGatesNumCalibMinbias = datastream_info.gate_id;
	fLastTriggerCalibMinbias = datastream_info.wr_event_no;
      }
    }
    else{
      fLastTimestampOther = ts;
      fLastGatesNumOther = datastream_info.gate_id;
    }
    fLastTriggerType = datastream_info.gate_type;
    fLastSourceType = datastream_info.trigger_type;
    ++fEventCounter;
  }

  std::chrono::duration<double> delta = std::chrono::steady_clock::now()-start;
  //std::cout << "getNext_ function takes: " << delta.count() << " seconds." << std::endl; 
  return true;

}


void sbndaq::ICARUSTriggerV3::start()
{
  if(initialization(n_init_retries_,n_init_timeout_ms_) < 0) //comment out for fake trigger tests
  {
     TLOG(TLVL_ERROR) << "Was not able to initialize SPEXI" << "\n";
     abort();

  }
  //send_TRIG_ALLW();
  close(datafd_);
  socklen_t datalen = sizeof((struct sockaddr_in&) si_data_);
  bind(datasocket_, (struct sockaddr *) &si_data_, datalen);                     
  if(listen(datasocket_, 1) >= 0)            
  {               
    TLOG(TLVL_INFO) << "Moving to accept function -- data transfer" << "\n";
    dataconnfd_ = accept(datasocket_, (struct sockaddr *) &si_data_, &datalen);
  }                                                                                   
  else                                                                                
    {                               
    TLOG(TLVL_ERROR) << "Unable to accept request to connect to SPEXI -- data transfer" << "\n";
    abort();
  }                                                                            
}
void sbndaq::ICARUSTriggerV3::stop()
{
  //send_TRIG_VETO();
}
void sbndaq::ICARUSTriggerV3::pause()
{
  //send_TRIG_VETO();
}
void sbndaq::ICARUSTriggerV3::resume()
{
  //send_TRIG_ALLW();
}

//send a command
void sbndaq::ICARUSTriggerV3::sendUDP(const Command_t cmd)
{  
  TLOG(TLVL_DEBUG + 10) << "send:: COMMAND " << cmd << " to " << ip_config_.c_str() << ":" << configport_ << "\n";

  sendto(configsocket_,&cmd,sizeof(Command_t), 0, (struct sockaddr *) &si_config_, sizeof(si_config_));

  TLOG(TLVL_DEBUG + 10) << "send:: COMMAND " << cmd << " to " << ip_config_.c_str() << ":" << configport_ << "\n";
}

//return size of available data
int sbndaq::ICARUSTriggerV3::poll_with_timeout(int socket, std::string ip, struct sockaddr_in& si, int timeout_ms)
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
      //int ret = recvfrom(socket, peekBuffer, sizeof(peekBuffer), MSG_PEEK,                             //(struct sockaddr *) &si, (socklen_t*)sizeof(si));
      int ret = recv(socket, peekBuffer, sizeof(peekBuffer), MSG_PEEK);
			 //(struct sockaddr *) &si, &slen);
      //std::cout << msg_size << std::endl;
      TLOG(TLVL_DEBUG) << "peek recvfrom:: " << ret << " " << errno << " size: " << (int)(peekBuffer[1]);
      return (int)(peekBuffer[1]);
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
int sbndaq::ICARUSTriggerV3::read(int socket, std::string ip, struct sockaddr_in& si, int size, char* buffer){
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

int sbndaq::ICARUSTriggerV3::readTCP(int socket, std::string ip, struct sockaddr_in& si, int size, char* buffer){
  TLOG(TLVL_DEBUG) << "in readTCP:: get " << size << " bytes from " << ip.c_str() << "\n";
  socklen_t slen = sizeof(si); 
  int size_rcv = recv(socket, buffer, size, 0); //for TCP/IP stuff
  if(size_rcv<0)
    TLOG(TLVL_ERROR) << "in readTCP:: error receiving data (" << size_rcv << " bytes from " << ip.c_str() << ")\n";
  else
    TLOG(TLVL_DEBUG) << "in readTCP:: received " << size_rcv << " bytes from " << ip.c_str() << "\n";

  return size_rcv;
}


//int sbndaq::ICARUSTriggerV3::send_TTLK_INIT(int retries, int sleep_time_ms)
int sbndaq::ICARUSTriggerV3::initialization(int retries, int sleep_time_ms)
{
  TLOG(TLVL_INFO) << "Sending Start of Run Command"; 
  while(retries>-1){
    char cmd[16];
    sprintf(cmd,"%s","TTLK_CMD_INIT");
    
    TLOG(TLVL_DEBUG) << "to send:: COMMAND " << cmd << "to " << ip_config_.c_str() << ":" << configport_ << "\n"; 
    //sendto(configsocket_,&cmd,16, 0, (struct sockaddr *) &si_config_, sizeof(si_config_));
    int sendcode = send(datafd_,&cmd,16, 0); //datafd
    TLOG(TLVL_INFO) << "retcode from send call is: " << sendcode;
    TLOG(TLVL_DEBUG) << "sent!:: COMMAND " << cmd << "to " << ip_config_.c_str() << ":" << configport_ << "\n";
    //int size_bytes = poll_with_timeout(configsocket_,ip_config_, si_config_, sleep_time_ms);
    int size_bytes = poll_with_timeout(datafd_,ip_config_, si_config_, sleep_time_ms);  
    if(size_bytes>0){
      buffer[size_bytes+1] = {'\0'};
      readTCP(datafd_,ip_config_,si_config_,size_bytes,buffer);
      TLOG(TLVL_DEBUG) << "TTLK_INIT step - received:: " << buffer;
      return 0;
    }
  
    retries--;
  }
  
  return -1;
  
}

void sbndaq::ICARUSTriggerV3::configure_socket(int socket, struct sockaddr_in& si)
{
  socklen_t socketlen = sizeof((struct sockaddr_in&) si);
  if(listen(socket, 1) >= 0)
    accept(socket, (struct sockaddr *) &si, &socketlen);
  else
    TLOG(TLVL_ERROR) << "Unable to accept request to connect to SPEXI" << "\n";
}

int sbndaq::ICARUSTriggerV3::send_init_params(std::vector<std::string> param_key_list, fhicl::ParameterSet pset)
{
  std::string init_send;
  for(auto const& key: param_key_list)
  {
    fhicl::ParameterSet key_pset = pset.get<fhicl::ParameterSet>(key);
    std::string data_name = key_pset.get<std::string>("name", "");
    std::string data_value = key_pset.get<std::string>("value", "");
    init_send += data_name + " = \"" + data_value + "\", ";
  }
  init_send += "\r\n";
  TLOG(TLVL_INFO) << "Initialization step - sending:: " << init_send;
  int sendcode = send(datafd_,&init_send[0],init_send.size(),0);
  int size_bytes = 0;
  int attempts = 0;
  while(size_bytes <= 0 && attempts < n_init_retries_)
  {
      size_bytes = poll_with_timeout(datafd_,ip_config_, si_config_, n_init_timeout_ms_);
      if(size_bytes>0){
	buffer[size_bytes+1] = {'\0'};
	readTCP(datafd_,ip_config_,si_config_,size_bytes,buffer);
	TLOG(TLVL_DEBUG) << "Initialization step - received:: " << buffer;
	if(buffer[0] == '1')
	{
	  TLOG(TLVL_INFO) << "Parameters accepted, continuing";
	  return 1;
	}
	else if(buffer[0] == '0')
	{
	  TLOG(TLVL_WARNING) << "Parameters not communicated successfully communicated, failing";
	  return 0;
	}
	else
	{
	  TLOG(TLVL_WARNING) << "Received string from LabVIEW not as expected! Failing";
	  return 0;
	}
      }
      ++attempts;
      TLOG(TLVL_WARNING) << "Receive timeout. attempts = " << attempts << " of " << n_init_retries_;
  }
  return 0;
}

//no need for confirmation on these...
void sbndaq::ICARUSTriggerV3::send_TRIG_VETO()
{
  sendUDP(TRIG_VETO);
}

void sbndaq::ICARUSTriggerV3::send_TRIG_ALLW()
{
  sendUDP(TRIG_ALLW);
}

// The following macro is defined in artdaq's GeneratorMacros.hh header
DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(sbndaq::ICARUSTriggerV3)
