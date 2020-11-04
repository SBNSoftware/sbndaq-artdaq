//
// sbndaq-artdaq/Generators/Common/WhiteRabbitReadout_generator.cc 
//  (W.Badgett)
//
// Read events from a White Rabbit socket
//

#include "sbndaq-artdaq/Generators/Common/WhiteRabbitReadout.hh"
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

sbndaq::WhiteRabbitReadout::WhiteRabbitReadout(fhicl::ParameterSet const & ps):
  CommandableFragmentGenerator(ps),
  ps_(ps)
{
  configure();
}

sbndaq::WhiteRabbitReadout::~WhiteRabbitReadout()
{
  stopAll();
}

void sbndaq::WhiteRabbitReadout::configure()
{
  eventCounter = 0;
}

void sbndaq::WhiteRabbitReadout::start()
{
  // Magically start worker getdata thread.
  share::ThreadFunctor functor = std::bind(&WhiteRabbitReadout::getData,this);
  auto worker_functor = share::WorkerThreadFunctorUPtr(new share::WorkerThreadFunctor(functor,"GetDataWorkerThread"));
  auto getData_worker = share::WorkerThread::createWorkerThread(worker_functor);
  GetData_thread_.swap(getData_worker);
  
  GetData_thread_->start();
  running = true;
}

void sbndaq::WhiteRabbitReadout::stopAll()
{
  if ( running )
  {
    running = false;
    GetData_thread_->stop();
  }
}

void sbndaq::WhiteRabbitReadout::stop()
{
  stopAll();
}

void sbndaq::WhiteRabbitReadout::stopNoMutex()
{
  stopAll();
}

bool sbndaq::WhiteRabbitReadout::getData()
{
  return(true);
}

bool sbndaq::WhiteRabbitReadout::getNext_(artdaq::FragmentPtrs & frags)
{
  std::cout << frags.size();
  return true;
}

bool sbndaq::WhiteRabbitReadout::FillFragment(artdaq::FragmentPtrs &frags, bool)
{
  bool newData = true;

  std::cout << frags.size();
  return newData;
}

DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(sbndaq::WhiteRabbitReadout)

