/* Author: Matthew Strait <mstrait@fnal.gov>
 *         Andrew Olivier <aolivier@ur.rochester.edu>
 *         Ryan Howell    <rhowell3@ur.rochester.edu>
 */

#include "sbndaq-artdaq/Generators/ICARUS/CRTFragGen.hh"

#include "canvas/Utilities/Exception.h"

#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"

#include "artdaq/Generators/GeneratorMacros.hh"
#include "artdaq-core/Utilities/SimpleLookupPolicy.hh"

#include "fhiclcpp/ParameterSet.h"

#include <fstream>
#include <iomanip>
#include <iterator>
#include <iostream>

#include <unistd.h>
#include "cetlib_except/exception.h"

//#include "uhal/uhal.hpp"

#include "artdaq/DAQdata/Globals.hh"
#include "TRACE/tracemf.h"

CRT::FragGen::FragGen(fhicl::ParameterSet const& ps) :
    CommandableFragmentGenerator(ps)
  , configfile(ps.get<std::string>("configfile", ""))
  , readout_buffer_(nullptr)
  , hardware_interface_(new CRTInterface(ps))
  , timestamp_(0)
  , uppertime(0)
  , oldlowertime(0)
  , runstarttime(0)
  , partition_number(ps.get<int>("partition_number"))
  , startbackend(ps.get<bool>("startbackend"))
  , fUSBString(ps.get<std::string>("usbnumber"))
  , timingXMLfilename(ps.get<std::string>("connections_file",
    "/nfs/sw/control_files/timing/connections_v4b4.xml"))
  , timinghardwarename(ps.get<std::string>("hardware_select", "CRT_EPT"))
  //, timeConnMan("file://"+timingXMLfilename)
  //, timinghw(timeConnMan.getDevice(timinghardwarename))
  , gotRunStartTime(false)
{
  //uhal::setLogLevelTo(uhal::Error());

  // Tell the timing board what partition we are running in.
  // It's ok to do this in all four CRT processes.
  //timinghw.getNode("endpoint0.csr.ctrl.tgrp").write(partition_number);
  //timinghw.dispatch();

  hardware_interface_->AllocateReadoutBuffer(&readout_buffer_);

  // If we are the designated process to do so, start up Camillo's backend DAQ
  // here. A 5-10s startup time is acceptable since the rest of the ProtoDUNE
  // DAQ takes more than that.  Once we start up the backend, files will start
  // piling up, but that's ok since we will have a cron job to clean them up,
  // and we will always read only from the latest file when data is requested.
  //
  // Yes, a call to system() is awful.  We could improve this.
  //system(("/home/nfs/rhowell/test/ICARUS_DAQ/DAQ_CPP_v1/startallboards_fcl " + configfile).c_str());
  if(startbackend &&
     system(("/home/nfs/rhowell/test/ICARUS_DAQ/DAQ_CPP_v1/startallboards_fcl " + configfile).c_str())){
    throw cet::exception("CRT") << "Failed to start up CRT backend\n";
  }
  else  {
    TLOG(TLVL_INFO, "CRT") << "backend started succesfully\n";
  }

  // If we aren't the process that starts the backend, this will block
  // until the baselines are available.
  hardware_interface_->SetBaselines();
}

CRT::FragGen::~FragGen()
{
  // Stop the backend DAQ.
  if(system(("nohup /home/nfs/rhowell/test/ICARUS_DAQ/DAQ_CPP_v1/stopallboards_fcl " + configfile + " &").c_str())){
    TLOG(TLVL_WARNING, "CRT") << "Failed in call to stopallboards.pl\n"; // TODO review this, maybe exception?
  }
  else {
    TLOG(TLVL_INFO, "CRT") << "backend ended succesfully\n";
  }

  hardware_interface_->FreeReadoutBuffer(readout_buffer_);
}

bool CRT::FragGen::getNext_(
  std::list< std::unique_ptr<artdaq::Fragment> > & frags)
{
  /*if(!gotRunStartTime) 
  {
    //getRunStartTime();

    //Make sure runstarttime isn't too different from current UNIX timestamp.  If it 
    //is, then we probably won't write any useful data. When maintaining this code,  
    //remember that the timing endpoint won't give us a good runstarttime until 
    //everything has finished the start() transition.
    const uint64_t deltaT = labs(runstarttime*16/1.e9 - time(nullptr));
    if(deltaT > alarmDeltaT) //Hardcoded 16 ns per timestamp tick for ProtoDUNE-SP timing system
    {
      TLOG(TLVL_WARNING, "CRT") << "CRT board reader failed to get reasonable run start time "
                                << "from timing endpoint.  Difference from UNIX time of " << deltaT
                                << " > tolerance of " << alarmDeltaT << "\n";
      return true; //Keep trying to take data, but don't try to form any timestamps until we have 
                   //a "good" run start time
    }

    //runstarttime is OK if we got this far, so start sending back data.  
    //If it took more than 86 seconds to get a "good" runstarttime, then 
    //uppertime needs to be updated. 
    uppertime = (uint64_t)(deltaT*1e9/16)/std::numeric_limits<uint32_t>::max();
    TLOG(TLVL_INFO, "CRT") << "Set run start time to " << runstarttime << ", or " 
                              << (uint64_t)(runstarttime*16./1.e9) << " seconds at UNIX timestamp of "
                              << time(nullptr) << " seconds.  Looks like we skipped " << uppertime 
                              << " 32-bit rollovers, so set uppertime to " << uppertime << ".\n";
    gotRunStartTime = true;

    //CM 11/2019
    //initializing oldUNIX time
    oldUNIX = time(nullptr);

  }
   */                                                                                                  
  if(should_stop()){
    TLOG(TLVL_INFO, "CRT") << "getNext_ returning on should_stop()\n";
    return false;
  }

  // Maximum number of Fragments allowed per GetNext_() call.  I think we
  // should keep this as small as possible while making sure this Fragment
  // Generator can keep up.
  // TODO check this at some point
  const int maxFrags = 64;

  int fragIt = 0;
  for(; fragIt < maxFrags; ++fragIt){
    size_t bytes_read = 0;
    hardware_interface_->FillBuffer(readout_buffer_, &bytes_read);

    if(bytes_read == 0){
      // Pause for a little bit if we didn't get anything to keep load down.
      usleep(10); //Used to be 10000 usec, but that's more than 3 TPC readout windows!  

      // So that we still record metrics if we didn't write maxFrags but wrote
      // at least one.
      break;
    }

    // I have kept the following NOTE for historical purposes.  This
    // loop generalizes what it suggests.
    //     NOTE: we are always returning zero or one fragments, but we
    //     could return more at the cost of some complexity, maybe getting
    //     an efficiency gain.  Check back here if things are too slow.
    frags.emplace_back(buildFragment(bytes_read));
  } //for each Fragment

  if(metricMan != nullptr)
  {
    metricMan->sendMetric("Fragments Made", fragIt, "Fragments", 1, artdaq::MetricMode::Accumulate);
  }

  // TODO prune debug messages, make trace message BottomCRT
  // TODO add metric to send difference in local time and fragment timestamp

  if(fragIt > 0){//If we read at least one Fragment
    if (metricMan /* What is this? */ != nullptr)
      metricMan->sendMetric("Fragments Sent", ev_counter(), "Events", 3 /* ? */,
          artdaq::MetricMode::LastPoint);

    ev_counter_inc(); // from base CommandableFragmentGenerator

    TLOG(TLVL_INFO, "CRT") << "getNext_ is returning with hits in " << fragIt << " Fragments\n";
  }
  else{
    TLOG(TLVL_INFO, "CRT") << "getNext_ is returning with no data\n";
  }

  return true;
}

std::unique_ptr<artdaq::Fragment> CRT::FragGen::buildFragment(const size_t& bytes_read)
{
  assert(sizeof timestamp_ == 8);

  // A module packet must at least have the magic number (1B), hit count
  // (1B), module number (2B) and timestamps (8B).
  const std::size_t minsize = 4 + sizeof(timestamp_);
  if(bytes_read < minsize){
    throw cet::exception("CRT") << "Bad result with only " << bytes_read
       << " < " << minsize << " bytes from CRTInterface::FillBuffer.\n";
  }

  // Repair the CRT timestamp.  First get the lower bits that the CRT provides.
  // Then check if they rolled over and increment our stored upper bits if
  // needed, and finally concatenate the two.  Data does *not* come strictly
  // ordered in each USB stream (despite what Camillo told us on 2018-08-03),
  // but the trigger rate is sufficiently high and the data sufficiently close
  // to ordered that rollovers are pretty unambiguous.
  //
  // This only works if the CRT takes data continuously.  If we don't see data
  // for more than one 32-bit epoch, which is about 86 seconds, we'll fall out
  // of sync.  With additional work this could be fixed, since we leave the
  // backend DAQ running during pauses, but at the moment we skip over
  // intermediate data when we unpause (we start with the most recent file from
  // the backend, where files are about 20 seconds long).
  const uint64_t lowertime = *(uint32_t*)(readout_buffer_ + 4 + sizeof(uint64_t));

  // rolloverThreshold combats out-of-order data causing rollovers in the
  // timestamp.  Tuned by increasing by orders of magnitude until I saw the
  // number of rollovers so far match the total run time.  Lots of things have
  // changed since the last tuning, so we can probably back off on
  // rolloverThreshold.  Whatever we do, it should never be >
  // std::numeric_limits<uint32_t>::max().

  const uint64_t rolloverThreshold = 1000; //was 100000000

  uint64_t newUppertime = uppertime;

  // L. Jiang && C. Mariani Oct.2019
  uint64_t currentUNIX = time(nullptr);

  //determine the number of reset that occurred if the time passed is greater than 86s

  //uint64_t oldtimestamp = (((uint64_t)uppertime << 32) + lowertime + runstarttime)*16./1.e9;  //this is in sec.

  uint64_t deltaUNIX = currentUNIX - oldUNIX; //this is in sec.

  //uint64_t deltaUNIX = currentUNIX - oldtimestamp; //difference in time in sec.

  if(deltaUNIX > 0 ){ // there is a difference, check if a reset happen ( @86 sec )
    deltaUNIX /= (16./1.e9)*pow(2.,32.); //number of clock counter between two consecutive events considering the 16ns ticks=85.89s=86s
    deltaUNIX = (int)deltaUNIX; //lower end, need to check if it is off by one additional rollover    
    newUppertime += deltaUNIX; //adding an intenger number of seconds (86s) corresponding to how many resets we detect (should old do this when pausing the run)
  } 
  //detecting if there is an additional rollover, close to the edge of the clock reset
  else if((uint64_t)(lowertime + rolloverThreshold) < oldlowertime){
    /*TLOG(TLVL_DEBUG, "CRT") << "lowertime " << lowertime
      << " and oldlowertime " << oldlowertime << " caused a rollover.  "
      "uppertime is now " << uppertime << ".\n";*/
    newUppertime++;  
  }
  
  //building the new timestamp with the rollover if any
  timestamp_ = ((uint64_t)newUppertime << 32) + lowertime + runstarttime;


  //debug for the moment the new deltaUNIX time constructor
  if (deltaUNIX > 0) { //there was at least one reset

    TLOG(TLVL_WARNING, "CRT") << "Constructing a timestamp with deltaUNIX included, current linux time = "
			   << currentUNIX << " newUppertime is = " 
			   << newUppertime << " uppertime is = "
			   << uppertime << " difference between the old and the new UNIX time = "
			   << deltaUNIX << ", lowertime = " << lowertime
      //<< " Time Stamp without Rollover is " << oldtimestamp
			   << ".  New Timestamp is " << timestamp_ << "\n";
  }
  
  //Sanity check on timestamps, repeating somehow what we did before to cross check - TBF

  const uint64_t inSeconds = timestamp_*16./1.e9; //16 nanosecond ticks in the ProtoDUNE-SP timing system

  const int64_t deltaT = inSeconds - currentUNIX; //In seconds

  int64_t newtimediff;

  if(labs(deltaT) > alarmDeltaT) //Print a warning and don't update uppertime.  This might make us 
                                 //a little more robust against one or two boards having internal timing 
                                 //problems.  If it happens to all boards for too long, I won't try to 
                                 //recover.  
  {
    TLOG(TLVL_WARNING, "CRT") << "Got a large time difference of " << deltaT << " between CRT timestamp of " 
			      << timestamp_ << "( " << inSeconds << " seconds) - lowertime = " 
			      << lowertime << ", uppertime = " << uppertime 
			      << ", newuppertime = " << newUppertime  
			      << " and deltaUNIX = " << deltaUNIX 
			      << ".  Throwing out this Fragment - out of time.\n";

    if( labs(deltaT) == 1 ) {  //try to realign the time in case we are off by 1 cycle for whatever reason

      if(deltaT<0) {newUppertime++;} //try to futher correct the uppertime for this cycle: + 1 reset that was missed
      if(deltaT>0) {newUppertime--;} //try to futher correct the uppertime for this cycle: - 1 reset that was missed

      timestamp_ = ((uint64_t)newUppertime << 32) + lowertime + runstarttime;

      newtimediff = timestamp_*16./1.e9 - currentUNIX; //16 nanosecond ticks, convert timestamp in sec.

      TLOG(TLVL_WARNING, "CRT") << "deltaT=86s. Missed one reset cycle, try to correct it. deltaT = " 
				<< deltaT << ", new deltaT = " << newtimediff << "\n";

      if(labs(newtimediff) <= alarmDeltaT) { //repaired time stamp was successfull
	      uppertime = newUppertime;
	      oldUNIX = currentUNIX;
	      oldlowertime = lowertime;
      }//if not move on and try with the next event
    }
  }

  else { 
    uppertime = newUppertime; //This timestamp "makes sense", so keep track of 32-bit rollovers.
    oldUNIX = currentUNIX;
    oldlowertime = lowertime;
  }


  // And also copy the repaired timestamp into the buffer itself.
  // Code downstream in artdaq reads timestamp_, but both will always be
  // the same.
  memcpy(readout_buffer_ + 4, &timestamp_, sizeof(uint64_t));

  // See $ARTDAQ_CORE_DIR/artdaq-core/Data/Fragment.hh, also the "The
  // artdaq::Fragment interface" section of
  // https://cdcvs.fnal.gov/redmine/projects/artdaq-demo/wiki/How_to_write_an_overlay_class

  std::unique_ptr<artdaq::Fragment> fragptr
    = artdaq::Fragment::FragmentBytes(bytes_read);

  // ev_counter() from base CommandableFragmentGenerator
  fragptr->setSequenceID( ev_counter() );
  fragptr->setFragmentID( fragment_id() ); // Ditto
  fragptr->setUserType( sbndaq::detail::BottomCRT );
  // TODO timestamp calculated within 16 ns of  real hardware timestamp
  // TODO add trace message with frag ID and timestamp_
  fragptr->setTimestamp( timestamp_ );
  memcpy(fragptr->dataBeginBytes(), readout_buffer_, bytes_read);

  return fragptr;
}
/*
void CRT::FragGen::getRunStartTime()
{
  uhal::ValWord<uint32_t> status
    = timinghw.getNode("endpoint0.csr.stat.ep_stat").read();
  timinghw.dispatch();
  if(status != 8){
    throw cet::exception("CRT") << "CRT timing board in bad state, 0x"
      << std::hex << (int)status << ", can't read run start time\n";
  }

  uhal::ValWord<uint32_t> rst_l = timinghw.getNode("endpoint0.pulse.ts_l").read();
  timinghw.dispatch();
  uhal::ValWord<uint32_t> rst_h = timinghw.getNode("endpoint0.pulse.ts_h").read();
  timinghw.dispatch();
  runstarttime = ((uint64_t)rst_h << 32) + rst_l;
}
*/
void CRT::FragGen::start()
{ 
  runstarttime = time(nullptr);
  gotRunStartTime = true;
  TLOG(TLVL_INFO, "CRT") << "runstarttime set to " << runstarttime << "\n";
  hardware_interface_->StartDatataking();

  uppertime = 0;
  oldlowertime = 0;
}

void CRT::FragGen::stop()
{
  hardware_interface_->StopDatataking();
}

// The following macro is defined in artdaq's GeneratorMacros.hh header
DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(CRT::FragGen)
