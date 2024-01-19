/* Author: Matthew Strait <mstrait@fnal.gov> */

#include "sbndaq-artdaq/Generators/ICARUS/BottomInterface/CRTInterface.hh"
#include "sbndaq-artdaq/Generators/ICARUS/BottomInterface/CRTdecode.hh"
#define TRACE_NAME "CRTInterface"
#include "artdaq/DAQdata/Globals.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"

#include "fhiclcpp/ParameterSet.h"
#include "cetlib_except/exception.h"

#include <random>
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <fcntl.h>
#include <dirent.h>

#include <sys/inotify.h>

/**********************************************************************/
/* Buffers and whatnot */

// Maximum size of the data, after the header, in bytes
static const ssize_t RAWBUFSIZE = 0x10000;
static const ssize_t COOKEDBUFSIZE = 0x10000;

static char rawfromhardware[RAWBUFSIZE];
static char * next_raw_byte = rawfromhardware;

/**********************************************************************/

CRTInterface::CRTInterface(fhicl::ParameterSet const& ps) :
  indir(ps.get<std::string>("indir")),
  usbnumber(ps.get<unsigned int>("usbnumber")),
  state(CRT_FIRST_FILE | CRT_WAIT)
{
  indir = indir + "/runs1/DataFolder";
}

void CRTInterface::StartDatataking()
{
  TLOG(TLVL_INFO, "CRTInterface") << "CRTInterface: starting data-taking"; 
  if(inotifyfd != -1){
    TLOG(TLVL_WARNING, "CRTInterface")
      << "inotify already init'd. Ok if we stopped and restarted data taking.\n";
    return;
  }

  if(-1 == (inotifyfd = inotify_init())){
    TLOG(TLVL_WARNING, "CRTInterface") << "StartDatataking: " << strerror(errno);
    throw cet::exception("CRTInterface")
      << "CRTInterface::StartDatataking: " << strerror(errno);
  }

  // Set the file descriptor to non-blocking so that we can immediately
  // return from FillBuffer() if no data is available.
  fcntl(inotifyfd, F_SETFL, fcntl(inotifyfd, F_GETFL) | O_NONBLOCK);
}

void CRTInterface::StopDatataking()
{
  TLOG(TLVL_INFO, "CRTInterface") << "CRTInterface: stopping data-taking";
  errno = 0;
  if(-1 == inotify_rm_watch(inotifyfd, inotify_watchfd))
    TLOG(TLVL_WARNING, "CRTInterface") << "StopDatataking: " << strerror(errno);
}

// Return the name of the most recent file that we can read, without the
// directory path.  If the backend DAQ started up a long time ago, there
// will be a pile of files we never read, but that is OK.
std::string find_wr_file(const std::string & indir,
                         const unsigned int usbnumber)
{
  DIR * dp = NULL;
  errno = 0;
  if((dp = opendir(indir.c_str())) == NULL){
    if(errno == ENOENT){
      TLOG(TLVL_WARNING, "CRTInterface") << "No such directory "
        << indir << ", but will wait for it\n";
      usleep(100000);
      return NULL;
    }
    else{
      // Other conditions we are unlikely to recover from: permission denied,
      // too many file descriptors in use, too many files open, out of memory,
      // or the name isn't a directory.
      throw cet::exception("CRTInterface")
        << "find_wr_file opendir: " << strerror(errno);
    }
  }

  time_t most_recent_time = 0;
  std::string most_recent_file = "";

  struct dirent * de = NULL;
  while(errno = 0, (de = readdir(dp)) != NULL){
    char suffix[7];
    snprintf(suffix, 7, "_%d.wr", usbnumber);

    // Does this file name end in "_NN.wr", where NN is the usbnumber?
    //
    // Ignore baseline (a.k.a. pedestal) files, which are also given ".wr"
    // names while being written.  We could be even more restrictive and
    // require that the file be named like <unix time stamp>_NN.wr, but it
    // doesn't seem necessary.
    //
    // If somehow there ends up being a directory ending in ".wr", ignore it
    // (and all other directories).  I suppose all other types are fine, even
    // though we only really expect regular files.  But there's no reason not
    // to accept a named pipe, etc.
    if(de->d_type != DT_DIR &&
       strstr(de->d_name, "baseline") == NULL &&
       strstr(de->d_name, suffix) != NULL &&
       strlen(strstr(de->d_name, suffix)) == strlen(suffix)){
      struct stat thestat;

      if(-1 == stat((indir + de->d_name).c_str(), &thestat)){
        // We noticed this failure mode during testing when a file was renamed
        // between being found above and getting its timestamp.  Unless someone
        // is messing with the CRT data directory, this probably means that the
        // file we found *was* a .wr file, but the backend moved it.  Since it's
        // no longer the .wr file, it isn't the most recent file anyway.
        continue;
      }

      if(thestat.st_mtime > most_recent_time){
        most_recent_time = thestat.st_mtime;
        most_recent_file = de->d_name;
      }
    }
  }

  // If errno == 0, it just means we got to the end of the directory.
  // Otherwise, something went wrong.  This is unlikely since the only
  // error condition is "EBADF  Invalid directory stream descriptor dirp."
  if(errno)
    throw cet::exception("CRTInterface") << "find_wr_file readdir: " << strerror(errno);

  errno = 0;
  closedir(dp);
  if(errno)
    throw cet::exception("CRTInterface")
      << "find_wr_file closedir: " << strerror(errno);

  // slow down if the directory is there, but the file isn't yet
  if(most_recent_file == "") usleep(100);

  return most_recent_file; // even if it is "", which means none
}

/*
  Check if there is a file ending in ".wr" in the input directory.
  If so, open it, set an inotify watch on it, and return true.
  Else return false.
*/
bool CRTInterface::try_open_file()
{
  // I am sure it used to work to do find_wr_file().c_str() but now C++ is so
  // complicated that no one can understand it.  Apparently if you use the old
  // approach after C++11 or C++14 or thereabouts, the std::string destructor
  // is immediately called because the scope of a return value is only the
  // right hand side of the expression.
  const std::string cplusplusiscrazy =
    find_wr_file(indir + "/binary/", usbnumber);
  const char * filename = cplusplusiscrazy.c_str();

  //Since I'm now using this to detect the backend going to a new file, don't
  //open the same file again!  We're probably causing a problem if this is happening
  //often.
  if(cplusplusiscrazy == datafile_name){
    TLOG(TLVL_DEBUG, "CRTInterface")
      << "Looked for a new file from the backend, but found the current file instead!  "
      << "This could become a problem by slowing down the board reader if it's "
      << "happening too often.\n";
    return false;
  }

  if(strlen(filename) == 0){
    TLOG(TLVL_DEBUG, "CRTInterface") << "Failed to find input file "
      << filename << " in directory " << indir << "/binary/ for raw data.\n";
    return false;
  }
  else{
    TLOG(TLVL_DEBUG, "CRTInterface") << "Found file " << filename
      << " in directory " << indir << "/binary/ for raw data.\n";
  }

  const std::string fullfilename = indir + "/binary/" + filename;

  /*TLOG(TLVL_INFO, "CRTInterface") << "Found input file: " << filename
    << " Adding watch on it at " << fullfilename.c_str() << "\n";*/

  // At this point, we've succeeded in finding a new file.  We're not interested in
  // reading the old file anymore, and we're about to overwrite the file descriptor
  // for watching it with inotify.  So, remove the inotify watch on the old file.
  inotify_rm_watch(inotifyfd, inotify_watchfd);
  close(datafile_fd);
  TLOG(TLVL_DEBUG, "CRTInterface") << "Closed file " << datafile_name << " and "
                                  << "removed inotify watch on it.\n";

  // Start watching the new file for IN_MODIFY events.  If we can't
  // set a watch on it, try again later.
  if(-1 == (inotify_watchfd =
            inotify_add_watch(inotifyfd, fullfilename.c_str(),
                              IN_MODIFY))){
    if(errno == ENOENT){
      // It's possible that the file we just found has vanished by the time
      // we get here, probably by being renamed without the ".wr".  That's
      // OK, we'll just try again in a moment.
      TLOG(TLVL_INFO, "CRTInterface")
        << "File has vanished. We'll wait for another\n";
      return false;
    }
    else{
      // But other inotify_add_watch errors we probably can't recover from
      throw cet::exception("CRTInterface") << "CRTInterface: Could not open "
        << filename << ": " << strerror(errno);
    }
  }

  // Open the new file.  If it's already gone, try again later to find a new
  // file.
  if(-1 == (datafile_fd = open(fullfilename.c_str(), O_RDONLY))){
    if(errno == ENOENT){
      // The file we just set a watch on might already be gone, as above.
      // We'll just get the next one.
      inotify_rm_watch(inotifyfd, inotify_watchfd);
      return false;
    }
    else{
      throw cet::exception("CRTInterface") << "CRTInterface::StartDatataking: "
        << strerror(errno);
    }
  }

  state = CRT_READ_ACTIVE;
  datafile_name = filename;
  TLOG(TLVL_INFO, "CRTInterface") << "Just opened and now reading from file " << datafile_name << ".  Found it at time " 
                                  << time(nullptr) << "\n";

  return true;
}

/*
  Checks for inotify events that alert us to a file being appended to
  or renamed, and update 'state' appropriately.  If no events, return
  false, meaning there is nothing to do now.
*/
bool CRTInterface::check_events()
{
  // It is nice to be able to read more than one inotify event at a time so
  // that we can log the extra ones.  Eight is an arbitrary number; we've seen
  // as many as two come at once.  The man page's example is to use a buffer of size
  // 4096, which seems like bad practice!
  const size_t inotifybufsize = 8*(sizeof(struct inotify_event) + NAME_MAX + 1);
  char filechange[inotifybufsize]
    __attribute__ ((aligned(__alignof__(struct inotify_event))));

  ssize_t inotify_bread = 0;

  // read() is non-blocking because I set O_NONBLOCK above
  errno = 0;
  if(-1 ==
     (inotify_bread = read(inotifyfd, &filechange, sizeof(filechange)))){

    // If there are no events, we get this error
    if(errno == EAGAIN) return false;

    // Anything else maybe should be a fatal error.  If we can't read from
    // inotify once, we probably won't be able to again.
    throw cet::exception("CRTInterface")
      << "CRTInterface::FillBuffer: " << strerror(errno);
  }

  //Try to read all of the inotify events to figure out what we're missing.
  if(inotify_bread == inotifybufsize)
    TLOG(TLVL_WARNING, "CRTInterface")
      << "Filled buf when reading from inotify!  We might have missed some events.\n";
  TLOG(TLVL_DEBUG, "CRTInterface") << "Got " << inotify_bread/sizeof(struct inotify_event) << " inotify events.\n";
  /*const struct inotify_event* event;
  for(auto ptr = filechange;
      ptr < filechange + inotify_bread;
      ptr += sizeof(struct inotify_event) + event->len){
    event = (const struct inotify_event*)ptr;

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wstrict-aliasing"
      const uint32_t mask = ((struct inotify_event *)filechange)->mask;
    #pragma GCC diagnostic pop

    TLOG(TLVL_INFO, "CRTInterface") << "Got inotify event with mask " << mask << "\n";
  }*/

  if(inotify_bread == 0){
    // This means that the file has not changed, so we have no new data
    // (or maybe this never happens because we'd get EAGAIN above).
    return false;
  }

  if(inotify_bread < (ssize_t)sizeof(struct inotify_event)){
    throw cet::exception("CRTInterface") << "Non-zero, yet wrong number ("
      << inotify_bread << ") of bytes from inotify\n";
  }

  //If we got this far, then we've got a valid event from inotify.  We are only 
  //reacting to IN_MODIFIED events in the first place, so just ASSUME that we 
  //got an IN_MODIFIED event and react accordingly.  
  //TODO: Should we check what kind of event we got from inotify?  Since we're 
  //      only using inotify to check for events, we could probably just try a 
  //      non-blocking read on the backend file every time FillBuffer() is 
  //      called and return like we currently do based on the number of bytes.  
  //
  //      Looks like problem is instead that raw2cook() is returning 0 bytes despite 
  //      full buffer.  
  TLOG(TLVL_DEBUG, "CRTInterface") << "Got a \"modified\" event from inotify.\n";
  if(state & CRT_READ_ACTIVE) return true; //Note: Without the below error check, we remove an 
                                           //      if statement by returning state & CRT_READ_ACTIVE
                                           //      cast to bool directly.  
  
  TLOG(TLVL_WARNING, "CRTInterface") << "...but not watching a file!\n";
  return false;
}

/*
  Reads all available data from the open file.
*/
size_t CRTInterface::read_everything_from_file(char * cooked_data)
{
  // Oh boy!  Since we're here, it means we have a new file, or that the file
  // has changed.  Hopefully that means *appended to*, in which case we're
  // going to read the new bytes.  At the moment, let's ponderously read one at
  // a time.  (Dumb, but maybe if it works, we should just keep it
  // that way.)  If by "changed", in fact the file was truncated
  // or that some contents prior to our current position were changed, we'll
  // get nothing here, which will signal that such shenanigans occurred.

  ssize_t read_bread = 0;

  while(next_raw_byte < rawfromhardware + RAWBUFSIZE &&
        -1 != (read_bread = read(datafile_fd, next_raw_byte, 1))){
    if(read_bread != 1) break;

    next_raw_byte += read_bread;
  }

  // We're leaving unread data in the file, so we will need to come back and
  // read more even if we aren't informed that the file has been written to.
  // TODO: Something is wrong about when we set the CRT_READ_MORE bit here.
  //       We once got into a loop where this was always set even though 
  //       read_everything_from_file() returned nothing.  
  if(next_raw_byte >= rawfromhardware + RAWBUFSIZE)
    state |= CRT_READ_MORE;

  if(read_bread == -1){
    // All read() errors other than *maybe* EINTR should be fatal.
    throw cet::exception("CRTInterface")
      << "CRTInterface::read_everything_from_file: " << strerror(errno);
  }

  const int bytesleft = next_raw_byte - rawfromhardware;
  TLOG(TLVL_DEBUG, "CRTInterface")
    << bytesleft << " bytes in raw buffer after read.\n";

  if(bytesleft > 0) state |= CRT_DRAIN_BUFFER;

  return CRT::raw2cook(cooked_data, COOKEDBUFSIZE,
                       rawfromhardware, next_raw_byte, baselines, tpacket);
}

void CRTInterface::FillBuffer(char* cooked_data, size_t* bytes_ret)
{
  //TLOG(TLVL_DEBUG, "CRTInterface")
  //  << "Entering CRTInterface::FillBuffer";
  *bytes_ret = 0;

  // First see if we can decode another module packet out of the data already
  // read from the input files.
  const auto bytesBefore = next_raw_byte - rawfromhardware;
  if(state & CRT_DRAIN_BUFFER){
    if((*bytes_ret = CRT::raw2cook(cooked_data, COOKEDBUFSIZE,
                                   rawfromhardware, next_raw_byte, baselines, tpacket)))
      {
      //  TLOG(TLVL_WARNING, "CRTInterface") << "Tpacket is " << tpacket << "\n";
        return;
      }
    else
      state &= ~CRT_DRAIN_BUFFER;
  }

  // Then see if we need to read more out of the file, and do so
  if(state & CRT_READ_MORE){
    state &= ~CRT_READ_MORE;
    TLOG(TLVL_DEBUG, "CRTInterface") << "Read data from " << datafile_name
                                     << " because there was more to read from file.  Buffer had "
                                     << bytesBefore << " bytes in it before read.\n";
    if((*bytes_ret = read_everything_from_file(cooked_data))) return; 
  }

  // It seems like we don't have anything else to read in the current file
  // at this point.  If the file has been modified, that might need to change.

  // This should only happen when we open the first file.  Otherwise,
  // the first read to a new file is handled below.
  if(state & CRT_WAIT){
    const auto oldState = state; //try_open_file() will probably assign to state, 
                                 //so make sure we remember whether this is the first 
                                 //file.  
    if(!try_open_file()) return;

    if(oldState & CRT_FIRST_FILE)
    {
      // The backend started writing files during the CONFIGURE transition, but
      // lots of things could have happened since then.  Some time has almost
      // certainly passed, so we'll probably get to the first file as the backend
      // is in the middle of writing it.  To avoid introducing a hard-to-predict
      // offset into all of the data read for the rest of this run, skip all of
      // the data in the backend's file when we first find it and wait for the
      // next update to read anything.
      TLOG(TLVL_INFO, "CRTInterface")
        << "First input file, skipping to end: " << datafile_name << "\n";
      lseek(datafile_fd, 0, SEEK_END);
      state &= ~CRT_FIRST_FILE;
      return;
    }

    // Immediately read from the file, since we won't hear about writes
    // to it previous to when we set the inotify watch.  If there's nothing
    // there yet, don't bother checking the events until the next call to
    // FillBuffer(), because it's unlikely any will have come in yet.
    TLOG(TLVL_DEBUG, "CRTInterface") << "Read data from " << datafile_name
                                     << " because a new file was found.  ";
    TLOG(TLVL_DEBUG, "CRTInterface") << "Buffer had " << bytesBefore 
                                     << " bytes in it before read from new file\n";
    *bytes_ret = read_everything_from_file(cooked_data);
    return;
  }

  // If we're here, it means we have an open file which we've previously
  // read and decoded all available data from.  Check if it has changed.
  // TODO: This assumes that we will be told about a file rename.  That
  //       assumption seems to be wrong.
  //
  //       If we're at this point, we think we've read everything we can
  //       from the current file.  If check_events() returns false, that's
  //       still true.
  //
  //       Should we keep waiting for this file to change or just go to
  //       the next one?  I know something else about how readout works:
  //       When it creates a new .wr file, the old file is done being
  //       written to forever.  So, can I try to look for a new file
  //       anyway when there are no events?
  //
  // If there are no pending events on the current file, look for a new file.
  // If a new .wr file exists for this USB board, then the old file is no longer
  // being written to, and we are done with it here.  Otherwise, readout could
  // still write to the current file, and we should return with no bytes so that
  // we try to read from it again.
  if(!check_events() && !try_open_file()) return;

  // Ok, it has either been written to or renamed.  Because we first get
  // notified about the last write to a file and then about its renaming
  // in separate events (they can't be coalesced because they are different
  // event types), there will never be anything to read when we hear the
  // file has been renamed.

  // Either the file is already open and we can go ahead, or it isn't,
  // and we either find a new file and immediately try to read it, or return.
  if(!(state & CRT_READ_ACTIVE) && !try_open_file()) return;

  *bytes_ret = read_everything_from_file(cooked_data);
  TLOG(TLVL_DEBUG, "CRTInterface") << "Returning with " << bytes_ret
                                   << " bytes at the very end of FillBuffer()'s scope.\n";
}

void CRTInterface::SetBaselines()
{
  // Note that there is no check below that all channels are assigned a
  // baseline.  Indeed, since we have fewer than 64 modules, not all elements
  // of the array will be filled.  The check will be done in online monitoring.
  // If a channel has no baseline set, nothing will be subtracted and the ADC
  // values will be obviously shifted upwards from what's expected.
  memset(baselines, 0, sizeof(baselines));
  FILE * in = NULL;
  while(true){
    errno = 0;
    if(NULL == (in = fopen((indir + "/baselines.dat").c_str(), "r"))){
      if(errno == ENOENT){
        // File isn't there.  This probably means that we are not the process
        // that started up the backend. We'll just wait for the backend to
        // finish starting and the file to appear.
        TLOG(TLVL_DEBUG, "CRTInterface") << "Waiting for baseline file to appear\n";
        sleep(1);
      }
      else{
        throw cet::exception("CRTInterface") << "Can't open baseline file: "
          << strerror(errno) << "\n";
      }
    }
    else{
      break; //If I found the file, stop trying to open it!
    }
  }

  int usb = 0, module = 0, channel = 0;
  float fbaseline = 0, stddev = 0, nhit = 0;
  int nconverted = 0;
  int line = 0;
  while(EOF != (nconverted = fscanf(in, "%d,%d,%d,%f,%f,%f",
        &usb, &module, &channel, &fbaseline, &stddev, &nhit))){

    line++; // This somewhat erroneously assumes that we consume
            // one line per scanf.  In a pathological file with the right
            // format, but bad data, and with fewer line breaks than expected,
            // the reported line number will be wrong.

    if(nconverted != 6){
      TLOG(TLVL_WARNING, "CRTInterface") << "Warning: skipping invalid line "
        << line << " in baseline file";
      continue;
    }

    if(module >= 64){
      TLOG(TLVL_WARNING, "CRTInterface")
         << "Warning: skipping baseline with invalid module number "
         << module << ".  Valid range is 0-63\n";
      continue;
    }

    if(channel >= 64){
      TLOG(TLVL_WARNING, "CRTInterface")
         << "Warning: skipping baseline with invalid channel number "
         << module << ".  Valid range is 0-63\n";
      continue;
    }

    if(nhit < 100)
      TLOG(TLVL_WARNING, "CRTInterface")
        << "Warning: using baseline based on only " << nhit << " hits (PMT " << module << ", Ch. " << channel << ")\n";

    if(stddev > 7.0)
      TLOG(TLVL_WARNING, "CRTInterface")
        << "Warning: using baseline with large error: " << stddev << " ADC counts (PMT " << module << ", Ch. " << channel << ")\n"; 

    baselines[module][channel] = int(fbaseline + 0.5);
  }

  errno = 0;
  if(fclose(in) == EOF)
    throw cet::exception("CRTInterface") << "Can't close CRT baseline file: "
      << strerror(errno) << "\n";
}

void CRTInterface::AllocateReadoutBuffer(char** cooked_data)
{
  *cooked_data = new char[COOKEDBUFSIZE];
}

void CRTInterface::FreeReadoutBuffer(char* cooked_data)
{
  delete[] cooked_data;
}

uint64_t CRTInterface::GetTpacket()
{
  return tpacket;
}
