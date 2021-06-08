//
// sbndaq-artdaq/Generators/SBND/DAPHNEReader.hh
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

    protected:

     // "start" transition
     void start() override;
 
     // "stop" transition
     void stop() override;
     bool getNext_(artdaq::FragmentPtrs& output) override;
     void stopNoMutex() override {}

     void setupDAPHNE(fhicl::ParameterSet const& ps);

     void write(uint16_t address, uint16_t data, bool LC=false);
     uint16_t read(uint16_t address, bool LC=false);
     bool sendCommand(char *cmd);

     int localSocket;
     FILE *file;
     struct sockaddr_in remoteSocket;
     struct hostent *remoteHost;
     std::string addressString;
     uint32_t port;
    uint32_t timeOut;

     uint16_t pedestal;
  };
}

#endif
