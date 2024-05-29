#ifndef _sbndaq_readout_Generators_NevisTPCNUXMIT_generator
#define _sbndaq_readout_Generators_NevisTPCNUXMIT_generator

/**
 * Nevis TPC NU XMIT data generator
 *
 * Reads the neutrino stream of a Nevis XMIT board
 *
 * Author: J. I. Crespo-Anadón  <jcrespo@nevis.columbia.edu>, 2018/05/08
 */

#include "sbndaq-artdaq/Generators/SBND/NevisTPC/NevisTPC_generatorBase.hh"
#include "sbndaq-artdaq/Generators/SBND/NevisTPC/nevishwutils/ControllerModule.h"
#include "sbndaq-artdaq/Generators/SBND/NevisTPC/nevishwutils/Crate.h"
#include "sbndaq-artdaq/Generators/SBND/NevisTPC/nevishwutils/XMITReader.h"

#include <fstream> // temp

namespace sbndaq {

  class NevisTPCNUXMIT : public sbndaq::NevisTPC_generatorBase {
  public:
    explicit NevisTPCNUXMIT(fhicl::ParameterSet const& _p) :
      NevisTPC_generatorBase(_p),
      fControllerModule( new nevistpc::ControllerModule(_p) ),
      fNUXMITReader( new nevistpc::XMITReader("nu_xmit_reader", _p) )
    {
    }
    virtual ~NevisTPCNUXMIT() {}
    
  private:
    void ConfigureStart() override;
    void ConfigureStop() override;
    
    size_t GetFEMCrateData() override;

    nevistpc::ControllerModuleSPtr fControllerModule;
    nevistpc::XMITReaderSPtr fNUXMITReader;
    nevistpc::CrateSPtr fCrate;

    uint32_t fChunkSize;  //!< Number of bytes to read at once

    int fSubrunCheckPeriod_us;  //! Period in microseconds to check if a new subrun is needed

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

#endif // _sbndaq_readout_Generators_NevisTPCNUXMIT_generator

