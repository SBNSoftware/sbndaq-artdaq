//-------------------------------------------------
//---------------------------------------

////////////////////////////////////////////////////////////////////////
// Class:       FragmentToBinary
// Module Type: analyzer
// File:        FragmentToBinary_module.cc
// Description: Takes fragments and unpacks them into a binary file
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"

#include "canvas/Utilities/Exception.h"

#include "sbndaq-artdaq-core/Overlays/Common/CAENV1730Fragment.hh"
#include "sbndaq-artdaq-core/Overlays/SBND/CAENV1730FragmentSerial.hh"
#include "sbndaq-artdaq-core/Overlays/Common/WhiteRabbitFragment.hh"
#include "sbndaq-artdaq-core/Overlays/Common/BernCRTFragmentV2.hh"
#include "sbndaq-artdaq-core/Overlays/SBND/BernCRTFragmentV2Serial.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "artdaq-core/Data/Fragment.hh"
#include "artdaq-core/Data/ContainerFragment.hh"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <vector>
#include <iostream>
#include <bitset>
#include <random>

#include <boost/archive/binary_oarchive.hpp>

namespace sbndaq {
  class FragmentToBinary;
}

/**************************************************************************************************/

class sbndaq::FragmentToBinary : public art::EDAnalyzer {

public:
  struct Config {
    //--one atom for each parameter
    fhicl::Atom<bool> include_caen {
      fhicl::Name("include_caen"),
      fhicl::Comment("look for caen 1730 fragments true/false")
    };
    fhicl::Atom<bool> include_wr {
      fhicl::Name("include_wr"),
      fhicl::Comment("look for wr dio fragments true/false")
    };
    fhicl::Atom<bool> include_berncrt {
      fhicl::Name("include_berncrt"),
      fhicl::Comment("look for bern CRT V2 fragments true/false")
    };
    fhicl::Atom<unsigned> verbose {
      fhicl::Name("verbose"),
      fhicl::Comment("define the amount of text to print")
    };
    fhicl::Atom<std::string> caen_file_name {
      fhicl::Name("caen_file_name"),
      fhicl::Comment("name for binary output file for CAEN fragments")
    };
    fhicl::Atom<std::string> berncrt_file_name {
      fhicl::Name("berncrt_file_name"),
      fhicl::Comment("name for binary output file for BernCRT fragments")
    };
  }; //--configuration
  using Parameters = art::EDAnalyzer::Table<Config>;

  explicit FragmentToBinary(Parameters const & pset);
  virtual ~FragmentToBinary();

  void analyze(const art::Event& evt) override;
  void beginJob() override;
  void endJob() override;

  void ProcessCAENV1730(const artdaq::Fragment &frag);
  void ProcessWhiteRabbit(const artdaq::Fragment &frag);
  void ProcessBernCRTV2(const artdaq::Fragment &frag);

private:

  int fRun;
  art::EventNumber_t fEvent;

  bool finclude_caen;
  bool finclude_wr;
  bool finclude_berncrt;
  unsigned fverbose;
  std::string fberncrt_file_name, fcaen_file_name;

  std::ofstream fcaen_binary_file, fberncrt_binary_file;
}; //--class FragmentToBinary


sbndaq::FragmentToBinary::FragmentToBinary(FragmentToBinary::Parameters const& pset): art::EDAnalyzer(pset)
{
  finclude_caen      = pset().include_caen();
  finclude_wr        = pset().include_wr();
  finclude_berncrt   = pset().include_berncrt();
  fverbose           = pset().verbose();
  fcaen_file_name    = pset().caen_file_name();
  fberncrt_file_name = pset().berncrt_file_name();
}

void sbndaq::FragmentToBinary::beginJob()
{
  if (fverbose > 0)  std::cout << "Starting FragmentToBinary...\n";
  fcaen_binary_file    = std::ofstream(fcaen_file_name);
  fberncrt_binary_file = std::ofstream(fberncrt_file_name);
}

void sbndaq::FragmentToBinary::endJob()
{
  if (fverbose > 0)  std::cout << "Ending FragmentToBinary...\n";
}


sbndaq::FragmentToBinary::~FragmentToBinary()
{
  boost::archive::binary_oarchive caen_archive(fcaen_binary_file);
  caen_archive << sbndaq::detail::FragmentType::INVALID;

  boost::archive::binary_oarchive berncrt_archive(fberncrt_binary_file);
  berncrt_archive << sbndaq::detail::FragmentType::INVALID;
}


void sbndaq::FragmentToBinary::analyze(const art::Event& evt)
{
  fRun = evt.run();
  fEvent = evt.event();
  if (fverbose > 0)   std::cout << "Run: " << fRun << " event: " << fEvent << std::endl;
  
  std::vector<art::Handle<artdaq::Fragments>> fragmentHandles;
  
#if ART_HEX_VERSION < 0x30900
  evt.getManyByType(fragmentHandles);
#else
  fragmentHandles = evt.getMany<std::vector<artdaq::Fragment>>();
#endif



  /************************************************************************************************/

  // Loop fragment handles
  for (auto handle : fragmentHandles) {
    if (!handle.isValid() || handle->size() == 0) continue;

    // Container or standard?
    
    if (handle->front().type() == artdaq::Fragment::ContainerFragmentType) {
      for (auto cont : *handle) {
	artdaq::ContainerFragment contf(cont);

	// Type of fragment?

	if (contf.fragment_type() == sbndaq::detail::FragmentType::CAENV1730 && finclude_caen) {

	  if (fverbose > 0)
	    std::cout << "\tFound " << contf.block_count() << " CAEN Fragments in container " << std::endl;

	  for (size_t ii = 0; ii < contf.block_count(); ++ii)
	    ProcessCAENV1730(*contf[ii].get());
	} 
	else if (contf.fragment_type() == sbndaq::detail::FragmentType::WhiteRabbit && finclude_wr) {
	  
	  if (fverbose > 0)
	    std::cout << "\tFound " << contf.block_count() << " WR Fragments in container " << std::endl;
	  
	  for (size_t ii = 0; ii < contf.block_count(); ++ii)
	    ProcessWhiteRabbit(*contf[ii].get());
	}
	else if (contf.fragment_type() == sbndaq::detail::FragmentType::BERNCRTV2 && finclude_berncrt) {

	  if (fverbose > 0)
	    std::cout << "\tFound " << contf.block_count() << " BERNCRT Fragments in container " << std::endl;

	  for (size_t ii = 0; ii < contf.block_count(); ++ii)
	    ProcessBernCRTV2(*contf[ii].get());
	}
      }
    }
    else {
      
      // Type of fragment?

      if (handle->front().type() == sbndaq::detail::FragmentType::CAENV1730 && finclude_caen) {
	
	if (fverbose > 0)
	  std::cout << "\tFound " << handle->size() << " normal CAEN fragments" << std::endl;

	for (auto frag : *handle)
	  ProcessCAENV1730(frag);
      }

      else if (handle->front().type()==sbndaq::detail::FragmentType::WhiteRabbit && finclude_wr) {

	if (fverbose > 0)
	  std::cout << "\tFound " << handle->size() << " normal WR fragments" << std::endl;

	for (auto frag : *handle)
	  ProcessWhiteRabbit(frag);
      }
      else if (handle->front().type() == sbndaq::detail::FragmentType::BERNCRTV2 && finclude_berncrt) {

	if (fverbose > 0)
	  std::cout << "\tFound " << handle->size() << " normal BERNCRT fragments" << std::endl;

        for (auto frag : *handle)
	  ProcessBernCRTV2(frag);
      }
    }
  }
}

void sbndaq::FragmentToBinary::ProcessCAENV1730(const artdaq::Fragment &frag)
{
  boost::archive::binary_oarchive oa(fcaen_binary_file);
  CAENV1730Fragment caen_frag(frag);
  sbndaq::CAENV1730FragmentSerial serial;

  serial.fragment_type     = frag.type();
  serial.sequence_id       = frag.sequenceID();
  serial.fragment_id       = frag.fragmentID();
  serial.timestamp         = frag.timestamp();
  serial.metadata          = *caen_frag.Metadata();
  serial.event             = *caen_frag.Event();

  const uint16_t* data_begin = reinterpret_cast<const uint16_t*>(frag.dataBeginBytes() + sizeof(CAENV1730EventHeader));

  const uint32_t ev_size_quad_bytes         = serial.event.Header.eventSize;
  const uint32_t evt_header_size_quad_bytes = sizeof(CAENV1730EventHeader) / sizeof(uint32_t);
  const uint32_t data_size_double_bytes     = 2 * (ev_size_quad_bytes - evt_header_size_quad_bytes);
  const uint32_t wfm_length                 = data_size_double_bytes / serial.metadata.nChannels;

  const uint16_t* value_ptr = data_begin;
  uint16_t value = 0;
  size_t ch_offset = 0;

  serial.fWvfmsVec.resize(serial.metadata.nChannels);

  for (size_t i_ch = 0; i_ch < serial.metadata.nChannels; ++i_ch){
    serial.fWvfmsVec[i_ch].resize(wfm_length);
    ch_offset = (size_t)(i_ch * wfm_length);

    for(size_t i_t = 0; i_t < wfm_length; ++i_t){
      value_ptr = data_begin + ch_offset + i_t;
      value = *(value_ptr);
      serial.fWvfmsVec[i_ch][i_t] = value;
    }
  }

  oa << frag.type() << serial;
  if (fverbose > 1)
    std::cout << '\n' << frag << '\n' << caen_frag;
}

void sbndaq::FragmentToBinary::ProcessWhiteRabbit(const artdaq::Fragment &/*frag*/)
{
}

void sbndaq::FragmentToBinary::ProcessBernCRTV2(const artdaq::Fragment &frag)
{
  boost::archive::binary_oarchive oa(fberncrt_binary_file);
  BernCRTFragmentV2 bern_frag(frag);
  sbndaq::BernCRTFragmentV2Serial serial;

  serial.fragment_type     = frag.type();
  serial.sequence_id       = frag.sequenceID();
  serial.fragment_id       = frag.fragmentID();
  serial.timestamp         = frag.timestamp();
  serial.metadata          = *bern_frag.metadata();
  serial.n_hits            = bern_frag.metadata()->hits_in_fragment();
  serial.data_payload_size = bern_frag.DataPayloadSize();

  for(unsigned hit = 0; hit < serial.n_hits; ++hit)
    serial.bern_crt_hits.push_back(*bern_frag.eventdata(hit));

  oa << frag.type() << serial;
  if (fverbose > 1)
    std::cout << '\n' << frag << '\n' << bern_frag;
}

DEFINE_ART_MODULE(sbndaq::FragmentToBinary)
