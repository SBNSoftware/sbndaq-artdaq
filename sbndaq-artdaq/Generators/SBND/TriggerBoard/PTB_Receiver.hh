#ifndef SBNDAQ_ARTDAQ_PTB_RECEIVER_HH
#define SBNDAQ_ARTDAQ_PTB_RECEIVER_HH

#include <string>
#include <fstream>
#include <atomic>
#include <chrono>
#include <future>
#include <ratio>

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/lockfree/spsc_queue.hpp>

#include "fhiclcpp/fwd.h"
#include "sbndaq-artdaq-core/Overlays/SBND/PTB_content.h"

/*

  The PTB receiver is a a buffer in the form of a SPSC queue.
  The _receiver method create the server and whenever data are received they are pushed in the buffer

  The calling thread of the PTB_Receiver is supposed to pop these elements out
  using pop


*/ 


class PTB_Receiver {

public:
  PTB_Receiver( ) = delete ;

  // the buffer size is the number of word contained in the buffer
  PTB_Receiver( const unsigned int port, const unsigned int timeout = 10 /*microseconds*/, const unsigned int buffer_size = 5000 ) ;

  virtual ~PTB_Receiver() ;

  auto & Buffer() { return _word_buffer ; }

  auto & N_TS_Words() { return _n_TS_words ; }  
  
  bool SetCalibrationStream( const std::string & string_dir, 
			     const std::chrono::minutes & interval, 
			     const std::string & prefix = "" ) ; 

  bool stop() ; 
  bool start() ;

  static bool IsTSWord( const ptb::content::word::word_t & w ) noexcept ;
  static bool IsFeedbackWord( const ptb::content::word::word_t & w ) noexcept ;

  bool ErrorState() const { return _error_state.load() ; } 

private:

  // the raw buffer can contain 4 times the maximum TCP package size, which is 4 KB
  boost::lockfree::spsc_queue< ptb::content::word::word_t > _word_buffer ;  
  
  // this is the receiver thread to be called
  int _word_receiver() ;

  void _update_calibration_file() ;
  void _init_calibration_file() ;

  template<class T>
  bool _read( T & obj , boost::asio::ip::tcp::socket & socket ) ;

  //  std::future<int> _raw_fut ;
  std::future<int> _word_fut ;

  const unsigned int _port ;

  std::chrono::microseconds _timeout ;

  std::atomic<unsigned int> _n_TS_words ;

  std::atomic<bool> _stop_requested ;
  std::atomic<bool> _error_state ;

  // members related to calibration stream
  bool _has_calibration_stream ; 
  std::string _calibration_dir ; 
  std::string _calibration_prefix ; 
  std::chrono::minutes _calibration_file_interval ;  
  std::ofstream _calibration_file   ;
  std::chrono::steady_clock::time_point _last_calibration_file_update ;



};


#endif
