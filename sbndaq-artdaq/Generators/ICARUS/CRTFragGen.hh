/* Author: Matthew Strait <mstrait@fnal.gov> */

#ifndef artdaq_Generators_CRTFragGen_hh
#define artdaq_Generators_CRTFragGen_hh

#include "fhiclcpp/fwd.h"
#include "artdaq-core/Data/Fragment.hh"
#include "artdaq/Generators/CommandableFragmentGenerator.hh"

#include "BottomInterface/CRTInterface.hh"

#include <random>
#include <vector>
#include <atomic>
#include <memory> //For std::unique_ptr

//#include "uhal/uhal.hpp"

namespace CRT
{
  class FragGen : public artdaq::CommandableFragmentGenerator
  {
    public:

    /**
      \brief Initialize the CRT DAQ.
      In fact, start the whole backend DAQ up, but do not start
      passing data to artdaq yet.
    */
    explicit FragGen(fhicl::ParameterSet const& ps);
    virtual ~FragGen();

    private:

    /**
     * \brief The "getNext_" function is used to implement user-specific
     * functionality; it's a mandatory override of the pure virtual
     * getNext_ function declared in CommandableFragmentGenerator
     * \param output New FragmentPtrs will be added to this container
     * \return True if data-taking should continue
     */
    bool getNext_(std::list< std::unique_ptr<artdaq::Fragment> > & output) override;

    /**
     * \brief Perform start actions
     */
    void start() override;

    /** \brief Perform stop actions */
    void stop() override;

    void stopNoMutex() override {}

    // Name of CRT database table that gives the information about the set of modules
    // we will read from.
    std::string configfile;

    // Written to by the hardware interface
    char* readout_buffer_;

    /*********************************************************************
       And then all the rest of the members are here to deal with fixing
       the CRT hardware's 32-bit time stamp.
    *********************************************************************/
    // Emits one Fragment from hardware_interface_.  It seems like we'll ideally 
    // only call this once in GetNext_(), but we can call it multiple times when 
    // able to speed things up if we're too slow. 
    std::unique_ptr<artdaq::Fragment> buildFragment(const size_t& bytes_read); 

    std::unique_ptr<CRTInterface> hardware_interface_;

    // uint64_t (after unwinding a few layers of typedefs) for the
    // global clock.  For the CRT, we assemble this out of the 32-bit
    // timestamp directly kept by the CRT hardware and the timestamp of
    // the beginning of the run, keeping track of rollovers of the lower
    // 32 bits.
    artdaq::Fragment::timestamp_t timestamp_;

    // The list of fragment IDs for the bottom CRT, depending on module numebr
    std::vector<uint32_t> fragment_ids_;

    // The upper 32 bits of the timestamp. We keep track of this in the
    // fragment generator because the CRT hardware only keeps the lower
    // 32 bits.
    std::vector<uint32_t> uppertime_per_mod_syncs;
    std::vector<uint32_t> lowertime_per_mod_ns;
    uint64_t tpacket_sec = 0;
    uint64_t old_tpacket_sec = 0;

    //Keep track of the maximum lowertime among all boards
    std::vector<uint32_t> maxlowertime_ns;

    // The previous 32-bit timestamp received from the CRT hardware (or
    // the run start time if no events yet), so we can determine if we
    // rolled over and need to increment uppertime.
    uint32_t oldlowertime = 0;

    // Directory on local scratch to store backend DAQ data and logs
    std::string indir;

    uint64_t oldUNIX_ns = 0;

    // The 64-bit global timestamp of the start of the run. We need to
    // retrieve and store this to repair the CRT's internal 32-bit time.
    uint64_t runstarttime_ns;

    // The partition number from the FCL.  We need to write this into the
    // timing board to be able to retrieve the run start time.
    // int partition_number;

    // True if this process is the one designated to start the backend DAQ.
    bool startbackend;

    //Keep track of which USB board this board reader is reading from for debugging
    //const std::string fUSBString; 

    //std::string timingXMLfilename;
    //std::string timinghardwarename;
/*
    uhal::ConnectionManager timeConnMan;
    uhal::HwInterface timinghw;
*/
    //Have we set runstarttime yet?
    bool gotRunStartTime;

    //When should we get worried about timestamps getting out of sync?  In seconds
    static constexpr int64_t alarmDeltaT = 2; 
    static constexpr int64_t sync = 7;

    //Breakdown of cable offsets: 
    //225 +/- 0.2 ns SPEXI to West CRT TDU +
    //10 ns time for gate generator to get the signal out (needs check)+
    //50 ns coincidence module (needs check)+
    //15 +/- 5 ns doublechooz module +
    //15 +/- 5 ns fan in/out + (not applicable for module 6)
    //99 ns for 70 ft. of cable at 1.42 ns/ft =
    static constexpr int64_t cable_offset_ns = 414; //ns, +/-30
  };
}

#endif /* artdaq_Generators_CRTFragGen_hh */
