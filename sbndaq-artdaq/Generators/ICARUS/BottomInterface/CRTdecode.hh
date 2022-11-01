/* Author: Matthew Strait <mstrait@fnal.gov> */

namespace CRT{

/*
  Decodes the data in 'rawfromhardware' and puts the result in
  'cooked_data', returning the number of bytes put into cooked_data,
  which can be up to 'max_cooked'.

  'cooked_data' will consist of zero or one "module packets", which
  is a collection of hits from a single module sharing a single time
  stamp.

  If there is not a complete module packet in rawfromhardware, it returns zero
  and leaves all arguments unmodified.  Otherwise, it rotates the buffer
  rawfromhardware to put the first unused byte at the beginning and sets
  next_raw_byte to one past the end of the remaining data, i.e. to where the
  caller should write new data.

  If the data in rawfromhardware would decode to a module packet of more than
  max_cooked bytes, emits a warning and flushes the input buffer, i.e. sets
  next_raw_byte to rawfromhardware and returns zero.  This should never
  happen as long as a reaosnable max_cooked is given.
*/
unsigned int raw2cook(char * const cooked_data,
                      const unsigned int max_cooked,
                      char * rawfromhardware,
                      char * & next_raw_byte,
                      const int baselines[64 /*maxModules*/][64 /*numChannels*/]);

}
