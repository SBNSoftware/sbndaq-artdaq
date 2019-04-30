#ifndef sbndaq_artdaq_Generators_SBND_TriggerBoardReader_hh
#define sbndaq_artdaq_Generators_SBND_TriggerBoardReader_hh

// Some C++ conventions used:

// -Append a "_" to every private member function and variable

#include "fhiclcpp/fwd.h"
#include "artdaq-core/Data/Fragment.hh" 
#include "artdaq/Application/CommandableFragmentGenerator.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"

#include <boost/lockfree/spsc_queue.hpp>

#include "TriggerBoard/PTB_Controller.hh"
#include "TriggerBoard/PTB_Receiver.hh"

#include <random>
#include <vector>
#include <array>
#include <atomic>
#include <chrono>

namespace sbndaq {    

  class TriggerBoardReader : public artdaq::CommandableFragmentGenerator {
  public:
    explicit TriggerBoardReader(fhicl::ParameterSet const & ps);
    ~TriggerBoardReader();

    // useful information
    static double PTB_Clock() { return 50e6 ; }  

  private:

    // The "getNext_" function is used to implement user-specific
    // functionality; it's a mandatory override of the pure virtual
    // getNext_ function declared in CommandableFragmentGenerator
    bool getNext_(artdaq::FragmentPtrs & output) override;

    // The start, stop and stopNoMutex methods are declared pure
    // virtual in CommandableFragmentGenerator and therefore MUST be
    // overridden; note that stopNoMutex() doesn't do anything here

    void start() override;
    void stop() override;
    void stopNoMutex() override {}

    // threads
    int _FragmentGenerator() ;

    // utilities
    artdaq::Fragment* CreateFragment() ;
    void ResetBuffer() ;
    unsigned int _rollover ; 

    //conditions to generate a Fragments
    bool CanCreateFragments() ; 
    bool NeedToEmptyBuffer() ; 

    // members
    std::unique_ptr<PTB_Controller> _run_controller ; 
    std::unique_ptr<PTB_Receiver>   _receiver ;
    

    //multi thread parameters
    std::chrono::microseconds _timeout ;
    std::atomic<bool> _stop_requested ;
    std::atomic<bool> _error_state ;
    std::future<int> _frag_future ;

    boost::lockfree::spsc_queue<artdaq::Fragment* > _fragment_buffer ; 

    // fragment creation parameters
    unsigned int _group_size ;
    unsigned int _max_words_per_frag ;    

    bool _has_last_TS = false ;
    artdaq::Fragment::timestamp_t _last_timestamp = artdaq::Fragment::InvalidTimestamp ;

    bool throw_exception_;

    // metric parameters
    unsigned int _metric_TS_max ;
    unsigned int _metric_TS_counter = 0 ; 
    unsigned int _metric_Word_counter = 0 ; 

    unsigned int _metric_HLT_counter = 0 ;  // this is not the sum of the specific HLT_n, 
                                            // it is the counter of the HLT word
    unsigned int _metric_HLT_counters[8] = {0} ;
    unsigned int _metric_LLT_counter = 0 ; 

    unsigned int _metric_beam_trigger_counter = 0 ;
    unsigned int _metric_good_particle_counter = 0 ;  //LLT_1

    unsigned int _metric_CS_counter = 0 ;  // channel status


    // total trigger counter
    //<--unsigned long _run_gool_part_counter = 0;
    unsigned long _run_HLT_counter = 0;
    unsigned long _run_HLT_counters[8] = {0};

    // spill parameters
    const uint64_t _spill_width = 240000000 ;  //4.8 s
    artdaq::Fragment::timestamp_t _spill_start ;
    bool _is_beam_spill = false ;
    unsigned int _metric_spill_h0l0_counter = 0;
    unsigned int _metric_spill_h0l1_counter = 0;
    unsigned int _metric_spill_h1l0_counter = 0;
    unsigned int _metric_spill_h1l1_counter = 0;

    bool _close_to_good_part = false ;
    artdaq::Fragment::timestamp_t _latest_part_TS ;
    std::set<artdaq::Fragment::timestamp_t> _hp_TSs; // high pressure TS
    std::set<artdaq::Fragment::timestamp_t> _lp_TSs; //  low pressure TS
    unsigned int _cherenkov_coincidence = 25 ; // half a microsecond

    void update_cherenkov_buffer( std::set<artdaq::Fragment::timestamp_t> & buffer ) ; 
    // this update the content of the buffer based on the latest_part_TS
    // it dumps ts too old (or too new) wrt latest_part_TS xs  

    void update_cherenkov_counter( const artdaq::Fragment::timestamp_t & latest ) ; 
    // this methods should be called only when close_to_gool_part is true
    // it updates the cherenkov buffer
    // and if latest is after a certain amount of time from _latest_part_TS it evaluate the coincidence 
    
    bool store_run_trigger_counters( unsigned int run_number, const std::string & prefix = "" ) const ; 

    // metric utilities
    const std::array<std::string, 8> _metric_HLT_names  = { "PTB_HLT_0_rate",
							    "PTB_HLT_1_rate", 
							    "PTB_HLT_2_rate",
							    "PTB_HLT_3_rate",
							    "PTB_HLT_4_rate",
							    "PTB_HLT_5_rate",
							    "PTB_HLT_6_rate",
							    "PTB_HLT_7_rate" } ;


    // calibration stream parameters
    bool _has_calibration_stream = false ; 
    std::string _calibration_dir ; 
    std::chrono::minutes _calibration_update ; 
    
    // run trigger counters
    bool _has_run_trigger_report = false ;
    std::string _run_trigger_dir ;

  };

  inline bool TriggerBoardReader::CanCreateFragments() {

    return ( _group_size > 0  && _receiver -> N_TS_Words().load() >= _group_size ) || 
           ( _group_size <= 0 && _receiver -> Buffer().read_available() > 0 ) ;
  }

  inline bool TriggerBoardReader::NeedToEmptyBuffer() {
    
    unsigned int n_word = _receiver -> Buffer().read_available() ;
    return ( n_word >= _max_words_per_frag ) || 
           ( _receiver -> ErrorState() && n_word > 0 ) ;
  }

}

#endif /* sbndaq_artdaq_Generators_SBND_TriggerBoardReader_hh */
