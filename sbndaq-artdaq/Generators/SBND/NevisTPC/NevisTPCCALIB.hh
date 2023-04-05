#ifndef _sbndaq_readout_Generators_NevisTPCCALIB_generator
#define _sbndaq_readout_Generators_NevisTPCCALIB_generator

/**
 * Nevis TPC CALIB data generator
 *
 * Generates CALIB triggers and reads the neutrino stream of a Nevis XMIT board
 *
 * Author: J. I. Crespo-Anadón  <jcrespo@nevis.columbia.edu>, 2018/05/17
 */

#include "sbndaq-artdaq/Generators/SBND/NevisTPC/NevisTPC_generatorBase.hh"
#include "sbndaq-artdaq/Generators/SBND/NevisTPC/nevishwutils/ControllerModule.h"
#include "sbndaq-artdaq/Generators/SBND/NevisTPC/nevishwutils/Crate.h"
#include "sbndaq-artdaq/Generators/SBND/NevisTPC/nevishwutils/XMITReader.h"

#include <fstream> // temp

namespace sbndaq {

  class NevisTPCCALIB : public sbndaq::NevisTPC_generatorBase {
  public:
    explicit NevisTPCCALIB(fhicl::ParameterSet const& _p) :
      NevisTPC_generatorBase(_p),
      fControllerModule( new nevistpc::ControllerModule(_p) ),
      fNUXMITReader( new nevistpc::XMITReader("nu_xmit_reader", _p) )
    {
    }
    virtual ~NevisTPCCALIB() {}
    
  private:
    void ConfigureStart();
    void FireTriggers(){
      TLOG(TLVL_WARNING)<< "This function is not implemented. Ask Daisy K. for details.";
};

    void ReportTPCCrateInfo(){
      TLOG(TLVL_WARNING) << "This function is not implemented. Ask Daisy K. for details.";
    };

    void ConfigureStop();
    
    size_t GetFEMCrateData();

    nevistpc::ControllerModuleSPtr fControllerModule;
    nevistpc::XMITReaderSPtr fNUXMITReader;
    nevistpc::CrateSPtr fCrate;

    uint32_t fChunkSize;  //!< Number of bytes to read at once

    int fSubrunCheckPeriod_us;  //! Period in microseconds to check if a new subrun is needed

    bool FireCALIB(); //! Fire CALIB trigger
    share::WorkerThreadUPtr FireCALIB_thread_;
    double fCALIBFreq;  //!< Frequency in Hz of the CALIB trigger

    bool MonitorCrate(); //! Check status of the boards in the crate
    share::WorkerThreadUPtr MonitorCrate_thread_;
    int fMonitorPeriod;  //!< Period in seconds to read electronics status

    bool fDumpBinary; //!< Write binary file before the artdaq back-end
    std::string fDumpBinaryDir; //!< Directory for binary file dump
    std::ofstream binFile; // Binary dump file
    char binFileName[80]; // Name of binary dump file

    // Reimplemented with hardware-driven subruns
    bool FillFragment(artdaq::FragmentPtrs &,bool clear_buffer=false);

  };
  
}  // namespace sbndaq

#endif // _sbndaq_readout_Generators_NevisTPCCALIB_generator

