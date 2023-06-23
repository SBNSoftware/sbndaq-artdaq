#ifndef nevistpc_crate_h
#define nevistpc_crate_h 1

#include "ControllerModule.h"
#include "NevisTPCFEM.h"
#include "XMITReader.h"
#include "XMITModule.h"
#include "TriggerModule.h"
#include "NevisTBStreamReader.h"

namespace nevistpc{
  
  class Crate{
  public:
    //Crate();
    Crate(ControllerModuleSPtr ctr, XMITReaderSPtr nu_xmit_reader, fhicl::ParameterSet const& _p, XMITReaderSPtr sn_xmit_reader=NULL);
    Crate(ControllerModuleSPtr ctr, int xmit_module_number, int max_fem_slot);
    //~Crate();
    
    ControllerModuleSPtr getControllerModule();
    uint getNumberOfTPCFEMs();
    NevisTPCFEMSPtr getTPCFEM(uint i);
    XMITModuleSPtr getXMITModule();
    XMITReaderSPtr getXMITReader();
    XMITReaderSPtr getXMITReader(std::string name); // Two-stream overload
    TriggerModuleSPtr getTriggerModule();
    //    NevisTBStreamReaderSPtr getNevisTBStreamReader();
    
    bool hasTrigger;
    
    void linkSetup();
    // Configure Trigger Board, FEM, XMIT Module and XMITReader for readout of the NU stream with external triggers
    // It follows uboonedaq run2Stream but SN stream has been removed
    void runNUStream(fhicl::ParameterSet const& _p);
    // Configure Trigger Board, FEM, XMIT Module and XMITReader for readout of the NU and SN streams with external triggers
    void run2Stream(fhicl::ParameterSet const& _p);
    void runControllerTrigger2Stream(fhicl::ParameterSet const& _p);
    void runCalib2Stream(fhicl::ParameterSet const& _p);
    // Configure Trigger Board, FEM, XMIT Module and XMITReader for readout of the NU stream with internal CALIB triggers
    // It follows partially https://github.com/NevisSBND/BoardTest/blob/master/bnlnevistest.c
    void runCalib(fhicl::ParameterSet const& _p);
    
  private:
    ControllerModuleSPtr 			_ctrlr_module;
    XMITModuleSPtr					_xmit_module;
    XMITReaderSPtr					_xmit_reader; // Deafault (NU) stream
    XMITReaderSPtr					_sn_xmit_reader; // SN stream
    std::vector < NevisTPCFEMSPtr > _fem_modules;
    TriggerModuleSPtr				_trigger_module;
    //NevisTBStreamReaderSPtr    _nevistb_reader;
  };
  
  typedef std::unique_ptr<Crate> CrateUPtr;
  typedef std::shared_ptr<Crate> CrateSPtr;
  
}
#endif
