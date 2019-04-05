#include "PTB_Controller.hh"

#include <sstream>

//#include "dune-artdaq/DAQLogger/DAQLogger.hh" //replaced by traces
//#include "sbndaq-artdaq/DAQLogger/DAQLogger.hh" //replaced by traces
#include "sbndaq-artdaq-core/Trace/trace_defines.h"

//#include "json/json.h"
//#include "json/reader.h"

//#include <json/json.h>
//#include <json/reader.h>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/reader.h>


PTB_Controller::PTB_Controller( const std::string & host, const uint16_t & port ) :
  _ios(),
  _endpoint(),
  _socket( _ios ) ,
  stop_req_( false ),
  is_running_ (false), 
  is_conf_(false)
{
  
  boost::asio::ip::tcp::resolver resolver( _ios );
  
  boost::asio::ip::tcp::resolver::query query(host, std::to_string(port) ) ; 
  boost::asio::ip::tcp::resolver::iterator iter = resolver.resolve(query) ;
 
  _endpoint = iter->endpoint() ; 
  
  //shoudl we put this into a try?
  _socket.connect( _endpoint ) ;

    
}


PTB_Controller::~PTB_Controller() { 


  //check if running. and in case stop the run
  send_stop(); 

  _socket.close() ;

}

bool PTB_Controller::send_reset() {

  //sbndaq::DAQLogger::LogInfo("PTB_Controller") << "Sending a reset" << std::endl;

  bool ret = send_message( "{\"command\":\"HardReset\"}" ) ;

  // decide what to do with errors

  stop_req_.store( false ) ;
  is_running_.store( false ) ;
  is_conf_ = false ;

  return ret ;
}

bool PTB_Controller::send_start() {

  if (! is_conf_) {
    ///sbndaq::DAQLogger::LogError("PTB_Controller") << "ERROR: Can't start a run without configuring first" << std::endl;
    return false ;
  }

  ///sbndaq::DAQLogger::LogInfo("PTB_Controller") << "Sending a start run" << std::endl;

  if ( send_message( "{\"command\":\"StartRun\"}" )  ) {
    
    is_running_.store(true);
    return true ;
  }

  return false ;

}

bool PTB_Controller::send_stop() {

  if ( ! is_running_.load() ) {
    ///sbndaq::DAQLogger::LogWarning("PTB_Controller") << "Stop requested while not runninng" ;
    return true ;
  }

  ///sbndaq::DAQLogger::LogInfo("PTB_Controller") << "Sending a stop run" << std::endl;

  bool ret = send_message( "{\"command\":\"StopRun\"}" ) ;

  is_running_.store( false ) ;
  stop_req_.store( true ) ;

  return ret ;
}



bool PTB_Controller::send_config( const std::string & config ) {
  
  if ( is_conf_ ) {
    ///sbndaq::DAQLogger::LogInfo("PTB_Controller") << "Resetting before configuring" << std::endl;
    send_reset();
  }

  ///sbndaq::DAQLogger::LogInfo("PTB_Controller") << "Sending config" << std::endl;

  if ( send_message( config ) ) {

    is_conf_ = true ;
    return true ;
  }

  return false ;
  
}



bool PTB_Controller::send_message( const std::string & msg ) {

  //add error options                                                                                                
  boost::system::error_code error;

  ///sbndaq::DAQLogger::LogInfo("PTB_Controller") << "Sending message: " << msg << std::endl;
  
  boost::asio::write( _socket, boost::asio::buffer( msg ), error ) ;

  boost::array<char, 1024> reply_buf{" "} ;

  _socket.read_some( boost::asio::buffer(reply_buf ), error);

  std::stringstream raw_answer( std::string(reply_buf .begin(), reply_buf .end() ) ) ;
  
  ///sbndaq::DAQLogger::LogDebug("PTB_Controller") << "Unformatted answer: " << std::endl << raw_answer.str() << std::endl; 

  Json::Value answer ;
  raw_answer >> answer ;

  Json::Value & messages = answer["feedback"] ;

  ///sbndaq::DAQLogger::LogInfo("PTB_Controller") << "Received messages: " << messages.size() << std::endl;

  bool ret = true ;

  for ( Json::Value::ArrayIndex i = 0; i != messages.size(); ++i ) {

    std::string type = messages[i]["type"].asString() ;
    
    if ( type.find("error") != std::string::npos || type.find("Error") != std::string::npos || type.find("ERROR") != std::string::npos ) {
     /// sbndaq::DAQLogger::LogError("PTB_Controller") << "Error from the Board: " << messages[i]["message"].asString() << std::endl ;
      ret = false ;
    }
    else if ( type.find("warning") != std::string::npos || type.find("Warning") != std::string::npos || type.find("WARNING") != std::string::npos ) {
      ///sbndaq::DAQLogger::LogWarning("PTB_Controller") << "Warning from the Board: " << messages[i]["message"].asString() << std::endl;
    }
    else if ( type.find("info") != std::string::npos || type.find("Info") != std::string::npos || type.find("INFO") != std::string::npos) {
      ///sbndaq::DAQLogger::LogInfo("PTB_Controller") << "Info from the board: " << messages[i]["message"].asString() << std::endl;
    }
    else {
      std::stringstream blob;
      blob << messages[i] ;
      ///sbndaq::DAQLogger::LogInfo("PTB_Controller") << "Unformatted from the board: " << blob.str() << std::endl;
    }
  }
  
 
  return ret ;
}
