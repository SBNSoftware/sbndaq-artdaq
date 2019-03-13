#include "sbndaq-artdaq/Generators/Common/BernCRTZMQ_GeneratorBase.hh"

//#include "art/Utilities/Exception.h"
#include "cetlib/exception.h"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "fhiclcpp/ParameterSet.h"
//#include "artdaq-core/Utilities/SimpleLookupPolicy.h"
#include "artdaq/DAQdata/Globals.hh"

#include "BernCRT_TRACE_defines.h"

#include <fstream>
#include <iomanip>
#include <iterator>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <sys/time.h>
#include <sys/timeb.h>


sbndaq::BernCRTZMQ_GeneratorBase::BernCRTZMQ_GeneratorBase(fhicl::ParameterSet const & ps)
  :
  CommandableFragmentGenerator(ps),
  ps_(ps)
{
  TRACE(TR_LOG,"BernFeb constructor called");  
  Initialize();
  TRACE(TR_LOG,"BernFeb constructor completed");
}


/*---------------------------------------------------------------------*/

bool be_verbose_section = false;

void sbndaq::BernCRTZMQ_GeneratorBase::Initialize(){

if(be_verbose_section){
std::cout << std::endl;
std::cout << "---------------------------" << std::endl;
std::cout << "SECTION 1 - INITIALIZE" << std::endl;
std::cout << "---------------------------" << std::endl;  
std::cout << std::endl;
}

  TRACE(TR_LOG,"BernFeb::Initialze() called");
  
  //RunNumber_ = ps_.get<uint32_t>("RunNumber",999);
  RunNumber_ = 0;
  SubrunTimeWindowSize_ = ps_.get<uint64_t>("SubRunTimeWindowSize",60e9); //one minute
  SequenceTimeWindowSize_ = ps_.get<uint32_t>("SequenceTimeWindowSize",5e6); //5 ms
  nADCBits_  = ps_.get<uint8_t>("nADCBits",12);
  nChannels_ = ps_.get<uint32_t>("nChannels",32);
  ReaderID_ = ps_.get<uint8_t>("ReaderID",0x1);
  FEBIDs_ = ps_.get< std::vector<uint64_t> >("FEBIDs");

  FragmentCounter_ = 1;

  if(SequenceTimeWindowSize_<1e6)
    throw cet::exception("BernCRTZMQ_GeneratorBase::Initialize")
      << "Sequence Time Window size is less than 1 ms (1e6 ns). This is not supported.";

  FEBBufferCapacity_ = ps_.get<uint32_t>("FEBBufferCapacity",5e3);
  FEBBufferSizeBytes_ = FEBBufferCapacity_*sizeof(BernCRTZMQEvent);

  ZMQBufferCapacity_ = ps_.get<uint32_t>("ZMQBufferCapacity",1024*30);
  ZMQBufferSizeBytes_ = ZMQBufferCapacity_*sizeof(BernCRTZMQEvent);

  MaxTimeDiffs_   = ps_.get< std::vector<uint32_t> >("MaxTimeDiffs",std::vector<uint32_t>(FEBIDs_.size(),1e7));

  if(MaxTimeDiffs_.size()!=FEBIDs_.size()){
    if(MaxTimeDiffs_.size()==1){
      auto size = MaxTimeDiffs_.at(0);
      MaxTimeDiffs_ = std::vector<uint32_t>(FEBIDs_.size(),size);
    }
    else{
      throw cet::exception("BernCRTZMQ_GeneratorBase::Iniitalize")
	<< "MaxTimeDiffs must be same size as ZMQIDs in config!";
    }
  }

  throttle_usecs_ = ps_.get<size_t>("throttle_usecs", 100000);
  throttle_usecs_check_ = ps_.get<size_t>("throttle_usecs_check", 10000);
  
  if(nChannels_!=32)
    throw cet::exception("BernCRTZMQ_GeneratorBase::Initialize")
      << "nChannels != 32. This is not supported.";


  if (throttle_usecs_ > 0 && (throttle_usecs_check_ >= throttle_usecs_ ||
			      throttle_usecs_ % throttle_usecs_check_ != 0) ) {
    throw cet::exception("Error in BernCRTZMQ: disallowed combination of throttle_usecs and throttle_usecs_check (see BernCRTZMQ.hh for rules)");
  }
  
  TRACE(TR_LOG,"BernFeb::Initialize() completed");

						

  for( size_t i_id=0; i_id<FEBIDs_.size(); ++i_id){
    auto const& id = FEBIDs_[i_id];
    FEBBuffers_[id] = FEBBuffer_t(FEBBufferCapacity_,MaxTimeDiffs_[i_id],id);
  }
  ZMQBufferUPtr.reset(new BernCRTZMQEvent[ZMQBufferCapacity_]);

  TRACE(TR_DEBUG,"\tMade %lu ZMQBuffers",FEBIDs_.size());

  TRACE(TR_LOG,"BernFeb::Initialize() ... starting GetData worker thread.");
  share::ThreadFunctor functor = std::bind(&BernCRTZMQ_GeneratorBase::GetData,this);
  auto worker_functor = share::WorkerThreadFunctorUPtr(new share::WorkerThreadFunctor(functor,"GetDataWorkerThread"));
  auto getData_worker = share::WorkerThread::createWorkerThread(worker_functor);
  GetData_thread_.swap(getData_worker);

  SeqIDMinimumSec_ = 0;
}

/*-----------------------------------------------------------------------*/


void sbndaq::BernCRTZMQ_GeneratorBase::start() {

if(be_verbose_section){
std::cout << std::endl;
std::cout << "---------------------------" << std::endl;
std::cout << "SECTION 2 - START" << "\n";
std::cout << "---------------------------" << std::endl;
std::cout << std::endl;
}
  TRACE(TR_LOG,"BernFeb::start() called");

  RunNumber_ = run_number();
  current_subrun_ = 0;

  for(auto & buf : FEBBuffers_)
    buf.second.Init();
  
  ConfigureStart();
  GetData_thread_->start();

  TRACE(TR_LOG,"BernFeb::start() completed");
}

/*-----------------------------------------------------------------------*/


void sbndaq::BernCRTZMQ_GeneratorBase::stop() {

if(be_verbose_section){
std::cout << std::endl;
std::cout << "---------------------------" << std::endl;
std::cout << "SECTION 3 - STOP" << "\n";
std::cout << "---------------------------" << std::endl;
std::cout << std::endl;
}
  TRACE(TR_LOG,"BernFeb::stop() called");
  GetData_thread_->stop();
  ConfigureStop();
  TRACE(TR_LOG,"BernFeb::stop() completed");
}

/*-----------------------------------------------------------------------*/


void sbndaq::BernCRTZMQ_GeneratorBase::stopNoMutex() {

if(be_verbose_section){
std::cout << std::endl;
std::cout << "---------------------------" << std::endl;
std::cout << "SECTION 4 - stopNoMutex" << "\n";
std::cout << "---------------------------" << std::endl;
std::cout << std::endl;
}

  TRACE(TR_LOG,"BernFeb::stopNoMutex() called");
  GetData_thread_->stop();
  ConfigureStop();
  TRACE(TR_LOG,"BernFeb::stopNoMutex() completed");
}

/*-----------------------------------------------------------------------*/


void sbndaq::BernCRTZMQ_GeneratorBase::Cleanup(){

if(be_verbose_section){
std::cout << std::endl;
std::cout << "---------------------------" << std::endl;
std::cout << "SECTION 5 - CLEANUP" << "\n";
std::cout << "---------------------------" << std::endl;
std::cout << std::endl;
}
  TRACE(TR_LOG,"BernFeb::Cleanup() called");
  TRACE(TR_LOG,"BernFeb::Cleanup() completed");
}

/*-----------------------------------------------------------------------*/


sbndaq::BernCRTZMQ_GeneratorBase::~BernCRTZMQ_GeneratorBase(){

if(be_verbose_section){
std::cout << std::endl;
std::cout << "---------------------------" << std::endl;
std::cout << "SECTION 6 - BernCRTZMQ_GeneratorBase" << "\n";
std::cout << "---------------------------" << std::endl;
std::cout << std::endl;
}
  TRACE(TR_LOG,"BernFeb destructor called");  
  Cleanup();
  TRACE(TR_LOG,"BernFeb destructor completed");  
}

/*-----------------------------------------------------------------------*/


std::string sbndaq::BernCRTZMQ_GeneratorBase::GetFEBIDString(uint64_t const& id) const{

if(be_verbose_section){
std::cout << std::endl;
std::cout << "---------------------------" << std::endl;
std::cout << "SECTION 7 - GetFEBIDString" << "\n";
std::cout << "---------------------------" << std::endl;
std::cout << std::endl;
}
  std::stringstream ss_id;
  ss_id << "0x" << std::hex << std::setw(12) << std::setfill('0') << (id & 0xffffffffffff);
  //std::cout << "ss_id.str: " << ss_id.str() << std::endl;
  return ss_id.str();
}

/*-----------------------------------------------------------------------*/


void sbndaq::BernCRTZMQ_GeneratorBase::UpdateBufferOccupancyMetrics(uint64_t const& id,
								    size_t const& ) const { //buffer_size) const {
if(be_verbose_section){
std::cout << std::endl;
std::cout << "---------------------------" << std::endl;
std::cout << "SECTION 8 - UpdateBufferOccupancyMetrics" << "\n";
std::cout << "---------------------------" << std::endl;
std::cout << std::endl;
}

  std::string id_str = GetFEBIDString(id);
  //metricMan->sendMetric("BufferOccupancy_"+id_str,buffer_size,"events",5,true,"BernCRTZMQGenerator");    
  //metricMan->sendMetric("BufferOccupancyPercent_"+id_str,
  //			 ((float)(buffer_size) / (float)(FEBBufferCapacity_))*100.,
  //			 "%",5,true,"BernCRTZMQGenerator");    
}

/*-----------------------------------------------------------------------*/


size_t sbndaq::BernCRTZMQ_GeneratorBase::InsertIntoFEBBuffer(FEBBuffer_t & b,
							      size_t begin_index,
							      size_t nevents,
							      size_t total_events){
if(be_verbose_section){
std::cout << std::endl;
std::cout << "---------------------------" << std::endl;
std::cout << "SECTION 9 - InsertIntoFEBBuffer" << "\n";
std::cout << "---------------------------" << std::endl;
std::cout << std::endl;
}

	std::cout << "\n---InsertIntoFEBBuffer---\n";

  std::cout << "Calling insert into buffer on FEB " << (b.id & 0xff) << " (adding " << nevents << " events" << " to " << b.buffer.size() << " events into buffer" <<")" << std::endl;

  
  TRACE(TR_DEBUG,"FEB ID 0x%lx. Current buffer size %lu / %lu. Want to add %lu events.",
	b.id,b.buffer.size(),b.buffer.capacity(),nevents);

  timeval timenow; gettimeofday(&timenow,NULL); 

  //don't fill while we wait for available capacity...
  std::cout << "buffer capacity: " << b.buffer.capacity() << std::endl;
  std::cout << "buffer size: " << b.buffer.size() << std::endl;

  while( (b.buffer.capacity()-b.buffer.size()) < nevents){ usleep(10); }

  //obtain the lock
  std::unique_lock<std::mutex> lock(*(b.mutexptr));

  //ok, now, we need to do a few things:
  //(1) loop through our new events, and see if things are in order (time1)
  //(2) if they aren't, check if it's a reference or overflow condition
  //(3) if not, we're gonna stop right there and drop the remaining events.
  //(4) insert good events onto the list

  TRACE(TR_DEBUG,"FEB ID 0x%lx. Current buffer size %lu with times [%u,%u].",b.id,b.buffer.size(),
	b.buffer.front().Time_TS0(),
	b.buffer.back().Time_TS0());
  TRACE(TR_DEBUG,"Want to add %lu events with times [%u,%u].",nevents,
	ZMQBufferUPtr[begin_index].Time_TS0(),
	ZMQBufferUPtr[begin_index+nevents-1].Time_TS0());
	
  TRACE(TR_DEBUG,"Before sort, here's contents of buffer:");
  TRACE(TR_DEBUG,"============================================");
  for(size_t i_e=0; i_e<b.buffer.size(); ++i_e)
    TRACE(TR_DEBUG,"\t\t %lu : %u %d",i_e,b.buffer.at(i_e).Time_TS0(),b.buffer.at(i_e).IsReference_TS0());
  TRACE(TR_DEBUG,"--------------------------------------------");
  for(size_t i_e=begin_index; i_e<begin_index+nevents; ++i_e)
    TRACE(TR_DEBUG,"\t\t %lu : %u %d",i_e,ZMQBufferUPtr[i_e].Time_TS0(),ZMQBufferUPtr[i_e].IsReference_TS0());
  TRACE(TR_DEBUG,"============================================");

  BernCRTZMQEvent* last_event_ptr = &(ZMQBufferUPtr[begin_index]);
  BernCRTZMQEvent* this_event_ptr = last_event_ptr;

  size_t good_events=1;
  while(good_events<nevents){
    this_event_ptr = &(ZMQBufferUPtr[good_events+begin_index]);

    //if times not in order ...
    if(this_event_ptr->Time_TS0() <= last_event_ptr->Time_TS0())
      {

	// ... and the current is not an overflow or prev is not reference
	// then we need to break out of this.
	if( !(last_event_ptr->IsReference_TS0() || this_event_ptr->IsOverflow_TS0()) )
	  break;
      }

    //if time difference is too large
    else
      if( (this_event_ptr->Time_TS0()-last_event_ptr->Time_TS0())>b.max_time_diff )
	break;
    

    last_event_ptr = this_event_ptr;
    ++good_events;
  }

  //note, the order here is important. the buffer with events needs to be last, as that's
  //what is used later for the filling process to determing number of events. 
  //determining number of events is an unlocked procedure

  timeb time_poll_finished = *((timeb*)((char*)(ZMQBufferUPtr[total_events-1].adc)+sizeof(int)+sizeof(struct timeb)));

  TRACE(TR_DEBUG,"Last event looks like \n %s",ZMQBufferUPtr[total_events-1].c_str());
  TRACE(TR_DEBUG,"Time is %ld, %hu",time_poll_finished.time,time_poll_finished.millitm);

  timenow.tv_sec = time_poll_finished.time;
  timenow.tv_usec = time_poll_finished.millitm*1000;

  if(b.last_timenow.tv_sec==0){
    timeb time_poll_started = *((timeb*)((char*)(ZMQBufferUPtr[total_events-1].adc)+sizeof(int)));
    b.last_timenow.tv_sec = time_poll_started.time;
    b.last_timenow.tv_usec = time_poll_started.millitm*1000;
  }

  b.timebuffer.insert(b.timebuffer.end(),good_events,std::make_pair(b.last_timenow,timenow));
  b.droppedbuffer.insert(b.droppedbuffer.end(),good_events-1,0);
  b.droppedbuffer.insert(b.droppedbuffer.end(),1,nevents-good_events);
  b.buffer.insert(b.buffer.end(),&(ZMQBufferUPtr[begin_index]),&(ZMQBufferUPtr[good_events+begin_index]));
  
  b.last_timenow = timenow;

  TRACE(TR_DEBUG,"After insert, here's contents of buffer:");
  TRACE(TR_DEBUG,"============================================");
  for(size_t i_e=0; i_e<b.buffer.size(); ++i_e)
    TRACE(TR_DEBUG,"\t\t %lu : %u",i_e,b.buffer.at(i_e).Time_TS0());
  if(good_events!=nevents)
    TRACE(TR_DEBUG,"\tWE DROPPED %lu EVENTS.",nevents-good_events);
  TRACE(TR_DEBUG,"============================================");
  
  std::cout << "Inserted into buffer on FEB " << (b.id & 0xff) << " " << good_events << " events." << std::endl;

  return b.buffer.size();
}

/*-----------------------------------------------------------------------*/


size_t sbndaq::BernCRTZMQ_GeneratorBase::EraseFromFEBBuffer(FEBBuffer_t & b,
							     size_t const& nevents){

if(be_verbose_section){
std::cout << std::endl;
std::cout << "---------------------------" << std::endl;
std::cout << "SECTION 10 - EraseFromFEBBuffer" << "\n";
std::cout << "---------------------------" << std::endl;
std::cout << std::endl;
}
   
	std::cout << "\n---EraseFromFEBBuffer---\n" ;

  std::cout << "nevents " << nevents << std::endl; 
   std::cout << "buffer size before: " << b.buffer.size() << std::endl;

  std::unique_lock<std::mutex> lock(*(b.mutexptr));
  b.droppedbuffer.erase_begin(nevents); //std::cout << b.droppedbuffer.erase_begin(nevents) << std::endl;
  b.timebuffer.erase_begin(nevents);    //std::cout << b.timebuffer.erase_begin(nevents) << std::endl;
  b.correctedtimebuffer.erase_begin(nevents); // std::cout << b.correctedtimebuffer.erase_begin(nevents) << std::endl;
  b.buffer.erase_begin(nevents); //std::cout << b.buffer.erase_begin(nevents) << std::endl;
 std::cout << "buffer size after: " << b.buffer.size() << std::endl;
  return b.buffer.size();
}

/*-----------------------------------------------------------------------*/

bool sbndaq::BernCRTZMQ_GeneratorBase::GetData()
{

bool be_verbose = false;

if(be_verbose_section){
std::cout << std::endl;
std::cout << "---------------------------" << std::endl;
std::cout << "SECTION 11 - GETDATA" << "\n";
std::cout << "---------------------------" << std::endl;
std::cout << std::endl;
}

  TRACE(TR_GD_LOG,"BernFeb::GetData() called");

  if( GetDataSetup()!=1 ) return false;;

  
  //this fills the data from the ZMQ buffer
  size_t total_events = GetZMQData()/sizeof(BernCRTZMQEvent);


  TRACE(TR_GD_DEBUG,"\tBernCRTZMQ::GetData() got %lu total events",total_events);

  if(total_events>0){
    
    size_t i_e=0;
    size_t this_n_events=0;
    uint64_t prev_mac = (FEBIDs_[0] & 0xffffffffffffff00) + ZMQBufferUPtr[0].MAC5();
    size_t new_buffer_size = 0;

    TRACE(TR_GD_DEBUG,"\tBernCRTZMQ::GetData() start sorting with mac=0x%lx",prev_mac);

    while(i_e<total_events){
      
      auto const& this_event = ZMQBufferUPtr[i_e];

      if(be_verbose){
		//std::cout << "\n\t\ttotal_events " << total_events << std::endl;
		//std::cout << "\n\t\t----tote------ " << this_event << std::endl;
		std::cout << "\n\t\tthis_event " << this_event << std::endl;
		//std::cout << "\n\t\t----thise------ " << this_event << std::endl;
		std::cout << "\t\tIsOverflow_TS0() " << this_event.IsOverflow_TS0() << std::endl;
		//std::cout << "\n\t\t----OF TS0------ " << this_event << std::endl;
		std::cout << "\t\tIsOverflow_TS1() " << this_event.IsOverflow_TS1() << std::endl;
		//std::cout << "\n\t\t----OF TS1------ " << this_event << std::endl;
		std::cout << "\t\tIsReference_TS0() " << this_event.IsReference_TS0() << std::endl;
		//std::cout << "\n\t\t----REFF TS0------ " << this_event << std::endl;
		std::cout << "\t\tIsReference_TS1() " << this_event.IsReference_TS1() << std::endl;
		//std::cout << "\n\t\t----REFF TS0------ " << this_event << std::endl;
		//std::cout << "\t\tTS0() " << this_event.Time_TS0() << std::endl;
		//std::cout << "\n\t\t----TS0------ " << this_event << std::endl;
		//std::cout << "\t\tTS1() " << this_event.Time_TS1() << std::endl;
		//std::cout << "\n\t\t----TS1------ " << this_event << std::endl;
      }


      if((prev_mac&0xff)!=this_event.MAC5()){

	TRACE(TR_GD_DEBUG,"\tBernCRTZMQ::GetData() found new MAC (0x%x)! prev_mac=0x%lx, iterator=%lu this_events=%lu",
	      this_event.MAC5(),(prev_mac&0xff),i_e,this_n_events);

	new_buffer_size = InsertIntoFEBBuffer(FEBBuffers_[prev_mac],i_e-this_n_events,this_n_events,total_events);

	TRACE(TR_GD_DEBUG,"\tBernCRTZMQ::GetData() ... id=0x%lx, n_events=%lu, buffer_size=%lu",
	      FEBBuffers_[prev_mac].id,this_n_events,FEBBuffers_[prev_mac].buffer.size());

	auto id_str = GetFEBIDString(prev_mac);	
	//metricMan->sendMetric("EventsAdded_"+id_str,this_n_events,"events",5,true,"BernCRTZMQGenerator");
	UpdateBufferOccupancyMetrics(prev_mac,new_buffer_size);

	this_n_events=0;
      }

      prev_mac = (prev_mac & 0xffffffffffffff00) + this_event.MAC5();
      ++i_e; ++this_n_events;
    }

  }

  //metricMan->sendMetric("TotalEventsAdded",total_events-1,"events",5,true,"BernCRTZMQGenerator");

  if(total_events>0) return true;
  return false;
}

/*-----------------------------------------------------------------------*/


bool sbndaq::BernCRTZMQ_GeneratorBase::FillFragment(uint64_t const& feb_id,
						    artdaq::FragmentPtrs & frags)
{


if(be_verbose_section){
std::cout << std::endl;
std::cout << "---------------------------" << std::endl;
std::cout << "SECTION 12 - FILL FRAGMENT" << "\n";
std::cout << "---------------------------" << std::endl;
std::cout << std::endl;
}
 
 TRACE(TR_FF_LOG,"BernCRTZMQ::FillFragment(id=0x%lx,frags) called",feb_id);

 std::cout << "\n STARTING SIZE OF FRAGS IN FILLFRAGMENT IS " << frags.size() << std::endl;

  auto & feb = (FEBBuffers_[feb_id]);
  
  /*if(!clear_buffer && feb.buffer.size()<3) {
    TRACE(TR_FF_LOG,"BernCRTZMQ::FillFragment() completed, buffer (mostly) empty.");
    return false;
  }*/

  std::string id_str = GetFEBIDString(feb_id);

  size_t buffer_end = feb.buffer.size(); 
  //std::cout << "\nsize_of_buffer_end " << sizeof buffer_end << std::endl;
  std::cout << "\nbuffer_end " << buffer_end << std::endl;
  if(metricMan != nullptr) metricMan->sendMetric("feb_buffer_size", buffer_end, "CRT hits", 5, artdaq::MetricMode::Average);

  TRACE(TR_FF_LOG,"BernCRTZMQ::FillFragment() : Fragment Searching. Total events in buffer=%lu.",
	buffer_end);

  int time_correction; 

  //find GPS pulse and count wraparounds
  size_t i_gps=buffer_end; std::cout << "i_gps " << i_gps << std::endl; 
  size_t n_wraparounds=0;  //std::cout << "n_wraparounds " << n_wraparounds << std::endl;
  uint32_t last_time =0;   std::cout << "last_time " << last_time << std::endl;
  size_t i_e;

  //loop over all the CRTHit events in our buffer (for this FEB)
  for(i_e=0; i_e<buffer_end; ++i_e){

    //get this event!
    auto const& this_event = feb.buffer[i_e];
    std::cout << this_event << std::endl; //the same as in GETDATA section
    
    //if reference pulse, let's make a metric!
    if(this_event.IsReference_TS0()){
      TRACE(TR_FF_LOG,"BernCRTZMQ::FillFragment() Found reference pulse at i_e=%lu, time=%u",
	    i_e,this_event.Time_TS0());
      if(metricMan != nullptr) metricMan->sendMetric("GPS_Ref_Time", (long int)this_event.Time_TS0() - 1e9, "ns", 5, artdaq::MetricMode::LastPoint);
      //i_gps=i_e;
    }

   
     //std::cout << feb.buffer[i_e+1] << std::endl; // in principle should be empty

    /*    
    BernCRTZMQFragmentMetadata metadata(frag_begin_time_s,frag_begin_time_ns,
					frag_end_time_s,frag_end_time_ns,
					time_correction,time_offset,
					RunNumber_,
					seq_id,
					feb_id, ReaderID_,
					nChannels_,nADCBits_);
    */
    //create metadata!
    BernCRTZMQFragmentMetadata metadata(0,0,
					0,0,
					0,0,
					RunNumber_,
					FragmentCounter_++,
					feb_id, ReaderID_,
					nChannels_,nADCBits_);
    //increment n_events in metadata by 1 (to tell it we have one CRT Hit event!)
    metadata.inc_Events();
    std::cout << "\ncounter metadata " << metadata.sequence_number() << std::endl;

    //create our new fragment on the end of the frags vector
    frags.emplace_back( artdaq::Fragment::FragmentBytes(sizeof(BernCRTZMQEvent),  
							metadata.sequence_number(),feb_id,
							sbndaq::detail::FragmentType::BERNCRTZMQ, metadata, 
							0 //timestamp! to be filled!!!!
							) );

    //copy the BernCRTZMQEvent into the fragment we just created
    std::cout << "\n--copy the event into the fragment--\n";
    std::copy(feb.buffer.begin()+i_e,feb.buffer.begin()+i_e+1,(BernCRTZMQEvent*)(frags.back()->dataBegin()));

  }

  //delete from the buffer all the events we've just put into frags
  TRACE(TR_FF_DEBUG,"BernCRTZMQ::FillFragment() : Buffer size before erase = %lu",feb.buffer.size());
  size_t new_buffer_size = EraseFromFEBBuffer(feb,buffer_end);
  TRACE(TR_FF_DEBUG,"BernCRTZMQ::FillFragment() : Buffer size after erase = %lu",feb.buffer.size());
  
  //update 
  metricMan->sendMetric("FragmentsBuilt_"+id_str,buffer_end,"events/s",5,artdaq::MetricMode::Rate);
  UpdateBufferOccupancyMetrics(feb_id,new_buffer_size);

 std::cout << "ENDING SIZE OF FRAGS IN FILLFRAGMENT IS " << frags.size() << std::endl;

  return false;

  /*

    if(this_event.IsReference_TS0()){
      TRACE(TR_FF_LOG,"BernCRTZMQ::FillFragment() Found reference pulse at i_e=%lu, time=%u",
	    i_e,this_event.Time_TS0());
      if(metricMan != nullptr) metricMan->sendMetric("GPS_Ref_Time", (long int)this_event.Time_TS0() - 1e9, "ns", 5, artdaq::MetricMode::LastPoint);
      i_gps=i_e;
    }
    else if(this_event.Time_TS0()<last_time && this_event.IsOverflow_TS0()){
      n_wraparounds++;
      TRACE(TR_FF_LOG,"BernCRTZMQ::FillFragment() Found wraparound pulse at i_e=%lu, time=%u (last_time=%u), n=%lu",
	    i_e,this_event.Time_TS0(),last_time,n_wraparounds);
    }
    feb.correctedtimebuffer[i_e] = (this_event.Time_TS0()+(uint64_t)n_wraparounds*0x40000000);
    last_time = this_event.Time_TS0();
   
    std::cout << "last_time " << last_time << std::endl;


    if(i_gps!=buffer_end) break;
  }

  if(i_gps==buffer_end){
    TRACE(TR_FF_LOG,"BernCRTZMQ::FillFragment() completed, buffer not empty, but waiting for GPS pulse.");
    return false;
  }
    
  //determine how much time passed. should be close to corrected time.
  //then determine the correction.
  uint32_t elapsed_secs = (feb.correctedtimebuffer[i_gps]/1e9);
  if(feb.correctedtimebuffer[i_gps]%1000000000 > 500000000)
    elapsed_secs+=1;
  time_correction = (int)(elapsed_secs*1000000000) - (int)(feb.correctedtimebuffer[i_gps]);
  
  TRACE(TR_FF_LOG,"BernCRTZMQ::FillFragment() : time correction is %d, with elapsed_sec=%u and corrected_time=%lu",
	time_correction,elapsed_secs,feb.correctedtimebuffer[i_gps]);

  //ok, so now, determine the nearest second for the last event (closest to one second), based on ntp time
  //get the usecs from the timeval
  auto gps_timeval_pair = feb.timebuffer.at(i_gps);
  
  TRACE(TR_FF_LOG,"BernCRTZMQ::FillFragment() : GPS time was between (%ld sec, %ld usec) and (%ld sec, %ld usec)",
	gps_timeval_pair.first.tv_sec,gps_timeval_pair.first.tv_usec,
	gps_timeval_pair.second.tv_sec,gps_timeval_pair.second.tv_usec);

  //report remainder as a metric to watch for
 // std::string id_str = GetFEBIDString(feb_id);	REMEMBER TO UNCOMMENT

  ////metricMan->sendMetric("BoundaryTimeRemainder_"+id_str,(float)(gps_timeval.tv_usec),"microseconds",false,"BernCRTZMQGenerator");
  
  //round the boundary time to the nearest second.
  //ROUND DOWN!!
  //if(gps_timeval.tv_usec > 5e5)
  //gps_timeval.tv_sec+=1;
  //gps_timeval.tv_usec = 0;

  uint32_t first_time_sec = gps_timeval_pair.first.tv_sec;

  if( (gps_timeval_pair.second.tv_sec-gps_timeval_pair.first.tv_sec)==(elapsed_secs-1) )
    first_time_sec = first_time_sec -1;

  uint32_t frag_begin_time_s = first_time_sec;
  uint32_t frag_begin_time_ns = 0;
  uint32_t frag_end_time_s = first_time_sec;
  uint32_t frag_end_time_ns = SequenceTimeWindowSize_;
  
  if(SeqIDMinimumSec_==0){
    SeqIDMinimumSec_ = frag_begin_time_s - 10;
  }

  i_e=0;
  last_time=0;
  uint64_t time_offset=0;
  size_t i_b=0;
  while(frag_end_time_s < gps_timeval_pair.second.tv_sec){

    if(frag_end_time_ns>=1000000000){
      frag_end_time_ns = frag_end_time_ns%1000000000;
      frag_end_time_s+=1;
    }

    if(frag_begin_time_ns>=1000000000){
      frag_begin_time_ns = frag_begin_time_ns%1000000000;
      frag_begin_time_s+=1;
    }

    //ms since Jan 1 2015
    uint32_t seq_id = (frag_begin_time_s-SeqIDMinimumSec_)*1000 + (frag_begin_time_ns/1000000);

    //make metadata object
	std::cout << std::endl << "METADETA:" << std::endl;
	std::cout << "frag_begin_time_s " << frag_begin_time_s << std::endl;
     	std::cout << "frag_begin_time_ns " << frag_begin_time_ns << std::endl;
	std::cout << "frag_end_time_s " << frag_end_time_s << std::endl;
	std::cout << "frag_end_time_ns " << frag_end_time_ns << std::endl;
	std::cout << "time_correction " << time_correction << std::endl;
	std::cout << "time_offset " << time_offset << std::endl;
	std::cout << "RunNumber_ " << RunNumber_ << std::endl;
	std::cout << "seq_id " << seq_id << std::endl;
	std::cout << "feb_id " << feb_id << std::endl;
	std::cout << "ReaderID_ " << ReaderID_ << std::endl;
	std::cout << "nChannels_ " << time_offset << std::endl;
	std::cout << "nADCBits_ " << time_offset << std::endl;

    BernCRTZMQFragmentMetadata metadata(frag_begin_time_s,frag_begin_time_ns,
				     frag_end_time_s,frag_end_time_ns,
				     time_correction,time_offset,
				     RunNumber_,
				     seq_id,
				     feb_id, ReaderID_,
				     nChannels_,nADCBits_);

    double time_corr_factor = 1.0e9 / (1000000000 - time_correction);
    
    TRACE(TR_FF_LOG,"BernCRTZMQ::FillFragment() : looking for events in frag %u,%u (%u ms)",
	  frag_begin_time_s,frag_begin_time_ns,seq_id);
    
    while(i_e<i_gps+1){
      
      auto const& this_corrected_time = feb.correctedtimebuffer[i_e];
      auto const& this_event = feb.buffer[i_e];
      
      TRACE(TR_FF_LOG,"BernCRTZMQ::FillFragment() : event %lu, time=%u, corrected_time=%lf",
	    i_e,this_event.Time_TS0(),(double)this_corrected_time*time_corr_factor);
      
      if( (double)this_corrected_time*time_corr_factor > (frag_end_time_s-first_time_sec)*1e9+frag_end_time_ns )
	break;


      if(this_event.Time_TS0()<last_time)
	time_offset += 0x40000000;

      
      if(this_event.lostcpu > feb.overwritten_counter)
	metadata.increment(this_event.lostfpga,this_event.lostcpu-feb.overwritten_counter,feb.droppedbuffer[i_e]);
      else
	metadata.increment(this_event.lostfpga,0,feb.droppedbuffer[i_e]);
      feb.overwritten_counter = this_event.lostcpu;
      
      //metadata.increment(this_event.lostfpga,this_event.lostcpu,feb.droppedbuffer[i_e]);

      ++i_e;
    }
    
    //great, now add the fragment on the end.`<
    frags.emplace_back( artdaq::Fragment::FragmentBytes(metadata.n_events()*sizeof(BernCRTZMQEvent),  
							metadata.sequence_number(),feb_id,
							sbndaq::detail::FragmentType::BERNCRTZMQ, metadata) );
    std::copy(feb.buffer.begin()+i_b,feb.buffer.begin()+i_e,(BernCRTZMQEvent*)(frags.back()->dataBegin()));

	//for (auto i = frags.begin(); i != frags.end(); ++i)
    //std::cout << i << ' ';
	//std::cout << "fragment back " << frags.back()->dataBegin() << std::endl;

    std::cout << std::endl << "frags.size " << frags.size() << std::endl << std::endl;
	

    TRACE(TR_FF_DEBUG,"BernCRTZMQ::FillFragment() : Fragment created. Type %d. First event in fragment: %s",
	  sbndaq::detail::FragmentType::BERNCRTZMQ,
	  ((BernCRTZMQEvent*)(frags.back()->dataBegin()))->c_str() );
    
    TRACE(TR_FF_LOG,"BernCRTZMQ::FillFragment() : Fragment created. Events=%u. Metadata : %s",
	  metadata.n_events(),metadata.c_str());
        
    SendMetadataMetrics(metadata);


    i_b=i_e; //new beginning...
    frag_begin_time_ns += SequenceTimeWindowSize_;
    frag_end_time_ns += SequenceTimeWindowSize_;
  }

  TRACE(TR_FF_DEBUG,"BernCRTZMQ::FillFragment() : Buffer size before erase = %lu",feb.buffer.size());
  size_t new_buffer_size = EraseFromFEBBuffer(feb,i_gps+1);
  TRACE(TR_FF_DEBUG,"BernCRTZMQ::FillFragment() : Buffer size after erase = %lu",feb.buffer.size());

  //metricMan->sendMetric("FragmentsBuilt_"+id_str,1.0,"events",5,true,"BernCRTZMQGenerator");
  UpdateBufferOccupancyMetrics(feb_id,new_buffer_size);

  //return true;

  return false;
  */

}

/*-----------------------------------------------------------------------*/


void sbndaq::BernCRTZMQ_GeneratorBase::SendMetadataMetrics(BernCRTZMQFragmentMetadata const& m) {

if(be_verbose_section){
std::cout << std::endl;
std::cout << "---------------------------" << std::endl;
std::cout << "SECTION 13 - SendMetadataMetrics" << "\n";
std::cout << "---------------------------" << std::endl;
std::cout << std::endl;
}

  std::string id_str = GetFEBIDString(m.feb_id());
  //metricMan->sendMetric("FragmentLastTime_"+id_str,(uint64_t)(m.time_end_seconds()*1000000000+m.time_end_nanosec()),"ns",5,false,"BernCRTZMQGenerator");
  //metricMan->sendMetric("EventsInFragment_"+id_str,(float)(m.n_events()),"events",5,true,"BernCRTZMQGenerator");
  //metricMan->sendMetric("MissedEvents_"+id_str,     (float)(m.missed_events()),     "events",5,true,"BernCRTZMQGenerator");
  //metricMan->sendMetric("OverwrittenEvents_"+id_str,(float)(m.overwritten_events()),"events",5,true,"BernCRTZMQGenerator");
  //float eff=1.0;
  //if((m.n_events()+m.missed_events()+m.overwritten_events())!=0)
  //eff = (float)(m.n_events()) / (float)(m.n_events()+m.missed_events()+m.overwritten_events());
  
  //metricMan->sendMetric("Efficiency_"+id_str,eff*100.,"%",5,true,"BernCRTZMQGenerator");
}

/*-----------------------------------------------------------------------*/


bool sbndaq::BernCRTZMQ_GeneratorBase::getNext_(artdaq::FragmentPtrs & frags) {

if(be_verbose_section){
std::cout << std::endl;
std::cout << "---------------------------" << std::endl;
std::cout << "SECTION 14 - getNext" << "\n";
std::cout << "---------------------------" << std::endl;
std::cout << std::endl;
}

  std::cout << "Calling getNext_" << std::endl;

  TRACE(TR_FF_LOG,"BernCRTZMQ::getNext_(frags) called");

  std::cout << std::endl << "STARTING SIZE OF frags.size IN GETNEXT_ IS " << frags.size() << std::endl << std::endl;
  
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
  
  for(auto const& id : FEBIDs_){
    while(true){
      if(!FillFragment(id,frags)) break;
    }
  }

  TRACE(TR_FF_LOG,"BernCRTZMQ::getNext_(frags) completed");

  std::cout << std::endl << "ENDING SIZE OF frags.size IN GETNEXT_ IS " << frags.size() << std::endl << std::endl;
  std::cout << "---PRINT OUT FRAGMENT IN GETNEXT_---" << std::endl;

  if(frags.size()!=0){
    BernCRTZMQFragment bb(*frags.back());
    std::cout << *(bb.eventdata(0)) << std::endl;
  }


  if(frags.size()!=0) TRACE(TR_FF_DEBUG,"BernCRTZMQ::geNext_() : last fragment is: %s",(BernCRTZMQFragment(*frags.back())).c_str());

  return true;

}

/*-----------------------------------------------------------------------*/

// The following macro is defined in artdaq's GeneratorMacros.hh header
//DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(demo::BernCRTZMQ) 
