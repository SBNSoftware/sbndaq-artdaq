#ifndef _sbndaq_readout_Generators_NevisTBStream_generator
#define _sbndaq_readout_Generators_NevisTBStream_generator

/**                                                                                                                                                                        
 * Nevis TB data generator                                                                                                                                                 
 *                                                                                                                                                                         
 * Author: D.Kalra  <dkalra@nevis.columbia.edu>, 2023/05/01                                                                                                                
 */

#include "sbndaq-artdaq/Generators/SBND/NevisTPC/NevisTB_generatorBase.hh"
#include "sbndaq-artdaq/Generators/SBND/NevisTPC/nevishwutils/ControllerModule.h"
#include "sbndaq-artdaq/Generators/SBND/NevisTPC/nevishwutils/Crate.h"
#include "sbndaq-artdaq/Generators/SBND/NevisTPC/nevishwutils/XMITReader.h"
#include <fstream> // temp                                                                                                                                                 

namespace sbndaq {

  class NevisTBStream : public sbndaq::NevisTB_generatorBase {
  public:
    explicit NevisTBStream(fhicl::ParameterSet const& _p) :
      NevisTB_generatorBase(_p),
      fntbreader( new nevistpc::NevisTBStreamReader("ntb_reader",_p))
      //      fControllerModule( new nevistpc::ControllerModule(_p) ),
      //fNUXMITReader( new nevistpc::XMITReader("nu_xmit_reader", _p) ),
      //fSNXMITReader( new nevistpc::XMITReader("sn_xmit_reader", _p) )
    {
      ConfigureNTBStart();
    }
    virtual ~NevisTBStream() {}
  private:
    void ConfigureNTBStart();
    void ConfigureNTBStop();

     size_t GetNevisTBData();

    /*  bool GetNTBData(); //! Get NTB stream data                                                                                                                  
    share::WorkerThreadUPtr GetNTBData_thread_;
    */
    std::unique_ptr<uint16_t[]> NTBDMABuffer_;
    CircularBuffer_t NTBCircularBuffer_;
    //bool WriteNTBData(); //! Write NTB stream data                                                                                                              
    // share::WorkerThreadUPtr WriteNTBData_thread_;
    uint16_t* NTBBuffer_;
   

    //    nevistpc::ControllerModuleSPtr fControllerModule;
    //nevistpc::XMITReaderSPtr fNUXMITReader;
    //nevistpc::XMITReaderSPtr fSNXMITReader;
    //nevistpc::CrateSPtr fCrate;

    nevistpc::NevisTBStreamReaderSPtr fntbreader;

    uint32_t fNTBChunkSize;
    bool fDumpNTBBinary; //!< Write binary file before the artdaq back-end                                                                                                 
    std::string fDumpNTBBinaryDir; //!< Directory for binary file dump                                                                                                     
    std::ofstream binFileNTB;
    char binFileNameNevisTB[80];

  };

}  // namespace sbndaq                                                                                                                                                     

#endif // _sbndaq_readout_Generators_NevisTBStream_generator                                                                                                               


