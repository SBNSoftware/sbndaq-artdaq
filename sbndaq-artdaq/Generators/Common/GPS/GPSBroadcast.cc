//
//   GPSBroadcast.cc
//
//   Fan out GPSInfo to DCS and db
//   (W.Badgett)
//

#include <GPSBroadcast.hh>
#include <iostream>
#include <string>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <boost/algorithm/string.hpp>


void callBack(struct connection_handler_args args)
{
  std::cout << "CallBack " << args.chid << std::endl;
  return;
}

GPSBroadcast::GPSBroadcast(std::string experiment,
			   std::string userName,
			   std::string passWord,
			   std::string hostName,
			   std::string dbName,
			   int port)
{
  int retcod;
  char hemiNS[2];
  char hemiEW[2];
  char message[2];

  hemiNS[1]  = '\0';
  hemiEW[1]  = '\0';
  message[1] = '\0';

  pvNames.push_back(experiment+"_GPS_GPS_0/status");   // 0
  pvNames.push_back(experiment+"_GPS_GPS_0/timeStamp"); // 1
  pvNames.push_back(experiment+"_GPS_GPS_0/systemDifference"); // 2
  pvNames.push_back(experiment+"_GPS_GPS_0/oscillatorQuality"); // 3 
  pvNames.push_back(experiment+"_GPS_GPS_0/ppsDifference"); // 4
  pvNames.push_back(experiment+"_GPS_GPS_0/finePhaseComparator"); // 5
  pvNames.push_back(experiment+"_GPS_GPS_0/message"); // 6
  pvNames.push_back(experiment+"_GPS_GPS_0/transferQuality"); // 7 
  pvNames.push_back(experiment+"_GPS_GPS_0/actualFrequency"); // 8 
  pvNames.push_back(experiment+"_GPS_GPS_0/holdoverFrequency"); // 9
  pvNames.push_back(experiment+"_GPS_GPS_0/eepromFrequency"); // 10
  pvNames.push_back(experiment+"_GPS_GPS_0/loopTimeConstantMode"); // 11
  pvNames.push_back(experiment+"_GPS_GPS_0/loopTimeConstantInUse");// 12
  pvNames.push_back(experiment+"_GPS_GPS_0/sigmaPPS"); // 13
  pvNames.push_back(experiment+"_GPS_GPS_0/messageStatus");// 14
  pvNames.push_back(experiment+"_GPS_GPS_0/latitude"); // 15 
  pvNames.push_back(experiment+"_GPS_GPS_0/hemisphereNS");// 16
  pvNames.push_back(experiment+"_GPS_GPS_0/longitude");// 17
  pvNames.push_back(experiment+"_GPS_GPS_0/hemisphereEW");// 18
  pvNames.push_back(experiment+"_GPS_GPS_0/location");// 19
  pvNames.push_back(experiment+"_GPS_GPS_0/TimeStampString");// 20
  pvNames.push_back(experiment+"_GPS_GPS_0/systemTimeSec");// 21
  pvNames.push_back(experiment+"_GPS_GPS_0/systemTimeNSec");// 22

  //std::cout << pvNames << std::endl;

  retcod = ca_context_create(ca_enable_preemptive_callback);
  SEVCHK(retcod,"ca_context_create error");  

  retcod = ca_pend_io(1.0);
  SEVCHK(retcod,"ca_pend_io error");  

  for (unsigned int i=0;i<pvNames.size(); i++)
  {
    chid channel;
    std::cout << pvNames[i] << std::endl;
    retcod = ca_create_channel(pvNames[i].c_str(), NULL, NULL, 99, &channel);
    SEVCHK(retcod,"ca_create_channel error");  

    int dataType = 0;
    switch ( i )
    {
      case 0: dataType = DBR_LONG;  break; // status
      case 1: dataType = DBR_LONG;   break; // timeStamp
      case 2: dataType = DBR_FLOAT; break; // systemDifference
      case 3:  // oscillatorDifference
      case 4:  // ppsDifference
      case 5:  // finePhaseComparator
      case 6:  // message
      case 7:  // transferQuality
      case 8:  // actualFrequency
      case 9:  // holdoverFrequency
      case 10: // eepromFrequency
      case 11: // loopTimeConstantMode
      case 12: // loopTimeConstantInUse
        dataType = DBR_LONG;     break; 
      case 13: dataType = DBR_FLOAT;  break; // sigmaPPS
      case 14: dataType = DBR_CHAR;   break; // messageStatus
      case 15: dataType = DBR_FLOAT;  break; // latitude
      case 16: dataType = DBR_CHAR;   break; // hemisphereNS
      case 17: dataType = DBR_FLOAT;  break; // longitude
      case 18: dataType = DBR_CHAR;   break; // hemisphereEW
      case 19: dataType = DBR_STRING; break; // location
      case 20: dataType = DBR_STRING; break; // timeStampString
      case 21:                               // systemTimeSec
      case 22:                               // systemTimeNSec
	dataType = DBR_LONG; break;
    }
    PVDescriptor desc(pvNames[i], dataType, channel);
    pv.push_back(desc);
    printf("PV:        %s\n", ca_name(channel));
    printf("DataType:  %d\n", pv[i].dataType);
    printf("State:     %d\n", ca_state(channel));
    printf("Host       %s\n", ca_host_name(channel));
    printf("Read:      %d\n", ca_read_access(channel));
    printf("Write:     %d\n", ca_write_access(channel));
    printf("Type:      %s\n", dbr_type_to_text(ca_field_type(channel)));
    printf("Count:     %d\n", (int)ca_element_count(channel));
  }

  dcsMQ = 0xAD0;
  dcsID = msgget(dcsMQ,0);
  if ( dcsID < 0 )
  {
    printf("Error msgget %X [%s]\n", dcsMQ,strerror(errno));
    exit(errno); 
  }

  insertGPS = "INSERT INTO GPS_STATUS (" 
    "timeStamp,"               //  1 
    "systemTimeSec,"           //  2
    "systemTimeNSec,"          //  3
    "systemDifference,"        //  4
    "status,"                  //  5
    "oscillatorQuality,"       //  6 
    "ppsDifference,"           //  7 
    "finePhaseComparator,"     //  8 
    "message,"                 //  9 
    "transferQuality,"         // 10
    "actualFrequency,"         // 11
    "holdoverFrequency,"       // 12 
    "eepromFrequency,"         // 13
    "loopTimeConstantMode,"    // 14 
    "loopTimeConstantInUse,"   // 15 
    "sigmaPPS,"                // 16
    "messageStatus,"           // 17
    "latitude,"                // 18
    "hemisphereNS,"            // 19
    "longitude,"               // 20
    "hemisphereEW"             // 21
    ") VALUES ("       
    "$1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11,$12,$13,$14,$15,$16,$17,$18,$19,$20,$21)";

  std::cout << insertGPS << std::endl;
  char config[1024];
  sprintf(config,"dbname=%s user=%s password=%s hostaddr=%s port=%d",
	    dbName.c_str(),userName.c_str(),passWord.c_str(),hostName.c_str(),port);
  std::cout << config << std::endl;
  try
  {
    dbConnection = new pqxx::connection(config);
    if ( dbConnection->is_open())
    {
      std::cout << "Connected to "<< dbName << " at " << port << "@" << hostName << std::endl;
      dbConnection->prepare("insertGPS",insertGPS);
    }
    else
    {
      std::cout << "Failed to connect to "<< dbName << " at " << port << "@" << hostName << std::endl;
      dbConnection->disconnect();
      exit(-1);
    }
  }
  catch ( const std::exception &e )
  {
    std::cout << "Failed to connect to "<< dbName << " at " << port << "@" << hostName << std::endl;
    std::cout << e.what() << std::endl;
    exit(-1);
  }

  ssize_t nBytes=1;
  retcod=0;
  while ( nBytes > 0 )
  {
    nBytes = msgrcv(dcsID,(void *)&gpsInfo,sizeof(gpsInfo), GPSInfo::GPS_INFO_MTYPE, 0);
    if ( nBytes <=0 )
    {
      printf("Error msgrcv %X (%d) [%s]\n", dcsMQ,errno, 
	     strerror(errno));
    }
    else
    {
      gps.fill(gpsInfo);
      //      gps.print();

      // Broadcast values over EPICS
      for (unsigned int i=0; i<pv.size(); ++i)
      {

	int   intData = 0;
	char  charData = '\0';
	float floatData = 0.0;
	char * charPtr = NULL;

	switch ( i )
	{
   	  case 0:  intData   = gpsInfo.data.status; break;
	  case 1:  intData   = gpsInfo.data.timeStamp;   break;
     	  case 2:  floatData = gpsInfo.data.systemDifference; break;
	  case 3:  intData   = gpsInfo.data.oscillatorQuality; break;
	  case 4:  intData   = gpsInfo.data.ppsDifference; break;
	  case 5:  intData   = gpsInfo.data.finePhaseComparator; break;
	  case 6:  intData   = gpsInfo.data.message; break;
	  case 7:  intData   = gpsInfo.data.transferQuality; break;
	  case 8:  intData   = gpsInfo.data.actualFrequency; break;
	  case 9:  intData   = gpsInfo.data.holdoverFrequency; break;
	  case 10: intData   = gpsInfo.data.eepromFrequency; break;
	  case 11: intData   = gpsInfo.data.loopTimeConstantMode; break;
	  case 12: intData   = gpsInfo.data.loopTimeConstantInUse; break;
          case 13: floatData = gpsInfo.data.sigmaPPS; break;
	  case 14: charData  = gpsInfo.data.messageStatus; break;
	  case 15: floatData = gpsInfo.data.latitude; break;
	  case 16: charData  = gpsInfo.data.hemisphereNS; break;
	  case 17: floatData = gpsInfo.data.longitude; break;
	  case 18: charData  = gpsInfo.data.hemisphereEW; break;
	  case 19: charPtr   = gpsInfo.data.location; break;
	  case 20: charPtr   = gpsInfo.data.timeStampString; break;
	  case 21: intData   = gpsInfo.data.systemTime.tv_sec; break;
	  case 22: intData   = gpsInfo.data.systemTime.tv_nsec; break;
	}


	uint32_t count = 1;
	if ( pv[i].dataType == DBR_LONG )
	{
	  //retcod = ca_put(pv[i].dataType,pv[i].channel,&intData);
	  retcod = ca_array_put(pv[i].dataType,count,pv[i].channel,&intData);
	  { std::cout << i << " " << pv[i].name << " I " << intData << " " << std::endl;}
	}
	else if ( pv[i].dataType == DBR_CHAR )
	{
	  retcod = ca_put(pv[i].dataType,pv[i].channel,&charData);
	  { std::cout << i << " " << pv[i].name << " C " << charData << " " << std::endl;}
	}
	else if ( pv[i].dataType == DBR_FLOAT )
	{
	  retcod = ca_put(pv[i].dataType,pv[i].channel,&floatData);
	  { std::cout << i << " " << pv[i].name << " F " << floatData << " " << std::endl;}
	}	
	else if ( pv[i].dataType == DBR_STRING )
	{
	  retcod = ca_put(pv[i].dataType,pv[i].channel,&charPtr);
	  { std::cout << i << " " << pv[i].name << " S " << charPtr << " " << std::endl;}
	}	
	SEVCHK(retcod,"ca_put error");
      }
      ca_flush_io();

      // Write values to runcon database
      hemiNS[0]  = gps.gps.data.hemisphereNS;
      hemiEW[0]  = gps.gps.data.hemisphereEW;
      message[0] = gps.gps.data.messageStatus;

      pqxx::work work(*dbConnection);
      pqxx::result result = work.prepared("insertGPS")
	(gps.gps.data.timeStamp)               
	(gps.gps.data.systemTimer.tv_sec)      
	(gps.gps.data.systemTimer.tv_nsec)     
	(gps.gps.data.systemDifference)        
	(gps.gps.data.status)                  
	(gps.gps.data.oscillatorQuality)       
	(gps.gps.data.ppsDifference)           
	(gps.gps.data.finePhaseComparator)     
	(gps.gps.data.message)                 
	(gps.gps.data.transferQuality)         
	(gps.gps.data.actualFrequency)         
	(gps.gps.data.holdoverFrequency)       
	(gps.gps.data.eepromFrequency)         
	(gps.gps.data.loopTimeConstantMode)    
	(gps.gps.data.loopTimeConstantInUse)   
	(gps.gps.data.sigmaPPS)                
	(message)           
	(gps.gps.data.latitude)                
	(hemiNS)            
	(gps.gps.data.longitude)               
	(hemiEW)            
	.exec();        
      work.commit();
    }
  }
  dbConnection->disconnect();
}

int main(int argc, char* argv[])
{
  if ( argc < 2 ) 
  {
    std::cout << "Expected one argument:" << std::endl;
    std::cout << "  gps-bcast <experiment, ICARUS or SBND>" 
	       << std::endl << std::endl;
    exit(-1);
  }

  FILE * ptr = fopen(".epics/.epics","r");
  std::string user, pass;
  if ( ptr != NULL )
  {
    char *u = (char *)malloc(80); 
    char *p = (char *)malloc(80);
    size_t i80 = 80;
    getline(&u,&i80,ptr);
    getline(&p,&i80,ptr);
    user = u;
    pass = p;
    boost::algorithm::trim(user);
    boost::algorithm::trim(pass);

  }

  GPSBroadcast gps(argv[1],user,pass);

}


