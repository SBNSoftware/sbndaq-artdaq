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

  fTimeOffsetNanoSec = ps_.get<uint32_t>("TimeOffsetNanoSec",1200000); //1.2ms by default

  event_offset_ = 0;
}

void icarus::PhysCrate_GeneratorBase::start() {

  BoardTemps1_.resize(nBoards_);
  BoardTemps2_.resize(nBoards_);

  current_subrun_ = 0;

  metadata_ = PhysCrateFragmentMetadata(RunNumber_,
					nBoards_,
					ChannelsPerBoard_,
					SamplesPerChannel_,
					nADCBits_,
					CompressionScheme_,
					BoardIDs_);

  fCircularBuffer = sbndaq::CircularBuffer<uint16_t>(fCircularBufferSize/sizeof(uint16_t));
  packSize_zero_counter_=0;

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
  metricMan->sendMetric(".getNext.ReturnTime",_tloop_getnext_duration.count()*1000.,"ms",11,
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

   TLOG(TLVL_DEBUG +1 ) << "getNext_(frags) called.";

  metricMan->sendMetric(".getNext.CircularBufferOccupancy",fCircularBuffer.Size()/2,"bytes",11,
			artdaq::MetricMode::LastPoint|artdaq::MetricMode::Maximum|artdaq::MetricMode::Minimum|artdaq::MetricMode::Average);


  if ( fCircularBuffer.Buffer().empty() ) {
    TLOG(TLVL_DEBUG +2 ) << "no data in circular buffer.";
    return true;
  }


  size_t data_size_bytes = 0;
  TLOG(TLVL_DEBUG +3 ) << "nBoards_: " << nBoards_;

  //check if we have enough data for a DataTile.
  //if not, sleep and return.
  if( fCircularBuffer.Size() <= sizeof(DataTile)/sizeof(uint16_t) ){
    TLOG(TLVL_DEBUG +4 ) << "not enough data for DataTile in circular buffer.";
    usleep(1000);
    return true;
  }
  
  //initialize for looking up data
  uint16_t iBoard = 0;
  uint16_t const* first_dt_begin_ptr = fCircularBuffer.LinearizeAndGetData();
  //auto const* first_dt [[gnu::unused]] = reinterpret_cast< DataTile const* >(first_dt_begin_ptr);
  
  //loop over number of boards
  while(iBoard < nBoards_){
    
    std::this_thread::sleep_for(1us);
    TLOG(TLVL_DEBUG +5) << "data_size_bytes/sizeof(uint16_t): " << data_size_bytes/sizeof(uint16_t);

    //check if we have enough data for a DataTile.
    //if not, sleep and return.
    if( fCircularBuffer.Size() < (data_size_bytes+sizeof(DataTile))/sizeof(uint16_t) ){
      TLOG(TLVL_DEBUG +4 ) << "not enough data for DataTile in circular buffer.";
      usleep(1000);
      return true;
    }

    if (not fCircularBuffer.Buffer().is_linearized()){
      TLOG(TLVL_DEBUG + 7)
        << "fCircularBuffer is not linear. Buffer will be linearized when setting next_dt_begin_ptr";
    }

    //  the Tile Header is at the beginning of the board data:
    auto const* next_dt_begin_ptr = fCircularBuffer.LinearizeAndGetData() + data_size_bytes/sizeof(uint16_t);
    auto const* next_dt = reinterpret_cast< DataTile const* >(next_dt_begin_ptr);
    auto const nt_header = next_dt->Header;
    uint32_t this_data_size_bytes = ntohl( nt_header.packSize );
    TLOG(TLVL_DEBUG +6) << "Board ID: " << (nt_header.info2 & 0x0000000F);
    TLOG(TLVL_DEBUG +6) << "this_data_size_bytes: " << this_data_size_bytes;

    //check if the DataTile is empty. If it is there's an issue
    if(this_data_size_bytes==0){
      TLOG(TLVL_DEBUG+6) << "No data in this DataTile? Happened " << packSize_zero_counter_ << " times before.";
      //don't let this happen too much
      if(packSize_zero_counter_==10){
        TLOG(TLVL_ERROR) << "Too many times (10) with zero data size in TileHeader. Exit.";
        return false;
      }
      //give it some time otherwise
      ++packSize_zero_counter_;
      usleep(1000);
      return true;
    }

    //check if there's enogh data for this board.
    //if not, sleep and return.
    if(fCircularBuffer.Size() < (data_size_bytes+this_data_size_bytes)/sizeof(uint16_t)){
      TLOG(TLVL_DEBUG +4 ) << "not enough data for DataTile " << iBoard << " in circular buffer.";
      usleep(1000);
      return true;
    }

    data_size_bytes += this_data_size_bytes;
    ++iBoard;
    if (not fCircularBuffer.Buffer().is_linearized()){
      TLOG(TLVL_DEBUG + 7)
        << "fCircularBuffer is not linear. Linearize for debugging statements.";
      first_dt_begin_ptr = fCircularBuffer.LinearizeAndGetData();
      next_dt_begin_ptr = fCircularBuffer.LinearizeAndGetData() + data_size_bytes/sizeof(uint16_t);
      next_dt = reinterpret_cast< DataTile const* >(next_dt_begin_ptr);
    }
    TLOG(TLVL_DEBUG +7 ) << "iBoard: " << iBoard 
                     << ", this_data_size_bytes: " << this_data_size_bytes 
                     << ", data_size_bytes: " << data_size_bytes
                     << ", first_dt_begin_ptr: " << first_dt_begin_ptr;

    auto const* board_block = reinterpret_cast< A2795DataBlock const * >( next_dt->data );
    TLOG(TLVL_DEBUG+8)<< "BoardEventNumber: " << board_block->header.event_number << ", BoardTimeStamp: " 
	<< board_block->header.time_stamp ;

  }

  //timestamp
  artdaq::Fragment::timestamp_t ts(0);
  {
    using namespace boost::gregorian;
    using namespace boost::posix_time;
  
    ptime t_now(microsec_clock::universal_time());
    ptime time_t_epoch(date(1970,1,1));
    time_duration diff = t_now - time_t_epoch;

    ts = diff.total_nanoseconds() - fTimeOffsetNanoSec;
  }

  metricMan->sendMetric(".getNext.Size",last_read_data_size_,"bytes",11,
			artdaq::MetricMode::LastPoint|artdaq::MetricMode::Minimum|artdaq::MetricMode::Maximum|artdaq::MetricMode::Average);
  
  frags.emplace_back( artdaq::Fragment::FragmentBytes( data_size_bytes,
						      0, fragment_id(),
						      sbndaq::detail::FragmentType::PHYSCRATEDATA, metadata_, ts) );

  //make sure the ptr to the first DataTile is valid
  first_dt_begin_ptr = fCircularBuffer.LinearizeAndGetData();
  TLOG(TLVL_DEBUG +8 ) << "Initialized data of size " << frags.back()->dataSizeBytes();
  
  TLOG(TLVL_DEBUG +9 ) << "Read data size was " << data_size_bytes;
  TLOG(TLVL_DEBUG +10 ) <<"fCircularBuffer.Buffer().front(): " 
                   << fCircularBuffer.Buffer().front()
                   << ", fCircularBuffer.Buffer().back(): " << fCircularBuffer.Buffer().back();
  std::copy( first_dt_begin_ptr, first_dt_begin_ptr + data_size_bytes/sizeof(uint16_t),
             (uint16_t*)( frags.back()->dataBeginBytes() ) );

  TLOG(TLVL_DEBUG + 11) << "copied the circular buffer to frags.";
  TLOG(TLVL_DEBUG + 12) << "Read data size was " <<  data_size_bytes;

  frags.back()->resizeBytes( data_size_bytes );
  TLOG(TLVL_DEBUG + 13) << "resized frags.";

  // Erase the copied part of the circular buffer
  fCircularBuffer.Erase( data_size_bytes/sizeof(uint16_t) );
  TLOG(TLVL_DEBUG + 14) << "erazed the circular buffer.";

  //give proper event number
  // ++ev_num;
  PhysCrateFragment const& newfrag = *frags.back();
  auto ev_num = newfrag.BoardEventNumber();
  TLOG(TLVL_DEBUG +15) << "Board 0 Event Number: " << newfrag.BoardEventNumber() << ", TimeStamp: " << newfrag.BoardTimeStamp();
  TLOG(TLVL_DEBUG +15) << "Board 1 Event Number: " << newfrag.BoardEventNumber(1) << ", TimeStamp: " << newfrag.BoardTimeStamp(1);
  TLOG(TLVL_DEBUG +15) << "Fragment: nBoards: " << newfrag.nBoards() << ", nChannels: " << newfrag.nChannels() << ", nSamplesPerChannel: "
            << newfrag.nSamplesPerChannel() << ", nChannelsPerBoard: " << newfrag.nChannelsPerBoard();
  TLOG(TLVL_DEBUG + 16) << "ev_num: " << ev_num;

  metricMan->sendMetric(".getNext.BoardEventNumber",(int)ev_num,"events",11,
			artdaq::MetricMode::LastPoint);
  metricMan->sendMetric(".getNext.BoardTimeStamp",(long unsigned int)newfrag.BoardTimeStamp(),"ticks",11,
			artdaq::MetricMode::LastPoint);

  int max_ev_num_diff=0;
  int max_ts_diff=0;

  bool found_inconsistent_boards=false;

  for ( uint16_t jBoard = 1; jBoard < nBoards_; ++jBoard ) {
    if ( newfrag.BoardEventNumber(jBoard) != ev_num ) {
      if( std::abs((int)newfrag.BoardEventNumber(jBoard) - (int)newfrag.BoardEventNumber(0))>max_ev_num_diff )
	max_ev_num_diff = std::abs((int)newfrag.BoardEventNumber(jBoard) - (int)newfrag.BoardEventNumber(0));

      if( std::abs((int)newfrag.BoardTimeStamp(jBoard) - (int)newfrag.BoardTimeStamp(0))>max_ev_num_diff )
	max_ts_diff = std::abs((int)newfrag.BoardTimeStamp(jBoard) - (int)newfrag.BoardTimeStamp(0));

      TLOG(TLVL_ERROR) << "Inconsistent event numbers in one fragment: Event " 
		       << ev_num << " in Board 0 while Event " 
		       << newfrag.BoardEventNumber(jBoard) 
		       << " in Board " << jBoard;
      found_inconsistent_boards=true;
    }
  }

  metricMan->sendMetric("MaxBoardEventNumDiff",max_ev_num_diff,"events",11,
			artdaq::MetricMode::LastPoint|artdaq::MetricMode::Minimum|artdaq::MetricMode::Maximum|artdaq::MetricMode::Average);
  metricMan->sendMetric("MaxBoardTimeStampDiff",max_ts_diff,"ticks",11,
			artdaq::MetricMode::LastPoint|artdaq::MetricMode::Minimum|artdaq::MetricMode::Maximum|artdaq::MetricMode::Average);

  if(found_inconsistent_boards){
    TLOG(TLVL_ERROR) << "Inconsistent event numbers within one of the fragments found."
                     << " Will return false in getNext_ and hopefully stop.";

    for(uint16_t iBoard=0; iBoard<nBoards_; ++iBoard)
      TLOG(TLVL_ERROR) << "Board " << iBoard << ":\n" << *(newfrag.BoardDataBlock(iBoard));

    return false;
  }

  if(ev_num==0)
    event_offset_ = 1;

  frags.back()->setSequenceID(ev_num+event_offset_);

  
  metricMan->sendMetric("FragmentsSent",ev_counter(), "Events", 11,artdaq::MetricMode::LastPoint);

	TLOG(TLVL_DEBUG + 18) << "completed.";
  _tloop_getnext_start = std::chrono::high_resolution_clock::now();
  _tloop_getnext_duration = std::chrono::duration_cast< std::chrono::duration<double> >(_tloop_getnext_start-_tloop_getnext_end);
  metricMan->sendMetric(".getNext.FinishTime",_tloop_getnext_duration.count()*1000.,"ms",11,
			artdaq::MetricMode::LastPoint | artdaq::MetricMode::Maximum | artdaq::MetricMode::Average);

  TLOG(TLVL_DEBUG+19)
		   << " : Send fragment with type " << (int)frags.back()->type()
		   << " (" << frags.back()->typeString() << "):  "
		   << " (id,seq,timestamp)=(" << frags.back()->fragmentID() << ","<<frags.back()->sequenceID()<< "," << frags.back()->timestamp();

  //if we're sending out the packet we can reset the counter
  packSize_zero_counter_=0;

  return true;

}
