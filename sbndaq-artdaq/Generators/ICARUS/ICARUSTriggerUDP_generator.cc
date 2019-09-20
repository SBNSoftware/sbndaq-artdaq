#define TRACE_NAME "ICARUSTriggerUDP"

#include "artdaq/DAQdata/Globals.hh"

#include "sbndaq-artdaq/Generators/ICARUS/ICARUSTriggerUDP.hh"

#include "canvas/Utilities/Exception.h"
#include "artdaq/Generators/GeneratorMacros.hh"
#include "cetlib_except/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "artdaq-core/Utilities/SimpleLookupPolicy.hh"
#include "messagefacility/MessageLogger/MessageLogger.h"
//#include "artdaq-core-demo/Overlays/UDPFragmentWriter.hh"

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
  , sendCommands_(ps.get<bool>("send_CAPTAN_commands", false))
  , rawOutput_(ps.get<bool>("raw_output_enabled", false))
  , rawPath_(ps.get<std::string>("raw_output_path", "/tmp"))
{
  datasocket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
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

  usleep(10000);
  return true;
  /*


  sbndaq::UDPFragment::Metadata metadata;
  metadata.port = dataport_;
  metadata.address = si_data_.sin_addr.s_addr;
  // And use it, along with the artdaq::Fragment header information
  // (fragment id, sequence id, and user type) to create a fragment
  // We'll use the static factory function 
  // artdaq::Fragment::FragmentBytes(std::size_t payload_size_in_bytes, sequence_id_t sequence_id,
  //  fragment_id_t fragment_id, type_t type, const T & metadata)
  // which will then return a unique_ptr to an artdaq::Fragment
  // object. The advantage of this approach over using the
  // artdaq::Fragment constructor is that, if we were to want to
  // initialize the artdaq::Fragment with a nonzero-size payload (data
  // after the artdaq::Fragment header and metadata), we could provide
  // the size of the payload in bytes, rather than in units of the
  // artdaq::Fragment's RawDataType (8 bytes, as of 3/26/14). The
  // artdaq::Fragment constructor itself was not altered so as to
  // maintain backward compatibility.
  std::size_t initial_payload_size = 0;
  frags.emplace_back(artdaq::Fragment::FragmentBytes(initial_payload_size,
						     ev_counter(), fragment_id(),
						     artdaq::Fragment::FirstUserFragmentType, metadata));
  // We now have a fragment to contain this event:
  sbndaq::UDPFragmentWriter thisFrag(*frags.back());
  bool haveData = false;
  int16_t burst_end = -1;
  uint8_t droppedPackets = 0;
  while (!haveData)
    {
      if (should_stop())
	{
	  return false;
	}
      struct pollfd ufds[1];
      ufds[0].fd = datasocket_;
      ufds[0].events = POLLIN | POLLPRI;
      int rv = poll(ufds, 1, 1000);
      if (rv > 0)
	{
	  //std::cout << "revents: " << ufds[0].revents << ", " << ufds[1].revents << std::endl;
	  if (ufds[0].revents == POLLIN || ufds[0].revents == POLLPRI)
	    {
	      uint8_t peekBuffer[2];
	      recvfrom(datasocket_, peekBuffer, sizeof(peekBuffer), MSG_PEEK,
		       (struct sockaddr *) &si_data_, (socklen_t*)sizeof(si_data_));
	      TLOG(TLVL_INFO) << "Recieved UDP Packet with sequence number " << std::hex << (int)peekBuffer[1] << "!";
	      //std::cout << "peekBuffer[1] == expectedPacketNumber_: " << std::hex << (int)peekBuffer[1] << " =?= " << (int)expectedPacketNumber_ << std::endl;
	      uint8_t seqNum = peekBuffer[1];
	      ReturnCode dataCode = getReturnCode(peekBuffer[0]);
	      if (seqNum >= expectedPacketNumber_ || (seqNum < 10 && expectedPacketNumber_ > 200) || droppedPackets > 0 || expectedPacketNumber_ - seqNum > 20)
		{
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
		      packetBuffers_.push_back(buffer);
		      TLOG(TLVL_DEBUG) << "Now placing UDP packet with sequence number " << std::hex << (int)seqNum << " into buffer.";
		      if (dataCode == ReturnCode::Read) { haveData = true; }
		      else
			{
			  droppedPackets = 0;
			  burst_end = -1;
			}
		    }
		  else if ((dataCode == ReturnCode::Middle || dataCode == ReturnCode::Last) && packetBuffers_.size() > 0)
		    {
		      packetBuffer_t buffer;
		      memset(&buffer[0], 0, sizeof(packetBuffer_t));
		      recvfrom(datasocket_, &buffer[0], sizeof(packetBuffer_t), 0, (struct sockaddr *) &si_data_, (socklen_t*)sizeof(si_data_));
		      if (droppedPackets == 0)
			{
			  packetBuffers_.push_back(buffer);
			}
		      else if (burst_end == -1 || seqNum < burst_end)
			{
			  bool found = false;
			  for (packetBuffer_list_t::iterator it = packetBuffers_.begin(); it != packetBuffers_.end(); ++it)
			    {
			      if (seqNum < (*it)[1])
				{
				  packetBuffers_.insert(it, buffer);
				  droppedPackets--;
				  expectedPacketNumber_--;
				}
			    }
			  if (!found)
			    {
			      packetBuffers_.push_back(buffer);
			    }
			}
		      TLOG(TLVL_DEBUG) << "Now placing UDP packet with sequence number " << std::hex << (int)seqNum << " into buffer.";
		      if (dataCode == ReturnCode::Last && droppedPackets == 0)
			{
			  while (getReturnCode(packetBuffers_.back()[0]) != ReturnCode::Last) { packetBuffers_.pop_back(); }
			  haveData = true;
			}
		      else if (dataCode == ReturnCode::Last) { burst_end = seqNum; }
		      else if (burst_end >= 0 && droppedPackets == 0)
			{
			  while (getReturnCode(packetBuffers_.back()[0]) != ReturnCode::Last) { packetBuffers_.pop_back(); }
			  haveData = true;
			}
		    }
		  ++expectedPacketNumber_;
		}
	      else
		{
		  packetBuffer_t discardBuffer;
		  recvfrom(datasocket_, &discardBuffer[0], sizeof(discardBuffer), 0, (struct sockaddr *) &si_data_, (socklen_t*)sizeof(si_data_));
		  TLOG(TLVL_WARNING) << "Out-of-sequence packet detected and discarded!";
		}
	    }
	}
    }
  packetBuffer_t& firstPacket = packetBuffers_.front();
  TLOG(TLVL_DEBUG) << "Recieved data, now placing data with UDP sequence number " << (int)firstPacket[1] << " into UDPFragment";
  thisFrag.resize(1500 * packetBuffers_.size() + 1);
  std::ofstream output;
  if (rawOutput_)
    {
      std::string outputPath = rawPath_ + "/ICARUSTriggerUDP-" + ip_ + ":" + std::to_string(dataport_) + ".bin";
      output.open(outputPath, std::ios::out | std::ios::app | std::ios::binary);
    }
  DataType dataType = getDataType(firstPacket[0]);
  thisFrag.set_hdr_type((int)dataType);
  int pos = 0;
  for (auto jj : packetBuffers_)
    {
      for (int ii = 2; ii < 1500; ++ii)
	{
	  // Null-terminate string types
	  if (jj[ii] == 0 && (dataType == DataType::JSON || dataType == DataType::String)) { break; }
	  if (rawOutput_) output.write((char*)&(jj[ii]), sizeof(uint8_t));
	  *(thisFrag.dataBegin() + pos) = jj[ii];
	  ++pos;
	}
    }
  if (dataType == DataType::JSON || dataType == DataType::String)
    {
      *(thisFrag.dataBegin() + pos) = 0;
      char zero = 0;
      if (rawOutput_) output.write(&zero, sizeof(char));
    }
  if (rawOutput_) output.close();
  return true;

  */
}
void sbndaq::ICARUSTriggerUDP::start()
{
  send(CommandType::Start_Burst);
}
void sbndaq::ICARUSTriggerUDP::stop()
{
  send(CommandType::Stop_Burst);
}
void sbndaq::ICARUSTriggerUDP::pause()
{
  send(CommandType::Stop_Burst);
}
void sbndaq::ICARUSTriggerUDP::resume()
{
  send(CommandType::Start_Burst);
}
void sbndaq::ICARUSTriggerUDP::send(CommandType command)
{
  if (sendCommands_)
    {
      CommandPacket packet;
      packet.type = command;
      packet.dataSize = 0;
      sendto(datasocket_, &packet, sizeof(packet), 0, (struct sockaddr *) &si_data_, sizeof(si_data_));
    }
}
// The following macro is defined in artdaq's GeneratorMacros.hh header
DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(sbndaq::ICARUSTriggerUDP)
