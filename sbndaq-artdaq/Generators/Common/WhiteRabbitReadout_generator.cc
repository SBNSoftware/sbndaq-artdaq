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

// Constructor
sbndaq::WhiteRabbitReadout::WhiteRabbitReadout(fhicl::ParameterSet const & ps):
  CommandableFragmentGenerator(ps),
  ps_(ps)
{
  device = ps.get<std::string>("device");
  configure();
}

sbndaq::WhiteRabbitReadout::~WhiteRabbitReadout()
{
  stopAll();
}

void sbndaq::WhiteRabbitReadout::openWhiteRabbitSocket(const char *deviceName)
{
  agentSocket = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if (agentSocket < 0) 
  {
    TLOG(TLVL_ERROR) << "WhiteRadoutReadout socket error " << strerror(errno);
    return;
  }

  memset(&agentDevice, 0, sizeof(agentDevice));
  strncpy(agentDevice.ifr_name, deviceName, sizeof(agentDevice.ifr_name));
  if (ioctl(agentSocket, PRIV_MEZZANINE_ID, &agentDevice) < 0
      /* EAGAIN is special: it means we have no ID to check yet */
		&& errno != EAGAIN) 
  {
    TLOG(TLVL_ERROR) << "WhiteRadoutReadout ioctl [1] error device " << deviceName << " " << strerror(errno);
    close(agentSocket);
    return;
  }

  /* Retrieve the interfaceindex */
  if (ioctl(agentSocket, SIOCGIFINDEX, &agentDevice) < 0) 
  {
    TLOG(TLVL_ERROR) << "WhiteRadoutReadout ioctl [2] error device " << deviceName << " " << strerror(errno);
    close(agentSocket);
    return;
  }

  /* Bind to the interface, so to be able to receive */
  memset(&address, 0, sizeof(address));
  address.sll_family    = AF_PACKET;
  address.sll_protocol  = htons(RULER_PROTO);
  address.sll_ifindex   = agentDevice.ifr_ifindex;
  address.sll_pkttype   = PACKET_BROADCAST; /* that's what ruler sends */
  if (bind(agentSocket, (struct sockaddr *)&address, sizeof(address)) < 0) 
  {
    TLOG(TLVL_ERROR) << "WhiteRadoutReadout bind error device " << deviceName << " " << strerror(errno);
    close(agentSocket);
    return;
  }
}


void sbndaq::WhiteRabbitReadout::configure()
{
  openWhiteRabbitSocket(device.c_str());
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
  int length;
  while ( running )
  {
    length = recv(agentSocket, &rabbitFrame, sizeof(rabbitFrame), MSG_TRUNC);
    if ( length != sizeof(rabbitFrame))
    {
      TLOG(TLVL_ERROR) << "WhiteRadoutReadout recv error device " << device << " " << strerror(errno);
      return(false);
    }

    if ( ntohs(rabbitFrame.header.ether_type) != RULER_PROTO )
    {
      TLOG(TLVL_ERROR) << "WhiteRadoutReadout recv unexpected eth typ " << device << " " 
		       << ntohs(rabbitFrame.header.ether_type);
      return(false);
    }

    sbndaq::WhiteRabbitEvent event;
    memcpy(&event,&rabbitFrame.rabbitCommand,sizeof(event));

    bufferLock.lock();
    buffer.emplace_back(event);
    bufferLock.unlock();
  }

  return(true);
}

bool sbndaq::WhiteRabbitReadout::getNext_(artdaq::FragmentPtrs & frags)
{
  FillFragment(frags,true);
  return true;
}

bool sbndaq::WhiteRabbitReadout::FillFragment(artdaq::FragmentPtrs &frags, bool)
{
  bool newData = false;
  uint32_t bytesWritten = sizeof(struct sbndaq::WhiteRabbitEvent);
  int boardId = 0 ;

  bufferLock.lock();

  for ( std::vector<struct sbndaq::WhiteRabbitEvent>::iterator i = buffer.begin(); i != buffer.end(); ++i)
  {
    std::unique_ptr<artdaq::Fragment> fragPtr(artdaq::Fragment::FragmentBytes(bytesWritten,
									      eventCounter, 
									      boardId,
									      FragmentType::WhiteRabbit,
									      WhiteRabbitFragmentMetadata()));
    memcpy(fragPtr->dataBeginBytes(), (void *)(*i), bytesWritten);
    frags.emplace_back(std::move(fragPtr));
    newData = true;
  }
  buffer.clear();
  bufferLock.unlock();
  eventCounter++;

  return newData;
}

DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(sbndaq::WhiteRabbitReadout)

