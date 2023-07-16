#define TRACE_NAME "NevisTBStreamGenerator"

#include "artdaq/DAQdata/Globals.hh"
#include "artdaq/Generators/GeneratorMacros.hh"
#include "sbndaq-artdaq/Generators/SBND/NevisTPC/NevisTBStream.hh"


#include <chrono>
#include <ctime>

void sbndaq::NevisTBStream::ConfigureNTBStart() {
  TLOG(TLVL_INFO) << "ConfigureStart NTB";

  fNTBChunkSize = ps_.get<int>("NTBChunkSize", 96);

  fDumpNTBBinary = ps_.get<bool>("DumpNTBBinary", false);
  fDumpNTBBinaryDir = ps_.get<std::string>("DumpNTBBinaryDir", ".");

 

  NTBDMABuffer_.reset(new uint16_t[fNTBChunkSize]);
  NTBCircularBuffer_ = CircularBuffer(1e9/sizeof(uint16_t)); // to do: define in fcl                                             
                                           
  NTBCircularBuffer_.Init();
  NTBBuffer_ = new uint16_t[fNTBChunkSize];
 
  if( fDumpNTBBinary ){
    // Get timestamp for binary file name                                                                                                                                  
    time_t t = time(0);
    struct tm ltm = *localtime( &t );

    sprintf(binFileNameNevisTB, "%s/sbndrawbin_run%06i_%4i.%02i.%02i-%02i.%02i.%02i_NevisTB.dat",
            fDumpNTBBinaryDir.c_str(), sbndaq::NevisTB_generatorBase::run_number(),
            ltm.tm_year + 1900, ltm.tm_mon + 1, ltm.tm_mday, ltm.tm_hour, ltm.tm_min, ltm.tm_sec);
    binFileNTB.open( binFileNameNevisTB, std::ofstream::out | std::ofstream::binary ); // temp                                                                         

    TLOG(TLVL_INFO)<< "Opening raw binary file " << binFileNameNevisTB;

  }

  TLOG(TLVL_INFO) << "Configure NTB PCIe card";
  fntbreader->configureReader();
  fntbreader->initializePCIeCard();
  fntbreader->setupTXModeRegister();
  TLOG(TLVL_INFO) << "Configured NTB PCIe card";

}

void sbndaq::NevisTBStream::ConfigureNTBStop() {

  if( fDumpNTBBinary ){
    TLOG(TLVL_INFO)<< "Closig raw binary file " << binFileNameNevisTB;
    binFileNTB.close();
  }


  TLOG(TLVL_INFO)<< "Successful " << __func__ ;
  mf::LogInfo("NevisTBStream") << "Successful " << __func__;
}

size_t sbndaq::NevisTBStream::GetNevisTBData() {
  TLOG(TLVL_INFO)<< "GetNevisTBData";

  std::streamsize bytesRead = fntbreader->readsome(reinterpret_cast<char*>(&DMABufferNTB_[0]), fNTBChunkSize);  

  binFileNTB.write( (char*)(&DMABufferNTB_[0]), bytesRead); //fNTBChunkSize );                         
  binFileNTB.flush();
 return bytesRead;

}


DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(sbndaq::NevisTBStream)
