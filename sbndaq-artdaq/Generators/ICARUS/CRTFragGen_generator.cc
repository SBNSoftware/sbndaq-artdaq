/* Author: Matthew Strait <mstrait@fnal.gov>
 *         Andrew Olivier <aolivier@ur.rochester.edu>
 *         Ryan Howell    <rhowell3@ur.rochester.edu>
 */

#include "sbndaq-artdaq/Generators/ICARUS/CRTFragGen.hh"
#include "sbndaq-artdaq/Generators/ICARUS/CRTFragGen.hh"
#include "sbndaq-artdaq/Generators/ICARUS/BottomInterface/Backend_DAQ/DAQ_CPP_v1/startallboards.cc"
#include "sbndaq-artdaq/Generators/ICARUS/BottomInterface/Backend_DAQ/DAQ_CPP_v1/stopallboards.cc"

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
  , fragment_ids_(ps.get<std::vector<uint32_t>>("fragment_ids"))
  , uppertime(0)
  , oldlowertime(0)
  , indir(ps.get<std::string>("indir"))
  , runstarttime(0)
  //, partition_number(ps.get<int>("partition_number"))
  , startbackend(ps.get<bool>("startbackend"))
  //, fUSBString(ps.get<std::string>("usbnumber"))
  //, timingXMLfilename(ps.get<std::string>("connections_file",
  //  "/nfs/sw/control_files/timing/connections_v4b4.xml"))
  //, timinghardwarename(ps.get<std::string>("hardware_select", "CRT_EPT"))
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

  string cmd = "killall bottomCRTreadout";
  system(cmd.c_str());

  cmd = "ipcrm -Q 0x0000270f -Q 0x00002737";
  system(cmd.c_str());

  if(startbackend) {
    startallboards(configfile.c_str(),indir.c_str());
  }

  // If we aren't the process that starts the backend, this will block
  // until the baselines are available.
  hardware_interface_->SetBaselines();
}

CRT::FragGen::~FragGen()
{
  hardware_interface_->FreeReadoutBuffer(readout_buffer_);
  // Stop the backend DAQ.
  TLOG(TLVL_INFO, "CRT") << "killing bottom CRT readout processes\n";
  stopallboards(configfile.c_str(),indir.c_str());
  string cmd = "killall bottomCRTreadout";
  system(cmd.c_str());
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
    //If it took more than 14 seconds to get a "good" runstarttime, then 
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
  oldUNIX = time(nullptr);
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

    TLOG(TLVL_DEBUG, "CRT") << "getNext_ is returning with hits in " << fragIt << " Fragments\n";
  }
  else{
    TLOG(TLVL_DEBUG, "CRT") << "getNext_ is returning with no data\n";
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
  // for more than one 32-bit epoch, which is about 1 seconds, we'll fall out
  // of sync.  With additional work this could be fixed, since we leave the
  // backend DAQ running during pauses, but at the moment we skip over
  // intermediate data when we unpause (we start with the most recent file from
  // the backend, where files are about 20 seconds long).

  // L. Jiang && C. Mariani Oct.2019
  uint64_t currentUNIX = time(nullptr);

  const uint64_t lowertime = *(uint32_t*)(readout_buffer_ + 4 + sizeof(uint64_t));

  if (!gotRunStartTime) {
    runstarttime = currentUNIX - lowertime*16./1e9;
    gotRunStartTime = true;
    oldUNIX = time(nullptr);
    TLOG(TLVL_INFO, "CRT") << "setting the runstarttime to " << runstarttime 
                              << " with initial lowertime of  "  <<  lowertime*16./1e9 << " seconds \n";
  }

  //const uint64_t module = *(uint16_t*)(readout_buffer_ + 2 + sizeof(uint16_t));
  const uint64_t module_id = *(uint16_t*)(readout_buffer_ + 2) - 1; //numbers in the buffer start at 1
  //const uint64_t module = 0;
    TLOG(TLVL_DEBUG, "CRT") << "getting data packet from module # = "
			   << module_id << " \n"; 
  // rolloverThreshold combats out-of-order data causing rollovers in the
  // timestamp.  Tuned by increasing by orders of magnitude until I saw the
  // number of rollovers so far match the total run time.  Lots of things have
  // changed since the last tuning, so we can probably back off on
  // rolloverThreshold.  Whatever we do, it should never be >
  // std::numeric_limits<uint32_t>::max().

  const uint64_t rolloverThreshold = 10000000; //was 100000000

  int64_t newUppertime = uppertime;


  //determine the number of reset that occurred if the time passed is greater than 7s

  //int64_t oldtimestamp = lowertime + ((uint64_t)uppertime + runstarttime)*1.e9/16.;  //this is in sec.

  int64_t deltaUNIX = currentUNIX - oldUNIX; //this is in sec.  
  //uint64_t deltaUNIX = currentUNIX - oldtimestamp; //difference in time in sec.

  //debug for the moment the new deltaUNIX time constructor
//  if (labs(deltaUNIX) > 0) { //there was at least one reset

  if(deltaUNIX > 3 ){ // there is a difference, check if a reset happen ( @14 sec )
    TLOG(TLVL_WARNING, "CRT") << "Detected a difference between currentUNIX = "
			   << currentUNIX << " and oldUNIX = " 
			   << oldUNIX << " \n";
    //deltaUNIX /= 7; //number of clock counter between two consecutive events considering the 16ns ticks=7s
    //deltaUNIX = (int)deltaUNIX; //lower end, need to check if it is off by one additional rollover    
    //newUppertime += deltaUNIX*7; //adding an intenger number of seconds (7s) corresponding to how many resets we detect (should old do this when pausing the run)
  }

  if(lowertime > maxlowertime){
    maxlowertime = lowertime;
    metricMan->sendMetric("Highest 32bit timestamp (in seconds):", maxlowertime*16./1.e9, "Fragments", 1, artdaq::MetricMode::Accumulate); 
  }

  if(lowertime * 16./1e9 > 7.1) {
    TLOG(TLVL_WARNING, "CRT") << "module did not receive a sync. Module # "
			   << module_id << " and lowertime is = " 
                           << lowertime << "( " << lowertime*16./1e9 << " seconds )"<< " \n";
  }
  //detecting if there is an additional rollover, close to the edge of the clock reset
  /*if(lowertime + rolloverThreshold < oldlowertime && module_id == 1){ // TODO check we get events in order
    newUppertime+=7;  
    TLOG(TLVL_WARNING, "CRT") << "Detected a sync, current linux time = "
			   << currentUNIX << " lowertime is = " 
			   << lowertime << " old lowertime is = " 
			   << oldlowertime << " newUppertime is = " 
			   << newUppertime << " module # is = " 
                           << module_id << ", syncs so far: " 
			   << newUppertime/7 <<" \n";
  }*/
  //}

  //building the new timestamp with the rollover if any
  //timestamp_ = lowertime + ((uint64_t)newUppertime + runstarttime)*1.e9/16.;
  timestamp_ = lowertime + (runstarttime)*1.e9/16.; //in ticks


  //Sanity check on timestamps, repeating somehow what we did before to cross check - TBF

  const uint64_t inSeconds = timestamp_*16./1.e9; //convert to seconds using 16 nanosecond ticks in the ICARUS timing system

  const int64_t deltaT = currentUNIX - inSeconds; //In seconds

  int64_t newtimediff;

  uppertime = (int)(labs(deltaT)/7);

  timestamp_ += uppertime*7*1.e9/16.; //in ticks

  newtimediff = currentUNIX - timestamp_*16./1.e9; //16 nanosecond ticks, convert timestamp in sec.
      
  if(labs(newtimediff) <= alarmDeltaT) { //repaired time stamp was successfull
    //TLOG(TLVL_WARNING, "CRT") << "Repaired time stamp was successful in module " << module_id << ". Uppertime set to = "
    //                          << uppertime  << ", newtimediff is now " << newtimediff <<" lowertime set to = "
    //                          << lowertime << ", deltaT was " << deltaT << "  \n";
  }
  else{
      TLOG(TLVL_WARNING, "CRT") << "Generated time stamp was unsuccessful in module " << module_id <<". Newtimediff is "
			        << newtimediff << ", uppertime: " << uppertime << ", deltaT: " << deltaT << ", lowertime: " << lowertime << "\n";
  }  

  int delta_now = newtimediff; //this is in sec.  

  if(labs(delta_now) > alarmDeltaT ){ // there is a difference, check if a reset happen ( @7 sec )
    TLOG(TLVL_WARNING, "CRT") << "Detected a difference between currentUNIX = "
			   << currentUNIX << " and timestamp = " 
			   << (uint64_t)(timestamp_*16./1e9) 
                           << "of " << delta_now << " seconds \n";
  
  }
  metricMan->sendMetric("Timestamp difference", delta_now, "Fragments", 1, artdaq::MetricMode::Average | artdaq::MetricMode::Maximum | artdaq::MetricMode::Minimum);

/*  if(labs(deltaT) > alarmDeltaT) //Print a warning and don't update uppertime.  This might make us 
                                 //a little more robust against one or two boards having internal timing 
                                 //problems.  If it happens to all boards for too long, I won't try to 
                                 //recover.  
  {

    TLOG(TLVL_WARNING, "CRT") << "Got a large time difference of " << deltaT << " between CRT timestamp of " 
			      << timestamp_ << "( " << inSeconds << " seconds) - lowertime = " 
			      << lowertime
			      << ", old lowertime= " << oldlowertime
			      << ", uppertime = " << uppertime 
			      << " and deltaUNIX = " << deltaUNIX 
			      << " and currentUNIX = " << currentUNIX
			      << " and oldUNIX = " << oldUNIX 
                              << " and module # = " << module_id
			      << ".  Throwing out this Fragment - out of time.\n";

    if( labs(deltaT) == 7 && lowertime + rolloverThreshold < oldlowertime ) {  //try to realign the time in case we are off by 1 cycle for whatever reason

      //if(deltaT<0) {newUppertime+=7;} //try to futher correct the uppertime for this cycle: + 1 reset that was missed
      //if(deltaT>0) {newUppertime-=7;} //try to futher correct the uppertime for this cycle: - 1 reset that was missed

      timestamp_ = lowertime + ((uint64_t)newUppertime + runstarttime)*1.e9/16.;

      newtimediff = timestamp_*16./1.e9 - currentUNIX; //16 nanosecond ticks, convert timestamp in sec.

      TLOG(TLVL_WARNING, "CRT") << "deltaT=7s. Missed one reset cycle, try to correct it. deltaT = " 
				<< deltaT << ", new deltaT = " << newtimediff << "\n";

      if(labs(newtimediff) <= alarmDeltaT) { //repaired time stamp was successfull
        TLOG(TLVL_WARNING, "CRT") << "Repaired time stamp was successful. Uppertime set to = "
                                << newUppertime  << " oldlowertime set to = "
                                << lowertime << "  \n";
	      uppertime = newUppertime;
	      oldUNIX = currentUNIX;
	      oldlowertime = lowertime;
      }//if not move on and try with the next event
    }
  }

  else { 
    uppertime = newUppertime; //This timestamp "makes sense", so keep track of 32-bit rollovers.
    oldUNIX = timestamp_*16./1.e9;
    oldlowertime = lowertime;
  }
*/

  // And also copy the repaired timestamp into the buffer itself.
  // Code downstream in artdaq reads timestamp_, but both will always be
  // the same.
  memcpy(readout_buffer_ + 4, &timestamp_, sizeof(uint64_t));

  // See $ARTDAQ_CORE_DIR/artdaq-core/Data/Fragment.hh, also the "The
  // artdaq::Fragment interface" section of
  // https://cdcvs.fnal.gov/redmine/projects/artdaq-demo/wiki/How_to_write_an_overlay_class

  std::unique_ptr<artdaq::Fragment> fragptr
    = artdaq::Fragment::FragmentBytes(bytes_read);

  fragptr->setSequenceID( ev_counter_inc() );
  fragptr->setFragmentID(fragment_ids_[module_id]); // each board has it's own fragment ID
  fragptr->setUserType( sbndaq::detail::BottomCRT );
  fragptr->setTimestamp( timestamp_*16. ); // want timestamp, in 16 nanosecond ticks, in nanoseconds
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
  hardware_interface_->StartDatataking();

  //runstarttime = time(nullptr);
  //TLOG(TLVL_INFO, "CRT") << "runstarttime set to " << runstarttime << "\n";

  uppertime = 0;
  oldlowertime = 0;
}

void CRT::FragGen::stop()
{
  hardware_interface_->StopDatataking();
}

// The following macro is defined in artdaq's GeneratorMacros.hh header
DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(CRT::FragGen)
