#ifndef nevistpc_crate_h
#define nevistpc_crate_h 1

#include "ControllerModule.h"
#include "NevisTPCFEM.h"
#include "XMITReader.h"
#include "XMITModule.h"
#include "TriggerModule.h"

namespace nevistpc{
  
  class Crate{
  public:
    //Crate();
    Crate(ControllerModuleSPtr ctr, XMITReaderSPtr xmit_reader, fhicl::ParameterSet const& _p);
    Crate(ControllerModuleSPtr ctr, int xmit_module_number, int max_fem_slot);
    //~Crate();
    
    ControllerModuleSPtr getControllerModule();
    uint getNumberOfTPCFEMs();
    NevisTPCFEMSPtr getTPCFEM(uint i);
    XMITModuleSPtr getXMITModule();
    XMITReaderSPtr getXMITReader();
    TriggerModuleSPtr getTriggerModule();
    
    bool hasTrigger;
    
    void linkSetup();
    // Configure Trigger Board, FEM, XMIT Module and XMITReader for readout of the NU stream with external triggers
    // It follows uboonedaq run2Stream but SN stream has been removed
    void runNUStream(fhicl::ParameterSet const& _p);
    void run2Stream(fhicl::ParameterSet const& _p); // Only NU stream so far
    // Configure Trigger Board, FEM, XMIT Module and XMITReader for readout of the NU stream with internal CALIB triggers
    // It follows partially https://github.com/NevisSBND/BoardTest/blob/master/bnlnevistest.c
    void runCalib(fhicl::ParameterSet const& _p);
    
  private:
    ControllerModuleSPtr 			_ctrlr_module;
    XMITModuleSPtr					_xmit_module;
    XMITReaderSPtr					_xmit_reader;
    std::vector < NevisTPCFEMSPtr > _fem_modules;
    TriggerModuleSPtr				_trigger_module;
    
  };
  
  typedef std::unique_ptr<Crate> CrateUPtr;
  typedef std::shared_ptr<Crate> CrateSPtr;
  
}
#endif
