/* Author: Matthew Strait <mstrait@fnal.gov> */

#ifndef artdaq_Generators_CRTInterface_CRTInterface_hh
#define artdaq_Generators_CRTInterface_CRTInterface_hh

#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"

#include "fhiclcpp/fwd.h"

#include <random>
#include <chrono>

// Either we have just started, in which case we go look for an input
// file ending in ".wr", or we just finished reading a file, which puts
// us in the same situation.
const unsigned int CRT_WAIT = 0x01,

// We are reading an input file ending in ".wr", i.e. one that is still
// being written to.
CRT_READ_ACTIVE = 0x02,

// We had to stop reading from the file because our internal buffer
// was filled by a large previous read.  Once we're done draining the buffer,
// go back to reading the file even though it has not changed.
CRT_READ_MORE = 0x04,

// We've read some data into our internal buffer and it may decode
// to one or more module packets, so read it before going back to
// the input files.
CRT_DRAIN_BUFFER = 0x08,

// On the very first file, we don't want to read everything when we 
// first find it.  So, create a special state to which CRTInterface 
// is initialized and which is never set again. 
CRT_FIRST_FILE = 0x10;

class CRTInterface
{
public:

  explicit CRTInterface(fhicl::ParameterSet const& ps);

  void StartDatataking();

  void StopDatataking();

  /**
   * \brief Fills a buffer with data from the CRT, if available.
   *
   * Provides zero or one "module packet", a collection of hits from
   * a single module sharing a time stamp.
   *
   * \param buffer Buffer that is filled with data
   * \param bytes_read Number of bytes passed back in buffer.  Nonzero
   * if and only if a module packet is returned in 'buffer'.
   */
  void FillBuffer(char* buffer, size_t* bytes_read);

  /**
   * \brief Request a buffer from the hardware
   * \param buffer (output) Pointer to buffer
   */
  void AllocateReadoutBuffer(char** buffer);

  /**
   * \brief Release the given buffer to the hardware
   * \param buffer Buffer to release
   */
  void FreeReadoutBuffer(char* buffer);

  void SetBaselines();

private:

  // Baselines, a.k.a. pedestals, for all of the channels in the run,
  // to be subtracted from the raw data before sending to artdaq.  These
  // are determined for each run as part of the run startup.
  int baselines[64 /*maxModules*/][64 /*numChannels*/];

  // The directory in which to look for input files.  This is probably
  // something like Run_0000123/binary/. It can be an absolute or relative
  // path.
  std::string indir;

  // One of {3, 13, 14, 22}, each of which is a USB stream with 8 modules
  // on it.
  unsigned int usbnumber;

  // State: whether we are reading an input file, waiting for one, etc.
  // bitmask of CRT_* defined above
  unsigned int state;

  // File descriptor associated with the inotify event queue, which is
  // used to find out when there is new data to read.
  int inotifyfd = -1;

  // File descriptor associated with the inotify watch on the data file
  int inotify_watchfd = -1;

  // File descriptor for the data file we are reading
  int datafile_fd = -1;

  // Name of the data file we are reading
  std::string datafile_name;

  // Documented in the implementation.
  bool try_open_file();
  bool check_events();
  size_t read_everything_from_file(char * );
};

#endif
