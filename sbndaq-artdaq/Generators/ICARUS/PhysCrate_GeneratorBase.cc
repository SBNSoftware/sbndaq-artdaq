#include "sbndaq-artdaq/Generators/ICARUS/PhysCrate_GeneratorBase.hh"

#include "art/Utilities/ExceptionMessages.h"
#include "artdaq/Application/GeneratorMacros.hh"
#include "cetlib/exception.h"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "fhiclcpp/ParameterSet.h"
#include "artdaq-core/Utilities/SimpleLookupPolicy.hh"

//#include "CAENComm.h"
//#include "keyb.h"
//#include "keyb.c"

#include <fstream>
#include <iomanip>
#include <iterator>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "sbndaq-artdaq-core/Trace/trace_defines.h"

icarus::PhysCrate_GeneratorBase::PhysCrate_GeneratorBase(fhicl::ParameterSet const & ps)
  :
  CommandableFragmentGenerator(ps),
  ps_(ps)
{
  Initialize();
}

void icarus::PhysCrate_GeneratorBase::Initialize(){

  RunNumber_ = ps_.get<uint32_t>("RunNumber",999);
  SamplesPerChannel_ = ps_.get<uint32_t>("SamplesPerChannel",3000);
  nADCBits_ = ps_.get<uint8_t>("nADCBits",12);
  ChannelsPerBoard_ = ps_.get<uint16_t>("ChannelsPerBoard",16);
  nBoards_ = ps_.get<uint16_t>("nBoards",9);
  CrateID_ = ps_.get<uint8_t>("CrateID",0x1);
  BoardIDs_ = ps_.get< std::vector<PhysCrateFragmentMetadata::id_t> >("BoardIDs");
  CompressionScheme_ = ps_.get<uint32_t>("CompressionScheme",0);

  throttle_usecs_ = ps_.get<size_t>("throttle_usecs", 100000);
  throttle_usecs_check_ = ps_.get<size_t>("throttle_usecs_check", 10000);
  fCircularBufferSize = ps_.get<size_t>("CircularBufferSize", 1e9);
  metadata_ = PhysCrateFragmentMetadata(RunNumber_,
					nBoards_,
					ChannelsPerBoard_,
					SamplesPerChannel_,
					nADCBits_,
					CompressionScheme_,
					BoardIDs_);

  if (throttle_usecs_ > 0 && (throttle_usecs_check_ >= throttle_usecs_ ||
			      throttle_usecs_ % throttle_usecs_check_ != 0) ) {
    throw cet::exception("Error in PhysCrate: disallowed combination of throttle_usecs and throttle_usecs_check (see PhysCrate.hh for rules)");
  }
  
  pcieLinks_ = ps_.get< std::vector<int> >("pcieLinks",std::vector<int>());
  
}

void icarus::PhysCrate_GeneratorBase::start() {
  current_subrun_ = 0;

  fCircularBuffer = sbndaq::CircularBuffer<uint16_t>(fCircularBufferSize/sizeof(uint16_t));

  ConfigureStart();

  share::ThreadFunctor functor = std::bind(&PhysCrate_GeneratorBase::Monitor,this);
  auto worker_functor = share::WorkerThreadFunctorUPtr(new share::WorkerThreadFunctor(functor,"MonitorWorkerThread"));
  auto monitor_worker = share::WorkerThread::createWorkerThread(worker_functor);
  Monitor_thread_.swap(monitor_worker);

  Monitor_thread_->start();
}

void icarus::PhysCrate_GeneratorBase::stop() {
  Monitor_thread_->stop();

  ConfigureStop();
}

void icarus::PhysCrate_GeneratorBase::Cleanup(){
}

icarus::PhysCrate_GeneratorBase::~PhysCrate_GeneratorBase(){
  Cleanup();
}

bool icarus::PhysCrate_GeneratorBase::getNext_(artdaq::FragmentPtrs & frags) {

  //throttling...
  if (throttle_usecs_ > 0) {
    size_t nchecks = throttle_usecs_ / throttle_usecs_check_;
    
    for (size_t i_c = 0; i_c < nchecks; ++i_c) {
      usleep( throttle_usecs_check_ );
      
      if (should_stop()) {
	return false;
      }
    }
  } else {
    if (should_stop()) {
      return false;
    }
  }
  
  // Set fragment's metadata

  TRACE(TR_LOG,"PhysCrate_GeneratorBase::getNext_(frags) called.");

  if ( fCircularBuffer.Buffer().empty() ) {
    TRACE(TR_LOG,"PhysCrate_GeneratorBase::getNext_(frags) no data in circular buffer.");
    return true;
  }

  //std::cout << metadata_ << std::endl;
  TRACE(TR_DEBUG,"\tPhysCrate_GeneratorBase::getNext_ Expected data size: %lu",metadata_.ExpectedDataSize());

  frags.emplace_back( artdaq::Fragment::FragmentBytes(metadata_.ExpectedDataSize()*1.2,  
						      0, fragment_id(),
						      sbndaq::detail::FragmentType::PHYSCRATEDATA, metadata_) );

  TRACE(TR_DEBUG,"\tPhysCrate_GeneratorBase::getNext_ Initialized data of size %lu",frags.back()->dataSizeBytes());

  PhysCrateFragment newfrag(*frags.back());

  size_t data_size = 0;
  std::cout << "PhysCrate_GeneratorBase::getNext_ : nBoards_: " << nBoards_ << std::endl;

  uint16_t iBoard = 0;
  do {
    uint16_t const* data_ptr = fCircularBuffer.LinearizeAndGetData();
    auto const* dt = reinterpret_cast< DataTile const* >(data_ptr);
    //  the Tile Header is at the beginning of the board data:
    auto const* nextTile = dt + data_size;
    auto const nt_header = nextTile->Header;;
    uint32_t this_data_size = ntohl( nt_header.packSize );
    if ( this_data_size == 0 ) continue;
    data_size += this_data_size;
    ++iBoard;
    std::cout << "PhysCrate_GeneratorBase::getNext_ : iBoard: " << iBoard << ", this_data_size: " << this_data_size << ", data_size: " << data_size << std::endl;

    auto const* board_block = reinterpret_cast< A2795DataBlock const * >( nextTile->data );
    std::cout << "BoardEventNumber: " << board_block->header.event_number << ", BoardTimeStamp: " 
	<< board_block->header.time_stamp << std::endl;

  } while ( iBoard < nBoards_ );

  // std::cout << "Go to a new event..." << std::endl;
  //GAL: metricMan->sendMetric(".GetData.Size",last_read_data_size_,"bytes",1,artdaq::MetricMode::LastPoint);
  //GAL: metricMan->sendMetric(".GetData.Size.last",last_read_data_size_,"bytes",1,artdaq::MetricMode::LastPoint);
  //GAL: metricMan->sendMetric(".GetData.Size.min",last_read_data_size_,"bytes",1,artdaq::MetricMode::LastPoint);
  //GAL: metricMan->sendMetric(".GetData.Size.max",last_read_data_size_,"bytes",1,artdaq::MetricMode::LastPoint);
  
  std::cout << "PhysCrate_GeneratorBase::getNext_ : Read data size was " << data_size << std::endl;
  std::copy( fCircularBuffer.Buffer().begin(), fCircularBuffer.Buffer().begin() + data_size/sizeof(uint16_t),
             (uint16_t*)( frags.back()->dataBeginBytes() ) );
  // TRACE(42,"42");

  std::cout << "PhysCrate_GeneratorBase::getNext_ : copied the circular buffer to frags." << std::endl;
  TRACE(TR_DEBUG,"\tPhysCrate_GeneratorBase::getNext_ : Read data size was %lu", (long unsigned int)data_size );

  frags.back()->resizeBytes( data_size );
  std::cout << "PhysCrate_GeneratorBase::getNext_ : resized frags." << std::endl;
  // Erase the copied part of the circular buffer
  fCircularBuffer.Erase( data_size/sizeof(uint16_t) );
  std::cout << "PhysCrate_GeneratorBase::getNext_ : erazed the circular buffer." << std::endl;

  //give proper event number
  // ++ev_num;
  auto ev_num = newfrag.BoardEventNumber()+1;
  std::cout << "Board 0 Event Number: " << newfrag.BoardEventNumber() << ", TimeStamp: " << newfrag.BoardTimeStamp() << std::endl;
  std::cout << "Board 1 Event Number: " << newfrag.BoardEventNumber(1) << ", TimeStamp: " << newfrag.BoardTimeStamp(1) << std::endl;
  std::cout << "Fragment: nBoards: " << newfrag.nBoards() << ", nChannels: " << newfrag.nChannels() << ", nSamplesPerChannel: "
            << newfrag.nSamplesPerChannel() << ", nChannelsPerBoard: " << newfrag.nChannelsPerBoard() << std::endl;
  std::cout << "ev_num: " << ev_num << std::endl;
  frags.back()->setSequenceID(ev_num);

  //GAL: metricMan->sendMetric(".getNext.EventNumber.last",(int)ev_num,"events",1,artdaq::MetricMode::LastPoint);

  /*
  //create fragment with no metadata
  frags.emplace_back( std::unique_ptr<artdaq::Fragment>( new artdaq::Fragment(ev_num, fragment_id()+1000,
									      icarus::detail::FragmentType::PHYSCRATESTAT)) );
  frags.back()->resizeBytes( sizeof(statpack) );
  FillStatPack(last_stat_pack_);
  std::copy((char*)(&last_stat_pack_),(char*)(&last_stat_pack_)+sizeof(statpack),frags.back()->dataBeginBytes());
  */

  /*
  if(metricMan != nullptr) {
    //GAL: metricMan->sendMetric("Fragments Sent",ev_counter(), "Events", 1,artdaq::MetricMode::LastPoint);
  }
  */

  TRACE(TR_LOG,"PhysCrate_GeneratorBase::getNext_ completed.");
  return true;

}
