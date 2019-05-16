//
// sbnddaq-readout/Generators/NevisTPCData_generator.cc
//

#define TRACE_NAME "NevisTPCGeneratorData"
#include "artdaq/DAQdata/Globals.hh"

#include "sbnddaq-readout/Generators/NevisTPCData.hh"
#include "artdaq/Application/GeneratorMacros.hh"

#include <iomanip>
#include <iterator>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

sbnddaq::NevisTPCData::NevisTPCData(fhicl::ParameterSet const & ps): NevisTPC_generatorBase(ps){

  TRACE(TINFO,"NevisTPCData constructor called");
  TRACE(TINFO,"NevisTPCData constructor completed");
}

sbnddaq::NevisTPCData::~NevisTPCData(){

  TRACE(TINFO,"NevisTPCData destructor called");
  TRACE(TINFO,"NevisTPCData destructor completed");
}

void sbnddaq::NevisTPCData::ConfigureStart(){

  TRACE(TINFO,"NevisTPCData::ConfigureStart() called");
  
  engine_ = std::mt19937(ps_.get<int64_t>("random_seed", 314159));
  data_wait_time_ = ps_.get<int>("data_wait_time",5e5);
  events_per_packet_ = ps_.get<size_t>("events_per_packet",5);
  time_increment_per_event_ = ps_.get<unsigned int>("time_increment_per_event",49999);
  
  //cp = 0;
  local_event_number=0;
  
}

size_t sbnddaq::NevisTPCData::GetFEMCrateData(){

  usleep(data_wait_time_);
  
  TRACE(TGETDATA,"GETTIN' DATA!");
  NevisTPCHeader tpc_header(0,4,2,local_event_number++,0,2455,1,1);
  TRACE(TGETDATA,"Creating TPC data with FEMID=%u,Slot=%u,ADCWordCount=%u,Event=%u,Frame=%u",
	tpc_header.getFEMID(),tpc_header.getSlot(),
	tpc_header.getADCWordCount(),
	tpc_header.getEventNum(),
	tpc_header.getFrameNum());

  //std::cout << tpc_data;

  uint16_t garbo[18];
  garbo[0]  = 0xFFFF;
  garbo[1]  = 0xFFFF;
  garbo[2]  = (uint16_t)(tpc_header.id_and_slot & 0xFFFF);
  garbo[3]  = (uint16_t)(tpc_header.id_and_slot >> 16);
  garbo[4]  = (uint16_t)(tpc_header.word_count & 0xFFFF);
  garbo[5]  = (uint16_t)(tpc_header.word_count >> 16);
  garbo[6]  = (uint16_t)(tpc_header.event_num & 0xFFFF);
  garbo[7]  = (uint16_t)(tpc_header.event_num >> 16);
  garbo[8]  = (uint16_t)(tpc_header.frame_num & 0xFFFF);
  garbo[9]  = (uint16_t)(tpc_header.frame_num >> 16);
  garbo[10] = (uint16_t)(tpc_header.checksum & 0xFFFF);
  garbo[11] = (uint16_t)(tpc_header.checksum >> 16);
  garbo[12] = (uint16_t)(tpc_header.trig_frame_sample & 0xFFFF);
  garbo[13] = (uint16_t)(tpc_header.trig_frame_sample > 16);
  garbo[14] = 0x4959;
  garbo[15] = 0x4959;
  garbo[16] = 0x0000;
  garbo[17] = 0xE000;
  std::copy(garbo,garbo+18,&DMABuffer_[0]);
  TRACE(TGETDATA,"Copied data of size %lu into DMA Buffer",sizeof(garbo));
  return sizeof(garbo);
  /*  
  if(cp==0){
    uint16_t garboinit[2] = { 0xFFFF, 0xFFFF };
    std::copy(garboinit,garboinit+2,&DMABuffer_[0]);
    cp ++;
    return sizeof(garboinit);
  }	
  else if(cp==1){
    // trash data to see if we can get things filling regularly
    uint16_t garbo[14] = {	(uint16_t)(tpc_data.id_and_slot & 0xFFFF), 			(uint16_t)(tpc_data.id_and_slot >> 16),
				(uint16_t)(tpc_data.word_count & 0xFFFF),			(uint16_t)(tpc_data.word_count >> 16),
				(uint16_t)(tpc_data.event_num & 0xFFFF),			(uint16_t)(tpc_data.event_num >> 16),
				(uint16_t)(tpc_data.frame_num & 0xFFFF),			(uint16_t)(tpc_data.frame_num >> 16),
				(uint16_t)(tpc_data.checksum & 0xFFFF),				(uint16_t)(tpc_data.checksum >> 16),
				(uint16_t)(tpc_data.trig_frame_sample & 0xFFFF), 	(uint16_t)(tpc_data.trig_frame_sample > 16),
				0x4959, 0x4959	};
    std::copy(garbo,garbo+14,&DMABuffer_[0]);
    
    cp ++;
    return sizeof(garbo);
  }
  */
  // generate the fakedata here and fill up our pointer friend. NOWWEREINBUSINESS
  
  
  //	std::generate_n(feb_data.adc,nChannels_,[&](){ return static_cast<uint16_t>((*uniform_distn_)( engine_)); });
  //  FEBDTPBufferUPtr[i_e] = feb_data;
  
  
  /*
    
    
  // Okay, so now comes the adc data!
  int nchannels = 64;
  int nwords = 9600;
  bool compressed = false;
  uint16_t adcstart, adcend, adcword;
  for(int i = 0; i < nchannels; i++){
  // channel start
  adcstart = 0x4000 || (0x3F & i);
  binFile.write((char*)&adcstart,sizeof(adcstart));
  
  for(int n = 0; n < nwords; n++){
  // just fill with a set thing now. we can do random stuff later.
  if(compressed)  adcword = 0x8000 || (n*i & 0x5FFF);
  else adcword = 0x0000 || (n*i &0x0FFF);
  binFile.write((char*)&adcword,sizeof(adcword));
  }
  
  adcend = 0x5000 || (0x3F & i);
  binFile.write((char*)&adcend,sizeof(adcend));
  }
  */
  
  //return sizeof(tpc_data);
  return 0;
}

DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(sbnddaq::NevisTPCData)
