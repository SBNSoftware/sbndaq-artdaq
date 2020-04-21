// For an explanation of this class, look at its header,
// sbnToySimulator.hh, as well as
// https://cdcvs.fnal.gov/redmine/projects/artdaq-demo/wiki/Fragments_and_FragmentGenerators_w_Toy_Fragments_as_Examples

#include "sbnToySimulator.hh"

#include "canvas/Utilities/Exception.h"

#include "artdaq-core/Utilities/SimpleLookupPolicy.hh"
#include "artdaq/Generators/GeneratorMacros.hh"

#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"

#include "sbndaq-artdaq-core/Overlays/Common/BernCRTZMQFragment.hh" //print timestamp

#include "fhiclcpp/ParameterSet.h"

#include <fstream>
#include <iomanip>
#include <iostream>

#include <thread>

#include <unistd.h>
#define TRACE_NAME "sbnToySimulator"
#include "cetlib_except/exception.h"
#include "tracemf.h"  // TRACE, TLOG*

//Constructor
sbndaq::sbnToySimulator::sbnToySimulator(fhicl::ParameterSet const& ps)
    : CommandableFragmentGenerator(ps)
    , metadata_({0})
//    , header_({0})
{

  fragment_id_ = ps.get<int>("fragment_id", 0);

  time_step_ = std::chrono::milliseconds( ps.get<int>("time_step_ms", 199) );
  timestamp_ = std::chrono::system_clock::now() + time_step_;
}

sbndaq::sbnToySimulator::~sbnToySimulator() { }

bool sbndaq::sbnToySimulator::getNext_(artdaq::FragmentPtrs& frags) {
//  TLOG(TLVL_INFO)<<__func__<<" called";
  if (should_stop())  return false; 

  std::this_thread::sleep_until (timestamp_);
  if(should_stop()) return false;

  //loop over all fragments
  //normally, if getNext_ is called often, the loop will be executed only once
  while(std::chrono::system_clock::now() > timestamp_)  {
    metadata_.fragment_fill_time_ = std::chrono::system_clock::now().time_since_epoch().count();
    //    header_.seven_ = 7;

    TLOG(TLVL_INFO)<<__func__
      <<" Sending fragment "<<ev_counter()<<" ("
      <<sbndaq::BernCRTZMQFragment::print_timestamp(timestamp_.time_since_epoch().count())
      <<") after "<<
      (metadata_.fragment_fill_time_ - timestamp_.time_since_epoch().count())/1e6
      << " ms delay";

    std::unique_ptr<artdaq::Fragment> fragptr(
        artdaq::Fragment::FragmentBytes(
          sizeof(sbnToyFragment),
          ev_counter(),
          fragment_id_,
          sbndaq::detail::FragmentType::sbnToySimulator,
          metadata_,
          timestamp_.time_since_epoch().count()
          )
        );

    //fragment contains no payload, thus no need to fill it
    //   artdaq::Fragment::FragmentBytes(std::size_t payload_size_in_bytes, sequence_id_t sequence_id,
    //   fragment_id_t fragment_id, type_t type, const T & metadata)
    //    memcpy(frags.back()->dataBeginBytes(), &header_, sizeof(sbnToyFragment::Header));
    frags.emplace_back(std::move(fragptr));

    ev_counter_inc();

    //calculate timestamp of the next event
    timestamp_ += time_step_;
  }

  if (metricMan != nullptr) {
    metricMan->sendMetric("Fragments Sent", ev_counter(), "Events", 3, artdaq::MetricMode::LastPoint);
  }

  return true;
}



void sbndaq::sbnToySimulator::start() { }

void sbndaq::sbnToySimulator::stop() { }

// The following macro is defined in artdaq's GeneratorMacros.hh header
DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(sbndaq::sbnToySimulator)
