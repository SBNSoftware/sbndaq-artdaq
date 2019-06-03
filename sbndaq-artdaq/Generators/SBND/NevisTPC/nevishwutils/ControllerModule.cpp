#include "ControllerModule.h"
#include "daqExceptions.h"
#include "trace.h"

#define TRACE_NAME "ControllerModule"

namespace nevistpc
{

  ControllerModule::ControllerModule ( fhicl::ParameterSet const & ps )
    : _params {ps}
  {  
    try {
      fhicl::ParameterSet streamconfig;       
      // NOTE: the name of the block in the fcl file must match
      if( ! _params.get_if_present<fhicl::ParameterSet> ( "controller_module", streamconfig ) ){ 
	TLOG(TLVL_ERROR)<< "ControllerModule: Missing controller_module configuration block in fcl file" ;
	throw RuntimeErrorException ( std::string("Missing controller_module configuration block in fcl file") );
      }

      // Read the parameters identifying the PCIe card
      nevisDeviceInfo = std::unique_ptr<DeviceInfo>( new DeviceInfo(streamconfig) );

      TLOG(TLVL_INFO)<< "ControllerModule: Configuration read" ;
    } catch ( ... ) {
      TLOG(TLVL_ERROR)<< "ControllerModule: Failed reading of controller_module configuration" ;
      throw RuntimeErrorException ( std::string("Failed reading of controller_module configuration") );
    }

    nevisPCIeCard = std::unique_ptr<NevisControllerPCIeCard>( new NevisControllerPCIeCard(*nevisDeviceInfo) );
    nevisPCIeCard->deviceOpen();
    TLOG(TLVL_INFO)<< "ControllerModule: called "<< __func__ ;
  }

  ControllerModule::~ControllerModule()
  {
  }


  void ControllerModule::send(ControlDataPacket const &packet,
			      useconds_t const sleep_time_us)
  {
    nevisPCIeCard->send(packet,sleep_time_us);
    TLOG(TLVL_DEBUG) <<  "ControllerModule: called " <<  __func__ ;
  }

  void ControllerModule::receive(StatusPacket &packet)
  {
    nevisPCIeCard->receive(packet);
    TLOG(TLVL_DEBUG) <<  "ControllerModule: called " <<  __func__ ;
  }

  void ControllerModule::query(ControlDataPacket const& control,
				  StatusPacket &status,
			       useconds_t const sleep_time_us)
  {
    nevisPCIeCard->query(control,status,sleep_time_us);
    TLOG(TLVL_DEBUG) <<  "ControllerModule: called " <<  __func__ ;
  }

  //Chip id=0
  void ControllerModule::initialize()
  {
    nevisPCIeCard->send(ControlDataPacket(0, device::rcvr, receiver::INITIALIZE));
    TLOG(TLVL_INFO) << "ControllerModule: called " <<  __func__ ;
  }

  void ControllerModule::setLoopbackOn()
  {
    nevisPCIeCard->send(ControlDataPacket(0, device::rcvr, receiver::LOOPBACK_ON));
    TLOG(TLVL_INFO) << "ControllerModule: called " <<  __func__ ;
  }

  void ControllerModule::resetLoopback()
  {
    nevisPCIeCard->send(ControlDataPacket(0, device::rcvr, receiver::RESET_LOOPBACK));
    TLOG(TLVL_INFO) << "ControllerModule: called " <<  __func__ ;
  }

  //Chip id=1
  void ControllerModule::testOff()
  {
    nevisPCIeCard->send(ControlDataPacket(0, device::trg, trigger::TEST_OFF));
    TLOG(TLVL_INFO) << "ControllerModule: called " <<  __func__ ;
  }

  void ControllerModule::testOn()
  {
    nevisPCIeCard->send(ControlDataPacket(0, device::trg, trigger::TEST_ON));
    TLOG(TLVL_INFO) << "ControllerModule: called " <<  __func__ ;
  }

  void ControllerModule::runOn()
  {
    runOnSyncOn();
    TLOG(TLVL_INFO) << "ControllerModule: called " <<  __func__ ;
  }

  void ControllerModule::runOnSyncOn()
  {
    nevisPCIeCard->send(ControlDataPacket(0, device::trg, trigger::RUN_ON__SYNC_ON));
    TLOG(TLVL_INFO) << "ControllerModule: called " <<  __func__ ;
  }

  void ControllerModule::runOff()
  {
    nevisPCIeCard->send(ControlDataPacket(0, device::trg, trigger::RUN_OFF));
    TLOG(TLVL_INFO) << "ControllerModule: called " <<  __func__ ;
  }

  void ControllerModule::sendNUTrigger()
  {
    nevisPCIeCard->send(ControlDataPacket(0, device::trg, trigger::SEND_NU_TRIGGER));
    TLOG(TLVL_INFO) << "ControllerModule: called " <<  __func__ ;
  }

  void ControllerModule::sendSNTrigger()
  {
    nevisPCIeCard->send(ControlDataPacket(0, device::trg, trigger::SEND_SN_TRIGGER));
    TLOG(TLVL_INFO) << "ControllerModule: called " <<  __func__ ;
  }

  void ControllerModule::setFrameLength(data_payload_t const &size)
  {
    nevisPCIeCard->send(ControlDataPacket(0, device::trg, trigger::FRAME_SIZE,size));
    TLOG(TLVL_INFO) << "ControllerModule: called " <<  __func__ << " with size " << size ;
  }

  void ControllerModule::setNUTrigPosition(data_payload_t const &position)
  {
    nevisPCIeCard->send(ControlDataPacket(0, device::trg, trigger::TRIGGER_POSITION, position));
    TLOG(TLVL_INFO) << "ControllerModule: called " <<  __func__ << " with position " << position ;
  }

  UsesControllerModule::UsesControllerModule() :
    _controller(nullptr)
  {
  }

  ControllerModuleSPtr UsesControllerModule::controller()
  {
    if(!_controller){
      throw   RuntimeErrorException("Controller module was not set.");
    }
    return _controller;
  }

  void UsesControllerModule::useController(ControllerModuleSPtr controller)
  {
    _controller = controller;
    TLOG(TLVL_INFO) << "UsesControllerModule: called " <<  __func__ << " " << controller ;
  }

  void ControllerModule::setupTXModeRegister()
  {
    nevisPCIeCard->writeAddr32 ( dma_detail::cs_bar, dma_detail::tx_mode_reg, 0xf0000008 );
    nevisPCIeCard->writeAddr32 ( dma_detail::cs_bar, dma_detail::r2_cs_reg, dma_detail::cs_init );
    nevisPCIeCard->writeAddr32 ( dma_detail::cs_bar, dma_detail::r2_cs_reg, dma_detail::cs_start+0xffffff );
    TLOG(TLVL_INFO) << "ControllerModule: called " <<  __func__ ;
  }

} // end of namespace nevistpc




