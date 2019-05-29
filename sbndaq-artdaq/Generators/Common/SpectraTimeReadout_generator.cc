//
// sbndaq-artdaq/Generators/Common/SpectraTimeReadout_generator.cc 
//  (D.Cianci,W.Badgett)
//
// Read from a message queue GPS status messages, buffer them and send them 
//    out on request to the EVBs
//
// Strategery:
//    GPS status messages arrive shortly after a PPS
//    Every event should have at least the latest GPS status message
//    If there are others in the circular buffer, send them, too
//    Any messages older than two minutes (120 PPS) are discarded -- 
//       -- if we're not taking data, who cares?
//    The uint32_t timeStamp is the Unix time since the so-called Epoch, 
//      Jan 1, 1970 UTC
//      and there should be only one message per second, no more and no less
//    Note these data are also sent to EPICS and its archiving system

#include "sbndaq-artdaq/Generators/Common/SpectraTimeReadout_generator.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "artdaq/Generators/GeneratorMacros.hh"
#include <fstream>
#include <iomanip>
#include <iterator>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

sbndaq::SpectraTimeReadout::SpectraTimeReadout(fhicl::ParameterSet const & ps):
  CommandableFragmentGenerator(ps),
  ps_(ps)
{
  readoutMode = ps.get<int>("readoutMode");
  statusMode  = ps.get<int>("statusMode");
  nextSleep   = ps.get<int>("nextSleep");
  dataSleep   = ps.get<int>("dataSleep");
  init();
}

sbndaq::SpectraTimeReadout::~SpectraTimeReadout()
{
  stopAll();
}

void sbndaq::SpectraTimeReadout::init()
{
  eventCounter = 0;
  messageCounter = 0;
  currentMessage = messageCounter;

  for ( int i=0; i<BUFFER_SIZE; i++)  { buffer[i].unsent = false;  }

  // Connect to GPS message queue
  daqMQ = (key_t)GPS_MQ;
  daqID = msgget(daqMQ,0);
  if ( daqID < 0 )
  {
    LOG_ERROR("SpectraTime") << "Error on msgget 0x" << 
      std::hex << daqMQ << std::dec << " " << strerror(errno) << " " << errno;

  }
}

void sbndaq::SpectraTimeReadout::start()
{
  // Magically start worker getdata thread.
  share::ThreadFunctor functor = std::bind(&SpectraTimeReadout::getData,this);
  auto worker_functor = share::WorkerThreadFunctorUPtr(new share::WorkerThreadFunctor(functor,"GetDataWorkerThread"));
  auto getData_worker = share::WorkerThread::createWorkerThread(worker_functor);
  GetData_thread_.swap(getData_worker);
  
  GetData_thread_->start();
  running = true;
}

void sbndaq::SpectraTimeReadout::stopAll()
{
  if ( running )
  {
    running = false;
    GetData_thread_->stop();
  }
}

void sbndaq::SpectraTimeReadout::stop()
{
  stopAll();
}

void sbndaq::SpectraTimeReadout::stopNoMutex()
{
  stopAll();
}

bool sbndaq::SpectraTimeReadout::getData()
{
  ssize_t nBytes = 1;
  while ( ( nBytes > 0 ) && running )
  {
    uint32_t ptr = messageCounter % BUFFER_SIZE;

    bufferLock.lock();
    nBytes = msgrcv(daqID,(void *)&buffer[ptr],sizeof(buffer[ptr]), 
		    GPSInfo::GPS_INFO_MTYPE,IPC_NOWAIT);
    if ( nBytes <= 0 )
    {
      if (( errno != EAGAIN ) && ( errno != ENOMSG ))
      {
	LOG_ERROR("SpectraTime") << "FATAL Error on msgrcv 0x" << 
	  std::hex << daqMQ << std::dec << " " << 
	  strerror(errno) << " " << errno ;
      } // Otherwise there was no message in the queue, not a real error
    }
    else
    {
      buffer[ptr].unsent = true;
      currentMessage = ptr;
      messageCounter++;
    }
    bufferLock.unlock();

    usleep(dataSleep);
  }
  return(true);
}

bool sbndaq::SpectraTimeReadout::getNext_(artdaq::FragmentPtrs & frags)
{
  //Send our buffer over to fillfrag. If there's nothing there, we'll 
  // try again later.
  if ( readoutMode == READOUT_PUSH )
  {
    while ( FillFragment(frags) == false )
    { usleep(nextSleep);}
  }
  else 
  {
    FillFragment(frags);
    usleep(nextSleep);
  }
  return true;
}

bool sbndaq::SpectraTimeReadout::FillFragment(artdaq::FragmentPtrs &frags, bool)
{
  uint32_t boardId=0;
  uint32_t bytesWritten = sizeof(struct sbndaq::SpectratimeEvent);
  
  int messageCount = 0;
  bool newData = false;

  bufferLock.lock();
  for ( int i=0; i<BUFFER_SIZE; ++i)
  {
    if ( buffer[i].unsent )
    {
      std::unique_ptr<artdaq::Fragment> fragPtr(artdaq::Fragment::FragmentBytes(bytesWritten,
										eventCounter, 
										boardId,
										FragmentType::SpectratimeEvent,
										SpectratimeFragmentMetadata()));

      memcpy(fragPtr->dataBeginBytes(), (char *)&buffer[i].data, bytesWritten);
      frags.emplace_back(std::move(fragPtr));
      buffer[i].unsent = false;
      messageCount++;
      eventCounter++;
      newData = true;
    }
  }

  if (( messageCount == 0 ) && ( readoutMode != READOUT_PUSH ))  // Always return at least one fragment, here the latest
  {
    std::unique_ptr<artdaq::Fragment> fragPtr(artdaq::Fragment::FragmentBytes(bytesWritten,
									      eventCounter, 
									      0,
									      FragmentType::SpectratimeEvent,
									      SpectratimeFragmentMetadata()));
    
    memcpy(fragPtr->dataBeginBytes(), (char *)&buffer[currentMessage].data, bytesWritten);
    frags.emplace_back(std::move(fragPtr));
    buffer[currentMessage].unsent = false;
    eventCounter++;
    newData = false;
  }
  bufferLock.unlock();

  if ( messageCount > 0 )
  {
    LOG_INFO("SpectraTime") << "Found " << messageCount << " fragments" << std::endl;
  }

  return newData;
}

DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(sbndaq::SpectraTimeReadout)

