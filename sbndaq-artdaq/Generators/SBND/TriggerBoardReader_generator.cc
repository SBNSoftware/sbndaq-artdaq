#define TNAME "TriggerBoard"

#include <stdexcept>

#include "json/json.h"
#include "json/reader.h"

#include "sbndaq-artdaq/Generators/SBND/TriggerBoardReader.hh"
#include "artdaq/Generators/GeneratorMacros.hh"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "trace.h"
#include "artdaq/DAQdata/Globals.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"

#include "sbndaq-artdaq-core/Overlays/SBND/PTB_content.h"
#include "sbndaq-artdaq-core/Overlays/SBND/PTBFragment.hh"

#include "artdaq-core/Utilities/SimpleLookupPolicy.hh"

#include "canvas/Utilities/Exception.h"
#include "cetlib_except/exception.h"
#include "fhiclcpp/ParameterSet.h"

#include <fstream>
#include <iomanip>
#include <iterator>
#include <iostream>
#include <string>

#include <unistd.h>



sbndaq::TriggerBoardReader::TriggerBoardReader(fhicl::ParameterSet const & ps)
  :
  CommandableFragmentGenerator(ps),
  _run_controller(), _receiver(), _timeout(), _stop_requested(false), _error_state(false),
  _fragment_buffer( ps.get<unsigned int>( "data_buffer_depth_fragments", 1000 ) ),
  throw_exception_(ps.get<bool>("throw_exception",true) )
{
  //get board address and control port from the fhicl file
  const unsigned int control_port = ps.get<uint16_t>("control_port", 8991 ) ;
  const std::string address = ps.get<std::string>( "board_address", "sbnd-ptbmk2-01" );
  TLOG_INFO(TNAME) << "PTB at " << control_port << "@" << address << TLOG_ENDL;

  // get options for fragment creation
  _group_size = ps.get<unsigned int>( "group_size", 1 ) ;
  TLOG_INFO(TNAME) << "Creating fragments with " << _group_size << TLOG_ENDL;

  unsigned int word_buffer_size = ps.get<unsigned int>( "word_buffer_size", 5000 ) ;
  _max_words_per_frag = word_buffer_size / 5 * 4 ;  // 80 % of the buffer size


  //build the ptb_controller
  _run_controller.reset( new PTB_Controller( address, control_port ) ) ;


  // prepare the receiver

  // get the json configuration string
  std::stringstream json_stream( ps.get<std::string>( "board_config" ) ) ;

  Json::Value jblob;
  json_stream >> jblob ;

  // get the receiver port from the json
  const unsigned int receiver_port = jblob["ctb"]["sockets"]["receiver"]["port"].asUInt() ;

  _rollover = jblob["ctb"]["sockets"]["receiver"]["rollover"].asUInt() ;

  const unsigned int timeout_scaling = ps.get<uint16_t>("receiver_timeout_scaling", 2 ) ;

  const unsigned int timeout = _rollover / 50 / timeout_scaling; //microseconds
  //                                      |-> this is the board clock frequency which is 50 MHz

  _timeout = std::chrono::microseconds( timeout ) ;

  //build the ptb receiver and set its connecting port
  _receiver.reset( new PTB_Receiver( receiver_port, timeout, word_buffer_size ) ) ;

  // if necessary, set the calibration stream
  if ( ps.has_key( "calibration_stream_output") ) {
    _has_calibration_stream = true ; 
    _calibration_dir = ps.get<std::string>( "calibration_stream_output") ;
    _calibration_update = std::chrono::minutes( ps.get<unsigned int>( "calibration_update", 5 )  ) ; 
  }

  if ( ps.has_key( "run_trigger_output") ) {
    _has_run_trigger_report = true ; 
    _run_trigger_dir = ps.get<std::string>( "run_trigger_output") ;
    if ( _run_trigger_dir.back() != '/' ) _run_trigger_dir += '/' ;

  }

  // Receive hostname or IP address from config
  boost::asio::io_service io_service;
  boost::asio::ip::tcp::resolver resolver(io_service);  

  const std::string receiver_address = ps.get<std::string>("boardreader_address");
  // require the private hostname
  std::string priv ("-daq");

  TLOG_INFO(TNAME) << "Host name is " << receiver_address << TLOG_ENDL;
  std::for_each(resolver.resolve({receiver_address, ""}), {}, [](const auto& re) {
      TLOG_INFO(TNAME) << "Host IP address: " << re.endpoint().address() << TLOG_ENDL;
  });

  jblob["ctb"]["sockets"]["receiver"]["host"] = receiver_address ;

  TLOG_INFO(TNAME) << "Board packages recieved at " << receiver_address << ", port:" << receiver_port << TLOG_ENDL;

  // create the json string
  json_stream.str("");
  json_stream.clear() ;

  json_stream << jblob ;
  std::string config = json_stream.str() ;

  // send the configuration
  bool config_status = _run_controller -> send_config( config ) ;

 if ( ! config_status ) {
   TLOG_ERROR(TNAME) << "PTB failed to configure" << TLOG_ENDL;

   if ( throw_exception_ ) {
     throw std::runtime_error("PTB failed to configure") ;
    }
  }

 // metric parameters configuration
 _metric_TS_max = ps.get<unsigned int>( "metric_TS_interval", (unsigned int) (1. * TriggerBoardReader::PTB_Clock() / _rollover )  ) ; // number of TS words in which integrate the frequencies of the words. Default value the number of TS word in a second

 _cherenkov_coincidence = ps.get<unsigned int>( "cherenkov_coincidence", 25  ) ;

}

sbndaq::TriggerBoardReader::~TriggerBoardReader() {

  stop() ;

}

bool sbndaq::TriggerBoardReader::getNext_(artdaq::FragmentPtrs & frags) {

  if ( should_stop() ) {
    return false;
  }

  if ( _error_state.load() ||  _receiver -> ErrorState() ) {
    if ( throw_exception_ ) {
      TLOG_ERROR(TNAME) << "PTB in error state, shutting down" << TLOG_ENDL;
      stop() ;
      throw std::runtime_error("PTB sent a feedback word") ;
    }
  }

  std::size_t n_fragments = _fragment_buffer.read_available() ;
  if ( n_fragments == 0 ) {
    TLOG( 20, "TriggerBoardReader") << "getNext_ returns as no fragments are available" << std::endl ;
    return ! _error_state.load() ;
  }

  long unsigned int sent_bytes = 0 ;

  artdaq::Fragment* temp_frag = nullptr ;

  for ( std::size_t i = 0; i < n_fragments ; ++i ) {

    _fragment_buffer.pop( temp_frag ) ;

    if ( ! temp_frag ) continue ;

    sent_bytes += temp_frag -> dataSizeBytes() ;

    frags.emplace_back( temp_frag ) ;

  }

  TLOG( 20, "TriggerBoardReader") << "Sending " << frags.size() <<  " fragments" << std::endl ;

  if( artdaq::Globals::metricMan_ ) {
    artdaq::Globals::metricMan_->sendMetric("PTB_Fragments_Sent", (double) frags.size(), "Fragments", 11, artdaq::MetricMode::Average) ;
    artdaq::Globals::metricMan_->sendMetric("PTB_Bytes_Sent",     (double) sent_bytes,   "Bytes",     11, artdaq::MetricMode::Average) ;
  }

  return ! _error_state.load() ;
}


artdaq::Fragment* sbndaq::TriggerBoardReader::CreateFragment() {

  static ptb::content::word::word_t temp_word ;

  static const constexpr std::size_t word_bytes = sizeof( ptb::content::word::word_t ) ;

  const std::size_t n_words = _receiver -> Buffer().read_available() ;

  std::size_t initial_bytes = n_words * word_bytes ;

  artdaq::Fragment::timestamp_t timestamp = artdaq::Fragment::InvalidTimestamp ;
  bool has_TS = false ;

  if ( _has_last_TS ) {
    has_TS = true ;
    timestamp = _last_timestamp ;
  }

  unsigned int word_counter = 0 ;
  unsigned int group_counter = 0 ;

  artdaq::Fragment* fragptr = artdaq::Fragment::FragmentBytes( initial_bytes ).release() ;

  for ( word_counter = 0 ; word_counter < n_words ; ) {

    _receiver -> Buffer().pop( temp_word ) ;
    memcpy( fragptr->dataBeginBytes() + word_counter * word_bytes, & temp_word, word_bytes ) ;
    ++word_counter ;
    ++_metric_Word_counter ;

    if ( _close_to_good_part ) {
      if ( temp_word.timestamp_ns() != 0 ) {
	update_cherenkov_counter( temp_word.timestamp_ns() ) ;
      }
    }

    if ( _is_beam_spill ) {
    
      if ( temp_word.timestamp_ns() != 0 ) {
	
	if ( temp_word.timestamp_ns() > _spill_start + _spill_width ) {
	  TLOG( 20, "TriggerBoardReader") << "End of a beam spill at: " <<  temp_word.timestamp_ns() << " => Started at " << _spill_start << std::endl ;
	  _is_beam_spill = false ; 
	  
	  // publish the dedicated metrics
	  if ( artdaq::Globals::metricMan_ ) {
	    
	    artdaq::Globals::metricMan_->sendMetric("PTB_Spill_H0L0", (double) _metric_spill_h0l0_counter, "Particles", 11, artdaq::MetricMode::Accumulate ) ;
	    artdaq::Globals::metricMan_->sendMetric("PTB_Spill_H0L1", (double) _metric_spill_h0l1_counter, "Particles", 11, artdaq::MetricMode::Accumulate ) ;
	    artdaq::Globals::metricMan_->sendMetric("PTB_Spill_H1L0", (double) _metric_spill_h1l0_counter, "Particles", 11, artdaq::MetricMode::Accumulate ) ;
	    artdaq::Globals::metricMan_->sendMetric("PTB_Spill_H1L1", (double) _metric_spill_h1l1_counter, "Particles", 11, artdaq::MetricMode::Accumulate ) ;
	    
	  } // if there is a metric manager      
	  
	}  // if the beam is over
	
      }  // if valid timestamp
      
    } // if we were in a beam spill
    
    if ( PTB_Receiver::IsTSWord( temp_word ) ) {
      
      --(_receiver -> N_TS_Words()) ;
      ++group_counter ;
      
      if ( temp_word.timestamp_ns() != 0 ) {
	_last_timestamp = temp_word.timestamp_ns() ;
	_has_last_TS = true ;
	
	if ( ! has_TS ) {
	  has_TS = true ;
	  timestamp = _last_timestamp ;
	}
      }

      // increment _metric TS counters
      ++ _metric_TS_counter ;
      
      if ( _metric_TS_counter == _metric_TS_max ) {
	
	if( artdaq::Globals::metricMan_ ) {
	  // evaluate word rates
	  double word_rate = _metric_Word_counter * TriggerBoardReader::PTB_Clock() / _metric_TS_max / _rollover ;
	  double hlt_rate  = _metric_HLT_counter * TriggerBoardReader::PTB_Clock() / _metric_TS_max / _rollover ;
	  double llt_rate  = _metric_LLT_counter * TriggerBoardReader::PTB_Clock() / _metric_TS_max / _rollover ;

	  double beam_rate = _metric_beam_trigger_counter * TriggerBoardReader::PTB_Clock() / _metric_TS_max / _rollover ;
	  //double good_part_rate = _metric_good_particle_counter * TriggerBoardReader::PTB_Clock() / _metric_TS_max / _rollover ;
	  //double beam_eff = _metric_good_particle_counter != 0 ? _metric_beam_trigger_counter / (double) _metric_good_particle_counter : 1. ;

	  //double cs_rate   = _metric_CS_counter  * TriggerBoardReader::PTB_Clock() / _metric_TS_max / _rollover ;

	  // publish metrics
	  artdaq::Globals::metricMan_->sendMetric("PTB_Word_rate", word_rate, "Hz", 11, artdaq::MetricMode::Average) ;

	  artdaq::Globals::metricMan_->sendMetric("PTB_HLT_rate", hlt_rate, "Hz", 11, artdaq::MetricMode::Average) ;
	  artdaq::Globals::metricMan_->sendMetric("PTB_LLT_rate", llt_rate, "Hz", 11, artdaq::MetricMode::Average) ;

	  artdaq::Globals::metricMan_->sendMetric("PTB_Beam_Trig_rate", beam_rate, "Hz", 11, artdaq::MetricMode::Average) ;
	  //artdaq::Globals::metricMan_->sendMetric("PTB_Good_Part_rate", good_part_rate, "Hz", 11, artdaq::MetricMode::Average) ;

	  //artdaq::Globals::metricMan_->sendMetric("PTB_Beam_Eff", beam_eff, "ratio", 11, artdaq::MetricMode::Average) ;

	  //artdaq::Globals::metricMan_->sendMetric("PTB_CS_rate",  cs_rate,  "Hz", 11, artdaq::MetricMode::Average) ;

	  for ( unsigned short i = 0 ; i < _metric_HLT_names.size() ; ++i ) {
	    double temp_rate = _metric_HLT_counters[i] * TriggerBoardReader::PTB_Clock() / _metric_TS_max / _rollover ;
            if(i < 5 || i >= 29 || i == 20 || i == 21) { //is event or flash trigger (excl calibration) or crt reset
	      artdaq::Globals::metricMan_->sendMetric( _metric_HLT_names[i], temp_rate,  "Hz", 11, artdaq::MetricMode::Average) ;
            }
            else if(i == 5) { //calibration
	      artdaq::Globals::metricMan_->sendMetric( _metric_HLT_names[i], temp_rate,  "Hz", 12, artdaq::MetricMode::Average) ;
            }
            else { //spare level is 20
	      artdaq::Globals::metricMan_->sendMetric( _metric_HLT_names[i], temp_rate,  "Hz", 20, artdaq::MetricMode::Average) ;
            }
	  }
	  for ( unsigned short i = 0 ; i < _metric_LLT_names.size() ; ++i ) {
	    double temp_rate = _metric_LLT_counters[i] * TriggerBoardReader::PTB_Clock() / _metric_TS_max / _rollover ;
	    artdaq::Globals::metricMan_->sendMetric( _metric_LLT_names[i], temp_rate,  "Hz", 11, artdaq::MetricMode::Average) ;
	  }


	}  // if there is a metric manager

	// transfer HLT counters to run counters
	//<--_run_gool_part_counter += _metric_good_particle_counter ;
	_run_HLT_counter += _metric_HLT_counter ; 
	_run_LLT_counter += _metric_LLT_counter ; 
	
	// reset counters
	_metric_TS_counter =
	  _metric_Word_counter =
	  _metric_HLT_counter =
	  _metric_LLT_counter =
	  _metric_beam_trigger_counter = 0 ;
	  //_metric_good_particle_counter =
	  //_metric_CS_counter  = 0 ;

	for ( unsigned short i = 0 ; i < _metric_HLT_names.size() ; ++i ) {
	  _run_HLT_counters[i] += _metric_HLT_counters[i] ;
	  _metric_HLT_counters[i] = 0 ;
	}
	for ( unsigned short i = 0 ; i < _metric_LLT_names.size() ; ++i ) {
	  _run_LLT_counters[i] += _metric_LLT_counters[i] ;
	  _metric_LLT_counters[i] = 0 ;
	}

      }  // if it is necessary to publish the metric

      if ( _group_size > 0 ) {
	if ( group_counter >= _group_size ) {
	  TLOG( 20, "TriggerBoardReader") << "Final group_counter " << group_counter << std::endl ;
	  break ;
	}
      }

    } // if this was a TS word


    else if ( temp_word.word_type == ptb::content::word::t_lt ) {
      ++ _metric_LLT_counter ;

      const ptb::content::word::trigger_t * t = reinterpret_cast<const ptb::content::word::trigger_t *>( & temp_word  ) ;

      std::set<unsigned short> trigs = t -> Triggers(32) ;
      for ( auto it = trigs.begin(); it != trigs.end() ; ++it ) {
	++ _metric_LLT_counters[*it] ;
      }
/*
      if ( t -> IsTrigger(1) ) {
	_close_to_good_part = true ; 

	if ( t -> timestamp > _latest_part_TS + 2*_cherenkov_coincidence )  
	  _latest_part_TS = t -> timestamp ;
	++ _metric_good_particle_counter ;
      }  // this was a LLT_1


      // always fill the cherenkov counter because 
      // we have to evaluate a coincidence 
      // and we cannot know if a good particle is about to come
      if ( t -> IsTrigger(2) ) {
	_hp_TSs.insert( t -> timestamp ) ; 
      }  // if it is a high pressure cherenkov LLT

      if ( t -> IsTrigger(3) ) {
	_lp_TSs.insert( t -> timestamp ) ; 
      }  // if it is a low pressure cherenkov LLT
*/
      if ( ! _is_beam_spill ) {

	if ( t -> IsTrigger(6) )  {

	  TLOG( 20, "TriggerBoardReader") << "Start of a beam spill at: " <<  t -> timestamp << std::endl ;

	  _is_beam_spill = true ; 
	  _spill_start = t -> timestamp ;
	
	  // so reset spill counters 
	  _metric_spill_h0l0_counter = 
	    _metric_spill_h0l1_counter = 
	    _metric_spill_h1l0_counter = 
	    _metric_spill_h1l1_counter = 0 ;
	  
	}  // is trigger 6 
      }  // we were not in beam spill
      
    }  // if this was a LLT

    else if ( temp_word.word_type == ptb::content::word::t_gt ) {
      ++ _metric_HLT_counter ;

      const ptb::content::word::trigger_t * t = reinterpret_cast<const ptb::content::word::trigger_t *>( & temp_word  ) ;

      if ( t -> trigger_word & 0xEE )  // request at least a trigger not cosmic trigger nor random triggers
	++ _metric_beam_trigger_counter ;    // count beam related HLT

      std::set<unsigned short> trigs = t -> Triggers(32) ;
      for ( auto it = trigs.begin(); it != trigs.end() ; ++it ) {
	++ _metric_HLT_counters[*it] ;
      }
    }  // if this was a HLT

    //else if (  temp_word.word_type == ptb::content::word::t_ch )
    //  ++ _metric_CS_counter ;

    else if ( PTB_Receiver::IsFeedbackWord( temp_word ) ) {
      TLOG_ERROR(TNAME) << "PTB issued a feedback word" << TLOG_ENDL;
      const ptb::content::word::feedback_t * f = reinterpret_cast<const ptb::content::word::feedback_t *>( & temp_word  ) ;
      TLOG_ERROR(TNAME) << "PTB issued a feedback word!"<< TLOG_ENDL;
      TLOG_ERROR(TNAME) << "Feedback word Code: "       << f -> code      << TLOG_ENDL;
      TLOG_ERROR(TNAME) << "Feedback word Source: "     << f -> source    << TLOG_ENDL;
      TLOG_ERROR(TNAME) << "Feedback word Payload: "    << f -> payload   << TLOG_ENDL;
      TLOG_ERROR(TNAME) << "Feedback word Timestamp: "  << f -> timestamp << TLOG_ENDL;
      _error_state.store( true ) ;
    }

  }


  fragptr -> resizeBytes( word_counter * word_bytes ) ;
  fragptr -> setUserType( detail::FragmentType::PTB ) ;
  fragptr -> setSequenceID( ev_counter_inc() ) ;
  fragptr -> setFragmentID( fragment_id() ) ;

  fragptr -> setTimestamp( timestamp ) ;
  TLOG( 20, "TriggerBoardReader") << "fragment created with TS " << timestamp << " containing " << word_counter << " words" << std::endl ;

  if ( ! has_TS ) {
    TLOG( 20, "TriggerBoardReader") << "Created fragment with invalid TimeStamp" << std::endl ;
  }

  return fragptr ;

}


int sbndaq::TriggerBoardReader::_FragmentGenerator() {

  int frag_counter = 0 ;

  artdaq::Fragment* temp_ptr ;

  while ( ! _stop_requested.load() ) {

    if ( CanCreateFragments() || NeedToEmptyBuffer() ) {

      temp_ptr = CreateFragment() ;

      while ( ! _fragment_buffer.push( temp_ptr ) ) {
	TLOG_WARNING(TNAME) << "Fragment Buffer full: does not accept more fragments" << TLOG_ENDL;
	std::this_thread::sleep_for( _timeout ) ;
      }

      ++frag_counter ;

    }
    else {
      std::this_thread::sleep_for( _timeout ) ;
    }

    if ( should_stop() ) break ;

  }

  return frag_counter ;
}



void sbndaq::TriggerBoardReader::start() {

  _stop_requested = false ;

  _is_beam_spill = false ; 

  _close_to_good_part = false ; 

  _hp_TSs.clear() ;
  _lp_TSs.clear() ;

  //<--_run_gool_part_counter = 0 ;
  _run_HLT_counter = 0 ;
  for ( unsigned int i = 0 ; i < _metric_HLT_names.size() ; ++i ) {
    _run_HLT_counters[i] = 0 ; 
  }
  _run_LLT_counter = 0 ;
  for ( unsigned int i = 0 ; i < _metric_LLT_names.size() ; ++i ) {
    _run_LLT_counters[i] = 0 ; 
  }

  if ( _has_calibration_stream ) {
    std::stringstream run;
    run << "run" << run_number();
    _receiver -> SetCalibrationStream( _calibration_dir ,
                                       _calibration_update, 
				       run.str() ) ;
  }

  _frag_future =  std::async( std::launch::async, & TriggerBoardReader::_FragmentGenerator,  this ) ;

  _receiver -> start() ;

  _run_controller -> send_start() ;

}

void sbndaq::TriggerBoardReader::stop() {

  _stop_requested = true ;

  //note the order here is crucial!
  // for efficiency reasons, the receiver is locking the stop until the
  // Board closes the connection.
  // This won't happen until you send the stop to the board first.
  // so, first, send the stop to the board, only then, stop the receiver and the fragment creator

  _run_controller -> send_stop() ;

  _receiver -> stop() ;

  if ( _frag_future.valid() ) {
    _frag_future.wait() ;
    TLOG_INFO(TNAME) << "Created " << _frag_future.get() << " fragments" << TLOG_ENDL;
  }

  ResetBuffer() ;

  store_run_trigger_counters( run_number() ) ; 

}


void sbndaq::TriggerBoardReader::ResetBuffer() {

  _fragment_buffer.consume_all( [](auto* p){ delete p ; } ) ;

}


void sbndaq::TriggerBoardReader::update_cherenkov_buffer( std::set<artdaq::Fragment::timestamp_t> & buffer ) {

  // remove old stuff
  
  while ( buffer.size() > 0 ) {
    if ( _latest_part_TS > _cherenkov_coincidence + *buffer.begin() ) {
      buffer.erase( buffer.begin() ) ;
    }
    else break ; 
  } 
 
}


void sbndaq::TriggerBoardReader::update_cherenkov_counter( const artdaq::Fragment::timestamp_t & latest ) {


  update_cherenkov_buffer( _hp_TSs ) ;
  update_cherenkov_buffer( _lp_TSs ) ;

  if ( latest > _cherenkov_coincidence + _latest_part_TS ) {
    _close_to_good_part = false ;

    if ( _hp_TSs.size() > 0 && _lp_TSs.size() > 0 ) {
      ++ _metric_spill_h1l1_counter ; 
    }
    else if ( _hp_TSs.size() == 0 && _lp_TSs.size() == 0 ) {
      ++ _metric_spill_h0l0_counter ;
    }
    else if ( _hp_TSs.size() > 0 && _lp_TSs.size() == 0 ) {
      ++ _metric_spill_h1l0_counter ;
    }
    else if ( _hp_TSs.size() == 0 && _lp_TSs.size() > 0 ) {
      ++ _metric_spill_h0l1_counter ;
    }

  }

  
}


bool sbndaq::TriggerBoardReader::store_run_trigger_counters( unsigned int run_number, const std::string & prefix ) const {

  if ( ! _has_run_trigger_report ) {
    return false ;
  }

  std::stringstream out_name ;
  out_name << _run_trigger_dir << prefix << "run_" << run_number << "_triggers.txt";
  std::ofstream out( out_name.str() ) ;

  out << "Total \t " << _run_HLT_counter << std::endl ;
  for ( unsigned int i = 0; i < _metric_HLT_names.size() ; ++i ) {
    out << "HLT " << i << " \t " << _run_HLT_counters[i] << std::endl ;
  }
  out << "Total \t " << _run_LLT_counter << std::endl ;
  for ( unsigned int i = 0; i < _metric_LLT_names.size() ; ++i ) {
    out << "LLT " << i << " \t " << _run_LLT_counters[i] << std::endl ;
  }


  return true ; 
}


// The following macro is defined in artdaq's GeneratorMacros.hh header
DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(sbndaq::TriggerBoardReader)
