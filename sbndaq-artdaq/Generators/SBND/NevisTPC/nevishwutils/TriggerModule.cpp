#include <fstream>
#include <ctime>
#include <chrono>
#include "TriggerModule.h"
#include "daqExceptions.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "trace.h"
#define TRACE_NAME "TriggerModule"

namespace nevistpc
{

  void TriggerModuleStatus::report() const
  {
    std::ostringstream strstrm;
    
    strstrm << std::endl
	    << "  Trigger Module Status Info Report "      << std::endl
	    << " -----------------------------------"      << std::endl
	    << "  module         : " << module             << std::endl
	    << "  Frame          : " << frame_ctr          << std::endl
	    << "  Sample (2MHz)  : " << sample_ctr_tpc     << std::endl
	    << "  Sample (64MHz) : " << sample_ctr_trigger << std::endl
	    << "  Trigger Count  : " << trigger_ctr        << std::endl;
    
    mf::LogInfo("TriggerModuleStatus")  << strstrm.str();
  }
  

  TriggerModule::TriggerModule( uint8_t slot_number )
    : _slot_number(slot_number),
      _status(),
      _timeinfo_file_path("")//,
  {
    TLOG(TLVL_INFO) << "XMITModule: called constructor with slot number " <<  (int)slot_number ;   
  }

  TriggerModule::~TriggerModule()
  {
    
  }

  void TriggerModule::useController(ControllerModuleSPtr controller)
  {
    TLOG(TLVL_INFO) << "TriggerModule: calling " <<  __func__ ;
    UsesControllerModule::useController ( controller );
    //_reader_synch_object->useController ( controller );
  }
    
  void TriggerModule::runOnSyncOn()
  {
    controller()->send(ControlDataPacket(_slot_number, device::trg, trigger::RUN_ON__SYNC_ON, 1));
    TLOG(TLVL_INFO) << "TriggerModule: called " <<  __func__ ;
  }

  void TriggerModule::runOnSyncOff()
  {
    controller()->send(ControlDataPacket(_slot_number, device::trg, trigger::RUN_ON__SYNC_ON, 0));
    TLOG(TLVL_INFO) << "TriggerModule: called " <<  __func__ ;
  }

  void TriggerModule::disableTriggers(bool write_time_info)
  {
    if(write_time_info) writeTriggerTimeInfo( __func__ );
    controller()->send(ControlDataPacket(_slot_number, device::trg, trigger::ENABLE_TRIGGER, 0));
    TLOG(TLVL_INFO) << "TriggerModule: called " <<  __func__ ;
  }

  void TriggerModule::enableTriggers(bool write_time_info)
  {
    if(write_time_info) writeTriggerTimeInfo( __func__ );
    controller()->send(ControlDataPacket(_slot_number, device::trg, trigger::ENABLE_TRIGGER, 1));
    TLOG(TLVL_INFO) << "TriggerModule: called " <<  __func__ ;
  }

  void TriggerModule::runOff()
  {
    controller()->send(ControlDataPacket(_slot_number, device::trg, trigger::RUN_OFF));
    TLOG(TLVL_INFO) << "TriggerModule: called " <<  __func__ ;
  }
  
  void TriggerModule::setFrameLength(data_payload_t const &length)
  {
    controller()->send(ControlDataPacket(_slot_number, device::trg, trigger::FRAME_LENGTH, length));
    TLOG(TLVL_INFO) << "TriggerrModule: called " <<  __func__ << " " << length ;
  }

  void TriggerModule::setDeadtimeSize(data_payload_t const &size)
  {
    controller()->send(ControlDataPacket(_slot_number, device::trg, trigger::DEADTIME_SIZE, size));
    TLOG(TLVL_INFO) << "TriggerModule: called " <<  __func__ << " " << size ;
  }

  void TriggerModule::setTrigOutput(data_payload_t const &out)
  {
    controller()->send(ControlDataPacket(_slot_number,device::trg, trigger::OUT_PULSE_MODE_SELECT, out));
    TLOG(TLVL_INFO) << "TriggerModule: called " <<  __func__ << " " << out ;
  }

  void TriggerModule::sendOutPCtrigger()
  {
    controller()->send(ControlDataPacket(_slot_number, device::trg, trigger::SEND_PC_TRIGGER));
    TLOG(TLVL_INFO) << "TriggerModule: called " <<  __func__ ;
  }

  void TriggerModule::sendOutCalibTrigger()
  {
    controller()->send(ControlDataPacket(_slot_number, device::trg, trigger::SEND_CALIB_PULSE));
    TLOG(TLVL_INFO) << "TriggerModule: called " <<  __func__ ;
  }
  
  void TriggerModule::setMask0(data_payload_t const &mask)
  {
    controller()->send(ControlDataPacket(_slot_number, device::trg, trigger::MASK0,mask));
    TLOG(TLVL_INFO) << "TriggerModule: called " <<  __func__ << " " << mask ;
  }
  
  void TriggerModule::setMask1(data_payload_t const &mask)
  {
    controller()->send(ControlDataPacket(_slot_number, device::trg, trigger::MASK1,mask));
    TLOG(TLVL_INFO) << "TriggerModule: called " <<  __func__ << " " << mask ;
  }
  
  void TriggerModule::setMask8(data_payload_t const &mask)
  {
    controller()->send(ControlDataPacket(_slot_number, device::trg, trigger::MASK8,mask));
    TLOG(TLVL_INFO) << "TriggerModule: called " <<  __func__ << " " << mask ;
  }
  
  void TriggerModule::setPrescale0(data_payload_t const &prescale)
  {
    controller()->send(ControlDataPacket(_slot_number, device::trg, trigger::PRESCALE0,prescale));
    TLOG(TLVL_INFO) << "TriggerModule: called " <<  __func__ << " " << prescale ;
  }

  void TriggerModule::setPrescale1(data_payload_t const &prescale)
  {
    controller()->send(ControlDataPacket(_slot_number, device::trg, trigger::PRESCALE1,prescale));
    TLOG(TLVL_INFO) << "TriggerModule: called " <<  __func__ << " " << prescale ;
  }
  
  void TriggerModule::setPrescale8(data_payload_t const &prescale)
  {
    controller()->send(ControlDataPacket(_slot_number, device::trg, trigger::PRESCALE8,prescale));
    TLOG(TLVL_INFO) << "TriggerModule: called " <<  __func__ << " " << prescale ;
  }

  void TriggerModule::setPrescale9(data_payload_t const &prescale)
  {
    controller()->send(ControlDataPacket(_slot_number, device::trg, trigger::PRESCALE9,prescale));
    TLOG(TLVL_INFO) << "TriggerModule: called " <<  __func__ << " " << prescale ;
  }

  void TriggerModuleConfig::readConfiguration( fhicl::ParameterSet const & ps )
  {
    try {
      fhicl::ParameterSet triggerConfig;
      // NOTE: the name of the block in the fcl file must match
      if( ! ps.get_if_present<fhicl::ParameterSet> ( "trigger_module", triggerConfig ) ){
	TLOG(TLVL_ERROR)<< "TriggerModuleConfig: Missing trigger_module configuration block in fcl file" ;
	throw RuntimeErrorException ("Missing trigger_module configuration block in fcl file");
      }            

      framesize    = triggerConfig.get<data_payload_t>("framesize");
      deadtime     = triggerConfig.get<data_payload_t>("deadtime" );
      trigmask0    = triggerConfig.get<data_payload_t>("trigmask0");
      trigmask1    = triggerConfig.get<data_payload_t>("trigmask1");
      trigmask8    = triggerConfig.get<data_payload_t>("trigmask8");
      prescale0    = triggerConfig.get<data_payload_t>("prescale0");
      prescale1    = triggerConfig.get<data_payload_t>("prescale1");
      prescale8    = triggerConfig.get<data_payload_t>("prescale8");
      prescale9    = triggerConfig.get<data_payload_t>("prescale9");
      g1start      = triggerConfig.get<data_payload_t>("g1start");
      g1end        = triggerConfig.get<data_payload_t>("g1end");
      g1width      = triggerConfig.get<data_payload_t>("g1width");
      g1delay      = triggerConfig.get<data_payload_t>("g1delay");
      g2start      = triggerConfig.get<data_payload_t>("g2start");
      g2end        = triggerConfig.get<data_payload_t>("g2end");
      g2width      = triggerConfig.get<data_payload_t>("g2width");
      g2delay      = triggerConfig.get<data_payload_t>("g2delay");

      timeinfo_file_path = triggerConfig.get<std::string>("timeinfo_directory",std::string(::getenv("HOME")));
      if(timeinfo_file_path.empty())
	timeinfo_file_path = std::string(::getenv("HOME"));

      std::string run_number("UNKNOWN");
      if(getenv("RUNNO")) run_number = std::string(getenv("RUNNO"));

      timeinfo_file_path += "/run_" + run_number + "_timestamps.txt";

    }
    catch (std::exception &ex) {
      TLOG(TLVL_ERROR) << "TriggerModuleConfig: Failed reading of trigger_module configuration. Nested exception: " << ex.what() ;
      throw InvalidConfigurationException ( std::string("Failed reading of trigger_module configuration. Nested exception: ").append( ex.what() ) );
    }
    catch ( ... ) {
      TLOG(TLVL_ERROR)<< "TriggerModuleConfig: Failed reading of trigger_module configuration" ;
      throw InvalidConfigurationException ( "Failed reading of trigger_module configuration" );
    }
  }

  std::string TriggerModuleConfig::debugInfo () const noexcept
  {
    std::ostringstream os;
    //os << "Object " << demangle(typeid(this)) << ".";
    //os << "\nAttributes:" <<std::endl;
    os << "TriggerModuleConfig attributes:" << std::endl;
    os << "\n  framesize "<< framesize;
    os << "\n  mask0 " << trigmask0;
    os << "\n  mask1 " << trigmask1;
    os << "\n  mask8 " << trigmask8;
    os << "\n  prescale0 " << prescale0;
    os << "\n  prescale1 " << prescale1;
    os << "\n  prescale8 " << prescale8;
    os << "\n  prescale9 " << prescale9;
    os << "\n  g1start " << g1start;
    os << "\n  g1end " << g1end;
    os << "\n  g1width " << g1width;
    os << "\n  g1delay " << g1delay;
    os << "\n  g2start " << g2start;
    os << "\n  g2end " << g2end;
    os << "\n  g2width " << g2width;
    os << "\n  g2delay " << g2delay;
    
    return os.str();      
  }

  void TriggerModule::configureTrigger( fhicl::ParameterSet const & ps )
  {
  
    TriggerModuleConfigSPtr config = std::make_shared<TriggerModuleConfig>();
    
    config->readConfiguration( ps );
    
    setFrameLength(config->framesize);
    setDeadtimeSize(config->deadtime);
    setMask0(config->trigmask0); 
    setMask1(config->trigmask1); 
    setMask8(config->trigmask8); 
    setPrescale0(config->prescale0);
    setPrescale1(config->prescale1);
    setPrescale8(config->prescale8);
    setPrescale9(config->prescale9);
    setGate1ActiveStart(config->g1start);
    setGate1ActiveEnd(config->g1end);
    setGate1Width(config->g1width);
    setGate1Delay(config->g1delay);
    setGate2ActiveStart(config->g2start);
    setGate2ActiveEnd(config->g2end);
    setGate2Width(config->g2width);
    setGate2Delay(config->g2delay);
    
    _timeinfo_file_path=config->timeinfo_file_path;
    mf::LogDebug("TriggerModule")  << config->debugInfo();
  }

  void TriggerModule::loadStatus()
  {
    // Initialize the receiver
    StatusPacket status_rec(1);
    status_rec.resize(12);
    // Prepare receiver buffer
    controller()->receive(status_rec);
    status_rec.setStart(2);
    // Send command & retrieve data with 10 us sleep time
    controller()->query( ControlDataPacket(_slot_number,
					   TriggerModule::device::trg,
					   TriggerModule::trigger::READ_STATUS),
			 status_rec,
			 10);
  
    auto word_iter = status_rec.dataBegin();
    
    // Check header (must be of the form 0xffff20**, where ** is the module number)
    if( (((*word_iter) >> 8) & 0x7) ||
	(((*word_iter) >> 8) & 0xff) != 0x20 ||
	((*word_iter) >> 16) != 0xffff 
	)      
      {
	
	_status.set_badstat();
	return; // 
	
	std::ostringstream strstrm;
	strstrm << std::endl
		<< __PRETTY_FUNCTION__ << " : unexpected TRIGGER_STATUS header format detected..." << std::endl
		<<"  Header = " << std::hex << (*word_iter) << std::endl
		<<"  check bits 7:0   (0x20)   : " << std::hex << ((*word_iter) & 0xff) << std::endl
		<<"  check bits 10:8  (0x0)    : " << std::hex << (((*word_iter)>>8) & 0x7) << std::endl
		<<"  check bits 31:16 (0xffff) : " << std::hex << ((*word_iter) >> 16) << std::endl;
	TLOG(TLVL_ERROR) << strstrm.str();
	//throw RuntimeErrorException(strstrm.str());
      }
    
    // Store
    _status.module = ((*word_iter) >> 11 & 0x1f);
    
    ++word_iter;
    _status.frame_ctr = (*word_iter);
    
    ++word_iter;
    _status.sample_ctr_tpc = ((*word_iter) & 0xfff);
    _status.sample_ctr_trigger  = _status.sample_ctr_tpc * 8;
    _status.sample_ctr_trigger += (((*word_iter) >> 16) & 0x7);
    
    ++word_iter;
    _status.trigger_ctr = (*word_iter);
    
    for(size_t bit_index=0; bit_index < _status.pmt_trigger_bit_ctr.size(); ++bit_index){
      ++word_iter;
      _status.pmt_trigger_bit_ctr[bit_index] = (*word_iter);
    }
    
    // Get last GPS time stamp
    status_rec.resize(2);
    status_rec.setStart(1);
    
    // Prepare receiver buffer
    controller()->receive(status_rec);
    status_rec.setStart(2);
    // Send command & retrieve data with 10 us sleep time
    controller()->query(ControlDataPacket(_slot_number,device::gps,GPS::GPS_TB_FRAME_SAMPLE),
			status_rec,
			10);
    
    std::ostringstream strstrm;
    
    strstrm << "\nGPSTimingSystem " <<std::endl;
    strstrm << "----------------------------" << std::endl;
    strstrm << "frame  " << ( *status_rec.dataBegin() & 0xffffff ) << std::endl;
    strstrm << "sample " << ( *(status_rec.dataBegin() + 1) & 0xfff) << std::endl;
    strstrm << "div " << ( ( *(status_rec.dataBegin() + 1) & 0x70000) >> 16 ) << std::endl;
  }

  void TriggerModule::readStatus()
  {
    
    loadStatus();
    
    std::ostringstream strstrm;
    strstrm << std::endl
	    <<"  Trigger Module [" << std::dec << (int) _slot_number << "]"  << " Status Query " << std::endl
	    << "----------------------------" << std::endl;
    mf::LogInfo("TriggerModule") << strstrm.str();
    
    getStatus().report();
    
  }

  void TriggerModule::setGate1ActiveStart(data_payload_t const &size)
  {
    controller()->send(ControlDataPacket(_slot_number, device::trg, trigger::GATE1_ACTIVE_START, size));
    TLOG(TLVL_INFO) << "TriggerModule: called " <<  __func__ << " " << size ;
  }

  void TriggerModule::setGate1ActiveEnd(data_payload_t const &size)
  {
    controller()->send(ControlDataPacket(_slot_number, device::trg, trigger::GATE1_ACTIVE_END, size));
    TLOG(TLVL_INFO) << "TriggerModule: called " <<  __func__ << " " << size ;
  }

  void TriggerModule::setGate1Width(data_payload_t const &size)
  {
    controller()->send(ControlDataPacket(_slot_number, device::trg, trigger::GATE1_WIDTH, size));
    TLOG(TLVL_INFO) << "TriggerModule: called " <<  __func__ << " " << size ;
  }

  void TriggerModule::setGate1Delay(data_payload_t const &size)
  {
    controller()->send(ControlDataPacket(_slot_number, device::trg, trigger::GATE1_DELAY, size));
    TLOG(TLVL_INFO) << "TriggerModule: called " <<  __func__ << " " << size ;
  }

  void TriggerModule::setGate2ActiveStart(data_payload_t const &size)
  {
    controller()->send(ControlDataPacket(_slot_number, device::trg, trigger::GATE2_ACTIVE_START, size));
    TLOG(TLVL_INFO) << "TriggerModule: called " <<  __func__ << " " << size ;
  }

  void TriggerModule::setGate2ActiveEnd(data_payload_t const &size)
  {
    controller()->send(ControlDataPacket(_slot_number, device::trg, trigger::GATE2_ACTIVE_END, size));
    TLOG(TLVL_INFO) << "TriggerModule: called " <<  __func__ << " " << size ;
  }

  void TriggerModule::setGate2Width(data_payload_t const &size)
  {
    controller()->send(ControlDataPacket(_slot_number, device::trg, trigger::GATE2_WIDTH, size));
    TLOG(TLVL_INFO) << "TriggerModule: called " <<  __func__ << " " << size ;
  }

  void TriggerModule::setGate2Delay(data_payload_t const &size)
  {
    controller()->send(ControlDataPacket(_slot_number, device::trg, trigger::GATE2_DELAY, size));
    TLOG(TLVL_INFO) << "TriggerModule: called " <<  __func__ << " " << size ;
  }

  void TriggerModule::setCalibDelay(data_payload_t const &size)
  {
    controller()->send(ControlDataPacket(_slot_number, device::trg, trigger::CALIB_DELAY, size));
    TLOG(TLVL_INFO) << "TriggerModule: called " <<  __func__ << " " << size ;
  }

  // Simplified version of uboonedaq TriggerModule::readTriggerBoardGPSClockRegister
  TriggerModuleGPSStamp TriggerModule::getLastGPSClockRegister()
  {
    // Prepare receiver buffer
    StatusPacket status(1,2);//mode=1, size=2
    controller()->receive(status);
    
    // Send command & retrieve data with 10 us sleep time
    status.setStart(2);
    controller()->query(ControlDataPacket(_slot_number, device::gps, GPS::GPS_TB_FRAME_SAMPLE), status, 10);
    ControlDataPacket::iterator dataIterator{status.dataBegin()};
    
    unsigned int frame = *dataIterator;
    unsigned int sample = *++dataIterator;

    frame = uint32_t(frame & 0xffffff);
    sample =  uint16_t(sample & 0xfff);

    /*
    TriggerModuleGPSStamp lastStamp( uint32_t(*dataIterator & 0xffffff), 
				     uint16_t(*++dataIterator & 0xfff), 
				     uint16_t((*dataIterator & 0x70000) >> 16) );
    */
    TriggerModuleGPSStamp lastStamp(frame,
				    sample,
				    uint16_t((*dataIterator & 0x70000) >> 16) );

    TLOG(TLVL_INFO) << "TriggerModule: called " <<  __func__ ;
    TLOG(TLVL_INFO) << "TriggerModule: last GPS timestamp frame " << lastStamp.gps_frame 
		    << " sample " << lastStamp.gps_sample 
		    << " div " << lastStamp.gps_sample_div ;

    return lastStamp;
  }

  void TriggerModule::writeTriggerTimeInfo( std::string func ) {
    TLOG(TLVL_INFO) << "TriggerModule: calling " <<  __func__ << " " << func ;

    if(_timeinfo_file_path.empty()) {
      TLOG(TLVL_INFO) << "TriggerModule: Skipping writing time info into a file (file name not yet specified)" ;
      return;
    }

    FILE* fout=fopen(_timeinfo_file_path.c_str(),"a");
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    auto now_us_tp = std::chrono::time_point_cast<std::chrono::microseconds>(now);
    auto now_us = now_us_tp.time_since_epoch().count();
    if(!fout) {
      TLOG(TLVL_ERROR) << "TriggerModule: Failed to open file " << _timeinfo_file_path ;
      TLOG(TLVL_INFO) << "TriggerModule: Elapsed time " << now_us ;
      return;
    }

    fprintf(fout,"%s %zu\n",func.c_str(),now_us);
    fclose(fout);
  }    

} // end of namespace nevistpc
