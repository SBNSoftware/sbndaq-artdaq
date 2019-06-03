#define TRACE_NAME "NevisTPCGeneratorFile"

#include <algorithm>
#include <exception>
#include <fstream>
#include <iostream>

#include "artdaq/DAQdata/Globals.hh"
#include "artdaq/Application/GeneratorMacros.hh"
#include "sbnddaq-readout/Generators/NevisTPC/NevisTPCFile.hh"

void sbnddaq::NevisTPCFile::ConfigureStart() {
  TLOG(TLVL_INFO)<< "ConfigureStart";
  
  std::string inputFilename = ps_.get<std::string>("InputFilename");
  fWaitTime = ps_.get<int>("WaitTime", 500000);
  fChunkSize = ps_.get<int>("ChunkSize", 4096);

  // Open the input file
  fInputFile.open(inputFilename.c_str(), std::ios::in | std::ios::binary );
  if (!fInputFile.is_open()) {
    throw std::runtime_error("Error: Could not open input file.");
  }
}


void sbnddaq::NevisTPCFile::ConfigureStop() {
  // Close the input file
  if (fInputFile.is_open()) {
    fInputFile.close();
  }
}


size_t sbnddaq::NevisTPCFile::GetFEMCrateData() {
  usleep(fWaitTime);
  
  TRACE(TGETDATA, "GetFEMCrateData");

  while (fInputFile.peek() != EOF) {
    uint16_t* buffer = new uint16_t[fChunkSize];

    fInputFile.read(reinterpret_cast<char*>(buffer), fChunkSize);
    std::streamsize bytesRead = fInputFile.gcount();
    unsigned wordsRead = bytesRead * sizeof(char) / sizeof(uint16_t);

    std::copy(buffer, buffer + wordsRead, &DMABuffer_[0]);

    delete[] buffer;

    return bytesRead;
  }

  return 0;
}

DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(sbnddaq::NevisTPCFile)

