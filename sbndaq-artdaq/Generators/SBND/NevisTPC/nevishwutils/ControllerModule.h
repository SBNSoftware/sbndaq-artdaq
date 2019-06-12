#ifndef _NEVISTPC_CONTROLLERMODULE_H
#define _NEVISTPC_CONTROLLERMODULE_H 1

#include "NevisControllerPCIeCard.h"
#include "ControlDataPacket.h"
#include "fhiclcpp/ParameterSet.h"

namespace nevistpc{

  class ControllerModule{

  public:
    //Chip id=0
    enum chip_0:
    operation_id_t
    {
        INITIALIZE = 0,
        LOOPBACK_ON,
        RESET_LOOPBACK
    };
    typedef chip_0 receiver;
    
    //Chip id=1
    enum chip_1:
    operation_id_t
    {
        TEST_OFF = 0,
        TEST_ON,
        RUN_ON__SYNC_ON,
        RUN_OFF,
        SEND_NU_TRIGGER,
        SEND_SN_TRIGGER,
        FRAME_SIZE,
        TRIGGER_POSITION
    };
    typedef chip_1 trigger;

    enum device:
    chip_address_t
    {
        rcvr = 0,
        trg
    };

  public:
    explicit ControllerModule( fhicl::ParameterSet const & ps );
    virtual ~ControllerModule();

    void setupTXModeRegister();
    
    //Chip id=0
    void initialize();
    void setLoopbackOn();
    void resetLoopback();
    //Chip id=1
    void testOff();
    void testOn();
    void runOn();
    void runOnSyncOn();
    void runOff();
    void sendNUTrigger();
    void sendSNTrigger();
    void setFrameLength(data_payload_t const &size);
    void setNUTrigPosition(data_payload_t const &position);

    void send(ControlDataPacket const &packet,
	      useconds_t const sleep_time_us=0);
    void receive(StatusPacket &packet);
    void query(ControlDataPacket const& control,
	       StatusPacket & status,
	       useconds_t const sleep_time_us = 0);

  private:
    const fhicl::ParameterSet _params; // FHiCL parameter set
    std::unique_ptr<DeviceInfo> nevisDeviceInfo; // Pointer to the parameters identifying the PCIe card
    NevisControllerPCIeCardUPtr nevisPCIeCard; // Pointer to the PCIe card connected to the Controller

  };

  typedef std::unique_ptr<ControllerModule> ControllerModuleUPtr;
  typedef std::shared_ptr<ControllerModule> ControllerModuleSPtr;
  
  class UsesControllerModule{
  public:
    UsesControllerModule();
    virtual ~UsesControllerModule(){};
    ControllerModuleSPtr controller();
    virtual void useController(ControllerModuleSPtr Controller);
  private:
    ControllerModuleSPtr _controller;
  };

} // end of namespace nevistpc

#endif //_NEVISTPC_CONTROLLERMODULE_H
