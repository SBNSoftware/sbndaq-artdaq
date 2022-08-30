#include "sbndaq-artdaq/Generators/SBND/BernCRTSerialReader.hh"
#define TRACE_NAME "BernCRTSerialReader_Test"
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

#include "sbndaq-artdaq/Generators/Common/PoolBuffer.hh"

#include <boost/archive/binary_iarchive.hpp>

namespace bpo = boost::program_options;

using sbndaq::BernCRTSerialReader;
int main(int argc, char* argv[]) try {
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
  auto events_to_generate = pset.get<uint64_t>("events_to_generate", 0);
  auto brpset = pset.get<fhicl::ParameterSet>("fragment_receiver");
  brpset.put_or_replace("events_to_generate", events_to_generate);

  if (verbose) {
    brpset.put_or_replace("verbose", true);
  }

  try {
    auto metric_pset = pset.get<fhicl::ParameterSet>("metrics");
    if (metric_pset.is_empty()) {
      metricMan.reset();
    } else {
      metricMan->initialize(metric_pset, "artdaqDriver");
      metricMan->do_start();
    }
  } catch (...) {
    std::cerr << "Wrong metrics configuration, exiting.";
    return 129;
  }

  std::ifstream file("/home/nfs/sbnd/DAQ_DevAreas/DAQ_14Aug2022SimData/work/binary_test.dat");
  bool eof = false;

  artdaq::FragmentPtrs frags;

  while(!eof)
    {
      try
	{
	  sbndaq::BernCRTFragmentSerial serial;
	  boost::archive::binary_iarchive ia(file);
	  ia >> serial;

	  frags.emplace_back(artdaq::Fragment::FragmentBytes(
			     sizeof(sbndaq::BernCRTHitV2) * serial.n_hits,
			     serial.sequence_id,
			     serial.fragment_id,
			     serial.fragment_type,
			     serial.metadata,
			     serial.timestamp)
			     );

	  memcpy(frags.back()->dataBeginBytes(),
		 serial.bern_crt_hits.data(),
		 sizeof(sbndaq::BernCRTHitV2) * serial.n_hits
		 );

	  if(frags.back()->type() == sbndaq::detail::FragmentType::BERNCRTV2)
	    {
	      sbndaq::BernCRTFragmentV2 bern_frag(*frags.back());
	      std::cout << bern_frag;
	    }
	}
      catch (const std::exception& ex)
	{
	  std::cout << '\n'
		    << "Exited with std::exception: " << ex.what()
		    << "\n\n"
		    << "****************************************\n"
		    << "**  Reached end of file - finishing!  **\n"
		    << "****************************************\n" << std::endl;
	  eof = true;
	}
    }

  /*
  auto br = std::make_unique<BernCRTSerialReader>(brpset);

  br->StartCmd(1, 0, 0);

  artdaq::FragmentPtrs fps;

  while (true) {
    bool sts = br->getNext_(fps);
    fps.clear();
    if (!sts) break;
  }

  br->StopCmd(0, 0);
  */
  return 0;
  
} catch (...) {
  std::cerr << "Process exited with error: " << boost::current_exception_diagnostic_information();
  return 128;
}
