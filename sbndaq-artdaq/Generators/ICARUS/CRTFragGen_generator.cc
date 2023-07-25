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
  //, uppertime(0)
  //, oldlowertime(0)
  , indir(ps.get<std::string>("indir"))
  , runstarttime_ns(0)
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
  TLOG(TLVL_INFO,"BottomFragGen")<<"Starting bottom CRT fragment generator";
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

  string cmd = "killall -q bottomCRTreadout";
  system(cmd.c_str());

  cmd = "ipcrm -Q 0x0000270f -Q 0x00002737";
  system(cmd.c_str());

  if(startbackend) {
    stopallboards(configfile.c_str(),indir.c_str());
    startallboards(configfile.c_str(),indir.c_str());
  }

  // If we aren't the process that starts the backend, this will block
  // until the baselines are available.
  hardware_interface_->SetBaselines();
}

CRT::FragGen::~FragGen()
{
  hardware_interface_->FreeReadoutBuffer(readout_buffer_);
}

bool CRT::FragGen::getNext_(
  std::list< std::unique_ptr<artdaq::Fragment> > & frags)
{
  //oldUNIX_ns = time(nullptr);// seconds //*1.e9; //ns
  if(should_stop()){
    TLOG(TLVL_INFO, "BottomFragGen") << "getNext_ returning on should_stop()\n";
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
   
    //A tpacket is injected into the data stream every second by the USB board, with the current UNIX time in seconds 
    tpacket_sec = hardware_interface_->GetTpacket();
   
    if(tpacket_sec != 0 && !gotRunStartTime){
       TLOG(TLVL_WARNING, "BottomFragGen") << "Got first tpacket: " << tpacket_sec << " seconds.\n"; 
    } 
 
    if(tpacket_sec == 0){ //waiting for the first tpacket 
      usleep(10);
      break;
    }

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

    TLOG(TLVL_DEBUG+1, "BottomFragGen") << "getNext_ is returning with hits in " << fragIt << " Fragments\n";
  }
  else{
    TLOG(TLVL_DEBUG+1, "BottomFragGen") << "getNext_ is returning with no data\n";
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
    throw cet::exception("BottomFragGen") << "Bad result with only " << bytes_read
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


  // rolloverThreshold combats out-of-order data causing rollovers in the
  // timestamp.  Tuned by increasing by orders of magnitude until I saw the
  // number of rollovers so far match the total run time.  Lots of things have
  // changed since the last tuning, so we can probably back off on
  // rolloverThreshold.  Whatever we do, it should never be >
  // std::numeric_limits<uint32_t>::max().
  const uint64_t rolloverThreshold = 5000000; //was 100000000
  
  //uint64_t currentUNIX_ns = time(nullptr);
  //uint64_t currentUNIX_ns = std::chrono::steady_clock::now().time_since_epoch().count();
  uint64_t currentUNIX_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();


  const uint64_t lowertime_tick = *(uint32_t*)(readout_buffer_ + 4 + sizeof(uint64_t));
  const uint64_t module_id = *(uint16_t*)(readout_buffer_ + 2) - 1; //numbers in the buffer start at 1

  uint64_t lowertime_ns = lowertime_tick*16.; //Convert to ns

  //TLOG(TLVL_INFO, "BottomFragGen")<<"module: " << module_id << ", lowertime_ns: " << lowertime_ns;

  //uint64_t deltaT_process = 0;
  
  if (!gotRunStartTime && tpacket_sec > 0) {
    runstarttime_ns = tpacket_sec*1000000000 - lowertime_ns - cable_offset_ns;
    gotRunStartTime = true;
    //oldUNIX_ns = time(nullptr); //seconds
    oldUNIX_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    //deltaT_process = oldUNIX_ns - runstarttime;
    TLOG(TLVL_WARNING, "BottomFragGen") << "setting the runstarttime to " << runstarttime_ns 
                              << " ns, with tpacket: "<< tpacket_sec << " s, and initial lowertime of  "  <<  lowertime_ns << " ns\n";
  }

  if(tpacket_sec > 0 && tpacket_sec > old_tpacket_sec){
    old_tpacket_sec = tpacket_sec;
    TLOG(TLVL_DEBUG, "BottomFragGen") << "Found new tpacket: " << tpacket_sec 
			      << " seconds, currentUNIX is: " << (uint64_t)(currentUNIX_ns/1000000000)
			      << " seconds, diff is: " << (uint64_t)(currentUNIX_ns/1000000000 - tpacket_sec) << " s\n";
  }

  if(lowertime_per_mod_ns[module_id]==0){ //First event in each module
    lowertime_per_mod_ns[module_id]=lowertime_ns;
    timestamp_=lowertime_per_mod_ns[module_id]+runstarttime_ns;
    TLOG(TLVL_DEBUG, "BottomFragGen") << "setting first event timestamp for module " << module_id << " to " << timestamp_ 
                              << " ns, with initial lowertime of "  <<  lowertime_per_mod_ns[module_id] << " ns\n";
  }  
  else{ //Any other event
    TLOG(TLVL_DEBUG+1, "BottomFragGen") << "getting data packet from module # = "
			   << module_id << "\n"; 

    if(lowertime_ns+rolloverThreshold<lowertime_per_mod_ns[module_id]){
      //increase the uppertime because we have received a sync pulse
      uppertime_per_mod_syncs[module_id]++; 
      TLOG(TLVL_DEBUG, "BottomFragGen") << "Module " << module_id <<" received a sync. "
      				<< "Uppertime is now " << uppertime_per_mod_syncs[module_id] << " syncs\n";
    }
    timestamp_ = lowertime_ns + runstarttime_ns + uppertime_per_mod_syncs[module_id]*1000000000*sync;
    TLOG(TLVL_DEBUG+1, "BottomFragGen") << "runstarttime: " <<runstarttime_ns<< " + lowertime: " << lowertime_ns 
					<<" + sync addition: " << uppertime_per_mod_syncs[module_id]*1000000000*sync 
					<<" for module " << module_id << " = timestamp: " << timestamp_; 

    if(module_id == 1 || module_id == 2 || module_id == 3 || module_id == 4){ //Boards 2, 3, 4, 5 are wired directly to the 
      timestamp_ -= 14; 						      //doublechooz module instead of the fanout
    }

    int64_t deltaUNIX_ns = timestamp_ - oldUNIX_ns; //this is in ns.  
    int64_t consec_event_thres_sec = 2;//TODO
    if(labs(deltaUNIX_ns)/1.e9 > consec_event_thres_sec){ // there is a difference between two consecutive processed events
      TLOG(TLVL_WARNING, "BottomFragGen") << "Detected a difference between consecutive events of " 
                           << deltaUNIX_ns/1.e9 << " seconds. currentUNIX = "
			   << currentUNIX_ns/1.e9 << " sec, and timestamp = "
			   << int64_t(timestamp_/1.e9) << " sec, and oldUNIX = "
                           << oldUNIX_ns/1.e9 
      			   << " sec. Tpacket is " << tpacket_sec 
      			   << " sec, uppertime is " << uppertime_per_mod_syncs[module_id] 
			   << " syncs, lowertime is " << lowertime_ns << " ns.\n";
    }

  //Safety check that all modules receive a sync 
  //Tracks the highest lowertime each module has seen, verifies that it does not go above (sync+0.1) seconds
  if(lowertime_ns > maxlowertime_ns[module_id]){
    maxlowertime_ns[module_id] = lowertime_ns;
    metricMan->sendMetric("Highest 32bit timestamp in seconds:", maxlowertime_ns[module_id]/1.e9, "Seconds", 1, artdaq::MetricMode::Maximum); 
  }

  if(lowertime_ns > (sync + 0.1)*1000000000) { //0.1 seconds after we're supposed to receive a sync
    TLOG(TLVL_WARNING, "BottomFragGen") << "Module did not receive a sync. Module # "
			   << module_id << " and lowertime is = " 
                           << lowertime_ns << " ns ( " << lowertime_ns/1.e9 << " seconds )"<< " \n";
  }

  //Sanity check on timestamps, check that the distance in time between when the event was acquired and processed stays within alarmDeltaT

  //const int64_t deltaT = currentUNIX_ns/1.e9 - timestamp_/1.e9; //In seconds
  
  //if(labs(deltaT) > alarmDeltaT + deltaT_process) {
  //  TLOG(TLVL_WARNING, "BottomFragGen") << "Problem with timestamp in module " << module_id << ". Uppertime set to = "
  //                            << uppertime_per_mod[module_id]  << " lowertime set to = "
  //                            << lowertime << ",deltaT_process is: " << deltaT_process << ", deltaT is " << deltaT << "  \n";
  //}

  //Report the average time difference between consecutive events (ns)
  metricMan->sendMetric("Timestamp difference", deltaUNIX_ns/1.e9, "seconds", 1, artdaq::MetricMode::Average | artdaq::MetricMode::Maximum | artdaq::MetricMode::Minimum);

  oldUNIX_ns = timestamp_; // in nanoseconds
  lowertime_per_mod_ns[module_id] = lowertime_ns;
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

  fragptr->setSequenceID( ev_counter_inc() );
  fragptr->setFragmentID(fragment_ids_[module_id]); // each board has its own fragment ID
  fragptr->setUserType( sbndaq::detail::BottomCRT );
  //TLOG(TLVL_WARNING, "BottomFragGen") << "module: " << module_id << ", timestamp: " << timestamp_;
  fragptr->setTimestamp( timestamp_ ); //Timestamp is in ns
  memcpy(fragptr->dataBeginBytes(), readout_buffer_, bytes_read);

  return fragptr;
}

void CRT::FragGen::start()
{ 
  hardware_interface_->StartDatataking();
 
  for(int i=0;i<(int)fragment_ids_.size();i++){
    lowertime_per_mod_ns.push_back(0);
    uppertime_per_mod_syncs.push_back(0);
    maxlowertime_ns.push_back(0);
  }

  //runstarttime = time(nullptr);
  //TLOG(TLVL_INFO, "BottomFragGen") << "runstarttime set to " << runstarttime << "\n";

  //uppertime = 0;
  //oldlowertime = 0;
}

void CRT::FragGen::stop()
{
  hardware_interface_->StopDatataking();
  // Stop the backend DAQ.
  stopallboards(configfile.c_str(),indir.c_str());
  string cmd = "killall bottomCRTreadout";
  int ret=system(cmd.c_str());
  TLOG(TLVL_INFO) << "system("<< cmd << ") returned: " << ret;
}

// The following macro is defined in artdaq's GeneratorMacros.hh header
DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(CRT::FragGen)
