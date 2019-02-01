//
//   NetworkReceiver.hh
//
//   Comms between server and a serial type Networked device
//   (W.Badgett)
//
#ifndef _NETWORKRECEIVER_HH
#define _NETWORKRECEIVER_HH

#include <cstdlib>
#include <deque>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>

enum
{
  PORT = 9001
};
 
using boost::asio::ip::tcp;

class NetworkReceiver
{
public:
  NetworkReceiver(std::string ipAddress, std::string port);
  virtual ~NetworkReceiver();
  size_t write(std::string line);
  void readLauncher();
  virtual void processLine(char * line) { std::cout << line << std::endl; };

protected:
  void readHandler(const boost::system::error_code & error,
		   std::size_t nBytes);
  boost::asio::io_service ioService;
  boost::asio::ip::tcp::socket  socket;
  char lf;
  boost::asio::streambuf outputData;
  boost::asio::streambuf inputData;
  boost::system::error_code error;
  boost::thread * ioThread;
  size_t nBytes;
};

#endif
