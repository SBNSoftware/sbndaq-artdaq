#ifndef _sbndaq_readout_Generators_NevisTPC2StreamNUandSNXMIT_generator
#define _sbndaq_readout_Generators_NevisTPC2StreamNUandSNXMIT_generator

/**
 * Nevis TPC 2-Stream NU and SN XMIT data generator
 *
 * Configures 2 streams and reads the trigger stream of a Nevis XMIT board using artdaq, 
 * and the supernova stream strem using a worker thread
 *
 * Author: J. I. Crespo-Anadón  <jcrespo@nevis.columbia.edu>, 2019/07/03
 */

#include "sbndaq-artdaq/Generators/SBND/NevisTPC/NevisTPC_generatorBase.hh"
#include "sbndaq-artdaq/Generators/SBND/NevisTPC/nevishwutils/ControllerModule.h"
#include "sbndaq-artdaq/Generators/SBND/NevisTPC/nevishwutils/Crate.h"
#include "sbndaq-artdaq/Generators/SBND/NevisTPC/nevishwutils/XMITReader.h"

#include <fstream> // temp
#include <zmq.hpp>

namespace sbndaq {

  class NevisTPC2StreamNUandSNXMIT : public sbndaq::NevisTPC_generatorBase {
  public:
    explicit NevisTPC2StreamNUandSNXMIT(fhicl::ParameterSet const& _p) :
      NevisTPC_generatorBase(_p),
      fControllerModule( new nevistpc::ControllerModule(_p) ),
      fNUXMITReader( new nevistpc::XMITReader("nu_xmit_reader", _p) ),
      fSNXMITReader( new nevistpc::XMITReader("sn_xmit_reader", _p) ),
      context(1), _zmqGPSPublisher(context, ZMQ_PUB)
    {
      ConfigureStart();
    }
    virtual ~NevisTPC2StreamNUandSNXMIT() {}
    
  private:
    void ConfigureStart();
    void ConfigureStop();
    
    size_t GetFEMCrateData();

    nevistpc::ControllerModuleSPtr fControllerModule;
    nevistpc::XMITReaderSPtr fNUXMITReader;
    nevistpc::XMITReaderSPtr fSNXMITReader;
    nevistpc::CrateSPtr fCrate;

    uint32_t fChunkSize;  //!< Number of bytes to read at once in NU stream
    uint32_t fSNChunkSize;  //!< Number of bytes to read at once in SN stream
    int fGPSZMQPortNTB; //! Port used to connect NTB and TPC board readers. must be the same port defined in NTB fcl

    bool FireCALIB(); //! Fire CALIB trigger
    share::WorkerThreadUPtr FireCALIB_thread_;
    double fCALIBFreq;  //!< Frequency in Hz of the CALIB trigger

    bool FireController(); //! Fire Controller trigger
    share::WorkerThreadUPtr FireController_thread_;
    double fControllerTriggerFreq;  //!< Frequency in Hz of the Controller trigger

    bool MonitorCrate(); //! Check status of the boards in the crate
    share::WorkerThreadUPtr MonitorCrate_thread_;
    int fMonitorPeriod;  //!< Period in seconds to read electronics status

    bool GPSTime();
    share::WorkerThreadUPtr GPSTime_thread_;
    int fGPSTimeFreq;

    bool fSNReadout; //!< Do continuous readout (supernova stream)
    bool GetSNData(); //! Get SN stream data
    share::WorkerThreadUPtr GetSNData_thread_;
    std::unique_ptr<uint16_t[]> SNDMABuffer_;
    CircularBuffer_t SNCircularBuffer_;
    bool WriteSNData(); //! Write SN stream data
    share::WorkerThreadUPtr WriteSNData_thread_;
    uint16_t* SNBuffer_;

    zmq::context_t context;
    zmq::socket_t _zmqGPSPublisher;

    bool fDumpBinary; //!< Write binary file before the artdaq back-end
    std::string fDumpBinaryDir; //!< Directory for binary file dump
    std::ofstream binFileNU; // temp 
    std::ofstream binFileSN; // temp 
    char binFileNameNU[80]; // Name of binary dump file for NU stream
    char binFileNameSN[80]; // Name of binary dump file for SN stream

  };
  
}  // namespace sbndaq

#endif // _sbndaq_readout_Generators_NevisTPC2StreamNUandSNXMIT_generator

