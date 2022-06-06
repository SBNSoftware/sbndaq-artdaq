#define TRACE_NAME "SPECTDC_Test"
#include <fstream>
#include <iostream>

#include <boost/any.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/program_options.hpp>
#include <cstddef>

#include "artdaq/DAQdata/Globals.hh"
#include "cetlib_except/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/intermediate_table.h"
#include "fhiclcpp/parse.h"

#include "SPECTDC_Interface.hh"
#include "sbndaq-artdaq/Generators/Common/PoolBuffer.hh"

namespace bpo = boost::program_options;

int main(int argc, char* argv[]) try {
  metricMan.reset();
  std::ostringstream descstr;
  descstr << argv[0] << " <-c <config-file>> <other-options>";

  bpo::options_description desc = descstr.str();

  desc.add_options()("config,c", bpo::value<std::string>(), "Configuration file.")("help,h", "Produce help message.")(
      "verbose,v", "Verbose mode.");

  bpo::variables_map vm;

  try {
    bpo::store(bpo::command_line_parser(argc, argv).options(desc).run(), vm);
    bpo::notify(vm);
  } catch (bpo::error const& e) {
    std::cerr << "Exception from command line processing in " << argv[0] << ": " << e.what() << "\n";
    return -1;
  }

  if (vm.count("help") != 0u) {
    std::cout << desc << std::endl;
    return 1;
  }

  bool verbose = false;

  if (vm.count("verbose") != 0u) {
    verbose = true;
  }

  if (vm.count("config") == 0u) {
    std::cerr << "Exception from command line processing in " << argv[0] << ": no configuration file given.\n"
              << "For usage and an options list, please do '" << argv[0] << " --help"
              << "'.\n";
    return 2;
  }

  if (getenv("FHICL_FILE_PATH") == nullptr) {
    std::cerr << "INFO: environment variable FHICL_FILE_PATH was not set. Using \".\"\n";
    setenv("FHICL_FILE_PATH", ".", 0);
  }

  auto file_name = vm["config"].as<std::string>();
  std::ifstream is(file_name);
  std::string conf((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
  is.close();
  auto pset = fhicl::ParameterSet::make(conf);
  auto spec = pset.get<fhicl::ParameterSet>("spec");
  if (verbose) {
    spec.put_or_replace("verbose", true);
  }

  std::unique_ptr<sbndaq::PoolBuffer> buffer = std::make_unique<sbndaq::PoolBuffer>();
  auto frag_size = sizeof(sbndaq::TDCTimestamp);
  auto buff_size = pset.get<decltype(frag_size)>("data_buffer_depth_fragments", 512);
  buffer->allocate(frag_size, buff_size, true);
  auto tdcCard = std::make_unique<sbndaq::SPECTDCInterface::SPECCard>(spec, buffer);

  tdcCard->configure();

  tdcCard->start();

  while (!buffer->isEmpty()) tdcCard->read();

  tdcCard->stop();

  auto as_string = [](auto const& b) {
    std::stringstream os;
    os << *reinterpret_cast<sbndaq::TDCTimestamp*>(b->begin);
    return os.str();
  };

  while (buffer->activeBlockCount()) {
    auto block = buffer->takeActiveBlock();
    TLOG(TLVL_DEBUG + 30) << as_string(block);
    buffer->returnFreeBlock(block);
  }

  return 0;
} catch (...) {
  std::cerr << "Process exited with error: " << boost::current_exception_diagnostic_information();
  return 128;
}
