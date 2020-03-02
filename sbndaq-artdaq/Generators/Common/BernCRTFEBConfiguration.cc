#include "BernCRTFEBConfiguration.hh"

#define TRACE_NAME "BernCRTFEBConfiguration"

sbndaq::BernCRTFEBConfiguration::BernCRTFEBConfiguration() {
  /**
   * default constructor required by std::unordered_map
   */
}


sbndaq::BernCRTFEBConfiguration::BernCRTFEBConfiguration(fhicl::ParameterSet const & ps_, int iFEB) {
//Configuration in the FHiCL file can have two formats, and the format is detected automatically
//if bitstream exists in FHiCL file, it is loaded, otherwise human readable format is used
  std::string s = ps_.get<std::string>("SlowControlBitStream"+std::to_string(iFEB), "failure");
  if(s.compare("failure")) {
    TLOG(TLVL_INFO)<< __func__ << " Loading human-readable configuration for FEB" + iFEB;
    if(!read_human_readable_parameters(ps_, iFEB))
      throw cet::exception(std::string(TRACE_NAME) + "::" + __func__ + " Failed to load FHiCL configuration for FEB " + std::to_string(iFEB));
  }
  else {
    TLOG(TLVL_INFO)<< __func__ << " Loading bitstream configuration for FEB" + iFEB;
    if(!read_bitstream(ps_, iFEB))
      throw cet::exception(std::string(TRACE_NAME) + "::" + __func__ + " Failed to load FHiCL configuration for FEB " + std::to_string(iFEB));
  }
  
  //load HV on/off settings and cable delays
  std::vector<bool> hv_on_permissions = ps_.get< std::vector<bool> >("TurnOnHV");
  std::vector<int32_t> PPS_offsets = ps_.get< std::vector<int32_t> >("PPS_offset_ns");
  std::vector<uint64_t> mac5s = ps_.get< std::vector<uint64_t> >("MAC5s"); //read MAC5 list for validation purposes only

  //validate size of the arrays in the FHiCL file
  if(hv_on_permissions.size() != mac5s.size()) {
    TLOG(TLVL_ERROR)<< __func__ << " TurnOnHV array size differs from MAC5s array size";
    throw cet::exception(std::string(TRACE_NAME) + "::" + __func__ + " TurnOnHV array size differs from MAC5s array size");
  }
  hv_on_permission = hv_on_permissions[iFEB];
  
  if(PPS_offsets.size() != mac5s.size()) {
    TLOG(TLVL_ERROR)<< __func__ << " PPS_offset_ns array size differs from MAC5s array size";
    throw cet::exception(std::string(TRACE_NAME) + "::" + __func__ + " PPS_offset_ns array size differs from MAC5s array size");
  }
  PPS_offset = PPS_offsets[iFEB];
}

bool sbndaq::BernCRTFEBConfiguration::read_bitstream(fhicl::ParameterSet const & ps_, int iFEB) {
  /**
   * Constructor basing on FHiCL file
   */
  if(!ASCIIToBitStream(
        ps_.get<std::string>("ProbeBitStream"),
        ProbeBitStream,
        PROBE_BITSTREAM_NBITS)) {   
    TLOG(TLVL_ERROR)<<__func__ << " Failed to load PROBE bit stream";
    return false;
    
  }
  if(!ASCIIToBitStream(
        ps_.get<std::string>("SlowControlBitStream"+std::to_string(iFEB)),
        SlowControlBitStream,
        SLOW_CONTROL_BITSTREAM_NBITS)) {   
    TLOG(TLVL_ERROR)<< __func__  << " Failed to load Slow Control bit stream";
    return false;
    throw cet::exception(std::string(TRACE_NAME) + "::" + __func__ + " Failed to load Slow Control bit stream");
  }
  BitstreamToHuman();
  return true;
}


std::vector<std::string> sbndaq::BernCRTFEBConfiguration::channel_parameter_names = {
  /**
   * This vector is needed to know the order of the channel parameters in the FHiCL array
   */
  "time_threshold_adjustment",
  "charge_threshold_adjustment",
  "activate_discriminator",
  "hv",
  "high_high_bias",
  "HG_gain",
  "LG_gain",
  "test_HG",
  "test_LG",
  "enable_preamp"
};

void sbndaq::BernCRTFEBConfiguration::InitializeParameters() {
  /**
   * This function holds description of all parameters
   * It allows to manipulate them easily, read from FHiCL file and convert to
   * bitstream or vice versa.
   * 
   * It initializes four maps:
   *  · boolean_parameters
   *  · numerical_parameters
   *  · boolean_channel_parameters
   *  · numerical_channel_parameters
   * The first two hold information of parameters that appear only once for an FEB.
   * As boolean parameters require only 1 bit, and have values of false or true,
   * they use simpler structure.
   * The last two maps hold information of parameters for each of the 32 channels.
   * Their names are in format like "ch13_time_threshold_adjustment".
   * 
   * Each entry initializes dummy value for the parameter (0 or false), as well as
   * location in the bitstream, a comment (as needed) and, for the numerical values,
   * maximum allowed value
   */

  
  for(int channel = 0; channel < 32; channel++) {
    std::string ch = "channel"+std::to_string(channel)+"_";
    numerical_channel_parameters[ch+"time_threshold_adjustment"]   = {0, 15, uint16_t(  0 + channel* 4), uint16_t(  3 + channel* 4), ""};  
    numerical_channel_parameters[ch+"charge_threshold_adjustment"] = {0, 15, uint16_t(128 + channel* 4), uint16_t(131 + channel* 4), ""};  
    numerical_channel_parameters[ch+"hv"]                          = {0,255, uint16_t(331 + channel* 9), uint16_t(338 + channel* 9), ""};  
    numerical_channel_parameters[ch+"HG_gain"]                     = {0, 31, uint16_t(619 + channel*15), uint16_t(624 + channel*15), ""};  
    numerical_channel_parameters[ch+"LG_gain"]                     = {0, 31, uint16_t(625 + channel*15), uint16_t(630 + channel*15), ""};
    boolean_channel_parameters  [ch+"activate_discriminator"] = {false, uint16_t(265 + channel* 1), false, ""};
    boolean_channel_parameters  [ch+"high_high_bias"]         = {false, uint16_t(339 + channel* 9), false, ""};
    boolean_channel_parameters  [ch+"test_HG"]                = {false, uint16_t(631 + channel*15), false, ""};
    boolean_channel_parameters  [ch+"test_LG"]                = {false, uint16_t(632 + channel*15), false, ""};
    boolean_channel_parameters  [ch+"enable_preamp"]          = {false, uint16_t(633 + channel*15), false, ""};
  }

//                                                         value, bit, inverted_logic, comment
  boolean_parameters["enable_charge_discriminator"]     = {false, 256, false, ""};
  boolean_parameters["enable_charge_discriminator_PP"]  = {false, 257, true,  ""};
  boolean_parameters["latched_output"]                  = {false, 258, false, "True: latched output, False: direct output"};

  boolean_parameters["enable_time_discriminator"]       = {false, 259, false, ""};
  boolean_parameters["enable_time_discriminator_PP"]    = {false, 260, true,  ""};

  boolean_parameters["enable_4bit_charge_DAC"]          = {false, 261, false, ""};
  boolean_parameters["enable_4bit_charge_DAC_PP"]       = {false, 262, false, ""};
  boolean_parameters["enable_4bit_time_DAC"]            = {false, 263, false, ""};
  boolean_parameters["enable_4bit_time_DAC_PP"]         = {false, 264, false, ""};

  boolean_parameters["enable_HG_SCA_PP"]                = {false, 297, false, ""};
  boolean_parameters["enable_HG_SCA"]                   = {false, 298, true,  ""};
  boolean_parameters["enable_LG_SCA_PP"]                = {false, 299, false, ""};
  boolean_parameters["enable_LG_SCA"]                   = {false, 300, true,  ""};

  boolean_parameters["SCA_high_bias"]                   = {false, 301, false, "True: high bias 5MHz readout speed, False: weak bias"};

  boolean_parameters["HG_peak_detector_PP"]             = {false, 302, false, "True: power pin, False: forced"};
  boolean_parameters["enable_HG_peak_detector"]         = {false, 303, true,  ""};

  boolean_parameters["LG_peak_detector_PP"]             = {false, 304, false, "True: power pin, False: forced"};
  boolean_parameters["enable_LG_peak_detector"]         = {false, 305, true,  ""};

  boolean_parameters["select_HG_mode"]                  = {false, 306, false, "True: track & hold (SCA), False: peak detector"};
  boolean_parameters["select_LG_mode"]                  = {false, 307, false, ""};

  boolean_parameters["bypass_peak_sensing_cell"]        = {false, 308, false, ""};
  boolean_parameters["peak_sensing_cell"]               = {false, 309, false, "True: external trigger, False: internal trigger"};

  boolean_parameters["enable_fast_shaper_follower_PP"]  = {false, 310, true,  ""};
  boolean_parameters["enable_fast_shaper"]              = {false, 311, false, ""};
  boolean_parameters["enable_fast_shaper_PP"]           = {false, 312, true,  ""};

  boolean_parameters["enable_LG_slow_shaper_PP"]        = {false, 313, true,  ""};
  boolean_parameters["enable_LG_slow_shaper"]           = {false, 314, false, ""};
//                                                        value, max_value, first_bit last_bit, comment
  numerical_parameters["LG_shaper_time_constant"]       = {0, 7, 315, 317, "0: 87.5ns, 1: 75.0ns, 2: 62.5ns, 3: 50.0ns, 4: 37.5ns, 5: 25.0ns, 6: 12.5ns, 7: undefined?"};

  boolean_parameters["enable_HG_slow_shaper_PP"]        = {false, 318, true, ""};
  boolean_parameters["enable_HG_slow_shaper"]           = {false, 319, false, ""};
  numerical_parameters["HG_shaper_time_constant"]       = {0, 7, 320, 322, "0: 87.5ns, 1: 75.0ns, 2: 62.5ns, 3: 50.0ns, 4: 37.5ns, 5: 25.0ns, 6: 12.5ns, 7: undefined?"};

  boolean_parameters["LG_preamp_bias"]                  = {false, 323, true,  "True: normal, False: weak"};

  boolean_parameters["enable_HG_preamp_PP"]             = {false, 324, true,  ""};
  boolean_parameters["enable_HG_preamp"]                = {false, 325, false, ""};

  boolean_parameters["enable_LG_preamp_PP"]             = {false, 326, true,  ""};
  boolean_parameters["enable_LG_preamp"]                = {false, 327, false, ""};

  boolean_parameters["preamp_connected_to_fast_shaper"] = {false, 328, false, "True: HG, False: LG"};

  boolean_parameters["enable_32_input_8bit_DAC"]        = {false, 329, false, ""};
  boolean_parameters["input_DAC_voltage_reference"]     = {false, 330, false, "True: external (?) 4.5V, False: internal 2.5V"};

  boolean_parameters["enable_temperature_sensor_PP"]    = {false, 1099, true,  ""};
  boolean_parameters["enable_temperature_sensor"]       = {false, 1100, false, ""};

  boolean_parameters["enable_BandGap_sensor_PP"]        = {false, 1101, true,  ""};
  boolean_parameters["enable_BandGap_sensor"]           = {false, 1102, false, ""};

  boolean_parameters["enable_charge_DAC"]               = {false, 1103, false, ""};
  boolean_parameters["enable_charge_DAC_PP"]            = {false, 1104, true,  ""};

  boolean_parameters["enable_time_DAC"]                 = {false, 1105, false, ""};
  boolean_parameters["enable_time_DAC_PP"]              = {false, 1106, true,  ""};

  numerical_parameters["charge_threshold"]              = {0, 1023, 1107, 1116, "DAC1"};
  numerical_parameters["time_threshold"]                = {0, 1023, 1117, 1126, "DAC2"};

  boolean_parameters["enable_HG_OTA"]                   = {false, 1127, false, ""};
  boolean_parameters["enable_HG_OTA_PP"]                = {false, 1128, true,  ""};

  boolean_parameters["enable_LG_OTA"]                   = {false, 1129, false, ""};
  boolean_parameters["enable_LG_OTA_PP"]                = {false, 1130, true,  ""};

  boolean_parameters["enable_probe_OTA"]                = {false, 1131, false, ""};
  boolean_parameters["enable_probe_OTA_PP"]             = {false, 1132, true,  ""};

  boolean_parameters["enable_OTAQ_test_bit"]            = {false, 1133, false, ""};

  boolean_parameters["enable_VAL_EVT_OTA"]              = {false, 1134, false, ""};
  boolean_parameters["enable_VAL_EVT_OTA_PP"]           = {false, 1135, true,  ""};

  boolean_parameters["enable_RAZ_CHN_OTA"]              = {false, 1136, false, ""};
  boolean_parameters["enable_RAZ_CHN_OTA_PP"]           = {false, 1137, true,  ""};

  boolean_parameters["enable_digital_multiplexed_output"] = {false, 1138, false, ""};
  boolean_parameters["enable_OR32_output"]                = {false, 1139, false, ""};
  boolean_parameters["enable_OR32_open_collector_output"] = {false, 1140, false, ""};

  boolean_parameters["trigger_polarity"]                  = {false, 1141, false, "True: positive (rising edge), False: negative (falling edge)"};

  boolean_parameters["enable_digital_OR32_T_open_collector_output"] = {false, 1142, false, ""};
  boolean_parameters["enable_32_channels_triggers_output"]          = {false, 1143, false, ""};
  
}

bool sbndaq::BernCRTFEBConfiguration::read_human_readable_parameters(fhicl::ParameterSet const & ps_, int MAC5) {
  InitializeParameters();
  
  //read global parameters
  for(auto parameter : boolean_parameters) {
    boolean_parameters[parameter.first].value = ps_.get<bool>(parameter.first);
  }
  for(auto parameter : numerical_parameters) {
    numerical_parameters[parameter.first].value = ps_.get<uint16_t>(parameter.first);
    if(numerical_parameters[parameter.first].value > numerical_parameters[parameter.first].max_value) {
      TLOG(TLVL_ERROR)<<__func__ << " Failed to load "<<parameter.first<<" for MAC "<<MAC5<<": "<<numerical_parameters[parameter.first].value<<" > "<<numerical_parameters[parameter.first].max_value;
      return false;
    }
  }
  
  //read individual channel parameters
  //they are stored in an array, which we will split into individual parameters
  std::vector< std::vector<uint16_t> > fhicl_channel_array  = ps_.get< std::vector< std::vector<uint16_t> > >("channel_configuration");
  
  if(fhicl_channel_array.size() != 32) {
    TLOG(TLVL_ERROR)<<__func__ << " Invalid size of channel configuration for MAC "<<MAC5<<": "<<fhicl_channel_array.size()<<" ≠ 32";
    return false;
  }
  for(int channel = 0; channel < 32; channel++) {
    if(fhicl_channel_array[channel].size() != 10) {
      TLOG(TLVL_ERROR)<<__func__ << " Invalid size of channel "<<channel<<" configuration for MAC "<<MAC5<<": "<<fhicl_channel_array[channel].size()<<" ≠ 10";
      return false;
    }
    
    for(unsigned int parameter = 0; parameter < channel_parameter_names.size(); parameter++) {
      std::string name = "channel"+std::to_string(channel)+"_"+channel_parameter_names[parameter];
      //check if it is boolean parameter or numerical parameter
      if(boolean_channel_parameters.count(name)) {
        boolean_channel_parameters[name].value = fhicl_channel_array[channel][parameter];
      }
      else if(numerical_channel_parameters.count(name)) {
        if(fhicl_channel_array[channel][parameter] > numerical_channel_parameters[name].max_value) {
          TLOG(TLVL_ERROR)<<__func__ << " MAC "<<MAC5<<", channel "<<channel<<", parameter "<<parameter<<" ("<<name<<") value "<<fhicl_channel_array[channel][parameter]<<" > "<<numerical_channel_parameters[name].max_value;
          return false;
        }
        numerical_channel_parameters[name].value = fhicl_channel_array[channel][parameter];
      }
      else { //oopsie
        TLOG(TLVL_ERROR)<<__func__ << " Parameter "<<parameter<<" not found, internal error, blame the coder";
        return false;
      }
    }
  }
  
  HumanToBitstream();
  return true;
}

void sbndaq::BernCRTFEBConfiguration::HumanToBitstream() {
  /**
   * Converts parameters from human readable format to SlowControlBitStream
   */
  for(auto parameter : boolean_parameters)
    SetBit(parameter.second.bit_number, parameter.second.value);
  for(auto parameter : boolean_channel_parameters)
    SetBit(parameter.second.bit_number, parameter.second.value);
  for(auto parameter : numerical_parameters)
    SetBits(parameter.second.first_bit, parameter.second.last_bit, parameter.second.value);
  for(auto parameter : numerical_channel_parameters)
    SetBits(parameter.second.first_bit, parameter.second.last_bit, parameter.second.value);
}

void sbndaq::BernCRTFEBConfiguration::BitstreamToHuman() {
  /**
   * Converts parameters from human readable format to SlowControlBitStream
   */
  for(auto parameter : boolean_parameters)
    boolean_parameters[parameter.first].value = GetBit(parameter.second.bit_number);
  for(auto parameter : boolean_channel_parameters)
    boolean_channel_parameters[parameter.first].value = GetBit(parameter.second.bit_number);
  for(auto parameter : numerical_parameters)
    numerical_parameters[parameter.first].value = GetBits(parameter.second.first_bit, parameter.second.last_bit);
  for(auto parameter : numerical_channel_parameters)
    numerical_channel_parameters[parameter.first].value = GetBits(parameter.second.first_bit, parameter.second.last_bit);
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
   * returns value of the given bit from the SlowControlBitStream.
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

bool sbndaq::BernCRTFEBConfiguration::GetBit(unsigned int bit_number) {
  /**
   * Gets bit value of the Slow Control bit stream
   */
  return GetBit(bit_number, SlowControlBitStream, SLOW_CONTROL_BITSTREAM_NBITS);
}

void sbndaq::BernCRTFEBConfiguration::SetBits(unsigned int first_bit, unsigned int last_bit, uint16_t value) {
  /**
   * Set given range of bits from the SlowControlBitStream
   */
  for(unsigned int bit_number = first_bit; bit_number <= last_bit; bit_number++)
    SetBit(bit_number, value & (1<<(bit_number-first_bit)));
}

uint16_t sbndaq::BernCRTFEBConfiguration::GetBits(unsigned int first_bit, unsigned int last_bit) {
  /**
   * Get given range of bits from the SlowControl bitstream
   */
  uint16_t r = 0;
  for(unsigned int bit_number = first_bit; bit_number <= last_bit; bit_number++)
    r |= GetBit(bit_number) << (bit_number-first_bit);
  return r;
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

std::string sbndaq::BernCRTFEBConfiguration::BitsToASCII(unsigned int first_bit, unsigned int last_bit) {
  /**
   * Returns ASCII representation of given range in the Slow Control bitstream.
   * If last bit is not specified, only one bit is printed
   */
  if(last_bit >= SLOW_CONTROL_BITSTREAM_NBITS) {
    TLOG(TLVL_ERROR)<< __func__ << " Last bit " << std::to_string(last_bit) << "exceeds Slow Control bitstream length!";
    return "-";
  }
  std::string s = "";
  for(unsigned int iBit = first_bit; iBit <= last_bit; iBit++) {
    s += std::to_string(GetBit(iBit));
  }
  return s;
}

std::string sbndaq::BernCRTFEBConfiguration::BitsToASCII(unsigned int first_bit) {
  return sbndaq::BernCRTFEBConfiguration::BitsToASCII(first_bit, first_bit);
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
int32_t   sbndaq::BernCRTFEBConfiguration::GetPPSOffset()                  { return PPS_offset; }
