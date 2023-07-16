#include <fstream>
#include <sstream>
#include <iomanip>

#include "NevisControllerPCIeCard.h"
#include "NevisTBStreamReader.h"
#include "TriggerModule.h"
#include "daqExceptions.h"
#include "TimeUtils.h"
#include "StringUtils.h"
#include "trace.h"
#define TRACE_NAME "NevisTBStreamReader"

 namespace nevistpc {
 #define hex(w,d) std::setw(w) << std::setfill('0') << std::hex << d

 NevisTBStreamReader::NevisTBStreamReader ( std::string const& streamName , fhicl::ParameterSet const & ps )
   : _stream_name {streamName}, _params {ps}
   // , _triggerCallFunction {doNothing}
								   //    ,  _reader_synch_object {nullptr}
								   //    ,  _reader_synch_object(new NevisTBStreamReaderSynchronizationObject)
     , _trig_data_ctr {0xffffff}
     , _trig_ctr {0}
     , _trig_frame {0}
     , _trig_sample {0}
     , _trig_sample_remain_16MHz {0}
     , _trig_sample_remain_64MHz {0}
				 
   {
     //Should happen once.
     TLOG(TLVL_INFO) << "Calling constructor for NevisTB: " << std::hex << _trig_data_ctr ;

 }


   void NevisTBStreamReader::initializePCIeCard()
   {
   nevisPCIeCard = std::unique_ptr<NevisControllerPCIeCard>( new NevisControllerPCIeCard(*nevisDeviceInfo) );
   nevisPCIeCard->deviceOpen();
   nevisPCIeCard->writeAddr32 ( dma_detail::cs_bar, dma_detail::t2_cs_reg, dma_detail::cs_init );
   nevisPCIeCard->writeAddr32 ( dma_detail::cs_bar, dma_detail::r2_cs_reg, dma_detail::cs_init );
   nevisPCIeCard->writeAddr32 ( dma_detail::cs_bar, dma_detail::tx_mode_reg, 0xfff );
   TLOG(TLVL_INFO) << "NevisTBStreamReader " << _stream_name << ": called " <<  __func__ ;
 }

   void NevisTBStreamReader::setupTXModeRegister()
   {
     nevisPCIeCard->writeAddr32 ( dma_detail::cs_bar, dma_detail::tx_mode_reg, 0xf0000008 );
     nevisPCIeCard->writeAddr32 ( dma_detail::cs_bar, dma_detail::r2_cs_reg, dma_detail::cs_init );
     nevisPCIeCard->writeAddr32 ( dma_detail::cs_bar, dma_detail::r2_cs_reg, dma_detail::cs_start+0xffffff );

   }

						/*
   void NevisTBStreamReader::setReaderSynchronizationObject ( NevisTBStreamReaderSynchronizationObjectSPtr& reader_synch_object )
   {
     TLOG_DEBUG(1) << "start - reader_synch_object="<<reader_synch_object.get();
     assert ( !_reader_synch_object );
     assert ( reader_synch_object );
   //     _reader_synch_object = reader_synch_object;
     TLOG(TLVL_INFO)<< "ReaderSynObject" << _reader_synch_object;
   //reader_synch_object = std::make_shared<nevistpc::NevisTBStreamReaderSynchronizationObject>();
   _reader_synch_object = reader_synch_object;   

   }
						*/



   void NevisTBStreamReader::configureReader()
   {
     try {
       fhicl::ParameterSet streamconfig;       
       // NOTE: the name of the block in the fcl file must match
   //       if( ! _params.get_if_present<fhicl::ParameterSet> ( "trigger_module", streamconfig ) ){
   if( ! _params.get_if_present<fhicl::ParameterSet> ( "ntb_pciecard", streamconfig ) ){
 	TLOG(TLVL_ERROR)<< "TriggerModule: Missing trigger_module configuration block in fcl file" ;
 	throw RuntimeErrorException ( std::string("Missing trigger_module configuration block in fcl file") );
       }

       // Read the parameters identifying the PCIe card
       nevisDeviceInfo = std::unique_ptr<DeviceInfo>( new DeviceInfo(streamconfig) );

       TLOG(TLVL_INFO)<< "TriggerModule: Configuration read (PCIe card is shared with controller module)" ;
     } catch ( ... ) {
       TLOG(TLVL_ERROR)<< "TriggerModule: Failed reading of trigger_module configuration" ;
       throw RuntimeErrorException ( std::string("Failed reading of trigger_module configuration") );
     }

     nevisPCIeCard = std::unique_ptr<NevisControllerPCIeCard>( new NevisControllerPCIeCard(*nevisDeviceInfo) );
     nevisPCIeCard->deviceOpen();
     TLOG(TLVL_INFO)<< "TriggerModule: called "<< __func__ ;
  }


  std::streamsize NevisTBStreamReader::readsome ( char* buffer, std::streamsize size )
  {

    TLOG(TLVL_DEBUG_13) << "Readsome for NevisTB is called!!!!!!"; 

    struct timeval t1, t2;
          
    if(_do_timing) gettimeofday(&t1,NULL);
          
    assert ( buffer != NULL );
    assert ( size > 0 );
    //UNUSED(size);
          
    std::streamsize readSize = -1;
    readSize=( size_t ) size;


   //setReaderSynchronizationObject(reader_synch_object1);
   //setReaderSynchronizationObject(reader_synch_object);

   //   TLOG(TLVL_INFO)<< "Reader synch object is: "  << _reader_synch_object ;

   //   if ( _reader_synch_object && !_reader_synch_object->try_lock_TRIGGER())   
   //   if ( !_reader_synch_object || !_reader_synch_object->try_lock_TRIGGER())
   //{
   // TLOG(TLVL_WARN) <<  "\033[93m hello world\033[00m ... "<<_reader_synch_object ;
   //return 0;
   // }

   /*
   if (!_reader_synch_object) {
      TLOG(TLVL_WARN) << "Ignoring NTB data as the reader object synchronization is null.";
   //   return 0;
    }
    if (!_reader_synch_object->try_lock_TRIGGER()) {
    TLOG(TLVL_WARN) << "Ignoring NTB data as the reader object synchronization is not in line with hardware.";
     return 0;
    }
   */

    UINT64 trig_data_ctr {0};
    nevisPCIeCard->readAddr64 ( dma_detail::cs_bar, dma_detail::t2_cs_reg, trig_data_ctr );
    //extracting a 24-bit value from the upper 32 bits of the read value.
    trig_data_ctr = (trig_data_ctr>>32) & 0xffffff;
   //TLOG(TLVL_DEBUG_13)
   TLOG(TLVL_INFO) << "Nevis TB data (in bytes) reading from transceiver 2" << trig_data_ctr ;
   //TLOG(TLVL_DEBUG_13) << 
   TLOG(TLVL_INFO)<< " Hex (Nevis TB data (in bytes) reading from transceiver 2 24-bit value: " << std::hex << trig_data_ctr << " , manually defined " <<  _trig_data_ctr ;  


          
    if( (_trig_data_ctr - trig_data_ctr) < 16 ) {
   //      if(_reader_synch_object) _reader_synch_object->unlock();
      TLOG(TLVL_WARN) << "Triggered data is less than 16 bit word, unlocking reader sync object and returning zero"; 

      return 0;


    }
    if(trig_data_ctr<1){
      TLOG(TLVL_WARN) << "Triggered data is less than 1 bit; zero trigger data";
      throw RuntimeErrorException ( "Got 0 trigger data!" );
    }

    UINT64 trig_data0 {0xbad};
    UINT64 trig_data1 {0xbad};
    std::vector<uint16_t> trig_data(8,0xbad);
    nevisPCIeCard->readAddr64 ( dma_detail::t2_tr_bar, 0x0, trig_data0 );
    trig_data1 = trig_data0;
    nevisPCIeCard->readAddr64 ( dma_detail::t2_tr_bar, 0x0, trig_data1 );
          
    _trig_ctr = (trig_data0 >> 40);
    _trig_frame = ( ( ( (trig_data0 >> 32) & 0xff )<<16 ) + ( (trig_data0 >> 16) & 0xffff ) );
    _trig_sample = ( (trig_data0 >> 4) & 0xfff );
    _trig_sample_remain_16MHz = ( (trig_data0 >> 1) & 0x7 );
    _trig_sample_remain_64MHz = ( (trig_data1 >> 15) & 0x3 );

    std::ostringstream msg;
    msg << "[ NEW TRIGGER ]"
	<< " "
	<< " Trig: "   << _trig_ctr
	<< " Frame: "  << _trig_frame
	<< " Sample: " << _trig_sample
	<< " Remine (16MHz): " << _trig_sample_remain_16MHz
	<< " Remine (64MHz): " << _trig_sample_remain_64MHz
	<< " "
	<< " Bits: ";

    if((trig_data1>>8) & 0x1) msg << " PC";
    if((trig_data1>>9) & 0x1) msg << " EXT";
    if((trig_data1>>12) & 0x1) msg << " Gate1";
    if((trig_data1>>11) & 0x1) msg << " Gate2";
    if((trig_data1>>10) & 0x1) msg << " Active";
    if((trig_data1>>13) & 0x1) msg << " Veto";
    if((trig_data1>>14) & 0x1) msg << " Calib";
    if(trig_data1>>32 != 0xffffffff) {
     std::ostringstream errmsg;
      errmsg << "Trigger data ctr (local):  " << trig_data_ctr << "\n";

      errmsg << "Trigger data 0: " << trig_data0 << "\n"
	     << "Trigger data 1: " << trig_data1 << "\n";
      //      mf::LogError( _stream_name ) << errmsg.str();
      TLOG(TLVL_WARN) << "Invalid trigger data format" ;
      throw RuntimeErrorException("Invalid trigger data format!");

    }

    msg << " PMT data: 0x"
	<< (trig_data1 & 0xff);
    //  mf::LogDebug ( _stream_name ) << msg.str();
    TLOG(TLVL_DEBUG) << "PMT triggered data words" << _stream_name << msg.str() ;   
    trig_data[0] = ((trig_data0      ) & 0xffff); 
    trig_data[1] = ((trig_data0 >> 16) & 0xffff); 
    trig_data[2] = ((trig_data0 >> 32) & 0xffff);
    trig_data[3] = ((trig_data0 >> 48) & 0xffff); 
    trig_data[4] = ((trig_data1      ) & 0xffff); 
    trig_data[5] = ((trig_data1 >> 16) & 0xffff); 
    trig_data[6] = ((trig_data1 >> 32) & 0xffff); 
    trig_data[7] = ((trig_data1 >> 48) & 0xffff); 
    _trig_data_ctr -= 16;

    readSize = sizeof(uint16_t)*trig_data.size();
    TLOG(TLVL_DEBUG_5) << "Nevis TB data in bytes: " << readSize ;
    memcpy (buffer, (char*)(&trig_data[0]),sizeof(uint16_t)*trig_data.size());
    //    TLOG(TLVL_DEBUG_6) << "Size of Triggered data t "
 

   //    if ( _reader_synch_object ) {
   // _reader_synch_object->unlock();
      //_reader_synch_object->finishedDMAingTriggerData();
   //}
    if(_do_timing) {
      gettimeofday(&t2,NULL);
      //std::cout << "Checkpoint 6: " << utils::diff_time_microseconds(t2,t1) << " us" << std::endl;
      gettimeofday(&t1,NULL);
    }

    //return (sizeof(uint16_t)*trig_data.size());
    return readSize;
  }


} //end nampespace
