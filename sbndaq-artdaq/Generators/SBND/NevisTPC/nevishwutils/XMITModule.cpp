#include "XMITModule.h"
#include "FPGAFirmwareReader.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "trace.h"
#define TRACE_NAME "XMITModule"

namespace nevistpc {

  void XMITModuleStatus::describe(const XMITErrorFlag_t type, std::ostringstream &strm) const
  {
    switch(type) {
    case kAlignment:
      strm << "Alignment error" << std::endl; break;
    case kRecTimeOut:
      strm << "Receiver timeout" << std::endl; break;
    case kNUOptConn:
      strm << "NU optical connection not detected" << std::endl; break;
    case kSNOptConn:
      strm << "SN optical connection not detected" << std::endl; break;
    case kNUBusy:
      strm << "NU status busy" << std::endl; break;
    case kSNBusy:
      strm << "SN status busy" << std::endl; break;
    case kNULock:
      strm << "NU optical transmitter lock failure" << std::endl; break;
    case kSNLock:
      strm << "SN optical transmitter lock failure" << std::endl; break;
    case kTokenDPALock:
      strm << "DPA lock failure (token passing)" << std::endl; break;
    case kTokenRecLock:
      strm << "Receiver lock failure (token passing)" << std::endl; break;
    case kPLLLock:
      strm << "PLL lock failure" << std::endl; break;
    default:
    strm << "UNKNOWN ERROR TYPE!" << std::endl;
    break;
    };
  }


      bool XMITModuleStatus::isValid() const
    {
    for(size_t i = 0; i < error_flag_v.size(); ++i){
      //for(auto const& b : error_flag_v) if(b) return false; return true; }
      if(error_flag_v.at(i)){
	TLOG(TLVL_ERROR) << "XMIT status problem element in error_flag_v. i = "<<i;
	return false; 
      }
    }
    return true; 
    }
    
  void XMITModuleStatus::report() const
  {
    std::ostringstream strstrm;
    
    strstrm << std::endl
	    << "  XMIT Module Status Info Report " << std::endl
	    << " --------------------------------" << std::endl
	    << "  crate number          : "      << std::dec << config_crate << std::endl
	    << "  Alignment 1           : "      << align1        << std::endl
	    << "  Alignment 2           : "      << align2        << std::endl
	    << "  Timeout 1             : "      << timeout1      << std::endl
	    << "  Timeout 2             : "      << timeout2      << std::endl
	    << "  Optical status NU1    : "      << optical_nu1   << std::endl
	    << "  Optical status NU2    : "      << optical_nu2   << std::endl
	    << "  Optical status SN1    : "      << optical_sn1   << std::endl
	    << "  Optical status SN2    : "      << optical_sn2   << std::endl
	    << "  NU busy status        : "      << busy_nu       << std::endl
	    << "  SN busy status        : "      << busy_sn       << std::endl
	    << "  NU optical locked     : "      << opt_lock_nu   << std::endl
	    << "  SN optical locked     : "      << opt_lock_sn   << std::endl
	    << "  Token DPA  locked     : "      << dpa_lock      << std::endl
	    << "  Token receiver locked : "      << receiver_lock << std::endl
	    << "  PLL locked            : "      << pll_lock      << std::endl
	    << " ------------------------------------- " << std::endl
	    << "  XMIT Counter Info Report " << std::endl
	    << "    crate ID          : " << buffer_crate       << std::endl
	    << "    frame number      : " << frame_ctr   << std::endl
	    << "    trigger received  : " << event_ctr   << std::endl
	    << "    packed event (nu) : " << nu_read_ctr << std::endl
	    << "    packed event (sn) : " << sn_read_ctr << std::endl;
    strstrm << std::endl;
    
    TLOG(TLVL_INFO) << "XMITModuleStatus"  << strstrm.str();
    mf::LogInfo("XMITModuleStatus")  << strstrm.str();
}

  XMITModule::XMITModule ( uint8_t slot_number )
    : _slot_number(slot_number), _status(), _nu_stream_enabled(false), _sn_stream_enabled(false)
  {
    TLOG(TLVL_INFO) << "XMITModule: called constructor with slot number " <<  (int)slot_number ;
  }

  XMITModule::~XMITModule()
  {
  }

  void XMITModule::setMax3000Config()
  {
    controller()->send ( ControlDataPacket( _slot_number, device::MAX3000_FPGA, max3000_FPGA::SET_CONFIG_MODE ) );
    TLOG(TLVL_INFO) << "XMITModule: called " <<  __func__ ;
    usleep(1000);
  }

  void XMITModule::setFEMModuleCount ( data_payload_t const& size )
  {
    controller()->send ( ControlDataPacket( _slot_number, device::XMIT_CHIP, xmit_chip::XMIT_MODCOUNT, size ) );
    TLOG(TLVL_INFO) << "XMITModule: called " <<  __func__ << " " << size ;
  }

  void XMITModule::enableNUChanEvents ( data_payload_bool_t const& flag )
  {
    controller()->send ( ControlDataPacket( _slot_number, device::XMIT_CHIP, xmit_chip::XMIT_ENABLE_NU_EVENTS, flag ) );
    _nu_stream_enabled = flag;
    TLOG(TLVL_INFO) << "XMITModule: called " <<  __func__ << " " << flag ;
  }

  void XMITModule::enableSNChanEvents ( data_payload_bool_t const& flag )
  {
    controller()->send ( ControlDataPacket( _slot_number, device::XMIT_CHIP, xmit_chip::XMIT_ENABLE_SN_EVENTS, flag ) );
    _sn_stream_enabled = flag;
    TLOG(TLVL_INFO) << "XMITModule: called " <<  __func__ << " " << flag ;
  }

  void XMITModule::enableSNChanTest ( data_payload_bool_t const& flag )
  {
    controller()->send ( ControlDataPacket( _slot_number, device::XMIT_CHIP, xmit_chip::XMIT_SN_FIBER_TEST, flag ) );
    TLOG(TLVL_INFO) << "XMITModule: called " <<  __func__ << " " << flag ;
  }

  void XMITModule::enableNUChanTest ( data_payload_bool_t const& flag )
  {
    controller()->send ( ControlDataPacket( _slot_number, device::XMIT_CHIP, xmit_chip::XMIT_NU_FIBER_TEST, flag ) );
    TLOG(TLVL_INFO) << "XMITModule: called " <<  __func__ << " " << flag ;
  }

  void XMITModule::uploadTestDataPacket ( data_payload_t const& packet )
  {
    controller()->send ( ControlDataPacket( _slot_number, device::XMIT_CHIP, xmit_chip::XMIT_TESTDATA, packet ) );
    TLOG(TLVL_INFO) << "XMITModule: called " <<  __func__ ;
  }

  void XMITModule::loadStatus()
  {
    // Initialize reciever
    StatusPacket status_rec ( 1 );
    status_rec.resize (1);
    controller()->receive ( status_rec );
    // Prepare reciever buffer
    status_rec.setStart (2);
    
    
    // Send command & retreave status w/ 10 us sleep
    controller()->query( ControlDataPacket( _slot_number, device::XMIT_CHIP, xmit_chip::XMIT_RDSTATUS ),
			      status_rec,
			      10);
    
    // Parse it into a dedicated container
    uint32_t lo {*status_rec.dataBegin() };
    
    // Check header (must be of the form 0x****ffyy (yy should be binary: 000z zzzz), zzzzz is crate #
    // Binary digit 21 is always zero
    if( ((lo>>5) & 0x7) ||
	(((lo>>8) & 0xff) != 0xff) ||
	((lo>>21) & 0x1) ) {
      
      _status.set_badstat();
      return; // 
      
      std::ostringstream strstrm;
      strstrm << std::endl
	      << __PRETTY_FUNCTION__ << " : unexpected XMIT-STATUS header format detected..." << std::endl
	      <<"  Header = " << std::hex << lo << std::endl
	      <<"  check bits 7:5   (0x0)  : " << std::hex << ((lo>>5) & 0x7) << std::endl
	      <<"  check bits 15:8  (0xff) : " << std::hex << ((lo >> 8) & 0xff) << std::endl
	      <<"  check bits 21:21 (0x0)  : " << std::hex << ((lo >> 21) & 0x1) << std::endl;
      TLOG(TLVL_ERROR) << strstrm.str();
      //    throw RuntimeErrorException(strstrm.str());
    }
    
    // Store
    _status.pll_lock      = (lo >> 16) & 0x1;
    _status.receiver_lock = (lo >> 17) & 0x1;
    _status.dpa_lock      = (lo >> 18) & 0x1;
    _status.opt_lock_nu   = (lo >> 19) & 0x1;
    _status.opt_lock_sn   = (lo >> 20) & 0x1;
    _status.busy_sn       = (lo >> 22) & 0x1;
    _status.busy_nu       = (lo >> 23) & 0x1;
    _status.optical_sn2   = (lo >> 24) & 0x1;
    _status.optical_sn1   = (lo >> 25) & 0x1;
    _status.optical_nu2   = (lo >> 26) & 0x1;
    _status.optical_nu1   = (lo >> 27) & 0x1;
    _status.timeout1      = (lo >> 28) & 0x1;
    _status.timeout2      = (lo >> 29) & 0x1;
    _status.align1        = (lo >> 30) & 0x1;
    _status.align2        = (lo >> 31) & 0x1;
    
    _status.timeout1 = !_status.timeout1;
    _status.timeout2 = !_status.timeout2;
    _status.align1   = !_status.align1;
    _status.align2   = !_status.align2;
    
    _status.error_flag_v[ kAlignment    ] = !_status.align1      || !_status.align2;
    //do NOT raise error flag if there is a problem with SN timeout, if SN isn't in the picture
    //at this point, _status.timeoutx is 0 if things are going well, 1 if there is a problem.
    if( _nu_stream_enabled && _sn_stream_enabled )
      _status.error_flag_v[ kRecTimeOut   ] = _status.timeout1    || _status.timeout2;
    else if ( _nu_stream_enabled && !_sn_stream_enabled )
      _status.error_flag_v[ kRecTimeOut   ] = _status.timeout1;
    
    _status.error_flag_v[ kNUOptConn    ] = !_status.optical_nu1 || !_status.optical_nu2;
    //_status.error_flag_v[ kSNOptConn    ] = !_status.optical_sn1 || !_status.optical_sn2;
    _status.error_flag_v[ kNUBusy       ] = _status.busy_nu;
    //_status.error_flag_v[ kSNBusy       ] = _status.busy_sn;
    _status.error_flag_v[ kNULock       ] = !_status.opt_lock_nu;
    //_status.error_flag_v[ kSNLock       ] = !_status.opt_lock_sn;
    _status.error_flag_v[ kTokenDPALock ] = !_status.dpa_lock;
    _status.error_flag_v[ kTokenRecLock ] = !_status.receiver_lock;
    _status.error_flag_v[ kPLLLock      ] = !_status.pll_lock;
    
    // Initialize reciever
    StatusPacket ctr_rec (1);
    ctr_rec.resize(6);
    controller()->receive (ctr_rec);
    // Prepare reciever buffer
    ctr_rec.setStart (2);
    
    // Send command & retreave status w/ 100 us sleep
    controller()->query( ControlDataPacket( _slot_number, device::XMIT_CHIP, xmit_chip::XMIT_RDCOUNTERS ),
			      ctr_rec,
			      100);
  
    uint32_t ctr_header {*ctr_rec.dataBegin() };
    uint32_t ctr_frame {*(ctr_rec.dataBegin()+1)};
    uint32_t ctr_event {*(ctr_rec.dataBegin()+2)};
    uint32_t ctr_nu    {*(ctr_rec.dataBegin()+3)};
    uint32_t ctr_sn    {*(ctr_rec.dataBegin()+4)};
    
    // Check the header format
    if( ((ctr_header >> 5) & 0x7) ||
	(((ctr_header >> 8) & 0xff) != 0xff) ||
	(((ctr_header >> 16) & 0xff) != 0xaa) ||
	(((ctr_header >> 24) & 0xff) != 0x55) ) {
      
      std::ostringstream strstrm;    
      strstrm << std::endl
	      << __PRETTY_FUNCTION__ << " : unexpected XMIT-COUNTER header format detected..." << std::endl
	      <<"  Data = " << std::hex 
	      << ctr_header << " : " << ctr_frame << " : " << ctr_event << " : " << ctr_nu << " : " 
	      << ctr_sn << std::endl
	      <<"  check bits 7:5   (0x0)  : " << std::hex << ((ctr_header>>5) & 0x7) << std::endl
	      <<"  check bits 8:15  (0xaa) : " << std::hex << ((ctr_header >> 8) & 0xff) << std::endl
	      <<"  check bits 15:23 (0xaa) : " << std::hex << ((ctr_header >> 16) & 0xff) << std::endl
	      <<"  check bits 24:31 (0xff) : " << std::hex << ((ctr_header >> 24) & 0xff) << std::endl;
      TLOG(TLVL_ERROR) << strstrm.str();
    //    throw RuntimeErrorException(strstrm.str());
    }
    
    _status.buffer_crate = ctr_header & 0x1f;
    
    _status.frame_ctr = ctr_frame;
    
    _status.event_ctr = ctr_event;
    
    _status.nu_read_ctr = ctr_nu;
    
    _status.sn_read_ctr = ctr_sn;
    
  }

  void XMITModule::readStatus()
  {
    loadStatus();
    
    getStatus().report();
    
    TLOG(TLVL_INFO) << "XMITModule: called " <<  __func__ ;
  }
  
  void XMITModule::resetLink()
  {
    controller()->send ( ControlDataPacket( _slot_number, device::XMIT_CHIP, xmit_chip::XMIT_LINK_RESET, 1 ) );
    TLOG(TLVL_INFO) << "XMITModule: called " <<  __func__ ;
    usleep(10000);
  }
  
  void XMITModule::resetOpticalTransceiver()
  {
    controller()->send ( ControlDataPacket( _slot_number, device::XMIT_CHIP, xmit_chip::XMIT_RESET_OPT_TRANSCEIVER, 1 ) );
    TLOG(TLVL_INFO) << "XMITModule: called " <<  __func__ ;
    usleep(10000);
  }
  
  void XMITModule::resetDPA()
  {
    controller()->send ( ControlDataPacket( _slot_number, device::XMIT_CHIP, xmit_chip::XMIT_DPA_FIFO_RESET, 1 ) );
    TLOG(TLVL_INFO) << "XMITModule: called " <<  __func__ ;
  }
  
  void XMITModule::alignDPA()
  {
    controller()->send ( ControlDataPacket( _slot_number, device::XMIT_CHIP, xmit_chip::XMIT_DPA_WORD_ALIGN, 1 ) );
    TLOG(TLVL_INFO) << "XMITModule: called " <<  __func__ ;
    usleep(1000);
  }
  
  void XMITModule::resetPLLLink()
  {
    controller()->send ( ControlDataPacket( _slot_number, device::XMIT_CHIP, xmit_chip::XMIT_LINK_PLL_RESET, 1 ) );
    TLOG(TLVL_INFO) << "XMITModule: called " <<  __func__ ;
  }
  
  void XMITModule::useController ( ControllerModuleSPtr controller )
  {
    TLOG(TLVL_INFO) << "XMITModule: calling " <<  __func__ ;
    UsesControllerModule::useController ( controller );
  }

  void XMITModule::programMax3000FPGAFirmware ( std::string const& firmwareName )
  {
    ControlDataPackets control_packets;

    ControlDataPacket target ( _slot_number, device::MAX3000_FPGA_PROGRAM, 0 );

    FPGAFirmwareReader reader ( firmwareName );
    std::size_t size = reader.readAndEncodeFor ( target, control_packets );
    
    assert ( size != 0 );
    (void)(size); // Avoid compiler error: unused variable    

    //close stratix programming cycle thru aria fpga by adding a trailer to the last packet
    auto packet = control_packets.end() - 1;
    packet->resize ( packet->size() + 2 );
    
    //pad with 0??
    * ( packet->dataBegin() + packet->size() - 2 ) = 0;

    //closes programming channel
    * ( packet->dataBegin() + packet->size() - 1 ) = ( data_payload_t ) ( _slot_number << 27 ) + ( device::MAX3000_FPGA << 24 );

    for ( auto packet : control_packets ) {
      controller()->send ( packet, 128);
    }

    TLOG(TLVL_INFO) << "XMITModule: called " <<  __func__ << " with arg " << firmwareName;
    usleep(2000);
  }

} // end of namespace nevistpc



