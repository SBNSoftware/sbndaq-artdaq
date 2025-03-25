#include "PTB_Controller.hh"
#define TNAME "PTB_Controller"

#include <sstream>

#include "sbndaq-artdaq-core/Trace/trace_defines.h"
#include "json/json.h"
#include "json/reader.h"

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

  TLOG_INFO(TNAME) << "Sending a reset" << TLOG_ENDL;

  bool ret = send_message( "{\"command\":\"HardReset\"}" ) ;

  // decide what to do with errors

  stop_req_.store( false ) ;
  is_running_.store( false ) ;
  is_conf_ = false ;

  return ret ;
}

bool PTB_Controller::send_start() {

  if (! is_conf_) {
    TLOG_ERROR(TNAME) << "ERROR: Can't start a run without configuring first" << TLOG_ENDL;
    return false ;
  }

  TLOG_INFO(TNAME) << "Sending a start run" << TLOG_ENDL;

  if ( send_message( "{\"command\":\"StartRun\"}" )  ) {
    
    is_running_.store(true);
    return true ;
  }

  return false ;

}

bool PTB_Controller::send_stop() {

  if ( ! is_running_.load() ) {
    TLOG_WARNING(TNAME) << "Stop requested while not runninng" ;
    return true ;
  }

  TLOG_INFO(TNAME) << "Sending a stop run" << TLOG_ENDL;

  bool ret = send_message( "{\"command\":\"StopRun\"}" ) ;

  is_running_.store( false ) ;
  stop_req_.store( true ) ;

  return ret ;
}



bool PTB_Controller::send_config( const std::string & config ) {
  
  if ( is_conf_ ) {
    TLOG_INFO(TNAME) << "Resetting before configuring" << TLOG_ENDL;
    send_reset();
  }

  TLOG_INFO(TNAME) << "Sending config" << TLOG_ENDL;

  if ( send_message( config ) ) {

    is_conf_ = true ;
    return true ;
  }

  return false ;
  
}



bool PTB_Controller::send_message( const std::string & msg ) {

  //add error options                                                                                                
  boost::system::error_code error;

  TLOG_INFO(TNAME) << "Sending message: " << msg << TLOG_ENDL;
  
  boost::asio::write( _socket, boost::asio::buffer( msg ), error ) ;

  boost::array<char, 1024> reply_buf{" "} ;

  _socket.read_some( boost::asio::buffer(reply_buf ), error);

  std::stringstream raw_answer( std::string(reply_buf .begin(), reply_buf .end() ) ) ;
  
  TLOG_DEBUG(TNAME)  << "Unformatted answer: " << TLOG_ENDL << raw_answer.str() << TLOG_ENDL; 

  Json::Value answer ;
  raw_answer >> answer ;

  Json::Value & messages = answer["feedback"] ;

  TLOG_INFO(TNAME) << "Received messages: " << messages.size() << TLOG_ENDL;

  bool ret = true ;

  for ( Json::Value::ArrayIndex i = 0; i != messages.size(); ++i ) {

    std::string type = messages[i]["type"].asString() ;
    
    if ( type.find("error") != std::string::npos || type.find("Error") != std::string::npos || type.find("ERROR") != std::string::npos ) {
      TLOG_ERROR(TNAME) << "Error from the Board: " << messages[i]["message"].asString() << TLOG_ENDL ;
      ret = false ;
    }
    else if ( type.find("warning") != std::string::npos || type.find("Warning") != std::string::npos || type.find("WARNING") != std::string::npos ) {
      TLOG_WARNING(TNAME) << "Warning from the Board: " << messages[i]["message"].asString() << TLOG_ENDL;
    }
    else if ( type.find("info") != std::string::npos || type.find("Info") != std::string::npos || type.find("INFO") != std::string::npos) {
      TLOG_INFO(TNAME) << "Info from the board: " << messages[i]["message"].asString() << TLOG_ENDL;
    }
    else {
      std::stringstream blob;
      blob << messages[i] ;
      TLOG_INFO(TNAME) << "Unformatted from the board: " << blob.str() << TLOG_ENDL;
    }
  }
  
 
  return ret ;
}
