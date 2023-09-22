//
// sbndaq-artdaq/Generators/Common/WhiteRabbitReadout_generator.cc 
//  (W.Badgett)
//
// Read events from a White Rabbit socket
//

#include "sbndaq-artdaq/Generators/Common/WhiteRabbitReadout.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "artdaq/Generators/GeneratorMacros.hh"
#include "artdaq/DAQdata/Globals.hh"
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
  generated_fragments_per_event_(ps.get<int>("generated_fragments_per_event",0)),
  ps_(ps)
{
  fragmentId  = ps.get<uint32_t>("fragmentId");
  device      = ps.get<std::string>("device");
  channelMask = ps.get<uint32_t>("channelMask");
  char name[80];
  for (int i=0; i<N_CHANNELS; i++)
  {
    sprintf(name,"channelMode%d", i);
    std::string mode = ps.get<std::string>(name,"X");
    channelMode[i] = mode[0];

    sprintf(name,"channelPPS%d", i);
    channelMode[i] = ps.get<bool>(name,false);
  }
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
    TLOG(TLVL_ERROR) << "WhiteRabbitReadout socket error [" << 
      errno <<  "] " << strerror(errno) <<" deviceName " << deviceName ;
    return;
  }

  memset(&agentDevice, 0, sizeof(agentDevice));
  strncpy(agentDevice.ifr_name, deviceName, sizeof(agentDevice.ifr_name));

  if (ioctl(agentSocket, PRIV_MEZZANINE_ID, &agentDevice) < 0
		&& errno != EAGAIN) 
  {
    /* EAGAIN is special: it means we have no ID to check yet */
    TLOG(TLVL_ERROR) << "WhiteRabbitReadout ioctl [1] error device " << deviceName << " [" << errno <<  "] " << strerror(errno);
    close(agentSocket);
    return;
  }

  /* Retrieve the interfaceindex */
  if (ioctl(agentSocket, SIOCGIFINDEX, &agentDevice) < 0) 
  {
    TLOG(TLVL_ERROR) << "WhiteRabbitReadout ioctl [2] error device " << deviceName << " [" << errno <<  "] " << strerror(errno);
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
    TLOG(TLVL_ERROR) << "WhiteRabbitReadout bind error device " << deviceName << " [" << errno <<  "] " << strerror(errno);
    close(agentSocket);
    return;
  }

  // Configure channel modes and PPS
  configureModes();
  configurePPS();
}


void sbndaq::WhiteRabbitReadout::configure()
{
  TLOG(TLVL_DEBUG) << "hello";
  TLOG(TLVL_INFO) << " **** added metric for grafana **** DT APRIL 13  2022";
  openWhiteRabbitSocket(device.c_str());
  eventSeqCounter = 0;
}

void sbndaq::WhiteRabbitReadout::configureModes()
{
  int retcod  = 0;
  struct sbndaq::WhiteRabbitData cmd;
  cmd.command = WR_DIO_CMD_INOUT;
  cmd.flags   = WR_DIO_F_MASK;
  cmd.value   = 0;
  cmd.channel = 0;
  agentDevice.ifr_data = (char *)&cmd;

  // Channel 0 cannot be changed
  for (int c = 1; c < N_CHANNELS; c++)
  {
    cmd.channel = ( 1 << c );
    if ( channelMode[c] != 'X' )
    {
      switch (channelMode[c] )
      {
	case 'D':
	  cmd.value |= ( WR_DIO_INOUT_TERM << c);
	  cmd.value |= ( WR_DIO_INOUT_DIO << c);
          cmd.value |= ( WR_DIO_INOUT_OUTPUT << c);
	break;

	case 'd':
	  cmd.value |= ( WR_DIO_INOUT_DIO << c);
          cmd.value |= ( WR_DIO_INOUT_OUTPUT << c);
	break;

	case 'C':
	  cmd.value |= ( WR_DIO_INOUT_TERM << c);
	  cmd.value |= ( WR_DIO_INOUT_DIO << c);
	  cmd.value |= ( WR_DIO_INOUT_VALUE << c);
	break;

	case 'c':
	  cmd.value |= ( WR_DIO_INOUT_DIO << c);
	  cmd.value |= ( WR_DIO_INOUT_VALUE << c);
	break;

	case 'P':
	  cmd.value |= ( WR_DIO_INOUT_TERM << c);
	  cmd.value |= ( WR_DIO_INOUT_DIO << c);
	  cmd.value |= ( WR_DIO_INOUT_VALUE << c);
	  cmd.value |= ( WR_DIO_INOUT_OUTPUT << c);
        break;

	case 'p':
	  cmd.value |= ( WR_DIO_INOUT_DIO << c);
	  cmd.value |= ( WR_DIO_INOUT_VALUE << c);
	  cmd.value |= ( WR_DIO_INOUT_OUTPUT << c);
	break;

	case 'I':
	  cmd.value |= ( WR_DIO_INOUT_TERM << c);
	break;

	case 'i':
	break;

	case '1':
	  cmd.value |= ( WR_DIO_INOUT_VALUE << c);
	  cmd.value |= ( WR_DIO_INOUT_OUTPUT << c);
        break;

	case '0':
	  cmd.value |= ( WR_DIO_INOUT_OUTPUT << c);
	break;

	retcod = ioctl(agentSocket,PRIV_MEZZANINE_CMD, agentDevice);
	if ( retcod < 0 )
	{
	  TLOG(TLVL_ERROR) << "WhiteRabbitReadout set channelMode error " << device 
			   << " [" << errno <<  "] " << strerror(errno);
	}
      }
    }
  }
}

void sbndaq::WhiteRabbitReadout::configurePPS()
{
  int retcod  = 0;
  struct sbndaq::WhiteRabbitData cmd;
  cmd.command = WR_DIO_CMD_PULSE;
  cmd.flags   = WR_DIO_F_REL | WR_DIO_F_LOOP;;
  cmd.value   = -1;
  cmd.channel = 0;
  cmd.timeStamp[0].tv_sec  = 2;
  cmd.timeStamp[1].tv_nsec = 1000000;
  cmd.timeStamp[2].tv_sec  = 1;

  agentDevice.ifr_data = (char *)&cmd;

  // Channel 0 cannot be changed
  for (int c = 1; c < N_CHANNELS; c++)
  {
    cmd.channel = c;
    if ( channelMode[c] )
    {
      retcod = ioctl(agentSocket,PRIV_MEZZANINE_CMD, agentDevice);
      if ( retcod < 0 )
      {
	TLOG(TLVL_ERROR) << "WhiteRabbitReadout enable channelPPS error " << device 
			 << " [" << errno <<  "] " << strerror(errno);
      }
    }
  }
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

  struct sbndaq::WhiteRabbitEvent event;
  struct sbndaq::WhiteRabbitData *data = &event.data;

  while ( running )
  {
    data->flags         = WR_DIO_F_WAIT | WR_DIO_F_MASK;
    data->channel       = channelMask;
    data->command       = WR_DIO_CMD_STAMP;
    data->value         = 0;
    agentDevice.ifr_data = (char *)data;
    TLOG(TLVL_DEBUG+1) << "WhiteRabbit data->command: " << data->command << "  data->flags: " << data->flags;
    retcod = ioctl(agentSocket, PRIV_MEZZANINE_CMD, &agentDevice);
    clock_gettime(CLOCK_REALTIME,&event.systemTime);
    if ( ( retcod < 0 ) && ( retcod != EAGAIN ))
    {
      TLOG(TLVL_ERROR) << "WhiteRabbitReadout read device error " << device 
		       << " [" << errno <<  "] " << strerror(errno);
      return(false);
    }

    TLOG(TLVL_DEBUG) << "WhiteReadout: data nstamp " << data->nstamp << " at " << event.systemTime.tv_sec << " " <<
      event.systemTime.tv_nsec;
    for (uint32_t i=0; i<data->nstamp; i++)
    {
      TLOG(TLVL_DEBUG+1) << "WhiteReadout: data " << i << " Ch  " << data->channel << " TS  " << data->timeStamp[i].tv_sec << 
	" " << data->timeStamp[i].tv_nsec; 

//added in the loop 
//    std::ostringstream tempStream;
//    tempStream << "Card: " << fBoardID
//               << ", Channel: " << ch << " temp.";
//    metricMan->sendMetric(tempStream.str(), int(ch_temps[ch]), "C", 1,
//                          artdaq::MetricMode::Average, "CAENV1730");

//    std::ostringstream tsStream;
//    tsStream << "DIO Channel " << data->channel
//               << ", Data: " << data->timeStamp[i].tv_sec << " ts.";

}

   std::ostringstream tsStream; 
   tsStream << "DIO Channel " << data->channel << " Rates";


//  PPS
    if(data->channel == 0) 
    metricMan->sendMetric(tsStream.str(), int(data->nstamp), "Hz", 11,
                          artdaq::MetricMode::Rate, "WR_CLK02_SPEC_DIO_PPS");
// $1D from MI12
    if(data->channel == 1) 
    metricMan->sendMetric(tsStream.str(), int(data->nstamp), "Hz", 11,
                          artdaq::MetricMode::Rate, "WR_CLK02_SPEC_DIO_$1D");

// gatedBES from MI12
    if(data->channel == 4)
    metricMan->sendMetric(tsStream.str(), int(data->nstamp), "Hz", 11,
                          artdaq::MetricMode::Rate, "WR_CLK02_SPEC_DIO_gatedBES");

// $AE from MI60
    if(data->channel == 2)
    metricMan->sendMetric(tsStream.str(), int(data->nstamp), "Hz", 11,
                          artdaq::MetricMode::Rate, "WR_CLK02_SPEC_DIO_$AE");

// $74 from MI60
    if(data->channel == 3) 
    metricMan->sendMetric(tsStream.str(), int(data->nstamp), "Hz", 11,
                          artdaq::MetricMode::Rate, "WR_CLK02_SPEC_DIO_$74");


//Average
//Rate


    if ( data->nstamp > 0 )
    {
      bufferLock.lock();
      buffer.emplace_back(event);
      bufferLock.unlock();
    }
  }
  return(true);
}

bool sbndaq::WhiteRabbitReadout::getNext_(artdaq::FragmentPtrs & frags)
{

  //copied from TriggerUDP code: if shouldn't send fragments, then don't create fragment/send
  if(generated_fragments_per_event_== 0){
    fLastEvent = fEventCounter;
    ++fEventCounter;
    return true;
  }

  FillFragment(frags,true);

  for (auto const& frag : frags) {
    TLOG(33) << " Sending setSequenceID " << frag->sequenceID() << ", fragmentID " << frag->fragmentID()
             << ", timestamp " << frag->timestamp();
	}
  return true;
}

bool sbndaq::WhiteRabbitReadout::FillFragment(artdaq::FragmentPtrs &frags, bool)
{
  bool newData = false;
  uint32_t bytesWritten = sizeof(struct sbndaq::WhiteRabbitEvent);
  int boardId = 0 ;
  uint64_t zeit;

  bufferLock.lock();

//  eventSeqCounter++;
  TLOG(TLVL_DEBUG+12) << "WhiteRabbit event " << eventSeqCounter;

  for ( std::vector<struct sbndaq::WhiteRabbitEvent>::iterator i = buffer.begin(); i != buffer.end(); ++i)
  {
    std::unique_ptr<artdaq::Fragment> fragPtr(artdaq::Fragment::FragmentBytes(bytesWritten,
									      eventSeqCounter++, 
									      fragmentId,
									      FragmentType::WhiteRabbit,
									      WhiteRabbitFragmentMetadata()));
    zeit = (uint64_t)((*i).systemTime.tv_sec) * 1E9 + uint64_t((*i).systemTime.tv_nsec);
    fragPtr->setTimestamp(zeit);
    memcpy(fragPtr->dataBeginBytes(), (void *)(&*i), bytesWritten);
    frags.emplace_back(std::move(fragPtr));
    newData = true;
  }
  buffer.clear();
  bufferLock.unlock();


  return newData;
}

DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(sbndaq::WhiteRabbitReadout)

