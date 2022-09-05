#define TRACE_NAME "BernCRTSerialReader_Test"

#include "sbndaq-artdaq/Generators/SBND/BernCRTSerialReader.hh"
#include <boost/program_options.hpp>

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
  auto brpset = pset.get<fhicl::ParameterSet>("fragment_receiver");

  if (verbose) {
    brpset.put_or_replace("verbose", true);
  }

  auto reader = std::make_unique<BernCRTSerialReader>(brpset);

  reader->StartCmd(1, 0, 0);

  artdaq::FragmentPtrs frags;

  while (true) {
    bool success = reader->getNext_(frags);
    for(auto itr = frags.begin(); itr != frags.end(); ++itr)
      {
	artdaq::Fragment frag(**itr);
	if(frag.type() == sbndaq::detail::FragmentType::BERNCRTV2)
	  std::cout << sbndaq::BernCRTFragmentV2(frag);
	else
	  std::cout << frag;
      }

    frags.clear();
    if (!success) break;
  }

  reader->StopCmd(0, 0);
  return 0;
  
}
catch (const std::exception& ex)
{
    std::cout << '\n'
	      << "Exited with std::exception: " << ex.what()
	      << "\n\n"
	      << "****************************************\n"
	      << "**  Reached end of file - finishing!  **\n"
	      << "****************************************\n" << std::endl;
}
catch (...)
{
   std::cerr << "Process exited with error: " << boost::current_exception_diagnostic_information();
   return 128;
}
