//
//   NetworkReceiver.cc
//
//   Comms between server and a generic serial type network device
//   (W.Badgett)
//

#include <NetworkReceiver.hh>
#include <iostream>
#include <string>

NetworkReceiver::NetworkReceiver(std::string ipAddress,
				 std::string port)
  : ioService(),
    socket(ioService)
{
  tcp::resolver resolver(ioService);
  tcp::resolver::query query(ipAddress,port);
  boost::asio::connect(socket,resolver.resolve(query),error);
  std::cout << "Connected " << port << "@" <<ipAddress << " " << error
	    << std::endl;

  lf = 13;

  //ioService.run();
  ioThread = new boost::thread(boost::bind(&boost::asio::io_service::run,
					   &ioService));

  // Start up the infinite read loop with a kick start
  readLauncher();
}

NetworkReceiver::~NetworkReceiver()
{
  ioThread->interrupt();
  delete(ioThread);
}

//
//  Keep reading lines from Network forever
//  
void NetworkReceiver::readLauncher()
{
  boost::asio::async_read_until(socket,
				inputData,
				"\r\n",
  				boost::bind(&NetworkReceiver::readHandler,
  					    this,
  					    error, nBytes)
  				);
}

size_t NetworkReceiver::write(std::string line)
{
  std::ostream out(&outputData);
  out << line << lf;
  size_t nBytes = boost::asio::write(socket,outputData,error);
  // std::cout << "Wrote " << line << " " << error << std::endl;
  return(nBytes);
}

void NetworkReceiver::readHandler(const boost::system::error_code & retcod,
				  std::size_t byteCount)
{
  char line [256];

  error = retcod;
  nBytes = byteCount;
  std::istream input(&inputData);
  input.getline(line,sizeof(line));
  readLauncher();
  processLine(line);
}

