
#include "PTB_Receiver.hh"
#define TNAME "PTB_Receiver"
#include "sbndaq-artdaq-core/Trace/trace_defines.h"
#include "sbndaq-artdaq-core/Overlays/SBND/PTB_content.h"

#include <ctime>

#include <boost/asio/read.hpp>



PTB_Receiver::PTB_Receiver( const unsigned int port,
			    const unsigned int timeout,
			    const unsigned int buffer_size ) :

  _word_buffer( buffer_size ), _port ( port ), _timeout(timeout), 
  _n_TS_words( 0 ),  _error_state( false ),
  _has_calibration_stream( false )

{ ; }

PTB_Receiver::~PTB_Receiver() {

  stop() ;

}



bool PTB_Receiver::stop() {

  _stop_requested = true ;

  if ( _word_fut.valid() ) {
    _word_fut.get() ;
  }

  return true ;

}


bool PTB_Receiver::start() {

  _stop_requested = false ;

  _word_buffer.reset() ;

  _n_TS_words = 0 ;

  _word_fut = std::async( std::launch::async, & PTB_Receiver::_word_receiver, this ) ;

  return true ;
}


int PTB_Receiver::_word_receiver() {

  std::size_t n_bytes = 0 ;
  std::size_t n_words = 0 ;

  const size_t header_size = sizeof( ptb::content::tcp_header_t ) ;
  const size_t word_size = ptb::content::word::word_t::size_bytes ;

  TLOG_INFO(TNAME) << "Header size: " << header_size << TLOG_ENDL
		   << "Word size: " << word_size << TLOG_ENDL ;

  //connect to socket
  boost::asio::io_service io_service;
  boost::asio::ip::tcp::acceptor acceptor(io_service, boost::asio::ip::tcp::endpoint( boost::asio::ip::tcp::v4(), _port ) );
  boost::asio::ip::tcp::socket socket(io_service);

  TLOG_INFO(TNAME) << "Watiting for an incoming connection on port " << _port << TLOG_ENDL;

  std::future<void> accepting = async( std::launch::async, [&]{ acceptor.accept(socket) ; } ) ;

  while ( ! _stop_requested.load() ) {
    if ( accepting.wait_for( _timeout ) == std::future_status::ready )
      break ;
  }

  TLOG_INFO(TNAME) << "Connection received: start reading" << TLOG_ENDL;

  ptb::content::tcp_header_t head ;
  ptb::content::word::word_t temp_word ;

  boost::system::error_code receiving_error;
  bool connection_closed = false ;

  while ( ! _stop_requested.load() ) {

    _update_calibration_file() ;

    if ( ! _read( head, socket ) ) {
      connection_closed = true ;
      break;
    }

    n_bytes = head.packet_size ;
    TLOG_INFO(TNAME) << "Package size "  << n_bytes << TLOG_ENDL ;

    // extract n_words
    n_words = n_bytes / word_size ;

    // read n words as requested from the header
    for ( unsigned int i = 0 ; i < n_words ; ++i ) {

      //read a word
      if ( ! _read( temp_word, socket ) ) {
        connection_closed = true ;
        break ;
      }

      // put it in the caliration stream
      if ( _has_calibration_stream ) {

        _calibration_file.write( reinterpret_cast<const char*>( & temp_word ), word_size ) ;
	_calibration_file.flush() ;
        TLOG_INFO(TNAME) << "Word Type: " << temp_word.word_type << TLOG_ENDL ;

      } 	 // word printing in calibration stream

      //check if it is a TS word and increment the counter
      if ( PTB_Receiver::IsTSWord( temp_word ) ) {
        _n_TS_words++ ;
      }
      else if ( PTB_Receiver::IsFeedbackWord( temp_word ) ) {
	_error_state.store( true ) ;
        ptb::content::word::feedback_t * feedback = reinterpret_cast<ptb::content::word::feedback_t*>( & temp_word ) ;

/*        TLOG_ERROR(TNAME) << "Feedback word: " << TLOG_ENDL
						  << std::hex 
						  << " \t Type -> " << feedback -> word_type << TLOG_ENDL 
						  << " \t TS -> " << feedback -> timestamp << TLOG_ENDL
						  << " \t Code -> " << feedback -> code << TLOG_ENDL
						  << " \t Source -> " << feedback -> source << TLOG_ENDL
						  << " \t Padding -> " << feedback -> padding << std::dec << TLOG_ENDL ;
*/
      }


      // push the word
      while ( ! _word_buffer.push( temp_word ) && ! _stop_requested.load() ) {
        TLOG_WARNING(TNAME) << "Word Buffer full and cannot store more" << TLOG_ENDL ;
      }

      if ( _stop_requested.load() ) break ;

    } // n_words loop

    if ( connection_closed )
      break ;

  }  // stop not requested

  boost::system::error_code closing_error;

  if ( _error_state.load() ) {
    socket.shutdown( boost::asio::ip::tcp::socket::shutdown_send, closing_error );
    if ( closing_error ) {
      TLOG_ERROR(TNAME) << "Error in shutdown: " << closing_error.message() << TLOG_ENDL ;
    }
  }

  socket.close( closing_error ) ;
  if ( closing_error ) {
    TLOG_ERROR(TNAME) << "Socket closing failed: " << closing_error.message() << TLOG_ENDL ;
  }

  TLOG_INFO(TNAME) << "Connection closed: stop receiving data from the PTB" << TLOG_ENDL ;
  // return because _stop_requested
  return 0 ;
}


bool PTB_Receiver::SetCalibrationStream( const std::string & string_dir,
					 const std::chrono::minutes & interval, 
					 const std::string & prefix ) {

  _calibration_dir = string_dir ;
  if ( _calibration_dir.back() != '/' ) _calibration_dir += '/' ;

  _calibration_prefix = prefix ; 
  if ( prefix.size() > 0 ) 
    _calibration_prefix += '_' ; 

  _calibration_file_interval = interval ;

  // possibly we could check here if the directory is valid and  writable before assuming the calibration stream is valid
  return _has_calibration_stream = true ;

}


bool PTB_Receiver::IsTSWord( const ptb::content::word::word_t & w ) noexcept {

  TLOG_INFO(TNAME) << "word type " <<  w.word_type  << TLOG_ENDL ;

  if ( w.word_type == ptb::content::word::t_ts ) {

    return true ;

  }

  return false ;

}


bool PTB_Receiver::IsFeedbackWord( const ptb::content::word::word_t & w ) noexcept {

  if ( w.word_type == ptb::content::word::t_fback ) {

    return true ;

  }

  return false ;

}




void PTB_Receiver::_init_calibration_file() {

  if ( ! _has_calibration_stream ) return ;

  char file_name[200] = "" ;

  time_t rawtime;
  time( & rawtime ) ;

  struct tm * timeinfo = localtime( & rawtime ) ;

  strftime( file_name, sizeof(file_name), "%F_%H.%M.%S.calib", timeinfo );

  std::string global_name = _calibration_dir + _calibration_prefix + file_name ;

  _calibration_file.open( global_name, std::ofstream::binary ) ;

  _last_calibration_file_update = std::chrono::steady_clock::now();

  // _calibration_file.setf ( std::ios::hex, std::ios::basefield );
  // _calibration_file.unsetf ( std::ios::showbase );

  TLOG_DEBUG(TNAME) << "New Calibration Stream file: " << global_name << TLOG_ENDL ;

}


void PTB_Receiver::_update_calibration_file() {

  if ( ! _has_calibration_stream ) return ;

  std::chrono::steady_clock::time_point check_point = std::chrono::steady_clock::now();

  if ( check_point - _last_calibration_file_update < _calibration_file_interval ) return ;

  _calibration_file.close() ;

  _init_calibration_file() ;

}


template<class T>
bool PTB_Receiver::_read( T & obj , boost::asio::ip::tcp::socket & socket ) {

  boost::system::error_code receiving_error;

  // std::future<std::size_t> reading = async( std::launch::async,
  // 					    [&]{ return boost::asio::read( socket,
  // 									   boost::asio::buffer( obj, sizeof(T) ),
  // 									   receiving_error ) ; } ) ;

  // while( ! _stop_requested.load() ) {

  //   if ( reading.wait_for( _timeout ) != std::future_status::ready ) {
  //     continue ;
  //   }

  // }

  boost::asio::read( socket, boost::asio::buffer( & obj, sizeof(T) ), receiving_error ) ;

  if ( ! receiving_error ) 
    return true ;

  if ( receiving_error == boost::asio::error::eof) {

    TLOG_INFO(TNAME) << "Socket closed: "<< receiving_error.message()  << TLOG_ENDL ;
    return false ;
  }
  
  if ( receiving_error ) {
    TLOG_ERROR(TNAME) <<"Read failure: " << receiving_error.message() << TLOG_ENDL ;
    return false ;
  }

  return true ;

}
