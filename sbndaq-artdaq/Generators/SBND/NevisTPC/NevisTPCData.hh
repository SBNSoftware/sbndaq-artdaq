//
// sbndaq-artdaq/Generators/SBND/NevisTPCData_generator.hh
//

#ifndef _sbndaq_readout_Generators_NevisTPCData_generator
#define _sbndaq_readout_Generators_NevisTPCData_generator

#include "sbndaq-artdaq/Generators/SBND/NevisTPC/NevisTPC_generatorBase.hh"

#include <random>

namespace sbndaq {

	class NevisTPCData : public sbndaq::NevisTPC_generatorBase {
		public:

			explicit NevisTPCData(fhicl::ParameterSet const & ps);
    		virtual ~NevisTPCData();

  		private:

			void ConfigureStart() override;
			void ConfigureStop() override {};

			size_t GetFEMCrateData() override;

			std::mt19937 engine_;
			int data_wait_time_;
			size_t events_per_packet_;
			unsigned int time_increment_per_event_;
			int cp;
	  uint32_t local_event_number;
	};
}

#endif
