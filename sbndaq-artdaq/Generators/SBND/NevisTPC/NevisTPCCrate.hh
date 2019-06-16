#ifndef _sbndaq_readout_Generators_NevisTPCFile_generator
#define _sbndaq_readout_Generators_NevisTPCFile_generator 1


#include "sbndaq-artdaq/Generators/SBND/NevisTPC/NevisTPC_generatorBase.hh"
#include "sbndaq-artdaq/Generators/SBND/NevisTPC/nevishwutils/Crate.h"

namespace sbndaq {

class NevisTPCCrate : public sbndaq::NevisTPC_generatorBase {
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

}  // namespace sbndaq

#endif  // _sbndaq_readout_Generators_NevisTPCCrate_generator

