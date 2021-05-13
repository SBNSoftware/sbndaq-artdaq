//
//   GPSInfo.hh
//
//   Status information from SpectraTime GPS
//   (W.Badgett)
//
#ifndef _GPSINFO_HH
#define _GPSINFO_HH

#define BOOST_BIND_GLOBAL_PLACEHOLDERS 1
#include <boost/bind/bind.hpp>
using namespace boost::placeholders;


#include <cstdlib>
#include <deque>
#include <iostream>
#include <boost/tokenizer.hpp>
#include <sbndaq-artdaq-core/Overlays/Common/SpectratimeEventFragment.hh>
#include <NetworkReceiver.hh>

class GPSInfo 
{
public:

  GPSInfo();

  // This GPS_UTC_OFFSET number is available in register 27 on the GPS
  // Should we read it from the device?
  // Is that number automatically updated for leap seconds?
  enum 
  {
    GPS_UTC_OFFSET = 16,
    GPS_INFO_MTYPE = 0xCDF
  };

#pragma pack(push,1)
  typedef struct GPSInformation
  {
    long mtype;
    struct sbndaq::SpectratimeEvent data;
  } GPSInformation_t;


#pragma pack(pop)

  GPSInfo(struct GPSInformation gpsInfo);
  void fill(struct GPSInformation gpsInfo);
  struct GPSInformation gps;
  std::string timeStampString;

  bool processLine(char * line);
  void parsePTNTA(std::string line);
  void parseGPRMC(std::string line);
  void parsePTNTSB(std::string line);
  const std::string statusString();
  const std::string oscillatorString();
  const std::string messageString();
  const std::string transferString();
  const std::string locationString();
  uint16_t hexToUint16(std::string hex);
  void print(std::ostream & os = std::cout);
  static constexpr const char *TIME_FORMAT="%Y%m%d%H%M%S%z";
};

std::ostream& operator<<(std::ostream& os,  GPSInfo& e);

#endif
