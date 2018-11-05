//
// sbnddaq-readout/Generators/SpectraTime_generatorBase.cc (D.Cianci,W.Badgett)
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

void sbndaq::SpectraTime_generatorBase::init(){

  SequenceTimeWindowSize_ = ps_.get<uint32_t>("SequenceTimeWindowSize",1e6); //one ms
  eventCounter = 0;
  spectratimeFD = -1;

	// Okay, let's see if the gps is there!
	device = "/dev/ttyUSB0";
	if(access(device.c_str(),F_OK) != 0){
		char line[256];
		sprintf(line,"Error finding Spectratime GPS device /dev/ttyUSB0");
		LOG_ERROR("SpectraTime") << line;
		// Throw some kind of error. Not sure how quite yet.
	}

	// He's home. Now, let's see if we can turn him on.
	spectratimeFD = open(device.c_str(),O_RDWR);
	if(spectratimeFD < 0){
		char line[256];
		sprintf(line,"Error opening Spectratime GPS device /dev/ttyUSB");
		LOG_ERROR("SpectraTime") << line;
		// Throw some kind of error. Not sure how quite yet.
	}

}

void sbndaq::SpectraTime_generatorBase::start()
{
	// Initialize our buffer
	SpectraTimeBuffer_t STBuffer_ = SpectraTimeBuffer_t();

	// Let's clear spectratime buffer (by pressing enter, essentially)
	char line[2];
	sprintf(line,"\n");
	ssize_t nBytes = write(spectratimeFD,line,2);
	if( nBytes != (ssize_t)2 )
	{
	  std::cout << "Failed to get through to our boy." << std::endl;
	  LOG_ERROR("SpectraTime") << "Failed to get through to our boy.";
	}

	// Now, let's tell him to start spitting out Data char line[4];
    sprintf(line,"BTA\n");
    LOG_INFO("SpectraTime") << "Telling our boy to go with: " << line;
    nBytes = write(spectratimeFD,line,5);
    if( nBytes != (ssize_t)5 )
            LOG_ERROR("SpectraTime") << "Failed to get through to our boy." << std::endl;

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

  if ( spectratimeFD >= 0 )
  {
    // Turn off status reports
    // char line[80];
    // sprintf(line,"BT0\n");
    // LOG_ERROR("SpectraTime") << "Telling our boy to stop with: " << line << std::endl;
    // ssize_t nBytes = write(spectratimeFD,line,5);
    // if( nBytes != (ssize_t)5 )
    //   LOG_ERROR("SpectraTime") << "Failed to get through to our boy." << std::endl;
    
    close(spectratimeFD);
    spectratimeFD = -1;
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
	// Okay, cool. Now let's see what he says!
  	int retcod = 0;
  	struct timeval timeOut;
  	timeOut.tv_sec = 10;
  	timeOut.tv_usec = 0;
  	fd_set set;
  	char letter;
  	letter = '\0';
  	std::stringstream output;
	LOG_INFO("SpectraTime") << "Starting getData()" << std::endl;

  	while(letter != '\n'){
		letter = '\0';
		FD_ZERO(&set);
		FD_SET(spectratimeFD,&set);
		retcod = select(spectratimeFD+1, &set, NULL, NULL, &timeOut);
		if(retcod < 0)
			LOG_ERROR("SpectraTime") << "Failed to select device" << std::endl;
		else if(retcod == 0){
			LOG_ERROR("SpectraTime") << "Device timeout" << std::endl;
			return 1;
		}
		else if( FD_ISSET(spectratimeFD, &set) ){
			retcod = read(spectratimeFD, &letter,1);
			if(retcod < 0)
				LOG_ERROR("SpectraTime") << "Failed to read from device" << std::endl;
		}
		else
			LOG_ERROR("SpectraTime") << "Device failes ISSIT response" << std::endl;
	  	if( letter > '\0' )
			output << letter;
 	}

	//Output is in the form: $PTNTA,20160619232817,2,T3,0000000,+050,3,,*11
	// Now, let's fill a buffer with the good parts of the output string that we want
    std::string nugget;
    int count = 0;
    LOG_INFO("SpectraTime") << output.str() ;
    while(std::getline(output, nugget, ',')) {
		if(count == 1){
			SpectraTimeTemp.time[0] = stoi(nugget.substr(0,2));
			SpectraTimeTemp.time[1] = stoi(nugget.substr(2,2));
			SpectraTimeTemp.time[2] = stoi(nugget.substr(4,2));
			SpectraTimeTemp.time[3] = stoi(nugget.substr(6,2));
			SpectraTimeTemp.time[4] = stoi(nugget.substr(8,2));
			SpectraTimeTemp.time[5] = stoi(nugget.substr(10,2));
			SpectraTimeTemp.time[6] = stoi(nugget.substr(12,2));
		}
		else if(count ==2)
			SpectraTimeTemp.quality = stoi(nugget);
		else if(count == 5)
			SpectraTimeTemp.phase   = stoi(nugget);
		else if(count == 6)
			SpectraTimeTemp.status  = stoi(nugget);

		count++;
    }
    SpectraTimeCircularBuffer_.buffer.push_front(SpectraTimeTemp);
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

