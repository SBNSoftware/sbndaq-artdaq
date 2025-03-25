#include "DummyGenerator.hh"

#include "artdaq/DAQdata/Globals.hh"
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
#define TRACE_NAME "DummyGenerator"
#include "cetlib_except/exception.h"
#include "tracemf.h"  // TRACE, TLOG*

sbndaq::DummyGenerator::DummyGenerator(fhicl::ParameterSet const& ps)
    : CommandableFragmentGenerator(ps)
    , metadata_({0})
{

  fragment_id_ = ps.get<int>("fragment_id", 0);
  time_step_   = std::chrono::milliseconds( ps.get<int>("time_step_ms", 199) );
  timestamp_   = std::chrono::system_clock::now() + time_step_;
}

sbndaq::DummyGenerator::~DummyGenerator() { }

bool sbndaq::DummyGenerator::getNext_(artdaq::FragmentPtrs& frags) {
  if (should_stop())  return false; 

  //loop over all fragments
  //normally, if getNext_ is called often, the loop will be executed only once
  while(std::chrono::system_clock::now() > timestamp_)  {
    metadata_.fragment_fill_time_ = std::chrono::system_clock::now().time_since_epoch().count();

    TLOG(TLVL_DEBUG)<<__func__
      <<"() Sending fragment "<<ev_counter()<<" ("
      <<sbndaq::BernCRTZMQFragment::print_timestamp(timestamp_.time_since_epoch().count())
      <<") after "<<
      (metadata_.fragment_fill_time_ - timestamp_.time_since_epoch().count())/1e6
      << " ms delay";

    std::unique_ptr<artdaq::Fragment> fragptr(
        artdaq::Fragment::FragmentBytes(
          sizeof(DummyFragment),
          ev_counter(),
          fragment_id_,
          sbndaq::detail::FragmentType::DummyGenerator,
          metadata_,
          timestamp_.time_since_epoch().count()
          )
        );

    //fragment contains no payload, thus no need to fill it
    frags.emplace_back(std::move(fragptr));

    ev_counter_inc();

    //calculate timestamp of the next event
    timestamp_ += time_step_;
  }

  if (metricMan != nullptr) {
    metricMan->sendMetric("Fragments Sent", ev_counter(), "Events", 11, artdaq::MetricMode::LastPoint);
  }

  return true;
}



void sbndaq::DummyGenerator::start() { }

void sbndaq::DummyGenerator::stop() { }

// The following macro is defined in artdaq's GeneratorMacros.hh header
DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(sbndaq::DummyGenerator)
