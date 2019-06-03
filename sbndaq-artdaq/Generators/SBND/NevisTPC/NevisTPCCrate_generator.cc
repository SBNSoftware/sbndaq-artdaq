#define TRACE_NAME "NevisTPCGeneratorFile"

#include <algorithm>
#include <exception>

#include "artdaq/DAQdata/Globals.hh"
#include "artdaq/Application/GeneratorMacros.hh"
#include "sbnddaq-readout/Generators/NevisTPC/NevisTPCCrate.hh"

#include "NevisTPCCrate.hh"

void sbnddaq::NevisTPCCrate::ConfigureStart() {

	TLOG(TLVL_INFO)<< "ConfigureStart";
	
    // Initialize our crate (using said crate controller)
    nevistpc::CrateSPtr nevisTestCrate(new nevistpc::Crate(fControllerModule,fNUXMITReader,ps_));

	// Run our recipe!
	nevisTestCrate->run2Stream(ps_);

}

void sbnddaq::NevisTPCCrate::ConfigureStop() {
}


size_t sbnddaq::NevisTPCCrate::GetFEMCrateData() {


	TRACE(TGETDATA, "GetFEMCrateData");

  	// Just for tests
  	// Taken from NevisTPCFile_generator and adapted to use an XMITReader
  	// To be reviewed
  	uint16_t* buffer = new uint16_t[fChunkSize];

  	std::streamsize bytesRead = fNUXMITReader->readsome(reinterpret_cast<char*>(buffer), fChunkSize);
  	unsigned wordsRead = bytesRead * sizeof(char) / sizeof(uint16_t);

  	std::copy(buffer, buffer + wordsRead, &DMABuffer_[0]);

  	delete[] buffer;

  	return bytesRead;
}

DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(sbnddaq::NevisTPCCrate)

