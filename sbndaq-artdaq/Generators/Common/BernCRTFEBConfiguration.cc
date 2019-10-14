#include "BernCRTFEBConfiguration.hh"

#define TRACE_NAME "BernCRTFEBConfiguration"

sbndaq::BernCRTFEBConfiguration::BernCRTFEBConfiguration(fhicl::ParameterSet const & ps_, int iFEB) {
  /**
   * Constructor basing on FHiCL file
   */
  if(!ASCIIToBitStream(
        ps_.get<std::string>("ProbeBitStream"),
        ProbeBitStream,
        PROBE_BITSTREAM_NBITS)) {   
    TLOG(TLVL_ERROR)<<__func__ << " Failed to load PROBE bit stream";
    throw cet::exception(std::string(TRACE_NAME) + "::" + __func__ + " Failed to load PROBE bit stream");
  }
  if(!ASCIIToBitStream(
        ps_.get<std::string>("SlowControlBitStream"+std::to_string(iFEB)),
        SlowControlBitStream,
        SLOW_CONTROL_BITSTREAM_NBITS)) {   
    TLOG(TLVL_ERROR)<< __func__  << " Failed to load Slow Control bit stream";
    throw cet::exception(std::string(TRACE_NAME) + "::" + __func__ + " Failed to load Slow Control bit stream");
  }
  std::vector<bool> hv_on_permissions = ps_.get< std::vector<bool> >("TurnOnHV");
  std::vector<uint64_t> mac5s = ps_.get< std::vector<uint64_t> >("FEBIDs"); //read MAC5 list for validation purposes only
  if(hv_on_permissions.size() != mac5s.size()) { //validate size of the array in the FHiCL file
    TLOG(TLVL_ERROR)<< __func__ << " TurnOnHV array size differs from FEBIDs array size";
    throw cet::exception(std::string(TRACE_NAME) + "::" + __func__ + " TurnOnHV array size differs from FEBIDs array size");
  }

  hv_on_permission = hv_on_permissions[iFEB];
}

int sbndaq::BernCRTFEBConfiguration::ASCIIToBitStream(std::string ASCIIBitStream, uint8_t *bitstream, unsigned int nBits) {
  /**
   * Converts bitstream saved in ASCII format to an actual bitstream
   * Bitstream is returned via argument
   * Returns false if the number of read bits differs from the parameter length
   * The format of the ASCII stream is the following:
   * Read '0' and '1' characters, ignoring spaces until you encounter character different than '0', '1' or ' ', then skip to the next line.
   */

  memset(bitstream,0,nBits/8); //reset
  unsigned int read_bits = 0;
  std::istringstream iASCIIBitStream(ASCIIBitStream);
  std::string line;
  while (std::getline(iASCIIBitStream, line)) { //loop over lines
    for(char& c : line) { //loop over characters
      if(c == ' ') continue; //ignore blank characters
      if(c == '0' || c == '1') { //encode the bit into the bitstream
        if(read_bits >= nBits) {
          TLOG(TLVL_WARNING)<< __func__ << " too long bitstream!!!";
          memset(bitstream,0,nBits/8); //reset
          return false;
        }

        if(c == '1') SetBit(read_bits, 1, bitstream, nBits);
        read_bits++;
      }
      else break; //if the character is not a space, '0' or '1', go to next line
    }
  }

  if(read_bits < nBits) {
    TLOG(TLVL_WARNING)<< __func__  << " too short bitstream!!!";
    memset(bitstream,0, nBits/8); //reset
    return false;
  }

  return true;
}

int sbndaq::BernCRTFEBConfiguration::GetBit(unsigned int bit_number, uint8_t * bitstream, unsigned int nBits) {
  /**
   * returns value of the given bit from the SlowControl bitstream.
   * nBits is the total length of the bitstream
   */
  if(bit_number >= nBits) {
    TLOG(TLVL_ERROR)<< __func__ << " requested bit " << std::to_string(bit_number) << "exceeds bitstream length!";
    return -1;
  }
  const int byte = (nBits - bit_number - 1) / 8; //reverse byte order
  const int bit = 7 - bit_number % 8;            //reverse bit order
  return ( bitstream[byte] & (1<<bit) ) > 0;
}

int sbndaq::BernCRTFEBConfiguration::GetBit(unsigned int bit_number) {
  /**
   * Gets bit value of the Slow Control bit stream
   */
  return GetBit(bit_number, SlowControlBitStream, SLOW_CONTROL_BITSTREAM_NBITS);
}

void sbndaq::BernCRTFEBConfiguration::SetBit(unsigned int bit_number, bool value, uint8_t * bitstream, unsigned int nBits) {
  /**
   * sets value of the given bit from the bitstream 
   * nBits is the total length of the bitstream
   */
  if(bit_number >= nBits) {
    TLOG(TLVL_ERROR)<< __func__ << " requested bit " << std::to_string(bit_number) << "exceeds bitstream length!";
    return;
  }
  const int byte = (nBits - bit_number - 1) / 8; //reverse byte order
  const int bit = 7 - bit_number % 8;            //reverse bit order
  if(value) bitstream[byte] |=  (1<<bit);
  else      bitstream[byte] &= ~(1<<bit);
  return;
}

void sbndaq::BernCRTFEBConfiguration::SetBit(unsigned int bit_number, bool value) {
  /**
   * Sets bit value of the Slow Control bit stream
   */
  SetBit(bit_number, value, SlowControlBitStream, SLOW_CONTROL_BITSTREAM_NBITS);
}

std::string sbndaq::BernCRTFEBConfiguration::BitsToASCII(unsigned int firstBit, unsigned int lastBit) {
  /**
   * Returns ASCII representation of given range in the Slow Control bitstream.
   * If last bit is not specified, only one bit is printed
   */
  if(lastBit >= SLOW_CONTROL_BITSTREAM_NBITS) {
    TLOG(TLVL_ERROR)<< __func__ << " Last bit " << std::to_string(lastBit) << "exceeds Slow Control bitstream length!";
    return "-";
  }
  std::string s = "";
  for(unsigned int iBit = firstBit; iBit <= lastBit; iBit++) {
    s += std::to_string(GetBit(iBit));
  }
  return s;
}

std::string sbndaq::BernCRTFEBConfiguration::BitsToASCII(unsigned int firstBit) {
  return sbndaq::BernCRTFEBConfiguration::BitsToASCII(firstBit, firstBit);
}

std::string sbndaq::BernCRTFEBConfiguration::GetString(std::string separator) {
  /**
   * Returns ASCII representation of the full SlowControl BitStream, including the comments
   */
  const int nCh = 32;
  std::string s="";
  for(int i = 0; i < nCh; i++) {
    s += BitsToASCII(i*4, i*4+3)
      + separator + "Ch"+std::to_string(i)+" 4-bit DAC_t ([0..3]), time trigger threshold\n";
  }
  for(int i = 0; i < nCh; i++) {
    s += BitsToASCII(128+i*4, 128+i*4+3)
      + separator + "Ch"+std::to_string(i)+"  4-bit DAC ([0..3]) charge trigger threshold\n";
  }
  s += BitsToASCII(256) + separator + "Enable discriminator\n";
  s += BitsToASCII(257) + separator + "Disable trigger discriminator power pulsing mode (force ON)\n";
  s += BitsToASCII(258) + separator + "Select latched (RS : 1) or direct output (trigger : 0)\n";
  s += BitsToASCII(259) + separator + "Enable Discriminator Two\n";
  s += BitsToASCII(260) + separator + "Disable trigger discriminator power pulsing mode (force ON)\n";
  s += BitsToASCII(261) + separator + "EN_4b_dac\n";
  s += BitsToASCII(262) + separator + "PP: 4b_dac\n";
  s += BitsToASCII(263) + separator + "EN_4b_dac_t\n";
  s += BitsToASCII(264) + separator + "PP: 4b_dac_t\n";
  s += BitsToASCII(265, 296) + separator + "Allows to Mask Discriminator (channel 0 to 31) [active low]\n";
  s += BitsToASCII(297) + separator + "Disable High Gain Track & Hold power pulsing mode (force ON)\n";
  s += BitsToASCII(298) + separator + "Enable High Gain Track & Hold\n";
  s += BitsToASCII(299) + separator + "Disable Low Gain Track & Hold power pulsing mode (force ON)\n";
  s += BitsToASCII(300) + separator + "Enable Low Gain Track & Hold\n";
  s += BitsToASCII(301) + separator + "SCA bias ( 1 = weak bias, 0 = high bias 5MHz ReadOut Speed)\n";
  s += BitsToASCII(302) + separator + "PP: HG Pdet\n";
  s += BitsToASCII(303) + separator + "EN_HG_Pdet\n";
  s += BitsToASCII(304) + separator + "PP: LG Pdet\n";
  s += BitsToASCII(305) + separator + "EN_LG_Pdet\n";
  s += BitsToASCII(306) + separator + "Sel SCA or PeakD HG\n";
  s += BitsToASCII(307) + separator + "Sel SCA or PeakD LG\n";
  s += BitsToASCII(308) + separator + "Bypass Peak Sensing Cell\n";
  s += BitsToASCII(309) + separator + "Sel Trig Ext PSC\n";
  s += BitsToASCII(310) + separator + "Disable fast shaper follower power pulsing mode (force ON)\n";
  s += BitsToASCII(311) + separator + "Enable fast shaper\n";
  s += BitsToASCII(312) + separator + "Disable fast shaper power pulsing mode (force ON)\n";
  s += BitsToASCII(313) + separator + "Disable low gain slow shaper power pulsing mode (force ON)\n";
  s += BitsToASCII(314) + separator + "Enable Low Gain Slow Shaper\n";
  s += BitsToASCII(315, 317) + separator + "Low gain shaper time constant commands (0...2)  [active low] 100\n";
  s += BitsToASCII(318) + separator + "Disable high gain slow shaper power pulsing mode (force ON)\n";
  s += BitsToASCII(319) + separator + "Enable high gain Slow Shaper\n";
  s += BitsToASCII(320, 322) + separator + "High gain shaper time constant commands (0...2)  [active low] 100\n";
  s += BitsToASCII(323) + separator + "Low Gain PreAmp bias ( 1 = weak bias, 0 = normal bias)\n";
  s += BitsToASCII(324) + separator + "Disable High Gain preamp power pulsing mode (force ON)\n";
  s += BitsToASCII(325) + separator + "Enable High Gain preamp\n";
  s += BitsToASCII(326) + separator + "Disable Low Gain preamp power pulsing mode (force ON)\n";
  s += BitsToASCII(327) + separator + "Enable Low Gain preamp\n";
  s += BitsToASCII(328) + separator + "Select LG PA to send to Fast Shaper\n";
  s += BitsToASCII(329) + separator + "Enable 32 input 8-bit DACs\n";
  s += BitsToASCII(330) + separator + "8-bit input DAC Voltage Reference (1 = external 4,5V , 0 = internal 2,5V)\n";
  for(int i = 0; i < nCh; i++) {
    s += BitsToASCII(331+i*9, 331+i*9+7) + " "
       + BitsToASCII(331+i*9+8)
       + separator + "Input 8-bit DAC Data channel "+std::to_string(i)+" - (DAC7...DAC0 + DAC ON), higher-higher bias (HV bias adjustment)\n";
  }
  for(int i = 0; i < nCh; i++){
    s += BitsToASCII(619+i*15, 619+i*15+5) + " "
       + BitsToASCII(619+i*15+6, 619+i*15+11) + " "
       + BitsToASCII(619+i*15+12, 619+i*15+14)
       + separator + "Ch"+std::to_string(i)+" PreAmp config (HG gain[5..0], LG gain [5..0], CtestHG, CtestLG, PA disabled)\n";
  }
  s += BitsToASCII(1099) + separator + "Disable Temperature Sensor power pulsing mode (force ON)\n";
  s += BitsToASCII(1100) + separator + "Enable Temperature Sensor\n";
  s += BitsToASCII(1101) + separator + "Disable BandGap power pulsing mode (force ON)\n";
  s += BitsToASCII(1102) + separator + "Enable BandGap\n";
  s += BitsToASCII(1103) + separator + "Enable DAC1\n";
  s += BitsToASCII(1104) + separator + "Disable DAC1 power pulsing mode (force ON)\n";
  s += BitsToASCII(1105) + separator + "Enable DAC2\n";
  s += BitsToASCII(1106) + separator + "Disable DAC2 power pulsing mode (force ON)\n";
  s += BitsToASCII(1107, 1108) + " "
     + BitsToASCII(1109, 1112) + " "
     + BitsToASCII(1113, 1116)
     + separator + "10-bit DAC1 (MSB-LSB): 00 1100 0000 for 0.5 p.e.\n";
  s += BitsToASCII(1117, 1118) + " "
     + BitsToASCII(1119, 1122) + " "
     + BitsToASCII(1123, 1126)
     + separator + "10-bit DAC2 (MSB-LSB): 00 1100 0000 for 0.5 p.e.\n";
  s += BitsToASCII(1127) + separator + "Enable High Gain OTA -- start byte 2\n";  
  s += BitsToASCII(1128) + separator + "Disable High Gain OTA power pulsing mode (force ON)\n";
  s += BitsToASCII(1129) + separator + "Enable Low Gain OTA\n";
  s += BitsToASCII(1130) + separator + "Disable Low Gain OTA power pulsing mode (force ON)\n";
  s += BitsToASCII(1131) + separator + "Enable Probe OTA\n";
  s += BitsToASCII(1132) + separator + "Disable Probe OTA power pulsing mode (force ON)\n";
  s += BitsToASCII(1133) + separator + "Otaq test bit\n";
  s += BitsToASCII(1134) + separator + "Enable Val_Evt receiver\n";
  s += BitsToASCII(1135) + separator + "Disable Val_Evt receiver power pulsing mode (force ON)\n";
  s += BitsToASCII(1136) + separator + "Enable Raz_Chn receiver\n";
  s += BitsToASCII(1137) + separator + "Disable Raz Chn receiver power pulsing mode (force ON)\n";
  s += BitsToASCII(1138) + separator + "Enable digital multiplexed output (hit mux out)\n";
  s += BitsToASCII(1139) + separator + "Enable digital OR32 output\n";
  s += BitsToASCII(1140) + separator + "Enable digital OR32 Open Collector output\n";
  s += BitsToASCII(1141) + separator + "Trigger Polarity\n";
  s += BitsToASCII(1142) + separator + "Enable digital OR32_T Open Collector output\n";
  s += BitsToASCII(1143) + separator + "Enable 32 channels triggers outputs\n";

  return s;
}


uint8_t * sbndaq::BernCRTFEBConfiguration::GetProbeBitStream()             { return ProbeBitStream; }
int       sbndaq::BernCRTFEBConfiguration::GetProbeBitStreamNBytes()       { return PROBE_BITSTREAM_NBITS/8; }
uint8_t * sbndaq::BernCRTFEBConfiguration::GetSlowControlBitStream()       { return SlowControlBitStream; }
int       sbndaq::BernCRTFEBConfiguration::GetSlowControlBitStreamNBytes() { return SLOW_CONTROL_BITSTREAM_NBITS/8; }

bool      sbndaq::BernCRTFEBConfiguration::GetHVOnPermission()             { return hv_on_permission; }
