#ifndef SBNDAQ_ARTDAQ_CTB_CONTROLLER_HH
#define SBNDAQ_ARTDAQ_CTB_CONTROLLER_HH

#include <string>
#include <atomic>

#include <boost/asio.hpp>
#include <boost/array.hpp>

#include "fhiclcpp/fwd.h"
#include "sbndaq-artdaq-core/Overlays/SBND/CTB_content.h"



class CTB_Controller {

public:

  CTB_Controller( const std::string &host = "sbnd-ptbmk2-01",const uint16_t &port = 8991 ) ;
  //  CTB_Controller( fhicl::ParameterSet const & ps ) ;
  virtual ~CTB_Controller() ;

  // the methods rely on the return of send_message()
  bool send_reset() ;
  bool send_start() ;
  bool send_stop() ;
  bool send_config( const std::string & config ) ;
  //void send_config( const std::string &host = "np04-srv-013",const uint16_t &port = 8992, unsigned long rollover = 50000 ) ;
  //void process_quit() ;

protected:

  // this returns false if an error is detected in the response to the message.
  // otherwise it returns true
  bool send_message( const std::string & msg ) ;


private:

  boost::asio::io_service _ios;
  boost::asio::ip::tcp::endpoint _endpoint;
  boost::asio::ip::tcp::socket _socket ;
  
  // control variables
  std::atomic<bool> stop_req_;
  std::atomic<bool> is_running_;
  bool is_conf_;


  // Aux vars
  

};


#endif
