/* Author: Matthew Strait <mstrait@fnal.gov> 
 * Author: Ryan Howell    <rhowell3@ur.rochester.edu>
*/

#include "artdaq/DAQdata/Globals.hh" //For TRACE debugging

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <deque>
#include <vector>

namespace CRT{

const int numChannels=64; // Number of channels in M64
const int maxModules=64; // Maximum number of modules PER USB
                         // (okay if less than total number of modules)
// A hit after decoding.
struct decoded_hit {
  uint8_t channel;
  int16_t charge;
};

// A module packet after decoding.  This is a possibly-unnecessary intermediate
// format that still needs serializing before it can be written out.
struct decoded_packet {
  decoded_packet()
  {
    module = 0;
    time16ns = 0;
  }

  uint16_t module;
  uint32_t time16ns;
  std::vector<decoded_hit> hits;
};

uint64_t tpacket_hi = 0;
uint64_t tpacket_raw = 0;

/*
  Return whether the input 24 bit word is part of a Unix timestamp packet, as
  revealed by its control code (bits 3-8).

  For reference, here's what's known about other control codes:

  # c1 dac pmt
  # c2 dac
  # c5 ....  -skipped
  # c6 data_hi    - number of received data words
  # c6 data_lo
  # c6 lost_hi     - number of lost data words
  # c6 lost_lo
  # c8 timestamp_hi
  # c9 timestamp_lo
*/
bool is_unix_time_word(const uint32_t wordin)
{
  const uint8_t control = (wordin >> 16) & 0xff;
  return control == 0xc8 || control == 0xc9;
}

// Convert a 24-bit word from upstream to a 16-bit word by stripping off the
// leading control octet.  Returns true and puts the result on the end of
// 'raw16bitdata' if a conversion was performed.  Returns false if we found
// a Unix timestamp packet or if we got rubbish.
bool raw24bit_to_raw16bit(std::deque<uint16_t> & raw16bitdata,
                          uint32_t in24bitword)
{
  // Old comment here said "command word, not data" for the case that
  // the first two bits were 01b. Apparently there are 24-bit words
  // undocumented in Matt Toups' thesis that start with values other
  // than 11b, but we just ignore them.
  if(((in24bitword >> 22) & 3) != 3) 
  {
    TLOG(TLVL_DEBUG, "CRTdecode") << "Got a 24-bit word that starts with " << ((in24bitword >> 22) & 3) << "\n";
    return false;
  }

  // In ProtoDUNE, we have no interest in Unix time stamp packets.
  if(is_unix_time_word(in24bitword)) 
  {
    //TLOG(TLVL_WARNING, "CRTdecode") << "Throwing out a UNIX time word\n";
    //TLOG(TLVL_WARNING, "CRTdecode") << "Found a UNIX time word (in24bitword << 16):"<< (in24bitword << 16)  
    //				    <<", (in24bitword >> 16) & 0xff: " << ((in24bitword >> 16) & 0xff) << "\n";
    if(((in24bitword >> 16) & 0xff) == 200){
      tpacket_hi = in24bitword & 65535;
    }
    else if(tpacket_hi > 0 && ((in24bitword >> 16) & 0xff) == 201) {
      tpacket_raw = (tpacket_hi << 16) | (in24bitword & 65535);
    }
    return false;
  }

  // If it is something other than a Unix time stamp or hit data (see various
  // other codes in the comments above is_unix_time_word()), discard it.
  if((in24bitword >> 16) != 0xc0) 
  {
    TLOG(TLVL_DEBUG+1, "CRTdecode") << "Throwing out a word that is not a UNIX time word or hit data: " << (in24bitword << 16) << "\n";
    return false;
  }

  raw16bitdata.push_back(in24bitword & 0xffff);

  return true;
}

/*
Converts the "decoded_packet" in-memory format in 'packet' to the serialized
output format and puts the result in 'cooked', provided that the result is
not more than max_cooked bytes long.  Returns the number of bytes written,
unless the result is too long, in which case returns -1.  In this case,
'cooked' may contain truncated data, but you probably don't want to use it.

The serialized format of a module packet is:

    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |  Magic number | Count of hits |         Module number         |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |   (Zero: will have upper bits of global timestamp inserted)   |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |   (Zero: will have lower bits of global timestamp inserted)   |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                   50 MHz counter time stamp                   |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |               .              hits             .               |
   |               .               .               .               |

  Magic number: 0x4D = "M"

  Count of hits: Unsigned 8 bit integer.

  Module number: Unsigned 16 bit integer.

  Zero:

    These 32 bits are left as all zeros.  Downstream they will get filled
    in by the upper 32 bits of the 64 bit ProtoDUNE timestamp, which the
    CRT hardware doesn't know about.

  Time stamp: Unsigned 32 bit integer

    Number of 16ns ticks since last sync pulse.  Shouldn't usually be
    more than 2^29 - 1.

The format of a hit is:

    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |  Magic number |    Channel    |             Charge            |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

  Magic number: 0x48 = "H"

  Channel number: Unsigned 8 bit integer

    Actually only uses 5 bits (0-63).

  Charge: *Signed* 16 bit integer

    Represents a 12-bit ADC value.  So we could store this in 13 bits, but
    there's no great motivation to pack the data so closely, especially
    because the minimum size of a hit is 18 bits, which I would tend to
    pad out to 32 anyway.

In all cases, it is assumed that the reader of this data is on the same
machine as the writer.  No attempt is made to standardize endianness.

*/
unsigned int serialize(char * cooked, const decoded_packet & packet,
                       const unsigned int max_cooked)
{
  if(packet.hits.empty()) return 0;

  const unsigned int size_needed = 2 /* magic number 'M' (1) +
    count of hits (1) */ + sizeof packet.module
    + 8 /* Global 64-bit timestamp will be filled in downstream */
    + sizeof packet.time16ns
    + packet.hits.size() *
      (1 /* magic number 'H' */
       + sizeof packet.hits[0].charge + sizeof packet.hits[0].channel);

  if(size_needed > max_cooked){
    TLOG(TLVL_WARNING, "CRTdecode") << "Can't write packet of size " << size_needed << " with max " << max_cooked << "\n";
    return 0;
  }

  unsigned int bytes = 0;
  cooked[bytes++] = 'M';
  cooked[bytes++] = packet.hits.size();
  memcpy(cooked+bytes, &packet.module, sizeof packet.module);
  bytes += sizeof packet.module;
  memset(cooked+bytes, 0, 8);
  bytes += 8;
  memcpy(cooked+bytes, &packet.time16ns, sizeof packet.time16ns);
  bytes += sizeof packet.time16ns;

  for(unsigned int m = 0; m < packet.hits.size(); m++) {
    cooked[bytes++] = 'H';
    cooked[bytes++] = packet.hits[m].channel;
    memcpy(cooked+bytes, &packet.hits[m].charge, sizeof packet.hits[m].charge);
    bytes += sizeof packet.hits[m].charge;
  }

  if(bytes != size_needed)
    TLOG(TLVL_WARNING, "CRTdecode") << "size mismatch " << bytes << " != " << size_needed << "\n";
  return bytes;
}

/*
  Decodes the 16-bit data in 'raw' to an ADC packet written into 'cooked' and
  returns the length of the packet in bytes.  Deletes the used contents of raw,
  leaving trailing unused words.  If the data in 'raw' cannot be
  decoded, or would decode to a size larger than max_cooked, returns zero and
  leaves 'cooked' undefined.

  The data written into 'cooked' is in the format described in the comment
  above serialize().  It consists of zero or more "module packets", each
  of which is a collection of hits from a single module sharing a time stamp.
*/
unsigned int make_a_packet(char * cooked, std::deque<uint16_t> & raw,
                           const unsigned int max_cooked,
                           const int baselines[maxModules][numChannels])
{
  // ADC packet word indices.  As per Toups thesis:
  //
  // 0xffff                         | Header word
  // 1, mod#[7 bits], wdcnt[8 bits] | Data type, module #, word count
  // time                           | High 16 bits of 62.5 MHz clock counter
  // time                           | Low  16 bits of 62.5 MHz clock counter
  // adc                            | ADC ...
  // channel                        | ... and channel number, repeated N times
  // parity                         | Parity
  enum ADC_WINX { ADC_WIDX_HEAD   = 0, ADC_WIDX_MODLEN = 1,
                  ADC_WIDX_CLKHI  = 2, ADC_WIDX_CLKLO  = 3,
                  ADC_WIDX_HIT    = 4 };

  // Try to decode the data in 'raw'. Stop trying if 'raw' is empty, or
  // if it starts out correctly with 0xffff but has nothing else, or if it is
  // shorter than the length it claims to have.  But otherwise, drop the
  // first word and try to decode again.

  // First word of all packets other than unix timestamp packets is 0xffff
  // If there's any junk before 0xffff, discard it.
  while(!raw.empty() && raw[0] != 0xffff){
    TLOG(TLVL_WARNING, "CRTdecode") << "Discarding word 0x" << raw[0] << " appearing before 0xffff\n";
    raw.pop_front();
  }

  if(raw.size() < 2) return 0;

  unsigned int len = raw[ADC_WIDX_MODLEN] & 0xff;
  if(len == 0){
    TLOG(TLVL_WARNING, "CRT") << "Discarding packet with declared length zero.\n";
    raw.clear();
    return 0;
  }

  // we don't have all the data in this packet yet
  if(raw.size() < len + 1) return 0;

  if(!(raw[ADC_WIDX_MODLEN] >> 15)){
    TLOG(TLVL_WARNING, "CRT") << "Non-ADC packet found, skipping\n";

    // Throw out what we have so far, and then rely upon looking for
    // the leading 0xffff data word to throw out the rest of whatever
    // this is.
    raw.clear();
    return 0;
  }

  unsigned int parity = 0;

  decoded_packet packet;
  packet.module = (raw[ADC_WIDX_MODLEN] >> 8) & 0x7f;

  for(unsigned int wordi = ADC_WIDX_MODLEN; wordi < len; wordi++){
    parity ^= raw[wordi];

    if(wordi == ADC_WIDX_CLKHI) {
      packet.time16ns |= ((uint32_t)raw[wordi] << 16);
    }
    else if(wordi == ADC_WIDX_CLKLO) {
      packet.time16ns |= raw[wordi];
    }
    else{ // we are in the words that give the hit info
      // hits start on even numbered words
      if(wordi%2 == 0) {
        decoded_hit hit;
        hit.channel = raw[wordi+1];
        hit.charge  = raw[wordi];

        if(hit.channel < numChannels && packet.module < maxModules)
          hit.charge -= baselines[packet.module][hit.channel];
        else
          TLOG(TLVL_DEBUG, "CRT") << "Not applying baseline to bad module/channel number " << packet.module << "/" << hit.channel << "\n";

        packet.hits.push_back(hit);
      }
    }
  }

  const bool goodparity = parity == raw[len];

  // Discard raw data that has been decoded
  for(unsigned int i = 0; i < len + 1; i++) raw.pop_front();

  if(!goodparity){
    TLOG(TLVL_WARNING, "CRT") << "Parity error.  Dropping packet.\n";
    return 0;
  }

  return serialize(cooked, packet, max_cooked);
}

unsigned int raw2cook(char * const cooked_data,
                      const unsigned int max_cooked,
                      char * rawfromhardware,
                      char * & next_raw_byte,
                      const int baselines[maxModules][numChannels],
		      uint64_t &tpacket)
{
  /*
    Undocumented input file format is revealed by inspection to be
    constructed like this:

    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |0 0|     A     |0 1|      B    |1 0|     C     |1 1|     D     |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

   Where the bits of A, B, C, and D concatenated make the 24-bit words
   described in Matt Toups' thesis.  Besides being protected against
   undetected errors by the 2-bit counter at the head of each octet, the
   24-bit data is parity checked; this is handled in make_a_packet().
 */

  uint32_t word = 0; // holds 24-bit word being built, must be unsigned
  char expcounter = 0; // expecting this 2-bit counter value next

  std::deque<uint16_t> raw16bitdata;

  unsigned int used_raw_bytes = 0;
  unsigned int cooked_bytes = 0;

  for(char * readptr = rawfromhardware; readptr < next_raw_byte; readptr++){
    const char counter = ((*readptr) >> 6) & 3; //TODO: If we kept getting a counter value of 0, I guess we could 
                                                //      get stuck in a loop here.  That doesn't seem very likely though.  
    const char payload = (*readptr) & 0x3f;
    if(counter == 0){
      expcounter = 1;
      word = payload;
    }
    else if(counter == expcounter){
      word = (word << 6) | payload;
      if(++expcounter == 4){ //TODO: If we don't get here, we never try to make a packet
        expcounter = 0;

        // Every time we assemble a 24-bit word that could be part of
        // an ADC packet, try to decode the whole stream so far.

        unsigned int newcookedbytes = 0;
        const auto raw24bit_success = raw24bit_to_raw16bit(raw16bitdata, word);
        //if(raw24bit_to_raw16bit(raw16bitdata, word) &&
        if(raw24bit_success &&
           (newcookedbytes = make_a_packet(cooked_data + cooked_bytes,
                                           raw16bitdata,
                                           max_cooked - cooked_bytes,
                                           baselines))){
          used_raw_bytes = readptr - rawfromhardware + 1;
          cooked_bytes += newcookedbytes;

          // Return only one module packet per call.  It is easy enough
          // to remove this line and return as many as can be constructed,
          // but I think this will make life easier downstream even though
          // it is computationally wasteful at this stage because it potentially
          // incurs many buffer rotations below.
          break;
        }
        TLOG(TLVL_DEBUG+1, "CRTdecode") << "Throwing out word " << word << " in raw2cook() because "
                                      << (raw24bit_success?"make_a_packet()":"raw24bit_to_raw16bit()") << " failed\n";
      }
    }
    else{
      TLOG(TLVL_WARNING, "CRT") << "corrupted data: expected counter " << expcounter << ", got " << counter << "\n";
      expcounter = 0;
    }
  }

  // Rotate buffer in the most wasteful way possible, by actually moving
  // the undecoded bytes to the front.
  if(used_raw_bytes){
    TLOG(TLVL_DEBUG+1, "CRT") << "Used " << used_raw_bytes << "bytes, and rotating " << next_raw_byte - rawfromhardware - used_raw_bytes << " to front for later use.\n";
    memmove(rawfromhardware, rawfromhardware + used_raw_bytes,
            next_raw_byte - rawfromhardware - used_raw_bytes);
  }
  else{
    TLOG(TLVL_DEBUG+1, "CRTdecode") << "Used 0 bytes\n";
  }

  next_raw_byte -= used_raw_bytes;
  //TLOG(TLVL_DEBUG, "CRTdecode") << "Cooked bytes: " << cooked_bytes<< "\n";
  tpacket = tpacket_raw;
  if(tpacket == 1){
    TLOG(TLVL_DEBUG, "CRTdecode") << "Tpacket error: " << tpacket <<"\n";
  }
  return cooked_bytes;
}

} // end namespace CRT
