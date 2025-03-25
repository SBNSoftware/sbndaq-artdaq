#ifndef nevistpc_nevistpccrate_h
#define nevistpc_nevistpccrate_h 1

#include "Crate.h"

namespace nevistpc{

	class NevisTPCCrate{
		public:
			NevisTPCCrate(fhicl::Parameterset const& params);
			~NevisTPCCrate();

		private:
			NevisControllerPCIeCard _NevisControllerPCIeCard;
        	NevisReadoutPCIeCard    _NevisReadoutPCIeCard;
        	CrateSptr _Crate;
	};
}
#endif
