#include "CrateControllerModule.h"

void nevistpc::CrateControllerModule::initialize(){
	ControlDataPacket packet = ControlDataPacket(0,0,0,0x0);
	packet.append(0x0);
	nevisControllerPCIeCard()->send(packet,0);
}
void nevistpc::CrateControllerModule::testOn(){
	nevisControllerPCIeCard()->send(ControlDataPacket(0,1,0x1,0x0),0);
}
void nevistpc::CrateControllerModule::runOff(){
	nevisControllerPCIeCard()->send(ControlDataPacket(15,0,0,0x0),0);
}
void nevistpc::CrateControllerModule::testOff(){
	nevisControllerPCIeCard()->send(ControlDataPacket(0,1,0x0,0x0),0);
}
void nevistpc::CrateControllerModule::Configure(){
	initialize();
	testOn();
	runOff();
	testOff();
}

nevistpc::CrateControllerModule::CrateControllerModule(NevisControllerPCIeCardSPtr nevisControllerPCIeCard) : _nevisControllerPCIeCard(nevisControllerPCIeCard){
}

void nevistpc::CrateControllerModule::useNevisControllerPCIeCard(NevisControllerPCIeCardSPtr nevisControllerPCIeCard){
	_nevisControllerPCIeCard = nevisControllerPCIeCard;
}
	
nevistpc::NevisControllerPCIeCardSPtr nevistpc::CrateControllerModule::nevisControllerPCIeCard(){
	return _nevisControllerPCIeCard;
}

void nevistpc::CrateControllerModule::send(ControlDataPacket const &packet, useconds_t const sleep_time_us){
	nevisControllerPCIeCard()->send(packet,sleep_time_us);
}
	

nevistpc::UsesCrateControllerModule::UsesCrateControllerModule() : _crateController(nullptr){
}

nevistpc::CrateControllerModuleSPtr nevistpc::UsesCrateControllerModule::crateController(){
	if(!_crateController)
		throw std::runtime_error("Controller module was not set.");

	return _crateController;
}

void nevistpc::UsesCrateControllerModule::useCrateController(CrateControllerModuleSPtr crateController){
	_crateController = crateController;
}
