#include "Crate.h"
#include "trace.h"
#define TRACE_NAME "Crate"

namespace nevistpc{

  // Old constructor kept so things compile 
  Crate::Crate(ControllerModuleSPtr ctr, int xmit_module_number, int max_fem_slot) :
    _ctrlr_module(ctr){
    
    for(int i = 1; i <= (max_fem_slot - xmit_module_number); i++){
      NevisTPCFEMSPtr fem_sptr(new NevisTPCFEM(xmit_module_number+i));
      fem_sptr->useController(ctr);
      _fem_modules.push_back(fem_sptr);
    }
  }

  // sn_xmit_reader is optional
  Crate::Crate(ControllerModuleSPtr ctr, XMITReaderSPtr nu_xmit_reader, fhicl::ParameterSet const& _p, XMITReaderSPtr sn_xmit_reader) : 
    _ctrlr_module(ctr), _xmit_reader(nu_xmit_reader), _sn_xmit_reader(sn_xmit_reader){
    
    fhicl::ParameterSet crateconfig;
    // NOTE: the name of the block in the fcl file must match 
    if( ! _p.get_if_present<fhicl::ParameterSet> ( "nevis_crate", crateconfig ) ){
      TLOG(TLVL_ERROR) << "Crate: Missing nevis_crate configuration block in fcl file" ;
    }
    
    // Setup trigger module
    hasTrigger = ( crateconfig.get<uint8_t>( "trigger_slot", -1 ) != (uint8_t)(-1) );
    if (hasTrigger){
      _trigger_module = std::make_shared<nevistpc::TriggerModule>( crateconfig.get<uint8_t>( "trigger_slot", -1 ) );
      _trigger_module->useController(ctr);
      TLOG(TLVL_INFO) << "Crate: TriggerModule initialized";
    }
    
    // Setup xmit module
    _xmit_module = std::make_shared<nevistpc::XMITModule>( crateconfig.get<uint8_t>( "xmit_slot", -1 ) );
    _xmit_module->useController(ctr);
    TLOG(TLVL_INFO) << "Crate: XMITModule initialized";        
    
    // Setup FEMs
    int xmit_module_number = crateconfig.get<uint8_t>( "xmit_slot", -1 );
    int max_fem_slot = crateconfig.get<uint8_t>( "max_fem_slot", -1 );
    for(int k = 1; k <= (max_fem_slot - xmit_module_number); k++){
      NevisTPCFEMSPtr fem_sptr(new NevisTPCFEM(xmit_module_number+k));
      fem_sptr->useController(ctr);
      _fem_modules.push_back(fem_sptr);
      TLOG(TLVL_INFO) << "Crate: Initialized FEM in slot " << (xmit_module_number+k);
    }
    TLOG(TLVL_INFO) << "Crate: " << getNumberOfTPCFEMs() << " FEMs initialized" ;
  }

  ControllerModuleSPtr Crate::getControllerModule(){
    return _ctrlr_module;
  }
  
  uint Crate::getNumberOfTPCFEMs(){
    return _fem_modules.size();
  }
  
  NevisTPCFEMSPtr Crate::getTPCFEM(uint i){
    return _fem_modules[i];
  }
  
  XMITModuleSPtr Crate::getXMITModule(){
    return _xmit_module;
  }
  
  XMITReaderSPtr Crate::getXMITReader(){
    return _xmit_reader;
  }

  // Two-stream overload
  XMITReaderSPtr Crate::getXMITReader(std::string name){
    if( name == "NU" ) return _xmit_reader;
    if( name == "SN" ) return _sn_xmit_reader;
    else return NULL;
  }

  //  NevisTBStreamReaderSPtr Crate::getNevisTBStreamReader(){
  //return _nevistb_reader;//("nevistb_reader");                                                                                          
  // }
  
  TriggerModuleSPtr Crate::getTriggerModule(){
    return _trigger_module;
  }

  void Crate::FEMtestfunction(){
    if(getNumberOfTPCFEMs() > 1){
    for(size_t tpc_it = 0; tpc_it < getNumberOfTPCFEMs(); tpc_it++){
      getTPCFEM(tpc_it)->readStatus();

    }}
  }

  void Crate::linkSetup(){
    
    // Reset link port receiver PLL for all FEMs except farthest left
    // (skip this part if only using one FEM)
    
    if(getNumberOfTPCFEMs() > 1){
      for(size_t iFEM = getNumberOfTPCFEMs()-1; iFEM != 0; iFEM--){
	getTPCFEM(iFEM-1)->resetPLLLink();
	usleep(1000);
      }
    } 
    
    getXMITModule()->setFEMModuleCount(getNumberOfTPCFEMs()-1);
    getXMITModule()->resetOpticalTransceiver();
    //for now hard coding using NU channel events but not SN. need to update this
    getXMITModule()->enableNUChanEvents(1);
    getXMITModule()->enableSNChanEvents(0);
    getXMITModule()->resetPLLLink();
    getXMITModule()->resetLink();
    getXMITModule()->resetDPA();
    getTPCFEM(0)->setLastModuleInTokenPassingOn();
    getXMITModule()->alignDPA();
    usleep(5000);
    getTPCFEM(0)->setLastModuleInTokenPassingOff();
    usleep(1000);
    
    // now reset all the link port receiver in weird looping configuration
    // if the FEM's are in these slots: 5, 4, 3, 2, 1 (with xmit in 0), do this
    // set last on to FEM (2), rxreset to module (1), alignment command to module (1), set last off to FEM (2)
    // set last on to FEM (3), rxreset to module (2), alignment command to module (2), set last off to FEM (3)
    // set last on to FEM (4), rxreset to module (3), alignment command to module (3), set last off to FEM (4)
    // set last on to FEM (5), rxreset to module (4), alignment command to module (4)
    
    for(size_t iFEM = 0; iFEM < getNumberOfTPCFEMs()-1; iFEM++){
      getTPCFEM(iFEM+1)->setLastModuleInTokenPassingOn();
      getTPCFEM(iFEM)->resetTokenPassingReceiver();
      getTPCFEM(iFEM)->pulseTokenPassingAllignmentCircuit();
      usleep(1000);
      if( (iFEM + 2) != getNumberOfTPCFEMs() )
	getTPCFEM(iFEM+1)->setLastModuleInTokenPassingOff();
    }
  }

  // Configure Trigger Board, FEM, XMIT Module and XMITReader for readout of the NU stream with external triggers
  // It follows uboonedaq run2Stream but SN stream has been removed
  void Crate::runNUStream(fhicl::ParameterSet const& _p){
    
    fhicl::ParameterSet crateconfig;
    // NOTE: the name of the block in the fcl file must match 
    if( ! _p.get_if_present<fhicl::ParameterSet> ( "nevis_crate", crateconfig ) ){
      TLOG(TLVL_ERROR) << "Crate: Missing nevis_crate configuration block in fcl file" ;
    }
    
    TLOG(TLVL_INFO) << "Crate: called " << __func__ << " recipe is about to start!"; 	
    // Setup XMIT reader for nustream
    getXMITReader()->configureReader();
    getXMITReader()->initializePCIeCard();
    
    // Setup controller module
    getControllerModule()->initialize();
    getControllerModule()->testOn();
    getControllerModule()->runOff();
    getControllerModule()->testOff();
    
    if(hasTrigger){
      getTriggerModule()->runOnSyncOff();
      getTriggerModule()->disableTriggers(false);
      getTriggerModule()->setDeadtimeSize(100); //0x59); //100);
    }
    
    // Load xmit firmware
    getXMITModule()->setMax3000Config();
    getXMITModule()->programMax3000FPGAFirmware( crateconfig.get<std::string>( "xmit_fpga", "") );
    
    // Setup FEMs
    if(getNumberOfTPCFEMs()){
      // Loop over FEMs
      for(size_t tpc_it = 0; tpc_it < getNumberOfTPCFEMs(); tpc_it++){
	usleep(10000);
	getTPCFEM(tpc_it)->fem_setup(crateconfig);
	TLOG(TLVL_INFO) << "Crate: FEM in slot " << getTPCFEM(tpc_it)->module_number() << " all set.";
      }
    }

    // Setup tx mode registers (this is done twice for some reason...)
    getXMITReader()->setupTXModeRegister();
    
    // Config trigger module
    if(hasTrigger)
      getTriggerModule()->configureTrigger( _p ); // To do: move it with the other trigger instructions above

    // set up link
    linkSetup();
    
    // Enable triggered stream & Disable continuous stream
    getXMITModule()->enableNUChanEvents(1);
    getXMITModule()->enableSNChanEvents(0);
    
    // setup tx mode registers (again, I know)
    getXMITReader()->setupTXModeRegister();
    getControllerModule()->setupTXModeRegister();
    //getXMITModule()->configureSNStreamReader_SNRunOnSyncOnMode();
    
    /*
      if (getTriggerModule()) {
      tTriggerModule()->setupTXModeRegisters();
      getXMITModule()->configureNUStreamReader_TRIGModuleTriggeredMode();
      
      }
    */
    
    getTriggerModule()->enableTriggers();
    getTriggerModule()->runOnSyncOn();

    TLOG(TLVL_INFO) << "Crate: called " << __func__ << " recipe is finished!"; 
  }

  // Configure Trigger Board, FEM, XMIT Module and XMITReader for readout of the NU and SN streams with external triggers
  // It would be good to consolidate all the Crate::run... functions into a single one, with portions enabled by fcl parameters
  void Crate::run2Stream(fhicl::ParameterSet const& _p){ 
    
    fhicl::ParameterSet crateconfig;
    // NOTE: the name of the block in the fcl file must match 
    if( ! _p.get_if_present<fhicl::ParameterSet> ( "nevis_crate", crateconfig ) ){
      TLOG(TLVL_ERROR) << "Crate: Missing nevis_crate configuration block in fcl file" ;
    }
    
    TLOG(TLVL_INFO) << "Crate: called " << __func__ << " recipe is about to start!"; 	
    // Setup XMIT reader for NU stream
    getXMITReader("NU")->configureReader();
    getXMITReader("NU")->initializePCIeCard();
    // Setup XMIT reader for SN stream
    getXMITReader("SN")->configureReader();
    getXMITReader("SN")->initializePCIeCard();
    
    // Setup controller module
    getControllerModule()->initialize();
    getControllerModule()->testOn();
    getControllerModule()->runOff();
    getControllerModule()->testOff();
    
    if(hasTrigger){
      getTriggerModule()->runOnSyncOff();
      getTriggerModule()->disableTriggers(false);
      getTriggerModule()->setDeadtimeSize(100);//0x59); //100);
    }
    
    // Load xmit firmware
    getXMITModule()->setMax3000Config();
    getXMITModule()->programMax3000FPGAFirmware( crateconfig.get<std::string>( "xmit_fpga", "") );
    
    // Setup FEMs
    if(getNumberOfTPCFEMs()){
      // Loop over FEMs
      for(size_t tpc_it = 0; tpc_it < getNumberOfTPCFEMs(); tpc_it++){
	usleep(10000);
	// To do: This function needs to be updated/overloaded to configure the zero suppression
	getTPCFEM(tpc_it)->fem_setup(crateconfig);
	TLOG(TLVL_INFO) << "Crate: FEM in slot " << getTPCFEM(tpc_it)->module_number() << " all set.";
      }
    }

    // Setup tx mode registers (this is done twice for some reason...)
    getXMITReader("NU")->setupTXModeRegister();
    getXMITReader("SN")->setupTXModeRegister();
    
    // Config trigger module
    if(hasTrigger)
      getTriggerModule()->configureTrigger( _p );

    // set up link
    linkSetup();
    
    // Enable triggered stream & Disable continuous stream
    getXMITModule()->enableNUChanEvents(1);
    getXMITModule()->enableSNChanEvents(1);
    
    // setup tx mode registers (again, I know)
    getXMITReader("NU")->setupTXModeRegister();
    getXMITReader("SN")->setupTXModeRegister();
    getControllerModule()->setupTXModeRegister();
    
    /*
      if (getTriggerModule()) {
      tTriggerModule()->setupTXModeRegisters();
      getXMITModule()->configureNUStreamReader_TRIGModuleTriggeredMode();
      
      }
    */
    
    getTriggerModule()->enableTriggers();
    getTriggerModule()->runOnSyncOn();

    TLOG(TLVL_INFO) << "Crate: called " << __func__ << " recipe is finished!"; 
  }

  void Crate::runControllerTrigger2Stream(fhicl::ParameterSet const& _p){ 
    
    fhicl::ParameterSet crateconfig;
    // NOTE: the name of the block in the fcl file must match 
    if( ! _p.get_if_present<fhicl::ParameterSet> ( "nevis_crate", crateconfig ) ){
      TLOG(TLVL_ERROR) << "Crate: Missing nevis_crate configuration block in fcl file" ;
    }
    
    TLOG(TLVL_INFO) << "Crate: called " << __func__ << " recipe is about to start!"; 	
    // Setup XMIT reader for NU stream
    getXMITReader("NU")->configureReader();
    getXMITReader("NU")->initializePCIeCard();
    // Setup XMIT reader for SN stream
    getXMITReader("SN")->configureReader();
    getXMITReader("SN")->initializePCIeCard();
    
    // Setup controller module
    getControllerModule()->initialize();
    getControllerModule()->testOn();
    getControllerModule()->runOff(); // using Controller triggers
    //getControllerModule()->testOff(); // using CALIB triggers
    // To do: move instructions below to fcl
    getControllerModule()->setFrameLength(0xffff & 9119); //20799);
    getControllerModule()->setNUTrigPosition(0xa);
    
    // Load xmit firmware
    getXMITModule()->setMax3000Config();
    getXMITModule()->programMax3000FPGAFirmware( crateconfig.get<std::string>( "xmit_fpga", "") );
    
    // Setup FEMs
    if(getNumberOfTPCFEMs()){
      // Loop over FEMs
      for(size_t tpc_it = 0; tpc_it < getNumberOfTPCFEMs(); tpc_it++){
	usleep(10000);
	// To do: This function needs to be updated/overloaded to configure the zero suppression
	getTPCFEM(tpc_it)->fem_setup(crateconfig);
	TLOG(TLVL_INFO) << "Crate: FEM in slot " << getTPCFEM(tpc_it)->module_number() << " all set.";
      }
    }

    // Setup tx mode registers (this is done twice for some reason...)
    getXMITReader("NU")->setupTXModeRegister();
    getXMITReader("SN")->setupTXModeRegister();
    
    // set up link
    linkSetup();
    
    // Enable triggered stream & Disable continuous stream
    getXMITModule()->enableNUChanEvents(1);
    getXMITModule()->enableSNChanEvents(1);
    
    // setup tx mode registers (again, I know)
    getXMITReader("NU")->setupTXModeRegister();
    getXMITReader("SN")->setupTXModeRegister();
    getControllerModule()->setupTXModeRegister();
    //getXMITModule()->configureSNStreamReader_SNRunOnSyncOnMode();
    
    getControllerModule()->runOn(); // calls runOnSyncOn

    TLOG(TLVL_INFO) << "Crate: called " << __func__ << " recipe is finished!"; 
  }

  void Crate::runCalib2Stream(fhicl::ParameterSet const& _p){ 
    
    fhicl::ParameterSet crateconfig;
    // NOTE: the name of the block in the fcl file must match 
    if( ! _p.get_if_present<fhicl::ParameterSet> ( "nevis_crate", crateconfig ) ){
      TLOG(TLVL_ERROR) << "Crate: Missing nevis_crate configuration block in fcl file" ;
    }
    
    TLOG(TLVL_INFO) << "Crate: called " << __func__ << " recipe is about to start!"; 	
    // Setup XMIT reader for NU stream
    getXMITReader("NU")->configureReader();
    getXMITReader("NU")->initializePCIeCard();
    // Setup XMIT reader for SN stream
    getXMITReader("SN")->configureReader();
    getXMITReader("SN")->initializePCIeCard();
    
    // Setup controller module
    getControllerModule()->initialize();
    getControllerModule()->testOn();
    //getControllerModule()->runOff(); // using CALIB triggers
    //getControllerModule()->testOff(); // using CALIB triggers
    
    if(hasTrigger){
      // using CALIB triggers
      assert( getTriggerModule() ); // using CALIB triggers
      getTriggerModule()->useController( getControllerModule() ); // using CALIB triggers


      getTriggerModule()->runOnSyncOff();

      ///////////////////////
      // Temp: from CALIB, to avoid leaving the function generator on all weekend
      getControllerModule()->testOff(); //v
      // To do: create two NevisTriggerModule_config.fcl, one for external, one for CALIB. Move instructions below to fcl
      getTriggerModule()->setDeadtimeSize(0x59);//0x59); //0x1);//v
      getTriggerModule()->setMask8(0x40 & 0xffff); // Just CALIB triggers
      getTriggerModule()->setCalibDelay(0x10);
      getTriggerModule()->setFrameLength(0xffff & 9119); // 9119);// 20799);// 9071); // 20799); //20479);// 20799);
      ///////////////////////

      getTriggerModule()->disableTriggers(false);
      // getTriggerModule()->setDeadtimeSize(100); using CALIB triggers
    }
    
    // Load xmit firmware
    getXMITModule()->setMax3000Config();
    getXMITModule()->programMax3000FPGAFirmware( crateconfig.get<std::string>( "xmit_fpga", "") );
    
    // Setup FEMs
    if(getNumberOfTPCFEMs()){
      // Loop over FEMs
      for(size_t tpc_it = 0; tpc_it < getNumberOfTPCFEMs(); tpc_it++){
	usleep(10000);
	// To do: This function needs to be updated/overloaded to configure the zero suppression
	getTPCFEM(tpc_it)->fem_setup(crateconfig);
	TLOG(TLVL_INFO) << "Crate: FEM in slot " << getTPCFEM(tpc_it)->module_number() << " all set.";
	getTPCFEM(tpc_it)->readStatus();

      }

    }

    // Setup tx mode registers (this is done twice for some reason...)
    getXMITReader("NU")->setupTXModeRegister();
    getXMITReader("SN")->setupTXModeRegister();
    
    // Config trigger module
    if(hasTrigger)
      //getTriggerModule()->configureTrigger( _p ); // To do: move it with the other trigger instructions above // using CALIB triggers

    // set up link
    linkSetup();
    
    // Enable triggered stream & Disable continuous stream
    getXMITModule()->enableNUChanEvents(1);
    getXMITModule()->enableSNChanEvents(1);
    
    // setup tx mode registers (again, I know)
    getXMITReader("NU")->setupTXModeRegister();
    getXMITReader("SN")->setupTXModeRegister();
    getControllerModule()->setupTXModeRegister();
    //getXMITModule()->configureSNStreamReader_SNRunOnSyncOnMode();
    
    /*
      if (getTriggerModule()) {
      tTriggerModule()->setupTXModeRegisters();
      getXMITModule()->configureNUStreamReader_TRIGModuleTriggeredMode();
      
      }
    */
    
    getTriggerModule()->enableTriggers();
    getTriggerModule()->runOnSyncOn();

    TLOG(TLVL_INFO) << "Crate: called " << __func__ << " recipe is finished!"; 
  }

  // Configure Trigger Board, FEM, XMIT Module and XMITReader for readout of the NU stream with internal CALIB triggers
  // It follows partially https://github.com/NevisSBND/BoardTest/blob/master/bnlnevistest.c
  void Crate::runCalib( fhicl::ParameterSet const& _p ) {

    fhicl::ParameterSet crateconfig;
    // NOTE: the name of the block in the fcl file must match 
    if( ! _p.get_if_present<fhicl::ParameterSet> ( "nevis_crate", crateconfig ) ){
      TLOG(TLVL_ERROR) << "Crate: Missing nevis_crate configuration block in fcl file" ;
    }
    TLOG(TLVL_INFO) << "Crate: called " << __func__ << " recipe is about to start!"; 	
    // Set up XMIT reader for the neutrino stream
    assert( getXMITReader() );
    getXMITReader()->configureReader();
    getXMITReader()->initializePCIeCard();

    // Set up Controller Module
    assert( getControllerModule() );
    getControllerModule()->initialize(); //v
    getControllerModule()->testOn(); //v
    //  getControllerModule()->runOff(); // this is used to finish run???
    //  getControllerModule()->testOff();

    // Set up Trigger Module
    if( hasTrigger ){
      assert( getTriggerModule() );
      getTriggerModule()->useController( getControllerModule() );
      getTriggerModule()->runOnSyncOff(); //v
      getControllerModule()->testOff(); //v
      // To do: create two NevisTriggerModule_config.fcl, one for external, one for CALIB. Move instructions below to fcl
      getTriggerModule()->setDeadtimeSize(0x59);//0x1);//v
      getTriggerModule()->setMask8(0x40 & 0xffff); // Just CALIB triggers
      getTriggerModule()->setCalibDelay(0x10);
      getTriggerModule()->setFrameLength(0xffff & 9119); // 20799);
      //getTriggerModule()->runOnSyncOn(); //v
      //usleep(5000);
      getTriggerModule()->disableTriggers(false);
    }
    
    // Load XMIT firmware
    assert( getXMITModule() );
    getXMITModule()->useController( getControllerModule() );
    getXMITModule()->setMax3000Config();
    getXMITModule()->programMax3000FPGAFirmware( crateconfig.get<std::string>( "xmit_fpga", "") );

    // Set up FEM modules
    if( getNumberOfTPCFEMs() ){
      // Loop over FEMs
      for( size_t tpc_it = 0; tpc_it < getNumberOfTPCFEMs(); tpc_it++ ){
	usleep(10000);
	getTPCFEM(tpc_it)->fem_setup(crateconfig);
	getTPCFEM(tpc_it)->readStatus();
	TLOG(TLVL_INFO) << "Crate: FEM in slot " << getTPCFEM(tpc_it)->module_number() << " all set.";
      }
    }

    getXMITReader()->setupTXModeRegister();
    
    // Configure Trigger Module
    if( hasTrigger ){
      //    getTriggerModule()->configureTrigger( _p ); // Already done above
    }
    
    linkSetup();

    // Enable triggered stream & Disable continuous stream
    getXMITModule()->enableNUChanEvents(1);
    getXMITModule()->enableSNChanEvents(0);
      
    // Set up TX mode registers
    getXMITReader()->setupTXModeRegister();
    getControllerModule()->setupTXModeRegister();

    getTriggerModule()->enableTriggers();
    getTriggerModule()->runOnSyncOn();

    TLOG(TLVL_INFO) << "Crate: called " << __func__ << " recipe is finished!"; 
  }

} // end of namespace nevistpc


