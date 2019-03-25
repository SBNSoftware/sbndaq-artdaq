#include "sbndaq-artdaq/Generators/Common/BernCRTZMQ_GeneratorBase.hh"

//#include "art/Utilities/Exception.h"
#include "cetlib_except/exception.h"
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


void sbndaq::BernCRTZMQ_GeneratorBase::Initialize(){


  TRACE(TR_LOG,"BernFeb::Initialze() called");
  
  //RunNumber_ = ps_.get<uint32_t>("RunNumber",999);
  RunNumber_ = 0;
  SubrunTimeWindowSize_ = ps_.get<uint64_t>("SubRunTimeWindowSize",60e9); //one minute
  SequenceTimeWindowSize_ = ps_.get<uint32_t>("SequenceTimeWindowSize",5e6); //5 ms
  nADCBits_  = ps_.get<uint8_t>("nADCBits",12);
  nChannels_ = ps_.get<uint32_t>("nChannels",32);
  ReaderID_ = ps_.get<uint8_t>("ReaderID",0x1);
  FEBIDs_ = ps_.get< std::vector<uint64_t> >("FEBIDs");

  //new variable added by me (see the header file)
  FragmentCounter_ = 0;
  GPSCounter_= 0;
  event_in_clock = 0;
  GPS_time = 0;


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


  TRACE(TR_LOG,"BernFeb::stop() called");
  GetData_thread_->stop();
  ConfigureStop();
  TRACE(TR_LOG,"BernFeb::stop() completed");
}

/*-----------------------------------------------------------------------*/


void sbndaq::BernCRTZMQ_GeneratorBase::stopNoMutex() {


  TRACE(TR_LOG,"BernFeb::stopNoMutex() called");
  GetData_thread_->stop();
  ConfigureStop();
  TRACE(TR_LOG,"BernFeb::stopNoMutex() completed");
}

/*-----------------------------------------------------------------------*/


void sbndaq::BernCRTZMQ_GeneratorBase::Cleanup(){


  TRACE(TR_LOG,"BernFeb::Cleanup() called");
  TRACE(TR_LOG,"BernFeb::Cleanup() completed");
}

/*-----------------------------------------------------------------------*/


sbndaq::BernCRTZMQ_GeneratorBase::~BernCRTZMQ_GeneratorBase(){


  TRACE(TR_LOG,"BernFeb destructor called");  
  Cleanup();
  TRACE(TR_LOG,"BernFeb destructor completed");  
}

/*-----------------------------------------------------------------------*/


std::string sbndaq::BernCRTZMQ_GeneratorBase::GetFEBIDString(uint64_t const& id) const{


  std::stringstream ss_id;
  ss_id << "0x" << std::hex << std::setw(12) << std::setfill('0') << (id & 0xffffffffffff);

  return ss_id.str();
}

/*-----------------------------------------------------------------------*/


void sbndaq::BernCRTZMQ_GeneratorBase::UpdateBufferOccupancyMetrics(uint64_t const& id,
								    size_t const& ) const { //buffer_size) const {

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
							      size_t /*total_events*/){


	std::cout << "\n---InsertIntoFEBBuffer---\n";

	std::cout << "Calling insert into buffer on FEB " << (b.id & 0xff) << " (adding " << nevents << " events" << " to " << 		b.buffer.size() << " events into buffer" <<")" << std::endl;

	TRACE(TR_DEBUG,"FEB ID 0x%lx. Current buffer size %lu / %lu. Want to add %lu events.",
	b.id,b.buffer.size(),b.buffer.capacity(),nevents);

	//don't fill while we wait for available capacity...
	while( (b.buffer.capacity()-b.buffer.size()) < nevents){ std::cout<< "no available capacity!"; usleep(10); }
	std::cout << "\nCheck the buffer: " << std::endl;	
	std::cout << "Buffer Capacity: " << std::setw(5) << b.buffer.capacity() << std::endl;
  	std::cout << "Buffer Size: " << std::setw(6) << b.buffer.size() <<  std::endl;
	

  	//obtain the lock
  	std::unique_lock<std::mutex> lock(*(b.mutexptr));

	
	// ------TRACE----------//
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
	// ---------------------//

	//some timestamp
	//timeval timenow; gettimeofday(&timenow,NULL); std::cout<<"timenow " << timenow << std::endl;
	//timeb time_poll_finished = *((timeb*)((char*)(ZMQBufferUPtr[total_events-1].adc)+sizeof(int)+sizeof(struct timeb)));
	//std::cout<<"time_poll_finished " << time_poll_finished << std::endl;

	size_t good_events = 1;
	while(good_events<nevents){
		++good_events;
	}


	TRACE(TR_DEBUG,"Last event looks like \n %s",ZMQBufferUPtr[nevents/*total_events-1*/].c_str());
  	//TRACE(TR_DEBUG,"Time is %ld, %hu",time_poll_finished.time,time_poll_finished.millitm);

	b.buffer.insert(b.buffer.end(),&(ZMQBufferUPtr[begin_index]),&(ZMQBufferUPtr[good_events+begin_index]));


	std::cout << "\nInserted into buffer on FEB " << (b.id & 0xff) << " " << good_events << " events." << std::endl;

	//let's print out the content of the buffer
	for(size_t i_e=0; i_e<b.buffer.size(); ++i_e){
		std::cout << i_e << ". Timestamp " << b.buffer.at(i_e).Time_TS0();
		std::cout << std::setw(10) << " --- IsRefTS0? " <<b.buffer.at(i_e).IsReference_TS0() << std::endl;
	}
	std::cout << std::endl;
  return b.buffer.size();

}

/*-----------------------------------------------------------------------*/


size_t sbndaq::BernCRTZMQ_GeneratorBase::EraseFromFEBBuffer(FEBBuffer_t & b,
							     size_t const& nevents){

   
	std::cout << "\n---EraseFromFEBBuffer---\n" ;

  	std::cout << "Buffer Size before erasing the events: " << std::setw(3) << b.buffer.size() << " events" << std::endl;

  	std::unique_lock<std::mutex> lock(*(b.mutexptr));
  
	b.buffer.erase_begin(nevents);
 	std::cout << "Buffer Size after erasing the events: " << std::setw(4) << b.buffer.size() << " events" << std::endl << std::endl;

  return b.buffer.size();
}

/*-----------------------------------------------------------------------*/

bool sbndaq::BernCRTZMQ_GeneratorBase::GetData()
{


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


 
 TRACE(TR_FF_LOG,"BernCRTZMQ::FillFragment(id=0x%lx,frags) called",feb_id);

 std::cout << "\n---FillFragment---\n";

 std::cout << "\nSTARTING SIZE OF FRAGS IN FILLFRAGMENT IS " << frags.size() << std::endl;

  auto & feb = (FEBBuffers_[feb_id]);
  
  /*if(!clear_buffer && feb.buffer.size()<3) {
    TRACE(TR_FF_LOG,"BernCRTZMQ::FillFragment() completed, buffer (mostly) empty.");
    return false;
  }*/

  std::string id_str = GetFEBIDString(feb_id);

  size_t buffer_end = feb.buffer.size(); 

  std::cout << "\nCurrent size of the Buffer: " << buffer_end << " events" << std::endl;
  if(metricMan != nullptr) metricMan->sendMetric("feb_buffer_size", buffer_end, "CRT hits", 5, artdaq::MetricMode::Average);

  TRACE(TR_FF_LOG,"BernCRTZMQ::FillFragment() : Fragment Searching. Total events in buffer=%lu.",
	buffer_end);

  int time_correction; 

  //find GPS pulse and count wraparounds
  //size_t i_gps=buffer_end;
  //size_t n_wraparounds=0;  
  //uint32_t last_time =0;  
  //size_t i_e;
	uint32_t frag_begin_time = 0;
	uint32_t frag_end_time = 0; 
	//size_t event_in_clock = 0;  
	//BernCRTZMQEvent event_before;
	//BernCRTZMQEvent event_GPS;

  //loop over all the CRTHit events in our buffer (for this FEB)
  for(size_t i_e=0; i_e<buffer_end; ++i_e){

    //get this event!
    auto const& this_event = feb.buffer[i_e];
    std::cout << this_event << std::endl; //the same as in GETDATA section
    std::cout << "TS0 of the event: " << this_event.Time_TS0() << " [ns]" <<std::endl;


    //if it is a reference pulse, let's count it!
    //start to define the beginning and ending time of the fragment as well

	if(this_event.IsReference_TS0()){
		++GPSCounter_;
		event_in_clock = 0;  //just a counter over the events within a clock of the FEB
		GPS_time += 1e9; //this_event.Time_TS0(); //time past from the very beginning
		//event_GPS = this_event;
		frag_begin_time = this_event.Time_TS0();//event_GPS.Time_TS0();
		frag_end_time = this_event.Time_TS0();//event_GPS.Time_TS0();
	} 
	else{
		++event_in_clock;
		frag_begin_time = GPS_time;//feb.buffer[i_e-event_in_clock].Time_TS0();
		frag_end_time = GPS_time + this_event.Time_TS0();//feb.buffer[i_e-event_in_clock].Time_TS0() + this_event.Time_TS0();		
	}

    
    
    //if reference pulse, let's make a metric!
    if(this_event.IsReference_TS0()){
      TRACE(TR_FF_LOG,"BernCRTZMQ::FillFragment() Found reference pulse at i_e=%lu, time=%u",
	    i_e,this_event.Time_TS0());
      if(metricMan != nullptr) metricMan->sendMetric("GPS_Ref_Time", (long int)this_event.Time_TS0() - 1e9, "ns", 5, artdaq::MetricMode::LastPoint);
      //i_gps=i_e;
    }

   
  

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
    BernCRTZMQFragmentMetadata metadata(frag_begin_time/1e9,frag_begin_time,
					frag_end_time/1e9,frag_end_time,
					0,0,
					RunNumber_,
					FragmentCounter_++,
					feb_id, ReaderID_,
					nChannels_,nADCBits_);
    //increment n_events in metadata by 1 (to tell it we have one CRT Hit event!)
    metadata.inc_Events();
    //std::cout << "\nEventCounter metadata: " << metadata.n_events() << std::endl;
    std::cout << "\nFragmentCounter metadata: " << metadata.sequence_number() << std::endl;
    std::cout << "TimeStart[ns]: " << metadata.time_start_nanosec() << std::endl;
    std::cout << "TimeEnd[ns]: " << metadata.time_end_nanosec() << std::endl;
    /*if(this_event.flags==5){
	++GPSCounter_;
	std::cout << "\nGPSCounter: " << GPSCounter_ << std::endl;
    }else{std::cout << "This is not a GPS count" << std::endl; ++GPSCounter_; }*/
    

	

    //create our new fragment on the end of the frags vector
    frags.emplace_back( artdaq::Fragment::FragmentBytes(sizeof(BernCRTZMQEvent),  
							metadata.sequence_number(),feb_id,
							sbndaq::detail::FragmentType::BERNCRTZMQ, metadata, 
							0//metadata.time_end_nanosec() //timestamp! to be filled!!!!
							) );

    //copy the BernCRTZMQEvent into the fragment we just created
    std::cout << "\n--Copy events into the fragment--\n";
    std::copy(feb.buffer.begin()+i_e,feb.buffer.begin()+i_e+1,(BernCRTZMQEvent*)(frags.back()->dataBegin()));

  }

	std::cout << "\nwe've got " << GPSCounter_ << " GPS-PPS" << std::endl;
	std::cout << "event_in_clock " << event_in_clock << std::endl;

	

  //delete from the buffer all the events we've just put into frags
  TRACE(TR_FF_DEBUG,"BernCRTZMQ::FillFragment() : Buffer size before erase = %lu",feb.buffer.size());
  size_t new_buffer_size = EraseFromFEBBuffer(feb,buffer_end);
  TRACE(TR_FF_DEBUG,"BernCRTZMQ::FillFragment() : Buffer size after erase = %lu",feb.buffer.size());
  
  //update 
  metricMan->sendMetric("FragmentsBuilt_"+id_str,buffer_end,"events/s",5,artdaq::MetricMode::Rate);
  UpdateBufferOccupancyMetrics(feb_id,new_buffer_size);

 std::cout << "ENDING SIZE OF FRAGS IN FILLFRAGMENT IS " << frags.size() << std::endl;

  return false;


}

/*-----------------------------------------------------------------------*/


void sbndaq::BernCRTZMQ_GeneratorBase::SendMetadataMetrics(BernCRTZMQFragmentMetadata const& m) {


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


  std::cout << "\n---Calling getNext_---" << std::endl;

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

  std::cout << "ENDING SIZE OF frags.size IN GETNEXT_ IS " << frags.size() << std::endl;
  //std::cout << "---PRINT OUT FRAGMENT IN GETNEXT_---" << std::endl  << std::endl;

 /* if(frags.size()!=0){
    BernCRTZMQFragment bb(*frags.back());
    std::cout << *(bb.eventdata(0)) << std::endl;
  }*/


  if(frags.size()!=0) TRACE(TR_FF_DEBUG,"BernCRTZMQ::geNext_() : last fragment is: %s",(BernCRTZMQFragment(*frags.back())).c_str());

  return true;

}

/*-----------------------------------------------------------------------*/

// The following macro is defined in artdaq's GeneratorMacros.hh header
//DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(demo::BernCRTZMQ) 
