#define TRACE_NAME "XMITReader"
#include "artdaq/DAQdata/Globals.hh"
#include <fstream>
#include <sstream>
#include <iomanip>

#include "XMITReader.h"
#include "daqExceptions.h"
#include "TimeUtils.h"
#include "StringUtils.h"

namespace nevistpc {

#define hex(w,d) std::setw(w) << std::setfill('0') << std::hex << d

  std::string dma_settings::toString()
  {
    std::stringstream os;
    os << "DMA settings: \n";
    os << "dma_buffer_size=                   " << ( long ) dma_buffer_size << " bytes\n";
    os << "dma_max_polling_time_microseconds= " << ( long ) dma_max_polling_time_microseconds << " mus\n";
    return os.str();
  }
  
  XMITReader::XMITReader ( std::string const& streamName, fhicl::ParameterSet const & ps )
    : _stream_name {streamName}, _params {ps}, _isFirstEverDMA {true}
  {  
  }

  void XMITReader::initializePCIeCard()
  {
    nevisPCIeCard = std::unique_ptr<NevisReadoutPCIeCard>( new NevisReadoutPCIeCard(*nevisDeviceInfo) );
    nevisPCIeCard->deviceOpen();
    nevisPCIeCard->writeAddr32 ( dma_detail::cs_bar, dma_detail::t1_cs_reg, dma_detail::cs_init );
    nevisPCIeCard->writeAddr32 ( dma_detail::cs_bar, dma_detail::t2_cs_reg, dma_detail::cs_init );
    nevisPCIeCard->writeAddr32 ( dma_detail::cs_bar, dma_detail::r1_cs_reg, dma_detail::cs_init );
    nevisPCIeCard->writeAddr32 ( dma_detail::cs_bar, dma_detail::r2_cs_reg, dma_detail::cs_init );
    nevisPCIeCard->writeAddr32 ( dma_detail::cs_bar, dma_detail::tx_mode_reg, 0xfff );
    TLOG(TLVL_INFO) << "XMITReader " << _stream_name << ": called " <<  __func__ ;
  }

  void XMITReader::setupTXModeRegister()
  {
    nevisPCIeCard->writeAddr32 ( dma_detail::cs_bar, dma_detail::tx_mode_reg, 0x00003fff );
    nevisPCIeCard->writeAddr32 ( dma_detail::cs_bar, dma_detail::t1_cs_reg, dma_detail::cs_mode_p );
    nevisPCIeCard->writeAddr32 ( dma_detail::cs_bar, dma_detail::t2_cs_reg, dma_detail::cs_mode_p );
    TLOG(TLVL_INFO) << "XMITReader " << _stream_name << ": called " <<  __func__ ;
  }


  void XMITReader::configureReader ()
  {
    try {
      fhicl::ParameterSet streamconfig;       
      // NOTE: the _stream_name must match the name of the block in the fcl file
      if( ! _params.get_if_present<fhicl::ParameterSet> ( _stream_name, streamconfig ) ){ 
	TLOG(TLVL_ERROR) << "XMITReader " << _stream_name << ": Missing configuration block in fcl file" ;
	throw RuntimeErrorException ( std::string("Missing configuration in fcl file for ").append( _stream_name ) );
      }

      // Read the parameters identifying the PCIe card
      nevisDeviceInfo = std::unique_ptr<DeviceInfo>( new DeviceInfo(streamconfig) );

      uint32_t dma_buffer_size = streamconfig.get<uint32_t>( "dma_buffer_size", 1000000 );
      _do_timing = streamconfig.get<bool>( "profile_time", true );       
      uint32_t dma_max_polling_time_microseconds = streamconfig.get<uint32_t>( "maxpoll", 10000000 ); 
      
      _dma_settings.reset( new nevistpc::dma_settings( dma_buffer_size, 
						       dma_max_polling_time_microseconds ) );
      
      TLOG(TLVL_INFO) << "XMITReader " << _stream_name << ": " << _dma_settings->toString() ;
      TLOG(TLVL_INFO) << "XMITReader " << _stream_name << ": Configuration read" ;
    } catch ( ... ) {
      TLOG(TLVL_ERROR) << "XMITReader " << _stream_name << ": Failed reading of XMITReader" ;
      throw RuntimeErrorException ( std::string("Failed reading of XMITReader configuration for ").append( _stream_name ) );
    }
  }

  bool XMITReader::dmaLockBuffer ( dma_buffer& dma )
  {
    assert ( _dma_settings );
    
    if ( !nevisPCIeCard->dmaContigBufLock ( &dma.pUserModeBuffer, dma_flags::receive, _dma_settings->dma_buffer_size , &dma.pDMABuffer ) )
      return false;
    
    dma.bufferSize = _dma_settings->dma_buffer_size;
    dma.readSize = _dma_settings->dma_buffer_size;
    
    TLOG(TLVL_INFO) << "XMITReader " << _stream_name << ": buffer allocation lower address = " 
	    << hex( 8, ( dma.pDMABuffer->Page->pPhysicalAddr & 0xffffffff ) ) ;
    TLOG(TLVL_INFO) << "XMITReader " << _stream_name << ": buffer allocation higher address = " 
	    << hex( 8, ( ( dma.pDMABuffer->Page->pPhysicalAddr >> 32 ) & 0xffffffff ) ) ;
    return true;
  }

  void XMITReader::dmaSetTXModeRegister()
  {
    nevisPCIeCard->writeAddr32 ( dma_detail::cs_bar, dma_detail::tx_md_reg, _stream_name == "nu_xmit_reader" ? 0x00002000 : 0x00001000 ); // NOTE: needs to match the trigger stream name
  }
  
  void XMITReader::dmaAbort()
  {
    nevisPCIeCard->writeAddr32 ( dma_detail::cs_bar, dma_detail::cs_dma_msi_abort, dma_detail::dma_abort );
  }
  
  void XMITReader::dmaClearRegister()
  {
    nevisPCIeCard->writeAddr32 ( dma_detail::cs_bar, dma_detail::cs_dma_msi_abort, 0 );
  }
  
  void XMITReader::dmaInitializeOnFirstLoop()
  {
    if ( ! dmaLockBuffer ( _dma1 ) || ! dmaLockBuffer ( _dma2 ) )
      throw RuntimeErrorException ( "Failed locking a contiguous DMA buffer" );
    
    dmaSetTXModeRegister();
    dmaAbort();
    dmaClearRegister();
    TLOG(TLVL_INFO) << "XMITReader " << _stream_name << ": called "<< __func__ ;
  }
  
  void  XMITReader::initializeReceivers()
  {
    _dma1.r_cs_reg = dma_detail::r1_cs_reg;
    _dma2.r_cs_reg = dma_detail::r2_cs_reg;
    nevisPCIeCard->writeAddr32 ( dma_detail::cs_bar, _dma1.r_cs_reg, dma_detail::cs_init );
    nevisPCIeCard->writeAddr32 ( dma_detail::cs_bar, _dma2.r_cs_reg, dma_detail::cs_init );
  }
  
  void  XMITReader::startReceiver ( dma_buffer& dma )
  {
    nevisPCIeCard->writeAddr32 ( dma_detail::cs_bar, dma.r_cs_reg, dma_detail::cs_start + std::min ( dma.bufferSize, dma.readSize ) );
  }
  
  void  XMITReader::startReceivers()
  {
    startReceiver ( _dma1 );
    startReceiver ( _dma2 );
  }
  
  void  XMITReader::setupTransceivers ( dma_buffer& dma )
  {
    nevisPCIeCard->writeAddr32 ( dma_detail::cs_bar, dma_detail::cs_dma_add_low_reg,
				 dma.pDMABuffer->Page->pPhysicalAddr & 0xffffffff );
    
    nevisPCIeCard->writeAddr32 ( dma_detail::cs_bar, dma_detail::cs_dma_add_high_reg,
				 ( dma.pDMABuffer->Page->pPhysicalAddr >> 32 ) & 0xffffffff );
    
    nevisPCIeCard->writeAddr32 ( dma_detail::cs_bar, dma_detail::cs_dma_by_cnt,
				 std::min ( dma.bufferSize, dma.readSize ) );
  }
  
  void  XMITReader::dmaStart ( dma_buffer& dma )
  {
    /* write this will start DMA */
    UINT32  u32Data {0};
    
    if ( ( ( dma.pDMABuffer->Page->pPhysicalAddr >> 32 ) & 0xffffffff ) == 0 ) {
      u32Data = dma_detail::dma_tr12 + dma_detail::dma_3dw_rec;
      TLOG(TLVL_INFO) << "XMITReader " << _stream_name << "DMA First address: " << u32Data;
    } else {
      u32Data = dma_detail::dma_tr12 + dma_detail::dma_4dw_rec;
      TLOG(TLVL_INFO) << "XMITReader " << _stream_name << "DMA Second address: " << u32Data;
    }
    
    nevisPCIeCard->writeAddr32 ( dma_detail::cs_bar, dma_detail::cs_dma_cntrl, u32Data );
  }
  
  XMITReader::dma_completion_status XMITReader::dmaWaitWithTimeout ( unsigned int microseconds )
  {
    struct timeval time_start, time_now;
    
    gettimeofday(&time_start,NULL);
    while(1){
      
      //check for timeout
      gettimeofday(&time_now,NULL);
      long diff_time = diff_time_microseconds(time_now,time_start);
      if(diff_time > microseconds){
	return dma_completion_status::timeout;
      }//end if timeout condition
      
      UINT32 dmaStatus {0};
      
      nevisPCIeCard->readAddr32 ( dma_detail::cs_bar, dma_detail::cs_dma_cntrl, dmaStatus );
      
      if ( ( dmaStatus & dma_detail::dma_in_progress ) == 0 ) {
	return dma_completion_status::complete;
      }//end if dma complete
    }//end infinite while
    
  }
  
  void XMITReader::dmaReportTransferStatus()
  {
    UINT64 dmaReadback {0};
    
    nevisPCIeCard->readAddr64 ( dma_detail::cs_bar, dma_detail::cs_dma_by_cnt, dmaReadback );
    
    TLOG(TLVL_INFO) << "XMITReader " << _stream_name << ": DMA word count = " 
	    << hex( 8, ( dmaReadback >> 32 ) ) << ", " <<  hex( 8, ( dmaReadback & 0xffff ) ) ;
    
    nevisPCIeCard->readAddr64 ( dma_detail::cs_bar, dma_detail::t1_cs_reg, dmaReadback );
    
    TLOG(TLVL_INFO) << "XMITReader " << _stream_name << ": status word for channel 1 after read = " 
	    << hex( 8, ( dmaReadback >> 32 ) ) << ", " <<  hex( 8, (dmaReadback & 0xffff ) ) ;    

    nevisPCIeCard->readAddr64 ( dma_detail::cs_bar, dma_detail::t2_cs_reg, dmaReadback );
    
    TLOG(TLVL_INFO) << "XMITReader " << _stream_name << ": status word for channel 2 after read = " 
	    << hex( 8, ( dmaReadback >> 32 ) ) << ", " <<  hex( 8, (dmaReadback & 0xffff ) ) ;    
  }
  
  void XMITReader::writeBufferToBinaryFile ( const char* buffer, const std::streamsize size )
  {
    static std::string fileName = std::string ( getenv ( "HOME" ) ) + "/dma-data-" + getTimestampAsString() + "-";
    static size_t chunk {0};
    
    std::ofstream fos ( fileName + boost::lexical_cast<std::string> ( chunk++ )  + ".dat", std::ios::out | std::ios::binary );
    fos.write ( buffer, size );
    fos.close();
  }

  std::streamsize XMITReader::readsome ( char* buffer, std::streamsize size )
  {
    std::cout << "READSOME FUNCTION CALLED!!!!!!!!!!!!" << std::endl;
    //    exit(0);

    static unsigned long int _loopNumber = 0;

    struct timeval t1, t2;
    
    if(_do_timing) gettimeofday(&t1,NULL);

    assert ( buffer != NULL );
    assert ( size > 0 );
      
    _dma1.readSize = ( size_t ) size;
    _dma2.readSize = ( size_t ) size;
    
    dma_buffer& dma = ( ( _loopNumber % 2 ) == 0 ) ? _dma1 : _dma2;
  
    if ( _isFirstEverDMA ) {

      dmaInitializeOnFirstLoop();
    
      ::WDC_DMASyncCpu ( dma.pDMABuffer );
      initializeReceivers();
      
      _loopNumber = 0;
      _isFirstEverDMA=false;
    }
    else {
      ::WDC_DMASyncCpu ( dma.pDMABuffer );
    }
  
    startReceivers();
    setupTransceivers ( dma );
    dmaStart ( dma );

    if(_do_timing) {
      gettimeofday(&t2,NULL);
      TLOG(TLVL_INFO) << "XMITReader " << _stream_name << ": Timestamp "<< (t2.tv_sec*1e6 + t2.tv_usec) 
	      << " us: Checkpoint 1 elapsed time "<< diff_time_microseconds(t2, t1) << " us" ;
      gettimeofday(&t1,NULL);
      }

    std::streamsize readSize = -1;
  

    if ( dmaWaitWithTimeout ( _dma_settings->dma_max_polling_time_microseconds ) == dma_completion_status::timeout ) {
    
      UINT64 dmaReadback {0};
      nevisPCIeCard->readAddr64 ( dma_detail::cs_bar, dma.r_cs_reg, dmaReadback );
    
      readSize = dma.readSize - ( dmaReadback & 0xffff );
    
      TLOG(TLVL_ERROR) << "XMITReader " << _stream_name << ": *** *** DMA timed out. DMAed " << readSize 
	      << " bytes. Set to readout " << dma.readSize << " bytes. *** ***" ;
      dmaAbort();
      dmaClearRegister();
      _loopNumber = 0;
      
      return 0;
    }
  
    if(_do_timing) {
      gettimeofday(&t2,NULL);
      TLOG(TLVL_INFO) << "XMITReader " << _stream_name << ": Timestamp "<< (t2.tv_sec*1e6 + t2.tv_usec) 
	      << " us: Checkpoint 2 elapsed time "<< diff_time_microseconds(t2, t1) << " us" ;
      gettimeofday(&t1,NULL);
    }
  
    ::WDC_DMASyncIo ( dma.pDMABuffer );
  
    if(_do_timing) {
      gettimeofday(&t2,NULL);
      TLOG(TLVL_INFO) << "XMITReader " << _stream_name << ": Timestamp "<< (t2.tv_sec*1e6 + t2.tv_usec) 
	      << " us: Checkpoint 3 elapsed time "<< diff_time_microseconds(t2, t1) << " us" ;
      gettimeofday(&t1,NULL);
    }

    readSize = dma.readSize;

    ::memcpy ( buffer, ( char* ) dma.pUserModeBuffer, readSize );
  
    if(_do_timing) {
      gettimeofday(&t2,NULL);
      TLOG(TLVL_INFO) << "XMITReader " << _stream_name << ": Timestamp "<< (t2.tv_sec*1e6 + t2.tv_usec) 
	      << " us: Checkpoint 4 elapsed time "<< diff_time_microseconds(t2, t1) << " us" ;
      gettimeofday(&t1,NULL);
    }

    _loopNumber++;
    //    exit(0);
    return readSize;

  }




}  // end of namespace nevistpc



