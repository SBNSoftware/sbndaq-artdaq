#define TRACE_NAME "PhysCrate_GeneratorBase"
#include "sbndaq-artdaq/Generators/ICARUS/PhysCrate_GeneratorBase.hh"

#include "art/Utilities/ExceptionMessages.h"
#include "artdaq/Generators/GeneratorMacros.hh"
#include "cetlib_except/exception.h"
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
using namespace std::chrono_literals;

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
  assignBoardID_ = ps_.get<bool>("AssignBoardID", false);
  if ( assignBoardID_ ) BoardIDs_ = ps_.get< std::vector<PhysCrateFragmentMetadata::id_t> >("BoardIDs");
  CompressionScheme_ = ps_.get<uint32_t>("CompressionScheme",0);

  throttle_usecs_ = ps_.get<size_t>("throttle_usecs", 100000);
  throttle_usecs_check_ = ps_.get<size_t>("throttle_usecs_check", 10000);
  fCircularBufferSize = ps_.get<size_t>("CircularBufferSize", 1e9);

  if (throttle_usecs_ > 0 && (throttle_usecs_check_ >= throttle_usecs_ ||
			      throttle_usecs_ % throttle_usecs_check_ != 0) ) {
    throw cet::exception("Error in PhysCrate: disallowed combination of throttle_usecs and throttle_usecs_check (see PhysCrate.hh for rules)");
  }
  
  pcieLinks_ = ps_.get< std::vector<int> >("pcieLinks",std::vector<int>());

  least_data_block_bytes_ = (size_t)SamplesPerChannel_*(size_t)ChannelsPerBoard_*2+12+32;
}

void icarus::PhysCrate_GeneratorBase::start() {


  current_subrun_ = 0;

  metadata_ = PhysCrateFragmentMetadata(RunNumber_,
					nBoards_,
					ChannelsPerBoard_,
					SamplesPerChannel_,
					nADCBits_,
					CompressionScheme_,
					BoardIDs_);

  fCircularBuffer = sbndaq::CircularBuffer<uint16_t>(fCircularBufferSize/sizeof(uint16_t));

  ConfigureStart();

  share::ThreadFunctor functor = std::bind(&PhysCrate_GeneratorBase::Monitor,this);
  auto worker_functor = share::WorkerThreadFunctorUPtr(new share::WorkerThreadFunctor(functor,"MonitorWorkerThread"));
  auto monitor_worker = share::WorkerThread::createWorkerThread(worker_functor);
  Monitor_thread_.swap(monitor_worker);

  Monitor_thread_->start();

  _tloop_getnext_end = std::chrono::high_resolution_clock::now();
  _tloop_getnext_start = std::chrono::high_resolution_clock::now();
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

  _tloop_getnext_end = std::chrono::high_resolution_clock::now();
  auto _tloop_getnext_duration = std::chrono::duration_cast< std::chrono::duration<double> >(_tloop_getnext_end-_tloop_getnext_start);
  metricMan->sendMetric(".getNext.ReturnTime",_tloop_getnext_duration.count()*1000.,"ms",1,
			artdaq::MetricMode::LastPoint | artdaq::MetricMode::Maximum | artdaq::MetricMode::Average);


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

  metricMan->sendMetric(".getNext.CircularBufferOccupancy",fCircularBuffer.Size()/2,"bytes",1,
			artdaq::MetricMode::LastPoint|artdaq::MetricMode::Maximum|artdaq::MetricMode::Minimum|artdaq::MetricMode::Average);


  if ( fCircularBuffer.Buffer().empty() ) {
    TRACE(TR_LOG,"PhysCrate_GeneratorBase::getNext_(frags) no data in circular buffer.");
    return true;
  }

  //std::cout << metadata_ << std::endl;
  // TRACE(TR_DEBUG,"\tPhysCrate_GeneratorBase::getNext_ Expected data size: %lu",metadata_.ExpectedDataSize());




  size_t data_size_bytes = 0;
  TLOG(TLVL_DEBUG) << "PhysCrate_GeneratorBase::getNext_ : nBoards_: " << nBoards_;

  if( fCircularBuffer.Size() <= least_data_block_bytes_/sizeof(uint16_t)*(size_t)( nBoards_ ) ){
    TRACE(TR_LOG,"PhysCrate_GeneratorBase::getNext_(frags) not enough data in circular buffer.");
    usleep(1000);
    return true;
  }

  uint16_t iBoard = 0;
  uint16_t const* first_dt_begin_ptr = fCircularBuffer.LinearizeAndGetData();


  do {

    do {
      std::this_thread::sleep_for(1us);
      TLOG(15) << "data_size_bytes/sizeof(uint16_t): " << data_size_bytes/sizeof(uint16_t);
    } while ( fCircularBuffer.Size() <= least_data_block_bytes_/sizeof(uint16_t)*(size_t)( nBoards_ - iBoard ) );

    auto const* first_dt = reinterpret_cast< DataTile const* >(first_dt_begin_ptr);
    //  the Tile Header is at the beginning of the board data:
    auto const* next_dt_begin_ptr = first_dt_begin_ptr + data_size_bytes/sizeof(uint16_t);
    auto const* next_dt = reinterpret_cast< DataTile const* >(next_dt_begin_ptr);
    auto const nt_header = next_dt->Header;
    uint32_t this_data_size_bytes = ntohl( nt_header.packSize );
    TLOG(20) << "PhysCrate_GeneratorBase::getNext_ : Board ID: " << (nt_header.info2 & 0x0000000F);
    TLOG(20) << "this_data_size_bytes: " << this_data_size_bytes;
    data_size_bytes += this_data_size_bytes;
    ++iBoard;
    TLOG(TLVL_DEBUG) << "PhysCrate_GeneratorBase::getNext_ : iBoard: " << iBoard 
                     << ", this_data_size_bytes: " << this_data_size_bytes 
                     << ", data_size_bytes: " << data_size_bytes
                     << ", first_dt_begin_ptr: " << first_dt_begin_ptr;

    auto const* board_block = reinterpret_cast< A2795DataBlock const * >( next_dt->data );
    TLOG(TLVL_DEBUG) << "BoardEventNumber: " << board_block->header.event_number << ", BoardTimeStamp: " 
	<< board_block->header.time_stamp << std::endl;

  } while ( iBoard < nBoards_ );

  // std::cout << "Go to a new event..." << std::endl;
  metricMan->sendMetric(".getNext.Size",last_read_data_size_,"bytes",1,
			artdaq::MetricMode::LastPoint|artdaq::MetricMode::Minimum|artdaq::MetricMode::Maximum|artdaq::MetricMode::Average);
  
  frags.emplace_back( artdaq::Fragment::FragmentBytes( data_size_bytes,
						      0, fragment_id(),
						      sbndaq::detail::FragmentType::PHYSCRATEDATA, metadata_) );
  TLOG(TLVL_DEBUG) << "PhysCrate_GeneratorBase::getNext_ Initialized data of size " << frags.back()->dataSizeBytes();
  
  TLOG(TLVL_DEBUG) << "PhysCrate_GeneratorBase::getNext_ : Read data size was " << data_size_bytes;
  TLOG(TLVL_DEBUG) << "PhysCrate_GeneratorBase::getNext_ : fCircularBuffer.Buffer().front(): " 
                   << fCircularBuffer.Buffer().front()
                   << ", fCircularBuffer.Buffer().back(): " << fCircularBuffer.Buffer().back();
  std::copy( first_dt_begin_ptr, first_dt_begin_ptr + data_size_bytes/sizeof(uint16_t),
             (uint16_t*)( frags.back()->dataBeginBytes() ) );
  // TRACE(42,"42");

  TLOG(TLVL_DEBUG) << "PhysCrate_GeneratorBase::getNext_ : copied the circular buffer to frags.";
  TRACE(TR_DEBUG,"PhysCrate_GeneratorBase::getNext_ : Read data size was %lu", (long unsigned int)data_size_bytes );

  frags.back()->resizeBytes( data_size_bytes );
  TLOG(TLVL_DEBUG) << "PhysCrate_GeneratorBase::getNext_ : resized frags.";
  // Erase the copied part of the circular buffer
  fCircularBuffer.Erase( data_size_bytes/sizeof(uint16_t) );
  TLOG(TLVL_DEBUG) << "PhysCrate_GeneratorBase::getNext_ : erazed the circular buffer.";

  //give proper event number
  // ++ev_num;
  PhysCrateFragment const& newfrag = *frags.back();
  auto ev_num = newfrag.BoardEventNumber();
  TLOG(TLVL_DEBUG) << "Board 0 Event Number: " << newfrag.BoardEventNumber() << ", TimeStamp: " << newfrag.BoardTimeStamp();
  TLOG(TLVL_DEBUG) << "Board 1 Event Number: " << newfrag.BoardEventNumber(1) << ", TimeStamp: " << newfrag.BoardTimeStamp(1);
  TLOG(TLVL_DEBUG) << "Fragment: nBoards: " << newfrag.nBoards() << ", nChannels: " << newfrag.nChannels() << ", nSamplesPerChannel: "
            << newfrag.nSamplesPerChannel() << ", nChannelsPerBoard: " << newfrag.nChannelsPerBoard();
  TLOG(TLVL_DEBUG) << "ev_num: " << ev_num;
  // TLOG(20) << "PhysCrate_GeneratorBase: event number: " << ev_num << ", DataPayloadSize: " << newfrag.DataPayloadSize();

  metricMan->sendMetric(".getNext.BoardEventNumber",(int)ev_num,"events",1,
			artdaq::MetricMode::LastPoint);
  metricMan->sendMetric(".getNext.BoardTimeStamp",(long unsigned int)newfrag.BoardTimeStamp(),"ticks",1,
			artdaq::MetricMode::LastPoint);

  int max_ev_num_diff=0;
  int max_ts_diff=0;


  for ( uint16_t jBoard = 1; jBoard < nBoards_; ++jBoard ) {
    if ( newfrag.BoardEventNumber(jBoard) != ev_num ) {
      //for ( uint16_t kB = 0; kB < nBoards_; ++kB ){
      //TLOG(20) << "PhysCrate_GeneratorBase: Board " << kB << " event number: " << newfrag.BoardEventNumber(kB) << ", timestamp: " << newfrag.BoardTimeStamp(kB) << ", DataTileHeaderLocation: " << newfrag.DataTileHeaderLocation(kB);
	//throw cet::exception("PhysCrate_GeneratorBase") << "Inconsistent event numbers in one fragment: Event " << ev_num << " in Board 0 while Event " << newfrag.BoardEventNumber(jBoard) << " in Board " << jBoard;
      //}

      if( std::abs((int)newfrag.BoardEventNumber(jBoard) - (int)newfrag.BoardEventNumber(0))>max_ev_num_diff )
	max_ev_num_diff = std::abs((int)newfrag.BoardEventNumber(jBoard) - (int)newfrag.BoardEventNumber(0));

      if( std::abs((int)newfrag.BoardTimeStamp(jBoard) - (int)newfrag.BoardTimeStamp(0))>max_ev_num_diff )
	max_ts_diff = std::abs((int)newfrag.BoardTimeStamp(jBoard) - (int)newfrag.BoardTimeStamp(0));

      TLOG(TLVL_ERROR) << "Inconsistent event numbers in one fragment: Event " 
		       << ev_num << " in Board 0 while Event " 
		       << newfrag.BoardEventNumber(jBoard) 
		       << " in Board " << jBoard;
    }
  }

  metricMan->sendMetric("MaxBoardEventNumDiff",max_ev_num_diff,"events",1,
			artdaq::MetricMode::LastPoint|artdaq::MetricMode::Minimum|artdaq::MetricMode::Maximum|artdaq::MetricMode::Average);
  metricMan->sendMetric("MaxBoardTimeStampDiff",max_ts_diff,"ticks",1,
			artdaq::MetricMode::LastPoint|artdaq::MetricMode::Minimum|artdaq::MetricMode::Maximum|artdaq::MetricMode::Average);

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
  
  metricMan->sendMetric("FragmentsSent",ev_counter(), "Events", 1,artdaq::MetricMode::LastPoint);

  TRACE(TR_LOG,"PhysCrate_GeneratorBase::getNext_ completed.");
  _tloop_getnext_start = std::chrono::high_resolution_clock::now();
  _tloop_getnext_duration = std::chrono::duration_cast< std::chrono::duration<double> >(_tloop_getnext_start-_tloop_getnext_end);
  metricMan->sendMetric(".getNext.FinishTime",_tloop_getnext_duration.count()*1000.,"ms",1,
			artdaq::MetricMode::LastPoint | artdaq::MetricMode::Maximum | artdaq::MetricMode::Average);

  return true;

}
