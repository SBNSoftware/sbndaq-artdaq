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
  device  = ps.get<std::string>("device");
  channel = ps.get<uint32_t>("channel");
  configure();
}

sbndaq::WhiteRabbitReadout::~WhiteRabbitReadout()
{
  stopAll();
}

void sbndaq::WhiteRabbitReadout::openWhiteRabbitSocket(const char *deviceName)
{
  TLOG(TLVL_DEBUG+1)<< "start";
  agentSocket = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if (agentSocket < 0) 
  {
    TLOG(TLVL_ERROR) << "WhiteRadoutReadout socket error [" << 
      errno <<  "] " << strerror(errno);
    return;
  }

  memset(&agentDevice, 0, sizeof(agentDevice));
  strncpy(agentDevice.ifr_name, deviceName, sizeof(agentDevice.ifr_name));
  if (ioctl(agentSocket, PRIV_MEZZANINE_ID, &agentDevice) < 0
      /* EAGAIN is special: it means we have no ID to check yet */
		&& errno != EAGAIN) 
  {
    TLOG(TLVL_ERROR) << "WhiteRadoutReadout ioctl [1] error device " << deviceName << " [" << errno <<  "] " << strerror(errno);
    close(agentSocket);
    return;
  }

  /* Retrieve the interfaceindex */
  if (ioctl(agentSocket, SIOCGIFINDEX, &agentDevice) < 0) 
  {
    TLOG(TLVL_ERROR) << "WhiteRadoutReadout ioctl [2] error device " << deviceName << " [" << errno <<  "] " << strerror(errno);
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
    TLOG(TLVL_ERROR) << "WhiteRadoutReadout bind error device " << deviceName << " [" << errno <<  "] " << strerror(errno);
    close(agentSocket);
    return;
  }
}


void sbndaq::WhiteRabbitReadout::configure()
{
  TLOG(TLVL_DEBUG) << "hello";
  openWhiteRabbitSocket(device.c_str());
  eventCounter = 0;
}

void sbndaq::WhiteRabbitReadout::start()
{
  TLOG(TLVL_DEBUG) << "start";
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
  int retcod;

  struct sbndaq::WhiteRabbitEvent _event;
  struct sbndaq::WhiteRabbitEvent *event = &_event;

  event->flags = WR_DIO_F_WAIT;
  event->channel = channel;
  agentDevice.ifr_data = (char *)event;
  while ( running )
  {
    retcod = ioctl(agentSocket, PRIV_MEZZANINE_CMD, &agentDevice);
    if ( ( retcod < 0 ) && ( retcod != EAGAIN ))
    {
      TLOG(TLVL_ERROR) << "WhiteReadoutReadout read error device " << device << " [" << errno <<  "] " 
		       << strerror(errno);
      return(false);
    }

    TLOG(TLVL_INFO) << "WhiteRadoutReadout event " << event->timeStamp[0].tv_sec << " " << event->timeStamp[0].tv_nsec; 

    bufferLock.lock();
    buffer.emplace_back(_event);
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
    memcpy(fragPtr->dataBeginBytes(), (void *)(&*i), bytesWritten);
    frags.emplace_back(std::move(fragPtr));
    newData = true;
  }
  buffer.clear();
  bufferLock.unlock();
  eventCounter++;

  return newData;
}

DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(sbndaq::WhiteRabbitReadout)

