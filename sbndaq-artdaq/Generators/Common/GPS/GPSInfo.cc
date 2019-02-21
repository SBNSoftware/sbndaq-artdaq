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
  bzero(gps.data.location,sizeof(gps.data.location));
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
  os << "  status                " << e.gps.data.status << " [" 
     << e.statusString() << "]" << std::endl;
  os << "  TimeStampString       " << e.gps.data.timeStampString << std::endl;
  os << "  TimeStamp             " << e.gps.data.timeStamp << " s" << std::endl;
  os << "  SystemTime            " << e.gps.data.systemTime.tv_sec << " s " << e.gps.data.systemTime.tv_nsec << " ns" <<std::endl;
  os << "  SystemDifference      " << e.gps.data.systemDifference << std::endl;
  os << "  OscillatorQuality     " << e.gps.data.oscillatorQuality << " [" << 
    e.oscillatorString() << "]" << std::endl;
  os << "  ppsDifference         " << e.gps.data.ppsDifference<< std::endl;
  os << "  finePhaseComparator   " << e.gps.data.finePhaseComparator<< std::endl;
  os << "  message               " << e.gps.data.message << " [" 
     << e.messageString() << "]" <<std::endl;
  os << "  transferQuality       " << e.gps.data.transferQuality << " [" 
     << e.transferString() << "]" << std::endl;
  os << "  actualFrequency       " << e.gps.data.actualFrequency << std::endl;
  os << "  holdoverFrequency     " << e.gps.data.holdoverFrequency << std::endl;
  os << "  eepromFrequency       " << e.gps.data.eepromFrequency << std::endl;
  os << "  loopTimeConstantMode  " << e.gps.data.loopTimeConstantMode << std::endl;
  os << "  loopTimeConstantInUse " << e.gps.data.loopTimeConstantInUse << std::endl;
  os << "  sigmaPPS              " << e.gps.data.sigmaPPS << std::endl;
  os << "  messageStatus         " << e.gps.data.messageStatus << std::endl;
  os << "  latitude              " << e.gps.data.latitude << std::endl;
  os << "  hemisphereNS          " << e.gps.data.hemisphereNS << std::endl;
  os << "  longitude             " << e.gps.data.longitude << std::endl;
  os << "  hemisphereEW          " << e.gps.data.hemisphereEW << std::endl;
  os << "  location              " << e.gps.data.location << std::endl;
  return(os);
}

const std::string GPSInfo::locationString()
{
  double lat = std::trunc(gps.data.latitude / 100.);
  double lon = std::trunc(gps.data.longitude / 100.);
  double latMin = gps.data.latitude - lat*100;
  double lonMin = gps.data.longitude - lon*100;
  char line [256];
  sprintf(line,"%2.0lf%c%4.2lf'%c %2.0lf%c%4.2lf'%c", lat,(char)176,
	  latMin,gps.data.hemisphereNS,lon,(char)176,lonMin,gps.data.hemisphereEW);

  std::string reply(line);
  return(reply);
}

const std::string GPSInfo::statusString()
{
  std::string reply("Undefined");
  switch ( gps.data.status )
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
  switch ( gps.data.oscillatorQuality )
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
  switch ( gps.data.message )
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
  switch ( gps.data.transferQuality )
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
  clock_gettime(CLOCK_REALTIME,&gps.data.systemTime); // $PTNTA arrives first, ~ 3msec after PPS
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
	strcpy(gps.data.timeStampString,t.c_str());
	struct tm tm;

        strptime(timeStampString.c_str(),TIME_FORMAT,&tm);
        tm.tm_isdst = 0;
	gps.data.timeStamp = mktime(&tm) - GPSInfo::GPS_UTC_OFFSET;
	gps.data.systemDifference = gps.data.timeStamp - gps.data.systemTime.tv_sec;
	if ( gps.data.timeStamp >= gps.data.systemTime.tv_sec )
	{
	  gps.data.systemDifference += ((double)gps.data.systemTime.tv_nsec) * 1E-9;
	}
	else
	{
	  gps.data.systemDifference -= ((double)gps.data.systemTime.tv_nsec) * 1E-9;
	}

      }
      break;
      case 2: gps.data.oscillatorQuality   = (uint16_t)std::stoi(t); break;
      case 3: break;                                               // T4 marker
      case 4: gps.data.ppsDifference       = std::stoi(t); break;
      case 5: gps.data.finePhaseComparator = std::stoi(t); break;
      case 6: gps.data.status              = (uint16_t)std::stoi(t); break;
      case 7: gps.data.message             = (uint16_t)std::stoi(t); break;
      case 8: gps.data.transferQuality     = (uint16_t)std::stoi(t); break;
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
      case 2: gps.data.messageStatus       = t[0]; break;
      case 3: gps.data.latitude            = std::stod(t); break;
      case 4: gps.data.hemisphereNS        = t[0]; break;
      case 5: gps.data.longitude           = std::stod(t); break;
      case 6: gps.data.hemisphereEW        = t[0]; break;
      default: break;
    }
    i++;
  }
  strcpy(gps.data.location,locationString().c_str());
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
      case 2: gps.data.status                = (uint16_t)std::stoi(t); break;
      case 3: gps.data.actualFrequency       = hexToUint16(t); break;
      case 4: gps.data.holdoverFrequency     = hexToUint16(t); break;
      case 5: gps.data.eepromFrequency       = hexToUint16(t); break;
      case 6: gps.data.loopTimeConstantMode  = (uint16_t)std::stoi(t); break;
      case 7: gps.data.loopTimeConstantInUse = (uint16_t)std::stoi(t); break;
      case 8: gps.data.sigmaPPS             = std::stod(t); break;
      default: break;
    }
    i++;
  }
}
