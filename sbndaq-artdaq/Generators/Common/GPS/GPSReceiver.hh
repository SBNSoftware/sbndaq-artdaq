//
//   GPSReceiver.hh
//
//   Comms between server and SpectraTime GPS Receiver device
//   (W.Badgett)
//
#ifndef _GPSRECEIVER_HH
#define _GPSRECEIVER_HH

#define BOOST_BIND_GLOBAL_PLACEHOLDERS 1
#include <cstdlib>
#include <deque>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <NetworkReceiver.hh>
#include <GPSInfo.hh>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <boost/bind/bind.hpp>

using namespace boost::placeholders;

using boost::asio::ip::tcp;

class GPSReceiver: public NetworkReceiver
{
public:
  GPSReceiver(std::string ipAddress,
	      std::string port);
  void processLine(char * line) override;

  GPSInfo gpsInfo;

  key_t daqMQ;
  key_t dcsMQ;

  int daqID;
  int dcsID;

};

#endif
