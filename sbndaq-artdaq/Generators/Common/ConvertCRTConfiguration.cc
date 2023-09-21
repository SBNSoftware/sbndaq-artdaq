#include "BernCRTFEBConfiguration.hh"

#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/intermediate_table.h"
#include "fhiclcpp/make_ParameterSet.h"
#include "fhiclcpp/parse.h"

// #include <cassert>
#include <cstdlib> //putenv
#include <fstream>
#include <string>
#include <iostream>

#include "boost/program_options.hpp"
#include <iterator>
namespace po = boost::program_options;

po::options_description desc("Allowed options");
void usage() {
  std::cout << desc << std::endl;
}

int main(int argc, char * argv[]) {
  std::string infile_name;
  std::string outfile_name = "";
  bool bitstream_output = false;
  
  try {
    desc.add_options()
        ("help", "produce help message")
        ("input,i", po::value<std::string>(), "input file name")
        ("output,o", po::value<std::string>(), "output file name (optional)")
        ("bitstream,b", "bitstream output format")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        usage();
        return 0;
    }
    
    
    if (vm.count("input")) {
        infile_name = vm["input"].as<std::string>();
    } else {
        std::cout << "Input file name was not set."<<std::endl;
        usage();
        return 1;
    }
    
    if (vm.count("output")) outfile_name = vm["output"].as<std::string>();
    if (vm.count("bitstream")) bitstream_output = true;
  }
  catch(std::exception& e) {
      std::cerr << "error: " << e.what() << "\n";
      return 1;
  }
  catch(...) {
      std::cerr << "Exception of unknown type!\n";
  }
  
  // allow to open FHiCL file from the current directory
  // it is needed to include the files
  putenv(const_cast<char*>("FHICL_FILE_PATH=.:$FHICL_FILE_PATH"));
  cet::filepath_lookup policy("FHICL_FILE_PATH");

	
   // parse a configuration file; obtain intermediate form
		fhicl::intermediate_table tbl =fhicl::parse_document(infile_name, policy);
   // convert to ParameterSet
   fhicl::ParameterSet pset= fhicl::ParameterSet::make(tbl);

  
  fhicl::ParameterSet fragment_receiver = pset.get<fhicl::ParameterSet>("fragment_receiver");
  
  std::vector<uint16_t> fragment_ids = fragment_receiver.get< std::vector<uint16_t> >("fragment_ids");
  std::sort(fragment_ids.begin(), fragment_ids.end());
  std::vector<uint8_t> mac5s;
  for( auto id : fragment_ids ) {
    uint8_t MAC5 = id & 0xff; //last 8 bits of fragment ID are last 8 bits of FEB MAC5
    mac5s.push_back(MAC5);
  }

  std::ofstream outfile;
  if(outfile_name.compare("")) {
    outfile.open (outfile_name);
  }
  std::string s = "";
  
  for(unsigned int iFEB = 0; iFEB < mac5s.size(); iFEB++) {
    std::cout<<"Processing MAC5 "<<mac5s[iFEB]<<std::endl;
    sbndaq::BernCRTFEBConfiguration c(fragment_receiver, iFEB);
    if(bitstream_output) s += c.GetBitStreamString();
    else                 s += c.GetHumanReadableString();
  }
  
  if(outfile_name.compare("")) {
    outfile<<s;
    outfile.close();
  }
  else std::cout<<s;

  return 0;
}

