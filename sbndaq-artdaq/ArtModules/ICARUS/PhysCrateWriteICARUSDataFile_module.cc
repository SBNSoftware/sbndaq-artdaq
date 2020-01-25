////////////////////////////////////////////////////////////////////////
// Class:       PhysCrateWriteICARUSDataFile
// Module Type: analyzer
// File:        PhysCrateWriteICARUSDataFile_module.cc
// Description: Writes data in a QSCAN ready format, hopefully...
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/Handle.h"

#include "art/Utilities/ExceptionMessages.h"
#include "sbndaq-artdaq-core/Overlays/ICARUS/PhysCrateFragment.hh"
#include "sbndaq-artdaq-core/Overlays/ICARUS/PhysCrateStatFragment.hh"
#include "artdaq-core/Data/Fragment.hh"

#include "sbndaq-artdaq/Generators/ICARUS/icarus-base/common.h"
#include "sbndaq-artdaq-core/Overlays/ICARUS/packs.h"


#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

#include <arpa/inet.h>

#define EVEN 0x4556454E
#define DATA 0x44415441
#define STAT 0x53544154

#ifdef TRACE_NAME
#undef TRACE_NAME
#endif

#define TRACE_NAME "PhysCrateWriteICARUSDataFile_module"

namespace icarus {
  class PhysCrateWriteICARUSDataFile;
}

class icarus::PhysCrateWriteICARUSDataFile : public art::EDAnalyzer {
public:
  explicit PhysCrateWriteICARUSDataFile(fhicl::ParameterSet const & pset);
  virtual ~PhysCrateWriteICARUSDataFile();

  virtual void analyze(art::Event const & evt);

  virtual void beginSubRun(art::SubRun const&) override;
  virtual void endSubRun(art::SubRun const&) override;

private:
  std::string raw_data_label_;
  std::string file_output_name_;
  std::string file_output_location_;

  std::ofstream output_file_;
};


icarus::PhysCrateWriteICARUSDataFile::PhysCrateWriteICARUSDataFile(fhicl::ParameterSet const & pset)
  : EDAnalyzer(pset),
    raw_data_label_(pset.get<std::string>("raw_data_label")),
    file_output_name_(pset.get<std::string>("file_output_name")),
    file_output_location_(pset.get<std::string>("file_output_location"))
    
{
}

icarus::PhysCrateWriteICARUSDataFile::~PhysCrateWriteICARUSDataFile()
{
  
}

void icarus::PhysCrateWriteICARUSDataFile::beginSubRun(art::SubRun const&)
{

}
void icarus::PhysCrateWriteICARUSDataFile::endSubRun(art::SubRun const&)
{
}

void icarus::PhysCrateWriteICARUSDataFile::analyze(art::Event const & evt)
{
  
  art::EventNumber_t eventNumber = evt.event();
  
  std::stringstream fn;
  fn << file_output_location_ << "/" << file_output_name_ << "_"
     << evt.run() << "_" << evt.subRun() << "_" << eventNumber << ".qscan";
  output_file_.open(fn.str(),std::ofstream::binary | std::ofstream::out);

  // ***********************
  // *** PhysCrate Fragments ***
  // ***********************
  
  // look for raw PhysCrate data
  
  art::Handle< std::vector<artdaq::Fragment> > raw;
  evt.getByLabel(raw_data_label_, "PHYSCRATEDATA", raw);
  
  art::Handle< std::vector<artdaq::Fragment> > raw_stat;
  evt.getByLabel(raw_data_label_, "PHYSCRATESTAT", raw_stat);

  if(!raw.isValid()){
    TLOG(TLVL_INFO) <<   "Run " << evt.run() << ", subrun " << evt.subRun()
              << ", event " << eventNumber << " has zero"
              << " PhysCrate fragments " << " in module " << raw_data_label_;
    return;
  }
  
  
  TLOG(TLVL_INFO) <<   "######################################################################";
  TLOG(TLVL_INFO) <<   "Run " << evt.run() << ", subrun " << evt.subRun()
	    << ", event " << eventNumber << " has " << raw->size()
	    << " PhysCrate fragment(s)." ;

  evHead header;
  header.token = (int)htonl(EVEN);
  header.Run = (int)htonl(evt.run());
  header.Event = (int)htonl(eventNumber);
  header.ToD = htonl(0);
  header.AbsTime = htonl(0);
  header.Conf = htonl(1); //number of crates
  header.Size = htonl(28);

  output_file_.write((char*)&header,sizeof(evHead));
  
  for (size_t idx = 0; idx < raw->size(); ++idx) {
    const auto& frag((*raw)[idx]);
    output_file_.write((char*)frag.dataBeginBytes(),frag.dataSizeBytes());
  }
  for(size_t idx = 0; idx < raw_stat->size(); ++idx){
    const auto& frag((*raw_stat)[idx]);
    output_file_.write((char*)frag.dataBeginBytes(),frag.dataSizeBytes());
  }

  output_file_.close();

}


DEFINE_ART_MODULE(icarus::PhysCrateWriteICARUSDataFile)
