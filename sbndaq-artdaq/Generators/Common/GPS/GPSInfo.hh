//
//   GPSInfo.hh
//
//   Status information from SpectraTime GPS
//   (W.Badgett)
//
#ifndef _GPSINFO_HH
#define _GPSINFO_HH

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

    // $PTNTA
    uint32_t timeStamp;
    uint16_t oscillatorQuality;
    int32_t  ppsDifference;        // ns
    int32_t  finePhaseComparator;  // ns
    uint16_t status;
    uint16_t message;
    uint16_t transferQuality;

    // $PTNTS,B
    uint16_t actualFrequency; // steps of 5.12e-13
    uint16_t holdoverFrequency; // steps of 5.12e-13
    uint16_t eepromFrequency; // steps of 5.12e-13
    uint16_t loopTimeConstantMode; // 0:fixed value, 1:automatic
    uint16_t loopTimeConstantInUse; // 100 to 999999 seconds
    float sigmaPPS; // in approx ns

    // $GPRMC
    char messageStatus; // A: valid; V: not valid
    float latitude;    // XXYY.YYYY  X=Degrees  Y=minutes
    char hemisphereNS;  // N or S
    float longitude;   // XXYY.YYYY  X=Degrees  Y=minutes
    char hemisphereEW;  // E or W

    // MISC
    struct timespec systemTime;
    struct timespec systemTimer;
    float   systemDifference;
    char location[22];  // 41°50.31'N 88°16.17'W
    char timeStampString[15];  // 20181207181920
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
