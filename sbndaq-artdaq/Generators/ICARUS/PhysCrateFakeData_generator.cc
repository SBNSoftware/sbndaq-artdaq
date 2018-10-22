#include "sbndaq-artdaq/Generators/ICARUS/PhysCrateFakeData.hh"
#include "artdaq/Application/GeneratorMacros.hh"
#include "artdaq/DAQdata/Globals.hh"
#include <iomanip>
#include <iterator>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "sbndaq-artdaq-core/Trace/trace_defines.h"

icarus::PhysCrateFakeData::PhysCrateFakeData(fhicl::ParameterSet const & ps)
  :
  PhysCrate_GeneratorBase(ps)
{
}

void icarus::PhysCrateFakeData::ConfigureStart(){
  TRACE(TR_LOG,"PhysCrateFakeData::ConfigureStart() called.");
  engine_ = std::mt19937(ps_.get<int64_t>("random_seed", 314159));
  uniform_distn_.reset(new std::uniform_int_distribution<int>(0, std::pow(2,metadata_.num_adc_bits() - 1 )));
  event_number_ = 1;
  TRACE(TR_LOG,"PhysCrateFakeData::ConfigureStart() completed.");

}

void icarus::PhysCrateFakeData::ConfigureStop(){}

bool icarus::PhysCrateFakeData::Monitor(){ return true; }

int icarus::PhysCrateFakeData::GetData(size_t & data_size, uint32_t* data_loc){

  data_size=0;

  for(size_t i_b=0; i_b<metadata_.num_boards(); ++i_b){
    size_t offset = (sizeof(DataTile::Header)/sizeof(uint32_t)) + 
      i_b*(2+metadata_.channels_per_board()*metadata_.samples_per_channel()/2);
    data_loc[offset]   = (event_number_ & 0xffffff);
    data_loc[offset+1] = (event_number_+100);

    std::generate_n((uint16_t*)(&(data_loc[offset+2])),
		    metadata_.channels_per_board()*metadata_.samples_per_channel(),
		    [&](){ return static_cast<uint16_t>((*uniform_distn_)( engine_)); });
    data_size += (metadata_.channels_per_board()*metadata_.samples_per_channel()/2 + 2)*sizeof(uint32_t);

  }
  
  data_size += sizeof(DataTile::Header);

  DataTile* dt = (DataTile*)data_loc;
  memcpy((&(dt->Header.token)),"DATA",4);
  dt->Header.info1=0;
  dt->Header.info2=0;
  dt->Header.info3=0;
  dt->Header.timeinfo=0;
  dt->Header.chID=0;
  dt->Header.packSize = htonl(data_size);
  /*
  std::copy((char*)(&header),
	    (char*)(&header)+data_size,
	    (char*)data_loc);
  */
  ++event_number_;
  return 0;
}

void icarus::PhysCrateFakeData::FillStatPack( statpack & pack )
{
  TRACE(TR_LOG,"PhysCrateFakeData::FillStatPack(pack) called.");
  memcpy(&(pack.token), "STAT", 4);
  pack.crateid = htonl(158);
  //pack.crateid = htonl(physCr->getId());
  pack.memstat1 = 0;
  pack.memstat2 = 0;
  pack.size = htonl(28);

  TRACE(TR_LOG,"PhysCrateFakeData::FillStatPack(pack) completed.");
}


DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(icarus::PhysCrateFakeData) 
