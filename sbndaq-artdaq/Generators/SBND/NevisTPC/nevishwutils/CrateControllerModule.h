#ifndef _sbnddaq_readout_Generators_nevistpc_CrateControllerModule_h
#define _sbnddaq_readout_Generators_nevistpc_CrateControllerModule_h 1

#include <memory>
#include "NevisControllerPCIeCard.h"


namespace nevistpc {

	class CrateControllerModule {
		public:
			CrateControllerModule();
			CrateControllerModule(NevisControllerPCIeCardSPtr nevisControllerPCIeCard);
			//~CrateControllerModule();

			NevisControllerPCIeCardSPtr nevisControllerPCIeCard();
			void useNevisControllerPCIeCard(NevisControllerPCIeCardSPtr nevisControllerPCIeCard);

			void Configure();
			void initialize();
			void testOn();
			void runOff();
			void testOff();

			void send(ControlDataPacket const &packet, useconds_t const sleep_time_us);
		
		private:
			NevisControllerPCIeCardSPtr _nevisControllerPCIeCard;
	};

	typedef std::unique_ptr<CrateControllerModule> CrateControllerModuleUPtr;
	typedef std::shared_ptr<CrateControllerModule> CrateControllerModuleSPtr;


	class UsesCrateControllerModule {
		public:
			UsesCrateControllerModule();
			virtual ~UsesCrateControllerModule(){};
			virtual void useCrateController(CrateControllerModuleSPtr crateController);
			CrateControllerModuleSPtr crateController();

		private:
			CrateControllerModuleSPtr _crateController;
	
	};
}
#endif
