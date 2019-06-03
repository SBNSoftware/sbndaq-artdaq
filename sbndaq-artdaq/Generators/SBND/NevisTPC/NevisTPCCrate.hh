#ifndef _sbnddaq_readout_Generators_NevisTPCFile_generator
#define _sbnddaq_readout_Generators_NevisTPCFile_generator 1


#include "sbnddaq-readout/Generators/NevisTPC/NevisTPC_generatorBase.hh"
#include "sbnddaq-readout/Generators/NevisTPC/nevishwutils/Crate.h"

namespace sbnddaq {

class NevisTPCCrate : public sbnddaq::NevisTPC_generatorBase {
	public:
		explicit NevisTPCCrate(fhicl::ParameterSet const& _p) : NevisTPC_generatorBase(_p), fControllerModule( new nevistpc::ControllerModule(_p) ), fNUXMITReader(new nevistpc::XMITReader("nu_xmit_reader", _p) ) {}
  		virtual ~NevisTPCCrate() {}

	private:
  		void ConfigureStart();
  		void ConfigureStop();

  		size_t GetFEMCrateData();

		nevistpc::ControllerModuleSPtr fControllerModule;
		nevistpc::XMITReaderSPtr fNUXMITReader;

		uint32_t fChunkSize;
};

}  // namespace sbnddaq

#endif  // _sbnddaq_readout_Generators_NevisTPCCrate_generator

