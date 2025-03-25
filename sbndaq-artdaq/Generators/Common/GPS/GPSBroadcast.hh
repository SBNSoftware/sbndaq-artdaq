//
//   GPSBroadcast.hh
//
//   Comms between server and SpectraTime GPS Broadcast device
//   (W.Badgett)
//
#ifndef _GPSBROADCAST_HH
#define _GPSBROADCAST_HH

#include <iostream>
#include <GPSInfo.hh>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pqxx/pqxx>
#include <pqxx/prepared_statement>
#include <pqxx/strconv>
#include <cadef.h>
#include <PVDescriptor.hh>

class GPSBroadcast
{
public:
  GPSBroadcast(std::string experiment,
	       std::string userName,
	       std::string passWord,
	       std::string hostName = "131.225.250.12",
	       std::string dbName   = "icarus_online_prd",
	       int port = 5434);

protected:
  pqxx::connection * dbConnection;
  const char * insertGPS;
  key_t dcsMQ;
  int dcsID;
  GPSInfo gps;
  std::vector<std::string> pvNames;
  std::vector<PVDescriptor> pv;
  struct GPSInfo::GPSInformation gpsInfo;
};

#endif
