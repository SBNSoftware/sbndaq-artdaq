//
// sbnddaq-readout/Generators/NevisTPCData_generator.hh
//

#ifndef _sbnddaq_readout_Generators_NevisTPCData_generator
#define _sbnddaq_readout_Generators_NevisTPCData_generator

#include "sbnddaq-readout/Generators/NevisTPC/NevisTPC_generatorBase.hh"

#include <random>

namespace sbnddaq {

	class NevisTPCData : public sbnddaq::NevisTPC_generatorBase {
		public:

			explicit NevisTPCData(fhicl::ParameterSet const & ps);
    		virtual ~NevisTPCData();

  		private:

			void ConfigureStart();
			void ConfigureStop(){};

			size_t GetFEMCrateData();

			std::mt19937 engine_;
			int data_wait_time_;
			size_t events_per_packet_;
			unsigned int time_increment_per_event_;
			int cp;
	  uint32_t local_event_number;
	};
}

#endif
