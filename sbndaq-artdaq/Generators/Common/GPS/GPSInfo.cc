//
// GPSInfo.cc
//

#include <GPSInfo.hh>
#include <iostream>
#include <string>
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>

GPSInfo::GPSInfo()
{
  gps.mtype = GPS_INFO_MTYPE;
  setenv("TZ","UTC",1);
  tzset();
  bzero(gps.location,sizeof(gps.location));
}

GPSInfo::GPSInfo(struct GPSInfo::GPSInformation gpsInfo)
{
  memcpy((void *)&gps, (void *)&gpsInfo, sizeof(gps));
}

void GPSInfo::fill(struct GPSInfo::GPSInformation gpsInfo)
{
  memcpy((void *)&gps, (void *)&gpsInfo, sizeof(gps));
}

void GPSInfo::print(std::ostream& os) 
{
  os << *this;
}

std::ostream& operator<<(std::ostream& os, GPSInfo & e)
{
  os << "GPSInfo Contents" << std::endl;
  os << "  status                " << e.gps.status << " [" 
     << e.statusString() << "]" << std::endl;
  os << "  TimeStampString       " << e.gps.timeStampString << std::endl;
  os << "  TimeStamp             " << e.gps.timeStamp << " s" << std::endl;
  os << "  SystemTime            " << e.gps.systemTime.tv_sec << " s " << e.gps.systemTime.tv_nsec << " ns" <<std::endl;
  os << "  SystemDifference      " << e.gps.systemDifference << std::endl;
  os << "  OscillatorQuality     " << e.gps.oscillatorQuality << " [" << 
    e.oscillatorString() << "]" << std::endl;
  os << "  ppsDifference         " << e.gps.ppsDifference<< std::endl;
  os << "  finePhaseComparator   " << e.gps.finePhaseComparator<< std::endl;
  os << "  message               " << e.gps.message << " [" 
     << e.messageString() << "]" <<std::endl;
  os << "  transferQuality       " << e.gps.transferQuality << " [" 
     << e.transferString() << "]" << std::endl;
  os << "  actualFrequency       " << e.gps.actualFrequency << std::endl;
  os << "  holdoverFrequency     " << e.gps.holdoverFrequency << std::endl;
  os << "  eepromFrequency       " << e.gps.eepromFrequency << std::endl;
  os << "  loopTimeConstantMode  " << e.gps.loopTimeConstantMode << std::endl;
  os << "  loopTimeConstantInUse " << e.gps.loopTimeConstantInUse << std::endl;
  os << "  sigmaPPS              " << e.gps.sigmaPPS << std::endl;
  os << "  messageStatus         " << e.gps.messageStatus << std::endl;
  os << "  latitude              " << e.gps.latitude << std::endl;
  os << "  hemisphereNS          " << e.gps.hemisphereNS << std::endl;
  os << "  longitude             " << e.gps.longitude << std::endl;
  os << "  hemisphereEW          " << e.gps.hemisphereEW << std::endl;
  os << "  location              " << e.gps.location << std::endl;
  return(os);
}

const std::string GPSInfo::locationString()
{
  double lat = std::trunc(gps.latitude / 100.);
  double lon = std::trunc(gps.longitude / 100.);
  double latMin = gps.latitude - lat*100;
  double lonMin = gps.longitude - lon*100;
  char line [256];
  sprintf(line,"%2.0lf%c%4.2lf'%c %2.0lf%c%4.2lf'%c", lat,(char)176,
	  latMin,gps.hemisphereNS,lon,(char)176,lonMin,gps.hemisphereEW);

  std::string reply(line);
  return(reply);
}

const std::string GPSInfo::statusString()
{
  std::string reply("Undefined");
  switch ( gps.status )
  {
    case 0: reply = "Warming Up"; break;
    case 1: reply = "Tracking Setup"; break;
    case 2: reply = "Track to PPSREF"; break;
    case 3: reply = "Synch to PPSREF"; break;
    case 4: reply = "Free Run"; break;
    case 5: reply = "PPSREF Unstable"; break;
    case 6: reply = "PPSREF Lost"; break;
    case 7: reply = "Freeze"; break;
    case 8: reply = "Factory Used"; break;
    case 9: reply = "Searching Rb Line"; break;
    default: break;
  }
  return(reply);
}

const std::string GPSInfo::oscillatorString()
{
  std::string reply("Undefined");
  switch ( gps.oscillatorQuality )
  {
    case 0: reply = "Warming Up"; break;
    case 1: reply = "Free Run"; break;
    case 2: reply = "Disciplined"; break;
    default: break;
  }
  return(reply);
}

const std::string GPSInfo::messageString()
{
  std::string reply("Undefined");
  switch ( gps.message )
  {
    case 0: reply = "Do not take account"; break;
    case 1: reply = "Take account, no message"; break;
    case 2: reply = "Take account, partially OK"; break;
    case 3: reply = "Take account, all OK"; break;
    default: break;
  }
  return(reply);
}

const std::string GPSInfo::transferString()
{
  std::string reply("Undefined");
  switch ( gps.transferQuality )
  {
    case 0: reply = "None"; break;
    case 1: reply = "Manual"; break;
    case 2: reply = "GPS old"; break;
    case 3: reply = "GPS fresh"; break;
    default: break;
  }
  return(reply);
}

bool GPSInfo::processLine(char * line)
{
  bool reply = false;
  if ( std::strncmp(line,"$PTNTA", 6) == 0 )
  {
    //std::cout << line << std::endl;
    parsePTNTA(std::string(line));
  }
  else if ( std::strncmp(line,"$PTNTS,B", 6) == 0 )
  {
    //std::cout << line << std::endl;
    parsePTNTSB(line);
  }
  else if ( std::strncmp(line,"$GPRMC", 6) == 0 )
  {
    //std::cout << line << std::endl;
    parseGPRMC(line);
    reply = true; // $GPRMC is the last message per PPS
  }
  return(reply);
}

void GPSInfo::parsePTNTA(std::string line)
{
  clock_gettime(CLOCK_REALTIME,&gps.systemTime); // $PTNTA arrives first, ~ 3msec after PPS
  boost::char_separator<char> sep(",");
  boost::tokenizer< boost::char_separator<char> > tok(line,sep);
  int i=0;
  BOOST_FOREACH( const std::string & t, tok ) 
  {
    switch ( i )
    {
      case 0: break;                                                // $PTNTA marker
      case 1: 
      {
	timeStampString         = t; 
	strcpy(gps.timeStampString,t.c_str());
	struct tm tm;

        strptime(timeStampString.c_str(),TIME_FORMAT,&tm);
        tm.tm_isdst = 0;
	gps.timeStamp = mktime(&tm) - GPSInfo::GPS_UTC_OFFSET;
	gps.systemDifference = gps.timeStamp - gps.systemTime.tv_sec;
	if ( gps.timeStamp >= gps.systemTime.tv_sec )
	{
	  gps.systemDifference += ((double)gps.systemTime.tv_nsec) * 1E-9;
	}
	else
	{
	  gps.systemDifference -= ((double)gps.systemTime.tv_nsec) * 1E-9;
	}

      }
      break;
      case 2: gps.oscillatorQuality   = (uint16_t)std::stoi(t); break;
      case 3: break;                                               // T4 marker
      case 4: gps.ppsDifference       = std::stoi(t); break;
      case 5: gps.finePhaseComparator = std::stoi(t); break;
      case 6: gps.status              = (uint16_t)std::stoi(t); break;
      case 7: gps.message             = (uint16_t)std::stoi(t); break;
      case 8: gps.transferQuality     = (uint16_t)std::stoi(t); break;
      default: break;
    }
    i++;
  }
}

void GPSInfo::parseGPRMC(std::string line)
{
  boost::char_separator<char> sep(",");
  boost::tokenizer< boost::char_separator<char> > tok(line,sep);
  int i=0;
  BOOST_FOREACH( const std::string & t, tok ) 
  {
    switch ( i )
    {
      case 0: break;                                                // $PTNTS,B marker
      case 1: break;                                                // Redundant time
      case 2: gps.messageStatus       = t[0]; break;
      case 3: gps.latitude            = std::stod(t); break;
      case 4: gps.hemisphereNS        = t[0]; break;
      case 5: gps.longitude           = std::stod(t); break;
      case 6: gps.hemisphereEW        = t[0]; break;
      default: break;
    }
    i++;
  }
  strcpy(gps.location,locationString().c_str());
}

uint16_t GPSInfo::hexToUint16(std::string hex)
{
  uint16_t reply;
  std::stringstream ss;
  ss << std::hex << hex;
  ss >> reply;
  return(reply);
}

void GPSInfo::parsePTNTSB(std::string line)
{
  boost::char_separator<char> sep(",");
  boost::tokenizer< boost::char_separator<char> > tok(line,sep);
  int i=0;
  BOOST_FOREACH( const std::string & t, tok ) 
  {
    switch ( i )
    {
      case 0: break;                                                // $PTNTS,B marker
      case 1: break;                                                // $PTNTS,B marker
      case 2: gps.status                = (uint16_t)std::stoi(t); break;
      case 3: gps.actualFrequency       = hexToUint16(t); break;
      case 4: gps.holdoverFrequency     = hexToUint16(t); break;
      case 5: gps.eepromFrequency       = hexToUint16(t); break;
      case 6: gps.loopTimeConstantMode  = (uint16_t)std::stoi(t); break;
      case 7: gps.loopTimeConstantInUse = (uint16_t)std::stoi(t); break;
      case 8: gps.sigmaPPS             = std::stod(t); break;
      default: break;
    }
    i++;
  }
}
