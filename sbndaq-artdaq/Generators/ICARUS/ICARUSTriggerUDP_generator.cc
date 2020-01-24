#define TRACE_NAME "ICARUSTriggerUDP"

#include "artdaq/DAQdata/Globals.hh"

#include "sbndaq-artdaq/Generators/ICARUS/ICARUSTriggerUDP.hh"

#include "canvas/Utilities/Exception.h"
#include "artdaq/Generators/GeneratorMacros.hh"
#include "cetlib_except/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "artdaq-core/Utilities/SimpleLookupPolicy.hh"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <fstream>
#include <iomanip>
#include <iterator>
#include <iostream>
#include <sys/poll.h>


sbndaq::ICARUSTriggerUDP::ICARUSTriggerUDP(fhicl::ParameterSet const& ps)
  : CommandableFragmentGenerator(ps)
  , dataport_(ps.get<int>("port", 6343))
  , ip_(ps.get<std::string>("ip", "127.0.0.1"))
  , expectedPacketNumber_(0)
  , sendCommands_(ps.get<bool>("send_CAPTAN_commands", true))
  , rawOutput_(ps.get<bool>("raw_output_enabled", false))
  , rawPath_(ps.get<std::string>("raw_output_path", "/tmp"))
  , n_sends_(ps.get<unsigned int>("n_sends",1))
{
  datasocket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  //datasocket_ = socket(AF_INET, SOCK_DGRAM, 0);
  if (datasocket_ < 0)
    {
      throw art::Exception(art::errors::Configuration) << "ICARUSTriggerUDP: Error creating socket!" << std::endl;
      exit(1);
    }
  struct sockaddr_in si_me_data;
  si_me_data.sin_family = AF_INET;
  si_me_data.sin_port = htons(dataport_);
  si_me_data.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(datasocket_, (struct sockaddr *)&si_me_data, sizeof(si_me_data)) == -1)
    {
      throw art::Exception(art::errors::Configuration) <<
	"ICARUSTriggerUDP: Cannot bind data socket to port " << dataport_ << std::endl;
      exit(1);
    }
  si_data_.sin_family = AF_INET;
  si_data_.sin_port = htons(dataport_);
  if (inet_aton(ip_.c_str(), &si_data_.sin_addr) == 0)
    {
      throw art::Exception(art::errors::Configuration) <<
	"ICARUSTriggerUDP: Could not translate provided IP Address: " << ip_ << "\n";
      exit(1);
    }
}
bool sbndaq::ICARUSTriggerUDP::getNext_(artdaq::FragmentPtrs& )//frags)
{
  if (should_stop())
    {
      return false;
    }

  usleep(1e5);

  std::cout << "Trying receive..." << std::endl;

  struct pollfd ufds[1];
  ufds[0].fd = datasocket_;
  ufds[0].events = POLLIN | POLLPRI;
  int rv = poll(ufds, 1, 1000);

  if (rv > 0)
    {
      std::cout << "revents: " << ufds[0].revents << std::endl;
      if (ufds[0].revents == POLLIN || ufds[0].revents == POLLPRI)
	{
	  uint8_t peekBuffer[2];
	  recvfrom(datasocket_, peekBuffer, sizeof(peekBuffer), MSG_PEEK,
		   (struct sockaddr *) &si_data_, (socklen_t*)sizeof(si_data_));

	  std::cout << "\tRECEIVED PACKET! First 16 bits: 0x" << std::hex << *((uint16_t*)(peekBuffer)) << std::dec << std::endl;
	  /*
	  //	  TLOG(TLVL_INFO) << "Recieved UDP Packet with sequence number " << std::hex << (int)peekBuffer[1] << "!";
	      //std::cout << "peekBuffer[1] == expectedPacketNumber_: " << std::hex << (int)peekBuffer[1] << " =?= " << (int)expectedPacketNumber_ << std::endl;
	      //uint8_t seqNum = peekBuffer[1];
	      //ReturnCode dataCode = getReturnCode(peekBuffer[0]);
	      //if (seqNum >= expectedPacketNumber_ || (seqNum < 10 && expectedPacketNumber_ > 200) || droppedPackets > 0 || expectedPacketNumber_ - seqNum > 20)
	  //{
		  if (seqNum != expectedPacketNumber_ && (seqNum >= expectedPacketNumber_ || (seqNum < 10 && expectedPacketNumber_ > 200)))
		    {
		      int deltaHi = seqNum - expectedPacketNumber_;
		      int deltaLo = 255 + seqNum - expectedPacketNumber_;
		      droppedPackets += deltaLo < 255 ? deltaLo : deltaHi;
		      TLOG(TLVL_WARNING) << "Dropped/Delayed packets detected: " << droppedPackets << std::endl;
		      expectedPacketNumber_ = seqNum;
		    }
		  else if (seqNum != expectedPacketNumber_)
		    {
		      int delta = expectedPacketNumber_ - seqNum;
		      TLOG(TLVL_WARNING) << "Sequence Number significantly different than expected! (delta: " << delta << ")";
		    }
		  if (dataCode == ReturnCode::Read || dataCode == ReturnCode::First)
		    {
		      packetBuffers_.clear();
		      packetBuffer_t buffer;
		      memset(&buffer[0], 0, sizeof(packetBuffer_t));
		      recvfrom(datasocket_, &buffer[0], sizeof(packetBuffer_t), 0, (struct sockaddr *) &si_data_, (socklen_t*)sizeof(si_data_));

		    }
		}
	  */
	}
    }
  else{
    std::cout << "nope" << std::endl;
  }
  
  return true;

}
void sbndaq::ICARUSTriggerUDP::start()
{
  unsigned int counter=n_sends_;
  while(counter!=0){
    send(CommandType::Start_Burst);
    --counter;
  }
}
void sbndaq::ICARUSTriggerUDP::stop()
{
  unsigned int counter=n_sends_;
  while(counter!=0){
    send(CommandType::Stop_Burst);
    --counter;
  }
}
void sbndaq::ICARUSTriggerUDP::pause()
{
  unsigned int counter=n_sends_;
  while(counter!=0){
    send(CommandType::Stop_Burst);
    --counter;
  }
}
void sbndaq::ICARUSTriggerUDP::resume()
{
  unsigned int counter=n_sends_;
  while(counter!=0){
    send(CommandType::Start_Burst);
  --counter;
  }

}
void sbndaq::ICARUSTriggerUDP::send(CommandType command)
{
  if (sendCommands_)
    {

      std::cout << "Going to send a command! " << (int)command << std::endl;
      usleep(1e6);

      CommandPacket packet;
      packet.type = command;
      packet.dataSize = 0;

      std::cout << "\tpacket size is " << sizeof(packet) << std::endl;
      sendto(datasocket_, &packet, sizeof(packet), 0, (struct sockaddr *) &si_data_, sizeof(si_data_));

      std::cout << "Send command!" << std::endl;

      usleep(1e6);

    }
}
// The following macro is defined in artdaq's GeneratorMacros.hh header
DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(sbndaq::ICARUSTriggerUDP)
