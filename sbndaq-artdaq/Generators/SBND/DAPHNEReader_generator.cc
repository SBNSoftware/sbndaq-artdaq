//
//  sbndaq-artdaq/Generators/SBND/DAPHNEReader_generator.cc   (W.Badgett)
//
//  Configuring board reader for the DAPHNE SiPM Readout
//

#include "DAPHNEReader.hh"
#include "artdaq/Generators/GeneratorMacros.hh"
#include "cetlib_except/exception.h"

#include <sstream>
#include <vector>
#include <memory>
#include <iomanip>
#include <chrono>
#include <thread>

using namespace sbndaq;

namespace sbndaq 
{

// "initialize" transition
DAPHNEReader::DAPHNEReader(fhicl::ParameterSet const& ps): CommandableFragmentGenerator(ps) 
{
  const std::string identification = "DAPHNEReader";
  TLOG_INFO(identification) << "DAPHNEReader constructor" << TLOG_ENDL;
  setupDAPHNE(ps);
}

void DAPHNEReader::setupDAPHNE(fhicl::ParameterSet const& ps) 
{
  int retcod;

  const std::string identification = "DAPHNEReader::setupDAPHNE";

  addressString = ps.get<std::string>("ipAddress");
  port = ps.get<uint32_t>("port");
  TLOG_INFO(identification) << "Starting setupDAPHNE at " << port << "@" << addressString << TLOG_ENDL;

  nFEBs = ps.get<int>("nFEBs");
  TLOG_INFO(identification) << "Running with " << nFEBs << " FEB(s)" << TLOG_ENDL;

  //pedestal = ps.get<uint16_t>("pedestal");
  //TLOG_INFO(identification) << "Using DAPHNE pedestal " << pedestal << TLOG_ENDL;

  // Vector of pedestals so we can set a different value for each FEB
  pedestals = ps.get<std::vector<uint16_t>>("pedestals");
  TLOG_INFO(identification) << "Using DAPHNE pedestals " << pedestals[0] << ", " << pedestals[1] << ", " << pedestals[2] << ", " << pedestals[3] << TLOG_ENDL;

  timeOut = ps.get<uint32_t>("timeOut");
  TLOG_INFO(identification) << "Using DAPHNE timeOut " << timeOut << TLOG_ENDL;

  voltage = ps.get<uint16_t>("voltage");
  TLOG_INFO(identification) << "Biasing SiPMs to " << voltage << " V " << TLOG_ENDL;

  // Connect network sockets
  localSocket = socket(AF_INET, SOCK_STREAM, 0);
  if ( localSocket < 0 )
  {
    std::stringstream line;
    line << "Error opening socket " <<  strerror(errno);

    TLOG_ERROR("DAPHNEReader") << line.str();
    throw std::runtime_error(line.str());
    return;
  }

  file = fdopen(localSocket,"rw"); 
  if ( file == NULL )
  {
    std::stringstream line;
    line << "Error creating file descriptor for DAPHNE controller " 
	      << strerror(errno);
    TLOG_ERROR("DAPHNEReader") << line.str();
    throw std::runtime_error(line.str());
    return;
  }

  remoteHost = gethostbyname(addressString.c_str());
  if ( remoteHost == NULL )
  {
    std::stringstream line;
    line << "Error finding host " << 
      addressString << " " << strerror(errno);
    TLOG_ERROR("DAPHNEReader") << line.str();
    throw std::runtime_error(line.str());
    return;
  }

  remoteSocket.sin_family = remoteHost->h_addrtype;
  remoteSocket.sin_port   = htons(port);
  remoteSocket.sin_addr   = *((struct in_addr *) remoteHost->h_addr ) ;

  char *addressStringPtr = inet_ntoa(remoteSocket.sin_addr);
  strcpy(addressStringPtr, addressString.c_str());

  retcod = connect(localSocket,
		   (const sockaddr*)&remoteSocket,sizeof(remoteSocket));
  if ( retcod < 0 )
  {
    std::stringstream line;
    line <<"Error connecting to " << port << 
      "@" <<  addressString << " " << strerror(errno);
    TLOG_ERROR("DAPHNEReader") << line.str();
    throw std::runtime_error(line.str());
    return;
  }

  struct timeval timer;
  timer.tv_sec  = timeOut;
  timer.tv_usec = 100000;
  retcod = setsockopt(localSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timer, 
	     sizeof(struct timeval));
  if ( retcod < 0 )
  {
    std::stringstream line;
    line << "Error setsockopt SO_RCVTIMEO " <<  strerror(errno);

    TLOG_ERROR("DAPHNEReader") << line.str();
    throw std::runtime_error(line.str());
    return;
  }

  int flag = 1;
  retcod = setsockopt(localSocket, IPPROTO_TCP,TCP_NODELAY,(char *)&flag, sizeof(flag)); 
  if ( retcod < 0 )
  {
    std::stringstream line;
    line << "Error setsockopt TCP_NODELAY " <<  strerror(errno);

    TLOG_ERROR("DAPHNEReader") << line.str();
    throw std::runtime_error(line.str());
    return;
  }

  // DAPHNE Initialization Specifics
  linkInit(); // LI
  selectPort(1); // Only port 1 works for now (FEB plugged in)
  //  writeLC(0x316, 0x100); // Set pedestal (*** note: just one value for "all FEBS" for now)

  // Write unique pedestal value for each FEB
  writeLC(FEB_1_PEDESTAL, pedestals[0]); 
  writeLC(0x400, pedestals[1]); 
  writeLC(0x800, pedestals[2]); 
  writeLC(0xC00, pedestals[3]); 

  write(0x800, 0x8); // Disable FPGA2
  write(0xc00, 0x8); // Disable FPGA3
  write(0x400, 0x100); // Initialize FPGA1
  write(0x400, 0xa8); // Enable FPGA1 sequencer
  writeLC(0x30e, 0x2); // Enable external triggers on FEB0
  writeLC(0x305, 0x100); // Beam on spill len
  writeLC(0x306, 0x100); // Beam off spill len
  writeLC(0x308, 0x5); // Front pipeline delay
  writeLC(0x304, 0x5); // Pipeline delay
  writeLC(0x30c, 0x8); // # ADC samples per flash above threshold
  takeData(); // UB1
  trigOld(1); 

  if ( read(0x27) != 0x0007 ) 
    {
      TLOG_INFO(identification) << "Register 0x27 not at nominal value. It is: " << read(0x27) << ". Setting to 7..." << TLOG_ENDL;      
      write(0x27, 0x300); // Are all FIFOs empty?
      TLOG_INFO(identification) << "0x27 now reads: " << read(0x27) << TLOG_ENDL;
    }

  if ( read(0x2) != 0x000C ) 
    {
      TLOG_INFO(identification) << "Register 0x2 not at nominal value. It is: " << read(0x2) << ". Setting to 12..." << TLOG_ENDL;      
      write(0x2, 0x1); // Input buffer state on fiber link
      TLOG_INFO(identification) << "0x2 now reads: " << read(0x2) << TLOG_ENDL;
    }

  writeLC(0x44, voltage); // bias the SiPMs
  writeLC(0x44, 0x00); // unbias SiPMs

  TLOG_INFO(identification) << "Done with setupDAPHNE" << TLOG_ENDL;  

}
  

// "shutdown" transition
DAPHNEReader::~DAPHNEReader() 
{
  int retcod;
  retcod = shutdown(localSocket,2);
  if ( retcod < 0 )
  {
    TLOG_ERROR("DAPHNEReader") << "Error on socket shutdown "<<
      strerror(errno);
    return;
  }

  retcod = close(localSocket);
  if ( retcod < 0 )
  {
    TLOG_ERROR("DAPHNEReader") << "Error on socket close "<< 
      strerror(errno);
  }
}

// "start" transition
void DAPHNEReader::start() 
{

}

// "stop" transition
void DAPHNEReader::stop() 
{

}

// Called by BoardReaderMain in a loop between "start" and "stop"
bool DAPHNEReader::getNext_(artdaq::FragmentPtrs& /*frags*/) 
{
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  return (! should_stop()); // returning false before should_stop makes all other BRs stop
}

} // namespace

// Network communications functions
void DAPHNEReader::flushSocket()
{
  //turn on non-blocking
  fcntl(localSocket, F_SETFL, fcntl(localSocket, F_GETFL)| O_NONBLOCK);
  int ret;
  char buffer[256];
  do { ret = recv(localSocket,buffer,sizeof(buffer),0); }
  while(ret != -1);

  // turn back on blocking
  fcntl(localSocket, F_SETFL, fcntl(localSocket, F_GETFL) & (~O_NONBLOCK));
}


bool DAPHNEReader::sendCommand(char * cmd)
{
  bool retcod = false;
  //  char lf = 10;
  std::stringstream barfer;

  int m = strlen(cmd);
  for ( int i=0; i<m; i++)
  {
    barfer << cmd[i] ;
    int retcod = send(localSocket, &cmd[i], 1, 0);
    if ( retcod <= 0 )
    {
      std::stringstream line;
      line << "DAPHNEReader: Error on send socket " << 
	strerror(errno);
      TLOG_ERROR("DAPHNEReader") << line.str();
      throw std::runtime_error(line.str());
      return(retcod);
    }
    usleep(100);
  }
  //  TLOG_INFO("DAPHNEReader") << barfer.str();
  usleep(100000);
  retcod = true;

  return(retcod);
}

void DAPHNEReader::write(uint16_t address, uint16_t data)
{
  char cmd[256];

  sprintf(cmd, "WR %x %x\r", address, data);
  sendCommand(cmd);
}

void DAPHNEReader::writeLC(uint16_t address, uint16_t data)
{
  char cmd[256];

  sprintf(cmd, "LC WR %x %x\r", address, data);
  sendCommand(cmd);
}

void DAPHNEReader::writeLCAll(uint16_t address, uint16_t data)
{
  char cmd[256];

  sprintf(cmd, "LCA WR %x %x\r", address, data);
  sendCommand(cmd);
}

// Select port for subsequent link reads/writes
void DAPHNEReader::selectPort(uint16_t port)
{
  char cmd[256];

  sprintf(cmd, "LP %x\r", port);
  sendCommand(cmd);
}

// Detect and initialize links
void DAPHNEReader::linkInit()
{
  char cmd[256];

  sprintf(cmd, "LI\r");
  sendCommand(cmd);
}

// Enable Trigger mode
void DAPHNEReader::trig(uint16_t onOff)
{
  char cmd[256];

  sprintf(cmd, "TRIG %x\r", onOff);
  sendCommand(cmd);
}

// Enable Old Trigger mode
void DAPHNEReader::trigOld(uint16_t onOff)
{
  char cmd[256];

  sprintf(cmd, "TRIG_OLD %x\r", onOff);
  sendCommand(cmd);
}

// Forward data requests (?)
void DAPHNEReader::takeData()
{
  char cmd[256];

  sprintf(cmd, "UB1\r");
  sendCommand(cmd);
}


uint16_t DAPHNEReader::read(uint16_t address)
{ return(read(address,false));}

uint16_t DAPHNEReader::readLC(uint16_t address)
{ return(read(address,true));}

uint16_t DAPHNEReader::read(uint16_t address, bool LC)
{
  flushSocket();

  uint32_t data;
  char cmd[256];
  char reply[65536];
  //int responseLength;
  int length=0,size=0,p=0;
  
  // avoid set but not used compiler warning
  length = size;
  size = length;

  if ( LC  ) {  sprintf(cmd, "LC RD %x\r", address);}
  else       {  sprintf(cmd, "RD %x\r", address);}
  sendCommand(cmd);

  data = 0; p = 1;
  size = sizeof(reply);
  bzero(reply,sizeof(reply));
  reply[0] = '0';
  while ( reply[p-1] != 10 && ( p < (int)(sizeof(reply) - 1 )) && (length<4))
  {
    length = recv(localSocket,&reply[p],sizeof(char),0);
    p++;
  }

  if ( p > 0 ) { sscanf(reply,"%x", &data); }
  else         { data = -1; }
  return(data);
}

DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(sbndaq::DAPHNEReader)
