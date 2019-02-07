//
// sbnddaq-readout/Generators/SpectraTime_generatorBase.cc (D.Cianci,W.Badgett)
//
// Read from a message queue GPS status messages, buffer them and send them out on request to the EVBs
//

#include "sbndaq-artdaq/Generators/Common/SpectraTime_generatorBase.hh"

#include <fstream>
#include <iomanip>
#include <iterator>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

sbndaq::SpectraTime_generatorBase::SpectraTime_generatorBase(fhicl::ParameterSet const & ps):
  CommandableFragmentGenerator(ps),
  ps_(ps)
{
  init();
}

sbndaq::SpectraTime_generatorBase::~SpectraTime_generatorBase()
{
  stopAll();
}

void sbndaq::SpectraTime_generatorBase::init()
{
  SequenceTimeWindowSize_ = ps_.get<uint32_t>("SequenceTimeWindowSize",1e6); //one ms
  eventCounter = 0;

  // Connect to GPS message queue
  daqMQ = (key_t)GPS_MQ;
  daqID = msgget(daqMQ,0);
  if ( daqID < 0 )
  {
    LOG_ERROR("SpectraTime_generator") << "Error on msgget 0x" << std::hex << daqMQ << std::dec 
				       << " " << strerror(errno);

  }
}

void sbndaq::SpectraTime_generatorBase::start()
{
	// Initialize our buffer
	SpectraTimeBuffer_t STBuffer_ = SpectraTimeBuffer_t();

	// Magically start worker getdata thread.
        share::ThreadFunctor functor = std::bind(&SpectraTime_generatorBase::getData,this);
 	auto worker_functor = share::WorkerThreadFunctorUPtr(new share::WorkerThreadFunctor(functor,"GetDataWorkerThread"));
 	auto getData_worker = share::WorkerThread::createWorkerThread(worker_functor);
 	GetData_thread_.swap(getData_worker);

 	GetData_thread_->start();
	running = true;
}

void sbndaq::SpectraTime_generatorBase::stopAll()
{
  if ( running )
  {
    running = false;
    GetData_thread_->stop();
  }
}

void sbndaq::SpectraTime_generatorBase::stop()
{
  stopAll();
}

void sbndaq::SpectraTime_generatorBase::stopNoMutex()
{
  stopAll();
}

bool sbndaq::SpectraTime_generatorBase::getData()
{
  ssize_t nBytes = 1;

  while ( nBytes > 0 )
  {
    nBytes = msgrcv(daqID,(void *)&gpsInfo,sizeof(gpsInfo), GPSInfo::GPS_INFO_MTYPE,0);
    if ( nBytes <= 0 )
    {
      LOG_ERROR("SpectraTime_generator") << "Error on msgrcv 0x" << std::hex << daqMQ << std::dec << " " << 
	strerror(errno);
    }
    else
    {
      SpectraTimeCircularBuffer_.buffer.push_front(SpectraTimeTemp);
    }
  }
  return(true);
}

bool sbndaq::SpectraTime_generatorBase::getNext_(artdaq::FragmentPtrs & frags)
{
	//Send our buffer over to fillfrag. If there's nothing there, we'll try again later.
  eventCounter++;
  FillFragment(frags);
  usleep(100);
  return true;
}

bool sbndaq::SpectraTime_generatorBase::FillFragment(artdaq::FragmentPtrs &frags, bool)
{
	// initialize SpectratimeFragmentMetadata
	auto & spectratime = SpectraTimeCircularBuffer_;

	//What the hell is going on here?
	//int64_t time = 0;
	//	size_t  local_time_resets = 0;
	//	int32_t local_last_time = spectratime.last_time_counter;
	//	bool found_fragment=false;

	//	auto it_start_fragment = spectratime.buffer.cbegin();
	//auto it_end_fragment= spectratime.buffer.cend();

	// I'm not sure how time ranging works, so comment out for now (wb)
	//This is to make sure that we've got something in the buffer and the times line up right (with something)
	// for(auto it=it_start_fragment; it!=it_end_fragment;++it){

	// 	// if((int64_t)it->time1.Time() <= local_last_time)
	// 	// ++local_time_resets;
	// 	// local_last_time = it->time1.Time();

	// 	// time = it->time1.Time()+(spectratime.time_resets+local_time_resets)*1e9;
	// 	// figure out ev_time, resets, local resets and time

	// 	if(time<spectratime.next_time_start)
	// 		LOG_ERROR("SpectraTime") << "Time out of place somehow." << std::endl;
	// 	else if(time>spectratime.next_time_start+SequenceTimeWindowSize_){
	// 		LOG_ERROR("SpectraTime") << "Found new time! Not sure what that means." << std::endl;
	// 		it_end_fragment = it;
	// 		found_fragment = true;
	// 		break;
	// 	}
	// }

	// //didn't get our last event in this time window ... return
	// if(!clear_buffer && !found_fragment){
	// 	LOG_ERROR("SpectraTime") << "Buffer's not empty, but we're waiting for more data, I guess." << std::endl;
	// 	return false;
	// }

	// //ok, queue was non-empty, and we saw our last event. Need to loop through and do proper accounting now.
	// //	local_time_resets = 0;
	// //	local_last_time = spectratime.last_time_counter;
	// for(auto it=it_start_fragment; it!=it_end_fragment; ++it){

	// 	// if((int32_t)it->time1.Time() <= spectratime.last_time_counter){
	// 	// 	++spectratime.time_resets;
	// 	// 	// Time reset if we're going back in time somehow
	// 	// }
	// 	//spectratime.last_time_counter = it->time1.Time();

	// 	//spectratime.overwritten_counter = it->flags.overwritten;
	// }
	// spectratime.next_time_start = spectratime.next_time_start+SequenceTimeWindowSize_;

	//Add the data to the fragment (I think...)  ...doesn't look quite right (wb)
	//std::copy(it_start_fragment,it_end_fragment,(SpectratimeEvent*)(frags.back()->dataBegin()));

	// Create fragments for each data buffer
	uint32_t boardId=0;
	uint32_t bytesWritten = sizeof(struct sbndaq::SpectratimeEvent);

	while ( !spectratime.buffer.empty() )
	{
	  // This is only thread safe with one producer and one consumer
	  sbndaq::SpectratimeEvent event = spectratime.buffer.front();
	  spectratime.buffer.pop_front();

	  std::unique_ptr<artdaq::Fragment> fragPtr(artdaq::Fragment::FragmentBytes(bytesWritten,  
										    eventCounter, 
										    boardId++,
										    12,
										    SpectratimeFragmentMetadata()));
	  memcpy(fragPtr->dataBeginBytes(), (char *)&event, bytesWritten);
	  frags.emplace_back(std::move(fragPtr));
	}
	LOG_INFO("SpectraTime") << "Found " << boardId << " fragments" << std::endl;

	return true;
}

