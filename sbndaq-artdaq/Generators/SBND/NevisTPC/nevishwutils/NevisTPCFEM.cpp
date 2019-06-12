#include "NevisTPCFEM.h"
#include "FPGAFirmwareReader.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "trace.h"
#define TRACE_NAME "NevisTPCFEM"

namespace nevistpc {

	void TPCFEMStatus::report() const{
  		std::ostringstream strstrm;

  		strstrm << std::endl
      			<<"  FEM Configuration Status Info Report " << std::endl
      			<< " -------------------------------------" << std::endl
      			<< std::dec
      			<< "  Config module number  : "     << config_module        << std::endl
      			<< "  Right ADC DPA locked  : "     << adc_dpa_lock_right   << std::endl
      			<< "  Left  ADC DPA locked  : "     << adc_dpa_lock_left    << std::endl
      			<< "  Right ADC PLL locked  : "     << adc_pll_lock_right   << std::endl
      			<< "  Left  ADC PLL locked  : "     << adc_pll_lock_left    << std::endl
      			<< "  SN pre-buf err        : "     << !buffer_sn           << std::endl
      			<< "  Neutrino pre-buf err  : "     << !buffer_nu           << std::endl
      			<< "  PLL locked            : "     << pll_lock             << std::endl
      			<< "  SN memory ready       : "     << mem_sn               << std::endl
      			<< "  Neutrino memory ready : "     << mem_nu               << std::endl
      			<< "  ADC align cmd  right  : "     << adc_align_cmd_right  << std::endl
      			<< "  ADC align cmd  left   : "     << adc_align_cmd_left   << std::endl
      			<< "  ADC align done right  : "     << adc_align_done_right << std::endl
      			<< "  ADC align done left   : "     << adc_align_done_left  << std::endl
      			<< " -------------------------------------" << std::endl
      			<< " FEM NU Buffer Status Info Report " << std::endl
      			<< "  Buffer module number  : "     << nu_buffer_module        << std::endl
      			<< "  NU block ID           : "     << nu_block_id          << std::endl
      			<< "  NU n-words            : "     << nu_nwords            << std::endl
      			<< " -------------------------------------" << std::endl
      			<< " FEM SN Buffer Status Info Report " << std::endl
      			<< "  Buffer module number  : "     << sn_buffer_module        << std::endl
      			<< "  SN block ID           : "     << sn_block_id          << std::endl
      			<< "  SN n-words            : "     << sn_nwords            << std::endl
      			<< " -------------------------------------" << std::endl;

    	for(size_t i=0; i < (size_t)kFEM_ERROR_TYPE_MAX; ++i) {
      		if(error_flag_v[i]) {
    			strstrm << "\033[5;1;33;41m[CRITICAL] ";
    			describe((FEMErrorFlag_t)i,strstrm);
    			strstrm << "\033[00m";
    			strstrm << std::endl;
      		}
    	}
	mf::LogInfo("TPCFEMStatus")  << strstrm.str();
	}
    bool TPCFEMStatus::isValid() const{
      bool status=true;
      for(size_t i = 0; i < error_flag_v.size(); ++i){
	if(error_flag_v.at(i)){
	  TLOG(TLVL_ERROR)<<"FEM status problem element in error_flag_v. i = "<<i;
	  status = false;
	}
      }
      return status;
    }
	void TPCFEMStatus::describe(const FEMErrorFlag_t type, std::ostringstream& strm) const{
 	 	switch(type) {
  			case kModAddressError:
    			strm << "Mismatch in module address among queried status (config, nu-buffer, and sn-buffer)"; break;
  			case kNUMemNotReady:
    			strm << "Neutrino memory block is not ready to be used"; break;
  			case kSNMemNotReady:
    			strm << "SuperNova memory block is not ready to be used"; break;
  			case kPLLLockError:
    			strm << "Failed to lock PLL (Phased Lock Loop)"; break;
  			case kNUPreBuffError:
    			strm << "Neutrino PreBuffer error"; break;
  			case kSNPreBuffError:
    			strm << "SuperNova PreBuffer error"; break;
  			case kDrainError:
    			strm << "Data drain failure (header and ADC data not drained simultaneously)"; break;
  			case kRightPLLLockError:
    			strm << "Failed to lock PLL for right ADC"; break;
  			case kLeftPLLLockError:
    			strm << "Failed to lock PLL for left ADC"; break;
  			case kRightDPALockError:
   	 			strm << "Failed to lock DPA for right ADC"; break;
  			case kLeftDPALockError:
    			strm << "Failed to lock DPA for left ADC"; break;
  			case kRightADCAlign:
    			strm << "Align failed for right ADC"; break;
  			case kLeftADCAlign:
    			strm << "Align failed for left ADC"; break;
  			case kNUDataDrainError:
    			strm << "Neutrino data drain incomplete (block ID unchanged in FEM buffer)"; break;
  			case kSNDataDrainError:
    			strm << "SuperNova data drain incomplete (block ID unchanged in FEM buffer)"; break;
  			default:
    			strm << "Not implemented for checking..."; break;
  		}
	}


  NevisTPCFEM::NevisTPCFEM ( uint8_t slot_number )
    : _slot_number( slot_number ), _status( slot_number )
  {
    TLOG(TLVL_INFO) << "NevisTPCFEM: called constructor with slot number " <<  (int)slot_number ;
  }

  NevisTPCFEM::~NevisTPCFEM()
  {
  }


    
  void NevisTPCFEM::useController ( ControllerModuleSPtr controller )
  {
    TLOG(TLVL_INFO) << "NevisTPCFEM: calling " <<  __func__ ;
    UsesControllerModule::useController ( controller );
  }

	void NevisTPCFEM::powerOnArriaFPGA(){
    	controller()->send(ControlDataPacket(_slot_number, device::ARRIA_FPGA, arria_fpga::POWER_ON));
    	TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ ;
    	usleep(200000);
	}

	void NevisTPCFEM::powerOffArriaFPGA(){
    	controller()->send(ControlDataPacket(_slot_number, device::ARRIA_FPGA, arria_fpga::POWER_OFF));
    	TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ ;
	}

	void NevisTPCFEM::configOnStratixFPGA(){
    	controller()->send(ControlDataPacket(_slot_number, device::STRATIX_FPGA_CFG, stratix_fpga_cfg::CONFIGURE_ON));
    	TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ ;
    	usleep(1000);
	}

	void NevisTPCFEM::configOffStratixFPGA(){
    	controller()->send(ControlDataPacket(_slot_number, device::STRATIX_FPGA_CFG, stratix_fpga_cfg::CONFIGURE_OFF));
    	TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ ;
	}

	void NevisTPCFEM::setLastModuleInTokenPassingOn(){
	    controller()->send(ControlDataPacket(_slot_number, device::TOKEN_PASSING_CONTROL, token_passing_control::LAST_MODULE_ON));
	    TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ ;
	}

	void NevisTPCFEM::setLastModuleInTokenPassingOff(){
    	controller()->send(ControlDataPacket(_slot_number, device::TOKEN_PASSING_CONTROL, token_passing_control::LAST_MODULE_OFF));
    	TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ ;
	}

	void NevisTPCFEM::resetTokenPassingReceiver(){
    	controller()->send(ControlDataPacket(_slot_number, device::TOKEN_PASSING_CONTROL, token_passing_control::TP_RECEIVER_RESET));
    	TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ ;
	}

	void NevisTPCFEM::pulseTokenPassingAllignmentCircuit(){
    	controller()->send(ControlDataPacket(_slot_number, device::TOKEN_PASSING_CONTROL, token_passing_control::PULSE_TP_RECEIVER));
    	TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ ;
	}

	void NevisTPCFEM::resetPLLLink(){
    	controller()->send(ControlDataPacket(_slot_number,device::TOKEN_PASSING_CONTROL, token_passing_control::PLL_LINK_RESET));
		TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ ;
	}

	void NevisTPCFEM::loadSlowMonitorInfo(){
  		loadStatus();
  		loadCounters();
	}

	void NevisTPCFEM::readSlowMonitorInfo(){
  		readStatus();
 		readCounters();
	}

	void NevisTPCFEM::readStatus(){
  		loadStatus();

  		getStatus().report();

  		TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ ;
	}

	void NevisTPCFEM::loadStatus(){

		//// Readout FEM Config Status

		// Init receiver
		 StatusPacket status_rec(1);
  		status_rec.resize(2);
  		controller()->receive(status_rec);
  		// Prepare receiver buffer
  		status_rec.setStart(2);
  		// Send command & read FEM status w/ 10 us sleep time
  		controller()->query(ControlDataPacket(_slot_number, device::STRATIX_FPGA, stratix_fpga::READ_CONFIG_STATUS),
	       status_rec,
	       10);

  		uint32_t lo {*status_rec.dataBegin() };
  		uint32_t hi {*(status_rec.dataBegin()+1) };

		// Check header
  		if( (lo & 0xff) != 0x14 || ((lo>>8) & 0x7) ) {
    		_status.set_badstat();
    		return;
    		std::ostringstream strstrm;
    		strstrm << std::endl
					<< __PRETTY_FUNCTION__ << " : unexpected FEM STATUS header format detected..." << std::endl
				<<"  Header = " << std::hex << lo << ","<<hi<<std::endl
				<<"  check bits 7:0   (0x14) : " << std::hex << (lo & 0xff) << std::endl
				<<"  check bits 10:8  (0x0)  : " << std::hex << ((lo >> 8) & 0x7) << std::endl;
    		TLOG(TLVL_INFO) << "NevisTPCFEM: " << strstrm.str();
  		}

  		_status.config_module = (lo >> 11) & 0x1f;
  		_status.adc_dpa_lock_right   = (lo >> 17) & 0x1;
  		_status.adc_dpa_lock_left    = (lo >> 18) & 0x1;
  		_status.buffer_sn            = (lo >> 19) & 0x1;
  		_status.buffer_nu            = (lo >> 20) & 0x1;
 	 	_status.pll_lock             = (lo >> 21) & 0x1;
  		_status.mem_sn               = (lo >> 22) & 0x1;
  		_status.mem_nu               = (lo >> 23) & 0x1;
  		_status.adc_pll_lock_right   = (lo >> 24) & 0x1;
  		_status.adc_pll_lock_left    = (lo >> 25) & 0x1;
  		_status.adc_align_cmd_right  = (lo >> 26) & 0x1;
  		_status.adc_align_cmd_left   = (lo >> 27) & 0x1;
  		_status.adc_align_done_right = (lo >> 28) & 0x1;
  		_status.adc_align_done_left  = (lo >> 29) & 0x1;
  		_status.nu_data_empty        = (lo >> 30) & 0x1;
  		_status.nu_header_empty      = (lo >> 31) & 0x1;

  		_status.buffer_nu = !_status.buffer_nu;
  		_status.buffer_sn = !_status.buffer_sn;

  		// Before reading out buffer status, store the older block_id & nwords to the record
  		_status.old_nu_block_id = _status.nu_block_id;
  		_status.old_nu_nwords   = _status.nu_nwords;
  		_status.old_sn_block_id = _status.sn_block_id;
  		_status.old_sn_nwords   = _status.sn_nwords;


  		//// Readout FEM NU Buffer Status

		StatusPacket nu_buff_rec(1);
  		// Prepare receiver buffer
  		controller()->receive(nu_buff_rec);
  		nu_buff_rec.setStart(2);
  		nu_buff_rec.resize(2);
  		// Send command & read FEM status w/ 10 us sleep time
  		controller()->query(ControlDataPacket(_slot_number, device::STRATIX_FPGA, stratix_fpga::READ_NU_BUFFER_STATUS), nu_buff_rec, 10);

  		lo = {*nu_buff_rec.dataBegin()};
  		hi = {*(nu_buff_rec.dataBegin()+1)};

  		// Check header
  		if( (lo>>16)    != 0xa255 || (lo & 0xff) != 0x22   || (lo>>8 & 0x7) ) {
    		std::ostringstream strstrm;
    		strstrm << std::endl
				<< __PRETTY_FUNCTION__ << " : unexpected FEM NU-BUFFER header format detected..." << std::endl
				<<"  Header = " << std::hex << lo << ","<<hi<<std::endl
				<<"  check bits 7:0   (0x22)   : " << std::hex << (lo & 0xff)   << std::endl
				<<"  check bits 10:8  (0x0)    : " << std::hex << (lo>>8 & 0x7) << std::endl
				<<"  check bits 32:16 (0xa255) : " << std::hex << (lo>>16)      << std::endl;
    		TLOG(TLVL_INFO) << "NevisTPCFEM: " << strstrm.str();
  		}

  		_status.nu_buffer_module = ((lo>>11) & 0x1f);
  		_status.nu_nwords        = (((hi >> 16) & 0xffff)+ ((hi & 0xff)<< 16));
  		_status.nu_block_id      = ((hi>>8) & 0xff);

  		//// Readout FEM SN Buffer Status

		StatusPacket sn_buff_rec(1);
  		sn_buff_rec.resize(2);
  		// Prepare receiver buffer
  		controller()->receive(sn_buff_rec);
  		sn_buff_rec.setStart(2);
  		// Send command & read FEM status w/ 10 us sleep time
  		controller()->query(ControlDataPacket(_slot_number, device::STRATIX_FPGA, stratix_fpga::READ_SN_BUFFER_STATUS), sn_buff_rec, 10);

  		lo = {*sn_buff_rec.dataBegin()};
  		hi = {*(sn_buff_rec.dataBegin()+1)};

  		// Check header
  		if( (lo>>16)    != 0xa255 || (lo & 0xff) != 0x23   || (lo>>8 & 0x7) ) {
    		std::ostringstream strstrm;
    		strstrm << std::endl
				<< __PRETTY_FUNCTION__ << " : unexpected FEM SN-BUFFER header format detected..." << std::endl
				<<"  Header = " << std::hex << lo << ","<<hi<<std::endl
				<<"  check bits 7:0   (0x23)   : " << std::hex << (lo & 0xff)   << std::endl
				<<"  check bits 10:8  (0x0)    : " << std::hex << (lo>>8 & 0x7) << std::endl
				<<"  check bits 32:16 (0xa255) : " << std::hex << (lo>>16)      << std::endl;
    		TLOG(TLVL_INFO) << "NevisTPCFEM: " << strstrm.str() ;
  		}

  		_status.sn_buffer_module = ((lo>>11) & 0x1f);
  		_status.sn_nwords        = (((hi >> 16) & 0xffff)+ ((hi & 0xff)<< 16));
  		_status.sn_block_id      = ((hi>>8) & 0xff);

  		//// Evaluate error flags
  		_status.error_flag_v[ kModAddressError   ] = ( _status.config_module != _status.nu_buffer_module );
  		_status.error_flag_v[ kPLLLockError      ] = !_status.pll_lock;
  		_status.error_flag_v[ kDrainError        ] = (_status.nu_data_empty != _status.nu_header_empty);
  		_status.error_flag_v[ kRightPLLLockError ] = !_status.adc_pll_lock_right;
  		_status.error_flag_v[ kLeftPLLLockError  ] = !_status.adc_pll_lock_left;
  		_status.error_flag_v[ kRightDPALockError ] = !_status.adc_dpa_lock_right;
  		_status.error_flag_v[ kLeftDPALockError  ] = !_status.adc_dpa_lock_left;
  		_status.error_flag_v[ kRightADCAlign     ] = !_status.adc_align_cmd_right || !_status.adc_align_done_right;
  		_status.error_flag_v[ kLeftADCAlign      ] = !_status.adc_align_cmd_left  || !_status.adc_align_done_left;
  		_status.error_flag_v[ kNUDataDrainError  ] = ( _status.nu_block_id > 5 &&
			(_status.nu_block_id == _status.old_nu_block_id &&
		_status.nu_nwords   == _status.old_nu_nwords ) );
	}

  void NevisTPCFEM::programStratixFPGAFirmware( std::string const& firmwarename ){

    ControlDataPackets ControlDataPackets;

    ControlDataPacket target ( _slot_number, device::STRATIX_FPGA_PROGRAM, stratix_fpga_program::PROGRAM_FIRMWARE );
    FPGAFirmwareReader reader (firmwarename );
    std::size_t size = reader.readAndEncodeFor ( target, ControlDataPackets );

    assert ( size != 0 );
    (void)(size); // Avoid compiler errors: unused variable

    //close stratix programming cycle thru aria fpga by adding a trailer to the last packet
    auto packet = ControlDataPackets.end() - 1;
    packet->resize ( packet->size() + 1 );
    //why do i need 0x80 ??
    * (packet->dataBegin() + packet->size() - 1) = (data_payload_t)(_slot_number << 27) + (device::STRATIX_FPGA_CFG << 24) + 0x80;

    for ( auto packet : ControlDataPackets ) {
      controller()->send ( packet, 128);
    }

    TLOG(TLVL_INFO) << "NevisTPCFEM: called " << __func__ << " with args " << firmwarename;
    usleep(2000);
  }

	void NevisTPCFEM::disableNUChanCompression(data_payload_bool_t const &flag){
    	controller()->send(ControlDataPacket(_slot_number, device::STRATIX_FPGA, stratix_fpga::NU_CHAN_COMPRESSION, flag));
		TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ << " with " << flag;
	}

	void NevisTPCFEM::disableSNChanCompression(data_payload_bool_t const &flag){
    	controller()->send(ControlDataPacket(_slot_number, device::STRATIX_FPGA, stratix_fpga::SN_CHAN_COMPRESSION, flag));
    	TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ << " with " << flag;
	}

	void NevisTPCFEM::setBlockSize(data_payload_t const &size){
    	controller()->send(ControlDataPacket(_slot_number, device::STRATIX_FPGA, stratix_fpga::BLOCK_SIZE, size));
    	TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ << " with " << size;
	}

	void NevisTPCFEM::setDriftTimeSize(data_payload_t const &size){
	    controller()->send(ControlDataPacket(_slot_number, device::STRATIX_FPGA, stratix_fpga::DRIFT_TIME_SIZE, size));
	    TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ << " with " << size;
	}

	void NevisTPCFEM::setModuleNumber(data_payload_t const &number){
	    controller()->send(ControlDataPacket(_slot_number, device::STRATIX_FPGA, stratix_fpga::MODULE_NUMBER, number));
	    TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ << " with " << number;
	}

	void NevisTPCFEM::setNUChannelID(data_payload_t const &id){
	    controller()->send(ControlDataPacket(_slot_number, device::STRATIX_FPGA, stratix_fpga::NU_CHAN_ID, id));
	    TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ << " with " << id;
	}

	void NevisTPCFEM::setSNChannelID(data_payload_t const &id){
	    controller()->send(ControlDataPacket(_slot_number, device::STRATIX_FPGA, stratix_fpga::SN_CHAN_ID, id));
	    TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ << " with " << id;
	}

	void NevisTPCFEM::setPrebufferSize(data_payload_t const &size){
	    controller()->send(ControlDataPacket(_slot_number, device::STRATIX_FPGA, stratix_fpga::PREBUFFER_SIZE, size));
	    TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ << " with " << size;
	}

	void NevisTPCFEM::setTestMode(data_payload_t const &mode){
	    controller()->send(ControlDataPacket(_slot_number, device::STRATIX_FPGA, stratix_fpga::TEST_MODE, mode));
	    TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ << " with " << mode;
	}

	void NevisTPCFEM::tm1_testSample(data_payload_t const &opt){
	    controller()->send(ControlDataPacket(_slot_number, device::STRATIX_FPGA, stratix_fpga::TM1_SAMPLE, opt));
	    TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ << " with " << opt;
	}

	void NevisTPCFEM::tm1_testFrame(data_payload_t const &opt){
	    controller()->send(ControlDataPacket(_slot_number, device::STRATIX_FPGA, stratix_fpga::TM1_FRAME, opt));
	    TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ << " with " << opt;
	}

	void NevisTPCFEM::tm1_testChannel(data_payload_t const &opt){
	    controller()->send(ControlDataPacket(_slot_number, device::STRATIX_FPGA, stratix_fpga::TM1_CHANNEL, opt));
	    TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ << " with " << opt;
	}

	void NevisTPCFEM::tm1_testPulseValue(data_payload_t const &opt){
	    controller()->send(ControlDataPacket(_slot_number, device::STRATIX_FPGA, stratix_fpga::TM1_PULSE_VALUE, opt));
	    TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ << " with " << opt;
	}

	void NevisTPCFEM::tm1_testBaselineValue(data_payload_t const &opt){
	    controller()->send(ControlDataPacket(_slot_number, device::STRATIX_FPGA, stratix_fpga::TM1_BASELINE_VALUE, opt));
		TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ << " with " << opt;
	}

	void NevisTPCFEM::tm2_testRamData(data_payload_t const &opt){
	    controller()->send(ControlDataPacket(_slot_number, device::STRATIX_FPGA, stratix_fpga::FEB_TEST_RAM_DATA, opt));
		TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ << " with " << opt;
	}

	void NevisTPCFEM::enableNUDataSlowReadback(data_payload_bool_t const &flag){
	    controller()->send(ControlDataPacket(_slot_number, device::STRATIX_FPGA, stratix_fpga::NU_DATA_SLOW_READBACK, flag));
	    TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ << " with " << flag;
	}

	void NevisTPCFEM::enableSNDataSlowReadback(data_payload_bool_t const &flag){
	    controller()->send(ControlDataPacket(_slot_number, device::STRATIX_FPGA, stratix_fpga::SN_DATA_SLOW_READBACK, flag));
	    TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ << " with " << flag;
	}

	void NevisTPCFEM::setNUMaxBufferSize(data_payload_t const &size){
	    controller()->send(ControlDataPacket(_slot_number, device::STRATIX_FPGA, stratix_fpga::NU_MAX_BUFFER, size));
	    TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ << " with " << size;
	}

	void NevisTPCFEM::setSNMaxBufferSize(data_payload_t const &size){
 	   controller()->send(ControlDataPacket(_slot_number, device::STRATIX_FPGA, stratix_fpga::SN_MAX_BUFFER, size));
 	   TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ << " with " << size;
	}

	void NevisTPCFEM::enableLinkPortHold(data_payload_bool_t const &flag){
    	controller()->send(ControlDataPacket(_slot_number, device::STRATIX_FPGA, stratix_fpga::LINK_PORT_HOLD_ENABLE, flag));
		TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ << " with " << flag;
	}

	void NevisTPCFEM::readEnablePulseNUEventHeader(data_payload_bool_t const &flag){
    	controller()->send(ControlDataPacket(_slot_number, device::STRATIX_FPGA, stratix_fpga::NU_HEADER_READ_PULSE, flag));
    	TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ << " with " << flag;
	}

	void NevisTPCFEM::readEnablePulseSNEventHeader(data_payload_bool_t const &flag){
    	controller()->send(ControlDataPacket(_slot_number, device::STRATIX_FPGA, stratix_fpga::SN_HEADER_READ_PULSE, flag));
    	TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ << " with " << flag;
	}

	void NevisTPCFEM::readEnablePulseNUEventData(data_payload_bool_t const &flag){
    	controller()->send(ControlDataPacket(_slot_number, device::STRATIX_FPGA, stratix_fpga::NU_DATA_READ_PULSE, flag));
    	TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ << " with " << flag;
	}

	void NevisTPCFEM::readEnablePulseSNEventData(data_payload_bool_t const &flag){
    	controller()->send(ControlDataPacket(_slot_number, device::STRATIX_FPGA, stratix_fpga::SN_DATA_READ_PULSE, flag));
    	TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ << " with " << flag;
	}

	void NevisTPCFEM::readPulseForTestMode1(data_payload_bool_t const &flag){
    	controller()->send(ControlDataPacket(_slot_number, device::STRATIX_FPGA, stratix_fpga::TM1_READ_PULSE, flag));
    	TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ << " with " << flag;
	}

	void NevisTPCFEM::resetDRAM(data_payload_bool_t const &flag){
    	controller()->send(ControlDataPacket(_slot_number, device::STRATIX_FPGA, stratix_fpga::DRAM_RESET, flag));
    	TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ << " with " << flag;
	}

	void NevisTPCFEM::resetDRAMUserLogic(){
    	controller()->send(ControlDataPacket(_slot_number, device::STRATIX_FPGA, stratix_fpga::DRAM_USER_LOGIC_RESET));
    	TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ ;
	}

	void NevisTPCFEM::prebufferStatusNU(){
	  //controller()->send(ControlDataPacket(_slot_number, device::STRATIX_FPGA, stratix_fpga::NU_PREBUFFER_STATUS));
	  controller()->send(ControlDataPacket(_slot_number, device::STRATIX_FPGA, stratix_fpga::READ_NU_BUFFER_STATUS));
    	TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ ;
	}

	void NevisTPCFEM::prebufferStatusSN(){
   		// controller()->send ( ControlDataPacket ( this, device::STRATIX_FPGA, stratix_fpga::SN_PREBUFFER_STATUS ) );
	  controller()->send ( ControlDataPacket ( _slot_number, device::STRATIX_FPGA, stratix_fpga::READ_SN_BUFFER_STATUS ) );
    	TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ ;
	}

	void NevisTPCFEM::setLoadThreshold(data_payload_t const &chan, data_payload_t const &size){
	    controller()->send(ControlDataPacket(_slot_number, device::STRATIX_FPGA, stratix_fpga::LOAD_THRESHOLD + chan, size));
    	usleep(10);
    	TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ << " with " << chan << ", " << size;
	}

	void NevisTPCFEM::setLoadThresholdMean(data_payload_t const &size){
	    controller()->send(ControlDataPacket(_slot_number, device::STRATIX_FPGA, stratix_fpga::LOAD_THRESHOLD_MEAN, size));
	    TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ << " with " << size;
	}

	void NevisTPCFEM::setLoadThresholdVariance(data_payload_t const &size){
	    controller()->send(ControlDataPacket(_slot_number, device::STRATIX_FPGA, stratix_fpga::LOAD_THRESHOLD_VARIANCE, size));
	    TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ << " with " << size;
	}

	void NevisTPCFEM::setLoadPresample(data_payload_t const &size){
	    controller()->send(ControlDataPacket(_slot_number, device::STRATIX_FPGA, stratix_fpga::LOAD_PRESAMPLE, size));
	    TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ << " with " << size;
	}

	void NevisTPCFEM::setLoadPostsample(data_payload_t const &size){
    	controller()->send(ControlDataPacket(_slot_number, device::STRATIX_FPGA, stratix_fpga::LOAD_POSTSAMPLE, size));
    	TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ << " with " << size ;
	}

	void NevisTPCFEM::setChannelThreshold(data_payload_bool_t const &flag){
    	controller()->send(ControlDataPacket(_slot_number, device::STRATIX_FPGA, stratix_fpga::CHANNEL_THRESHOLD, flag));
    	TLOG(TLVL_INFO) << "NevisTPCFEM: called " <<  __func__ << " with " << flag;
	}

	void NevisTPCFEM::fem_setup(fhicl::ParameterSet const& crateConfig){
	// Power On arria power supply
	powerOnArriaFPGA();
	// config on stratix fpga
	configOnStratixFPGA();
	// program stratix firmware
	programStratixFPGAFirmware(crateConfig.get<std::string>("fem_fpga",""));
	TLOG(TLVL_INFO)<<crateConfig.get<std::string>("fem_fpga","") ;

	// Uncomment block when using the FEM JTAG connector to deploy firmware
	// std::cout << "\nPause for using JTAG connector\n" << std::endl;
	// int aux;
	// std::cin >> aux;

	// Turn DRAM reset on
	resetDRAM(1);
	// Turn DRAM reset off
	resetDRAM(0);

	// Set module number
	setModuleNumber(_slot_number);

		// Set compression
		//set whether you want to use huffman compression (mb_feb_a_nocomp)
  		//0 for use compression, 1 for don't use compression
  		bool compress = !( crateConfig.get<bool>( "nu_compress", false )); //default off
  		TLOG(TLVL_INFO)<<"HEY! COMPRESS VALUE IS " << compress << std::endl;
  		disableNUChanCompression(compress); //default off
  		disableSNChanCompression(!( crateConfig.get<bool>( "sn_compress", true )));

  		//set mb_feb_timesize
  		setDriftTimeSize( crateConfig.get<uint32_t>("timesize",3199) );

  		//set mb_feb_b_id to chip3
  		setSNChannelID(0xf);
  		//set mb_feb_b_id to chip4 //???there is no chip4?
  		//going to guess this is what was meant
  		setNUChannelID(0xf);

  		//set mb_feb_max to 8000
  		setPrebufferSize(8000);

  		//set mb_feb_hold_enable
  		enableLinkPortHold(1);

    }
}
