//
// sbndaq-artdaq/Generators/SBND/DAPHNEReader.hh  (W.Badgett)
//

#ifndef _DAPHNEReader_h
#define _DAPHNEReader_h

#include <stdio.h>
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>

#include "artdaq-core/Data/Fragment.hh"
#include "artdaq/Generators/CommandableFragmentGenerator.hh"

namespace sbndaq 
{
  class DAPHNEReader : public artdaq::CommandableFragmentGenerator 
  {
    public:
      DAPHNEReader();
      // "configure" transition happens at the constructor
      explicit DAPHNEReader(fhicl::ParameterSet const& ps);
      // "shutdown" transition happens at the destructor
      virtual ~DAPHNEReader();

     // "start" transition
     void start() override;
 
     // "stop" transition
     void stop() override;
     bool getNext_(artdaq::FragmentPtrs& output) override;
     void stopNoMutex() override {}

     void setupDAPHNE(fhicl::ParameterSet const& ps);

     void write(uint16_t address, uint16_t data);
     void writeLC(uint16_t address, uint16_t data);
     void writeLCAll(uint16_t address, uint16_t data);
     uint16_t read(uint16_t address);
     uint16_t read(uint16_t address, bool LC);
     uint16_t readLC(uint16_t address);
     void flushSocket();
     bool sendCommand(char *cmd);
     void selectPort(uint16_t port);
     void linkInit();
     void trig(uint16_t onOff);
     void trigOld(uint16_t onOff);
     void takeData();
     int nFEBs;

    enum {
      ALL_FEB_PEDESTALS = 0x316,
      FEB_1_PEDESTAL = 0x000
    };

   private:
     int localSocket;
     FILE *file;
     struct sockaddr_in remoteSocket;
     struct hostent *remoteHost;
     std::string addressString;
     uint32_t port;
     uint32_t timeOut;
     uint16_t voltage;

     uint16_t pedestal;
     std::vector<uint16_t> pedestals;
  };
}

#endif
